#include <gtest/gtest.h>
#include <CANProtocol.hpp>

/********************************/
/*		CAN PROTOCOL			*/
/********************************/

class CANProtocolTest : public ::testing::Test {
protected:
    const std::string validInterface = "vcan0";

    void SetUp() override {
        system("sudo ip link add dev vcan0 type vcan");
        system("sudo ip link set vcan0 mtu 72");
        system("sudo ip link set up vcan0");
    }

    void TearDown() override {
        system("sudo ip link delete vcan0 2>/dev/null");
    }
};

// Test emergency brake
TEST_F(CANProtocolTest, EmergencyBrake) {
    CANController can(validInterface);

    ASSERT_NO_THROW(CANProtocol::sendEmergencyBrake(can, true));
    ASSERT_NO_THROW(CANProtocol::sendEmergencyBrake(can, false));
}

// Test drive command clamping
TEST_F(CANProtocolTest, DriveCommandClamping) {
    CANController can(validInterface);
    
    // Valid values
    ASSERT_NO_THROW(CANProtocol::sendDriveCommand(can, 60, 50));
    
    // Out of bounds
    ASSERT_NO_THROW(CANProtocol::sendDriveCommand(can, 127, 127));  // Above max
    ASSERT_NO_THROW(CANProtocol::sendDriveCommand(can, -128, -128)); // Below min
}

// Test drive modes
TEST_F(CANProtocolTest, DriveMode) {
    CANController can(validInterface);
    
    ASSERT_NO_THROW(CANProtocol::sendDriveMode(can, DriveMode::MANUAL));
    ASSERT_NO_THROW(CANProtocol::sendDriveMode(can, DriveMode::AUTONOMOUS));
}

// Test boundary values
TEST_F(CANProtocolTest, BoundaryValues) {
    CANController can(validInterface);
    
    // Steering boundaries
    ASSERT_NO_THROW(CANProtocol::sendDriveCommand(can, 0, 0));    // Min
    ASSERT_NO_THROW(CANProtocol::sendDriveCommand(can, 120, 0));  // Max
    
    // Throttle boundaries
    ASSERT_NO_THROW(CANProtocol::sendDriveCommand(can, 60, -100)); // Min
    ASSERT_NO_THROW(CANProtocol::sendDriveCommand(can, 60, 100));  // Max
}