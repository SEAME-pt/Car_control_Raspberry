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

    // Send a single field (name, value) to the connected client — generic for any QDataStream-serializable type
    template<typename T>
    void sendField(const QString &name, const T &value) {
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

// Test fixture for server communication tests
class ServerSendInt : public ::testing::TestWithParam<int> { };
class ServerSendDouble : public ::testing::TestWithParam<double> { };

// Verifies CarDataController connects to a listening server and reports connected state.
TEST(ServerCommunication, ConnectsToServer) {
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());

	EXPECT_EQ(ctrl.serverPort(), srv.port());

    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(5000));
    EXPECT_TRUE(ctrl.isConnected());
}

// Ensures setting a (wrong) server address is stored and does not connect.
TEST(ServerCommunication, SetServerAddressAndFailConnect)
{
	TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("192.168.1.42");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

	EXPECT_EQ(ctrl.serverAddress(), "192.168.1.42");
	EXPECT_FALSE(ctrl.isConnected());

	
}

// Checks disconnectFromServer() disconnects and updates connection state.
TEST(ServerCommunication, DisconnectFromServer) {
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(5000));

	ctrl.disconnectFromServer();

	EXPECT_FALSE(ctrl.isConnected());
}

// Simulates a disconnect and verifies attemptReconnect() establishes a new connection.
TEST(ServerCommunication, AttemptReconnectFromServer) {
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(5000));

	ctrl.disconnectFromServer();

	ctrl.testAttemptReconnect();
	ASSERT_TRUE(srv.waitForConnection(5000));
    EXPECT_TRUE(ctrl.isConnected());
}

// Tries connecting to a closed port and expects an errorOccurred signal (connection refused).
TEST(ServerCommunication, EmitsErrorOnConnectRefused) {
    CarDataController ctrl;
    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(9999); // Assuming nothing is listening on this port

    QSignalSpy errSpy(&ctrl, &CarDataController::errorOccurred);

    ctrl.connectToServer();

    // Expect the socket to report an error (connection refused) within a short timeout
    ASSERT_TRUE(errSpy.wait(3000));
    EXPECT_FALSE(ctrl.isConnected());
}

// Parameterized: server sends an integer speed; controller must emit speedChanged and update value.
TEST_P(ServerSendInt, ReceivesSpeedUpdate) {
    auto output = GetParam();
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
    srv.sendField("speed", output);

    // Wait for the signal (fail if not received within timeout)
	ctrl.setDistanceUnit("kilometers"); // Ensure no unit conversion for this test
    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.speed(), output);
}

// Parameterized: server sends an integer speed limit; controller must emit speedLimitChanged and update value.
TEST_P(ServerSendInt, ReceivesSpeedLimitUpdate) {
    auto output = GetParam();
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(2000));

    // Use QSignalSpy to wait for the speedLimitChanged signal reliably
    QSignalSpy speedSpy(&ctrl, &CarDataController::speedLimitChanged);

    // Server sends a speedLimit update
    srv.sendField("speedLimit", output);

    // Wait for the signal (fail if not received within timeout)
	ctrl.setDistanceUnit("kilometers"); // Ensure no unit conversion for this test
    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.speedLimit(), output);
}

// Parameterized: server sends an integer battery level; controller must emit batteryLevelChanged and update value.
TEST_P(ServerSendInt, ReceivesBatteryLevelUpdate) {
    auto output = GetParam();
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(2000));

    // Use QSignalSpy to wait for the batteryLevelChanged signal reliably
    QSignalSpy speedSpy(&ctrl, &CarDataController::batteryLevelChanged);

    // Server sends a batteryLevel update
    srv.sendField("batteryLevel", output);

    // Wait for the signal (fail if not received within timeout)
    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.batteryLevel(), output);
}


// Parameterized: server sends an integer battery range; controller must emit batteryRangeChanged and update value.
TEST_P(ServerSendInt, ReceivesBatteryRangeUpdate) {
    auto output = GetParam();
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(2000));

    // Use QSignalSpy to wait for the batteryRangeChanged signal reliably
    QSignalSpy speedSpy(&ctrl, &CarDataController::batteryRangeChanged);

    // Server sends a batteryRange update
    srv.sendField("batteryRange", output);

    // Wait for the signal (fail if not received within timeout)
	ctrl.setDistanceUnit("kilometers"); // Ensure no unit conversion for this test
    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.batteryRange(), output);
}

