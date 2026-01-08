#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QRandomGenerator>
#include <QScreen>
#include <iostream>
#include <time.h>
#include "../incs/radialbar.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <QSocketNotifier>
#include <QDateTime>
#include "uprotocol.h"

//static const char *SOCK_PATH = "/tmp/uprotocol_speed.sock";

static int g_sockfd = -1;
static QSocketNotifier *g_notifier = nullptr;
static QByteArray g_buffer;
static QObject *g_root = nullptr;

void stop_socket_listener();
// Internal function that reads from socket and parses uProtocol messages.
// Prints speed + timestamp to stdout (as requested).
static void receive_and_process()
{
    if (g_sockfd < 0) return;
    uint8_t tmp[4096];
    ssize_t r = ::recv(g_sockfd, tmp, sizeof(tmp), 0);
    if (r <= 0) {
        qWarning() << "recv returned" << r << "- stopping listener";
        if (g_notifier) g_notifier->setEnabled(false);
        return;
    }

    g_buffer.append(reinterpret_cast<const char*>(tmp), (int)r);

    // parse as many full messages as possible
    while ((size_t)g_buffer.size() >= UPROTO_HEADER_SIZE) {
        UProtoHeader hdr;
        memcpy(&hdr, g_buffer.constData(), UPROTO_HEADER_SIZE);
        uint32_t payload_len = header_payload_len(hdr);
        size_t total_needed = UPROTO_HEADER_SIZE + payload_len;
        if ((size_t)g_buffer.size() < total_needed) break; // wait for more bytes

        const char *payload = g_buffer.constData() + UPROTO_HEADER_SIZE;

        if (hdr.type == MSG_SPEED && payload_len == (int)(sizeof(uint64_t) + sizeof(double))) {
            uint64_t ts_be = 0;
            double speed = 0.0;
            memcpy(&ts_be, payload, sizeof(ts_be));
            memcpy(&speed, payload + sizeof(ts_be), sizeof(speed));
            uint64_t ts = ntohll(ts_be);
            QDateTime dt = QDateTime::fromMSecsSinceEpoch((qint64)ts);
            // Print to terminal
            std::cout << "[uProtocol] Speed: " << speed << " m/s"
                      << "  ts=" << dt.toString(Qt::ISODateWithMs).toStdString()
                      << std::endl;
            if (g_root) {
                QVariant val = QVariant::fromValue(speed); // double
                bool ok = g_root->setProperty("currentSpeed", val);
                if (!ok) {
                    qWarning() << "Failed to set QML property 'currentSpeed' on root object.";
                    // Optionally try setting as int: g_root->setProperty("currentSpeed", (int)qRound(speed));
                }
            }
        } else {
            qWarning() << "Unknown message type" << hdr.type << "len" << payload_len << "- skipping";
        }

        // consume bytes
        g_buffer.remove(0, (int)total_needed);
    }
}

int connect_to_daemon(const char *path)
{
    if (!path) return -1;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    sockaddr_un sun{};
    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, path, sizeof(sun.sun_path)-1);
    if (connect(fd, (sockaddr *)&sun, sizeof(sun)) < 0) {
        ::close(fd);
        return -1;
    }
    // make it blocking (we rely on QSocketNotifier to notify)
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0) fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    return fd;
}

bool start_socket_listener(int fd, QObject *parent)
{
    if (fd < 0) return false;
    // If already running, stop previous instance first
    if (g_notifier) stop_socket_listener();

    g_sockfd = fd;
    g_buffer.clear();

    // create notifier as a child of parent (so it is cleaned up automatically)
    g_notifier = new QSocketNotifier(g_sockfd, QSocketNotifier::Read, parent);
    // connect activated to a lambda that calls our free function parser
    QObject::connect(g_notifier, &QSocketNotifier::activated, [=](int) {
        // call processing function
        receive_and_process();
    });
    qInfo() << "Started socket listener on fd" << g_sockfd;
    return true;
}

void stop_socket_listener()
{
    if (g_notifier) {
        g_notifier->setEnabled(false);
        // delete notifier (it was created with optional parent; delete safe)
        delete g_notifier;
        g_notifier = nullptr;
    }
    if (g_sockfd >= 0) {
        ::close(g_sockfd);
        g_sockfd = -1;
    }
    g_buffer.clear();
    qInfo() << "Stopped socket listener";
}


int main(int argc, char *argv[])
{   
    QGuiApplication app(argc, argv);

    // Get primary screen and set orientation programmatically
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        qDebug() << "Screen geometry:" << screen->geometry();
        qDebug() << "Screen orientation:" << screen->orientation();
        qDebug() << "Available geometry:" << screen->availableGeometry();
    }

    QQmlApplicationEngine engine;
    //notifier = new QSocketNotifier(sockfd, QSocketNotifier::Read, this);
    qmlRegisterType<RadialBar>("CustomControls", 1, 0, "RadialBar");
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);
    g_root = engine.rootObjects().first();
    int fd = connect_to_daemon("/tmp/uprotocol_speed.sock");
    if (fd >= 0) {
        // pass &app (or &engine) so the notifier is parented into Qt's object tree
        start_socket_listener(fd, &app);
    } else {
        qWarning() << "Could not connect to speed daemon; UI will still run.";
    }

    return app.exec();
}