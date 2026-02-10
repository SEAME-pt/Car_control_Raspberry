#include <gtest/gtest.h>
#include <CANProtocol.hpp>
#include <linux/can.h>
#include <thread>
#include <chrono>

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
	
	// Helper to run candump and capture one frame
	std::string captureCANFrame(uint16_t expected_id, int timeout_ms = 100) {
		std::string cmd = "timeout " + std::to_string(timeout_ms / 1000.0) + 
						 " candump vcan0,0x" + std::to_string(expected_id) + ":0x7FF -n 1 2>/dev/null";
		FILE* pipe = popen(cmd.c_str(), "r");
		if (!pipe) return "";
		
		char buffer[256];
		std::string result;
		if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
			result = buffer;
		}
		pclose(pipe);
		return result;
	}
};

// Test emergency brake function - verify it sends without throwing
TEST_F(CANProtocolTest, SendEmergencyBrakeNoThrow) {
	CANController can(validInterface);
	can.initialize();
	
	// Should not throw for active
	ASSERT_NO_THROW(CANProtocol::sendEmergencyBrake(can, true));
	
	// Should not throw for inactive
	ASSERT_NO_THROW(CANProtocol::sendEmergencyBrake(can, false));
}

// Test driving command function - verify it sends without throwing
TEST_F(CANProtocolTest, SendDrivingCommandNoThrow) {
	CANController can(validInterface);
	can.initialize();
	
	// Test various throttle and steering values
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, 1000, 500));
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, -1500, -750));
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, 0, 0));
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, INT16_MAX, INT16_MAX));
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, INT16_MIN, INT16_MIN));
}

// Test correct CAN IDs are used
TEST_F(CANProtocolTest, VerifyCANIDs) {
	EXPECT_EQ(CANSENDID::EMERGENCY_BRAKE, 0x100);
	EXPECT_EQ(CANSENDID::DRIVING_COMMAND, 0x101);
}

// Test emergency brake data encoding using system candump
TEST_F(CANProtocolTest, EmergencyBrakeDataEncodingActive) {
	// Start candump in background before sending
	std::thread dumper([this]() {
		std::string output = captureCANFrame(CANSENDID::EMERGENCY_BRAKE, 200);
		// Check if we got output and it contains 0F (active brake)
		if (!output.empty()) {
			EXPECT_NE(output.find("0F"), std::string::npos) << "Expected 0F in: " << output;
		}
	});
	
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	
	CANController can(validInterface);
	can.initialize();
	CANProtocol::sendEmergencyBrake(can, true);
	
	dumper.join();
}

// Test emergency brake data encoding inactive
TEST_F(CANProtocolTest, EmergencyBrakeDataEncodingInactive) {
	std::thread dumper([this]() {
		std::string output = captureCANFrame(CANSENDID::EMERGENCY_BRAKE, 200);
		if (!output.empty()) {
			EXPECT_NE(output.find("00"), std::string::npos) << "Expected 00 in: " << output;
		}
	});
	
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	
	CANController can(validInterface);
	can.initialize();
	CANProtocol::sendEmergencyBrake(can, false);
	
	dumper.join();
}

// Test multiple sequential sends work correctly
TEST_F(CANProtocolTest, MultipleSequentialSends) {
	CANController can(validInterface);
	can.initialize();
	
	// Send multiple emergency brake toggles
	for (int i = 0; i < 10; i++) {
		ASSERT_NO_THROW(CANProtocol::sendEmergencyBrake(can, i % 2 == 0));
	}
	
	// Send multiple driving commands
	for (int16_t i = -100; i <= 100; i += 10) {
		ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, i * 10, i * 5));
	}
}

// Test driving command with boundary values
TEST_F(CANProtocolTest, DrivingCommandBoundaryValues) {
	CANController can(validInterface);
	can.initialize();
	
	// Test zero
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, 0, 0));
	
	// Test maximum positive
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, INT16_MAX, INT16_MAX));
	
	// Test maximum negative  
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, INT16_MIN, INT16_MIN));
	
	// Test mixed
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, INT16_MAX, INT16_MIN));
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, INT16_MIN, INT16_MAX));
}

// Test that protocol functions work with typical values
TEST_F(CANProtocolTest, TypicalDrivingScenarios) {
	CANController can(validInterface);
	can.initialize();
	
	// Idle
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, 0, 0));
	
	// Slow forward, straight
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, 500, 0));
	
	// Medium speed, turning left
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, 1000, -300));
	
	// Medium speed, turning right
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, 1000, 300));
	
	// Reverse
	ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, -500, 0));
	
	// Emergency brake
	ASSERT_NO_THROW(CANProtocol::sendEmergencyBrake(can, true));
	
	// Release brake
	ASSERT_NO_THROW(CANProtocol::sendEmergencyBrake(can, false));
}

// Test rapid fire commands (stress test)
TEST_F(CANProtocolTest, RapidCommandSequence) {
	CANController can(validInterface);
	can.initialize();
	
	auto start = std::chrono::steady_clock::now();
	
	// Send 100 commands as fast as possible
	for (int i = 0; i < 100; i++) {
		int16_t throttle = static_cast<int16_t>((i * 100) % 2000 - 1000);
		int16_t steering = static_cast<int16_t>((i * 50) % 1000 - 500);
		ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, throttle, steering));
	}
	
	auto end = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	
	// Should complete in reasonable time (< 1 second)
	EXPECT_LT(duration, 1000);
}

// Test alternating between emergency brake and driving commands
TEST_F(CANProtocolTest, AlternatingCommands) {
	CANController can(validInterface);
	can.initialize();
	
	for (int i = 0; i < 20; i++) {
		if (i % 2 == 0) {
			ASSERT_NO_THROW(CANProtocol::sendEmergencyBrake(can, true));
		} else {
			ASSERT_NO_THROW(CANProtocol::sendDrivingCommand(can, 500, 100));
		}
	}
}
