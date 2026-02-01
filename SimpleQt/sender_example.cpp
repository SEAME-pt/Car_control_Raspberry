// Example C++ sender using QDataStream
// Compile with: g++ -o sender sender.cpp -lQt5Core -lQt5Network -fPIC -I/usr/include/qt -I/usr/include/qt/QtCore -I/usr/include/qt/QtNetwork

#include <QCoreApplication>
#include <QTcpSocket>
#include <QDataStream>
#include <QTimer>
#include <QDebug>

class CarDataSender : public QObject {
    Q_OBJECT
public:
    CarDataSender(const QString &host, quint16 port) {
        socket = new QTcpSocket(this);
        
        connect(socket, &QTcpSocket::connected, this, [this]() {
            qDebug() << "Connected to dashboard";
            startSending();
        });
        
        connect(socket, &QTcpSocket::disconnected, this, []() {
            qDebug() << "Disconnected";
        });
        
        socket->connectToHost(host, port);
    }
    
private slots:
    void startSending() {
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &CarDataSender::sendData);
        timer->start(500); // Send every 500ms
    }
    
    void sendData() {
        static int t = 0;
        
        // Generate data
        int cycle = t % 40;
        int speed, motorPower;
        
        if (cycle < 20) {
            speed = (cycle * 100) / 20;
            motorPower = qMin(100, (cycle * 100) / 20 + 30);
        } else {
            speed = ((40 - cycle) * 100) / 20;
            motorPower = qMax(-100, -((cycle - 20) * 80) / 20);
        }
        
        int batteryLevel = qMax(20, 100 - t / 2);
        int batteryVoltage = 48;
        int batteryRange = batteryLevel * 3;
        bool motorActive = speed > 0 || qAbs(motorPower) > 0;
        int speedLimit = 120;
        
        // Send using QDataStream
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_15);
        
        out << QString("speed") << (qint32)speed;
        out << QString("speedLimit") << (qint32)speedLimit;
        out << QString("batteryLevel") << (qint32)batteryLevel;
        out << QString("batteryVoltage") << (qint32)batteryVoltage;
        out << QString("batteryRange") << (qint32)batteryRange;
        out << QString("motorActive") << motorActive;
        out << QString("motorPower") << (qint32)motorPower;
        
        socket->write(data);
        socket->flush();
        
        qDebug() << "Sent: speed=" << speed << "battery=" << batteryLevel << "% range=" << batteryRange << "km";
        
        t++;
    }
    
private:
    QTcpSocket *socket;
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    CarDataSender sender("127.0.0.1", 8888);
    
    return app.exec();
}

#include "sender.moc"