// Parameterized: server sends an integer motor power; controller must emit motorPowerChanged and update value.
TEST_P(ServerSendInt, ReceivesMotorPowerUpdate) {
    auto output = GetParam();
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(2000));

    // Use QSignalSpy to wait for the motorPowerChanged signal reliably
    QSignalSpy speedSpy(&ctrl, &CarDataController::motorPowerChanged);

    // Server sends a motorPower update
    srv.sendField("motorPower", output);

    // Wait for the signal (fail if not received within timeout)
    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.motorPower(), output);
}


INSTANTIATE_TEST_SUITE_P(
    ServerSend,
    ServerSendInt,
    ::testing::Values(
        5, 7, 42, 150, 1234, 99999
    )
);

// Parameterized: server sends a double temperature; controller must emit temperatureChanged and update value.
TEST_P(ServerSendDouble, ReceivesTemperatureUpdate) {
    auto output = GetParam();
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(2000));

    // Use QSignalSpy to wait for the temperatureChanged signal reliably
    QSignalSpy speedSpy(&ctrl, &CarDataController::temperatureChanged);

    // Server sends a temperature update
    srv.sendField("temperature", output);

	ctrl.setTemperatureUnit("celsius"); // Ensure no unit conversion for this test

    // Wait for the signal (fail if not received within timeout)
    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.temperature(), output);
}

// Parameterized: server sends a double totalDistance; controller must emit totalDistanceChanged and update value.
TEST_P(ServerSendDouble, ReceivesTotalDistanceUpdate) {
    auto output = GetParam();
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(2000));

    // Use QSignalSpy to wait for the totalDistanceChanged signal reliably
    QSignalSpy speedSpy(&ctrl, &CarDataController::totalDistanceChanged);

    // Server sends a totalDistance update
    srv.sendField("totalDistance", output);

	ctrl.setDistanceUnit("kilometers"); // Ensure no unit conversion for this test

    // Wait for the signal (fail if not received within timeout)
    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.totalDistance(), output);
}


INSTANTIATE_TEST_SUITE_P(
    ServerSend,
    ServerSendDouble,
    ::testing::Values(
        5.2, 7.6, 42.42, 150.05, 1234.01, 99999.99
    )
);

// Server sends motorActive boolean updates; controller must emit motorActiveChanged and update state.
TEST(ServerCommunication, ReceivesMotorActiveUpdate)
{
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(2000));

    // Use QSignalSpy to wait for the motorActiveChanged signal reliably
    QSignalSpy speedSpy(&ctrl, &CarDataController::motorActiveChanged);


	// Server sends a motorActive update
    srv.sendField("motorActive", true);

    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.motorActive(), true);

	// Server sends a motorActive update
    srv.sendField("motorActive", false);

    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.motorActive(), false);
}

// Server sends showError boolean updates; controller must emit showErrorChanged and update state.
TEST(ServerCommunication, ReceivesshowErrorUpdate)
{
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(2000));

    // Use QSignalSpy to wait for the showErrorChanged signal reliably
    QSignalSpy speedSpy(&ctrl, &CarDataController::showErrorChanged);


	// Server sends a showError update
    srv.sendField("showError", true);

    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.showError(), true);

	// Server sends a showError update
    srv.sendField("showError", false);

    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.showError(), false);
}

// Server sends an errorMessage QString; controller must emit errorMessageChanged and update the message.
TEST(ServerCommunication, ReceivesErrorMessageUpdate)
{
    TestTcpServer srv;
    ASSERT_TRUE(srv.listen());

    CarDataController ctrl;

    ctrl.setServerAddress("127.0.0.1");
    ctrl.setServerPort(srv.port());
    ctrl.connectToServer();

    ASSERT_TRUE(srv.waitForConnection(2000));

    // Use QSignalSpy to wait for the ErrorMessageChanged signal reliably
    QSignalSpy speedSpy(&ctrl, &CarDataController::errorMessageChanged);


	// Server sends a errorMessage update
    srv.sendField("errorMessage", QString("Test error message"));

    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.errorMessage(), "Test error message");

	// Server sends a errorMessage update
    srv.sendField("errorMessage", QString(""));

    ASSERT_TRUE(speedSpy.wait(5000));
    EXPECT_EQ(ctrl.errorMessage(), "");
}

// Allocates and deletes CarDataController to exercise the compiler "deleting" destructor path.
TEST(ServerCommunication, DeletingDestructorIsCovered) {
    // allocate on the heap and delete to exercise the compiler "deleting destructor" (D0Ev)
    auto *ctrl = new CarDataController();
    delete ctrl; // coverage will record the deleting-destructor symbol
}

#include "test_ServerCommunication.moc"
