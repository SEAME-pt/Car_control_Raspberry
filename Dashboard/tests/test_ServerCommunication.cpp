#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSignalSpy>
#include <QEventLoop>
#include <QTimer>
#include "../incs/CarDataController.hpp"

// Lightweight test helper that starts a QTcpServer and accepts one client.
class TestTcpServer : public QObject {
    Q_OBJECT
public:
    TestTcpServer(QObject *parent = nullptr) : QObject(parent), server(new QTcpServer(this)), clientSocket(nullptr) {}

    bool listen() {
        return server->listen(QHostAddress::Any, 0);
    }

    int port() const { return server->serverPort(); }

    // Wait for a single incoming connection (blocks up to timeout ms)
    bool waitForConnection(int timeout = 2000) {
        if (server->hasPendingConnections()) {
            clientSocket = server->nextPendingConnection();
            return true;
        }
        QEventLoop loop;
        QTimer t;
        t.setSingleShot(true);
        connect(server, &QTcpServer::newConnection, &loop, [&]() {
            clientSocket = server->nextPendingConnection();
            loop.quit();
        });
        connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
        t.start(timeout);
        loop.exec();
        return clientSocket != nullptr;
    }

    // Send a single (QString, qint32) field to the connected client
    void sendField(const QString &name, qint32 value) {
        if (!clientSocket) return;
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_15);
        out << name << value;
        clientSocket->write(data);
        clientSocket->flush();
    }

private:
    QTcpServer *server;
    QTcpSocket *clientSocket;
};

// Skip network tests when SKIP_HARDWARE_TESTS=1 is set in the environment
static bool skipNetwork() {
    return qEnvironmentVariableIntValue("SKIP_HARDWARE_TESTS") == 1;
}

TEST(ServerCommunication, ConnectsToServer) {
    //if (skipNetwork()) GTEST_SKIP();

    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(5000));
    EXPECT_TRUE(ctrl.isConnected());
}

TEST(ServerCommunication, ReceivesSpeedUpdate) {
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(2000));

    // Use QSignalSpy to wait for the speedChanged signal reliably
    QSignalSpy speedSpy(&ctrl, &CarDataController::speedChanged);

    // Server sends a speed update
    srv.sendField("speed", 42);

    // Wait for the signal (fail if not received within timeout)
    ASSERT_TRUE(speedSpy.wait(2000));
    EXPECT_EQ(ctrl.speed(), 42);
}

#include "test_ServerCommunication.moc"
