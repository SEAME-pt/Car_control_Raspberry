#include <gtest/gtest.h>
#include "CANController.hpp"

class CANControllerTest : public ::testing::Test {

protected:
	const std::string validInterface = "vcan0";
	const std::string invalidInterface = "invalid_can99";
	const std::string emptyInterface = "";

	void SetUp() override {
		// Setup vcan0
		system("sudo ip link add dev vcan0 type vcan");
		system("sudo ip link set vcan0 mtu 72");
		system("sudo ip link set up vcan0");
	}

	void TearDown() override {
		// Cleanup: remove virtual CAN interface
		system("sudo ip link delete vcan0 2>/dev/null");
	}
};

//constructor
TEST_F(CANControllerTest, ValidConstructor) {
	ASSERT_NO_THROW({
		CANController can(validInterface);
		EXPECT_TRUE(can.isInitialized());
		EXPECT_EQ(can.getInterface(), validInterface);
		EXPECT_GE(can.getSocket(), 0);
	});
}

//constructor
TEST_F(CANControllerTest, InvalidConstructor) {
	EXPECT_THROW({
		CANController can(invalidInterface);
	}, CANController::CANException);

	EXPECT_THROW({
		CANController can(emptyInterface);
	}, CANController::CANException);
}

//destructor
TEST_F(CANControllerTest, Destructor) {

	int socket;
	{
		// Create object in a scope
		CANController can(validInterface);
		EXPECT_TRUE(can.isInitialized());
		socket = can.getSocket();
		EXPECT_GE(socket, 0);

		// Destructor will be called when leaving this scope
	}
	char buf[1];
	EXPECT_EQ(read(socket, buf, 1), -1);
	EXPECT_EQ(errno, EBADF);
}

//move constructor
TEST_F(CANControllerTest, MoveConstructor) {

		//create original object
		CANController original(validInterface);
		EXPECT_TRUE(original.isInitialized());
		int originalSocket = original.getSocket();
		EXPECT_GE(originalSocket, 0);

		// Move construct new object from original
		CANController moved(std::move(original));

		// Verify moved-to object has the resources
		EXPECT_TRUE(moved.isInitialized());
		EXPECT_EQ(moved.getInterface(), validInterface);
		EXPECT_EQ(moved.getSocket(), originalSocket);

		// Verify moved from object is in safe empty state
		EXPECT_FALSE(original.isInitialized());
		EXPECT_LT(original.getSocket(), 0);
}

//move assignment operator
TEST_F(CANControllerTest, AssignmentOperator) {

	CANController original(validInterface);
	CANController target(validInterface);
	
	EXPECT_TRUE(original.isInitialized());
	EXPECT_TRUE(target.isInitialized());
	
	int originalSocket = original.getSocket();
	int targetSocket = target.getSocket();
	
	EXPECT_GE(originalSocket, 0);
	EXPECT_GE(targetSocket, 0);
	EXPECT_NE(originalSocket, targetSocket);

	target = std::move(original);

	// Verify moved to object has the resources
	EXPECT_TRUE(target.isInitialized());
	EXPECT_EQ(target.getInterface(), validInterface);
	EXPECT_EQ(target.getSocket(), originalSocket);

	// Verify moved from object is in safe empty state
	EXPECT_FALSE(original.isInitialized());
	EXPECT_LT(original.getSocket(), 0);
}

//cleanup method
TEST_F(CANControllerTest, Cleanup) {

		CANController can(validInterface);
		EXPECT_TRUE(can.isInitialized());
		EXPECT_GE(can.getSocket(), 0);
		
		can.cleanup();
		
		EXPECT_FALSE(can.isInitialized());
		EXPECT_LT(can.getSocket(), 0);
}

//initialize
TEST_F(CANControllerTest, RegularInitialized) {

	ASSERT_NO_THROW({
		CANController can(validInterface);
		ASSERT_TRUE(can.isInitialized());

		testing::internal::CaptureStderr();

		can.initialize();
		std::string output = testing::internal::GetCapturedStderr();

		EXPECT_NE(output.find("CAN already initialized"), std::string::npos);
		EXPECT_TRUE(can.isInitialized());
	});
}

//initialize
TEST_F(CANControllerTest, IrregularInitialized) {

	EXPECT_THROW({
		CANController can(invalidInterface);
		EXPECT_FALSE(can.isInitialized());
	}, CANController::CANException);

	EXPECT_THROW({
		CANController can(emptyInterface);
		EXPECT_FALSE(can.isInitialized());
	}, CANController::CANException);
}

//Classical CAN sendFrame
TEST_F(CANControllerTest, ValidSendFrame) {
	CANController can(validInterface);
	
	// Test 1: Random valid data
	{
		int8_t data[2];
		uint32_t can_id = 0x100;
		std::srand(std::time(nullptr));

		for (int i = 0; i < 100; i++) {
			data[0] = std::rand() % 121;         // 0-120
			data[1] = (std::rand() % 201) - 100; // -100-100
			ASSERT_NO_THROW(can.sendFrame(can_id, data, 2));
		}
	}

	// Test 2: Boundary values
	{
		uint32_t can_id = 0x100;
		
		int8_t data_min[2] = {0, -100};
		ASSERT_NO_THROW(can.sendFrame(can_id, data_min, 2));

		int8_t data_max[2] = {120, 100};
		ASSERT_NO_THROW(can.sendFrame(can_id, data_max, 2));

		int8_t data_zero[2] = {0, 0};
		ASSERT_NO_THROW(can.sendFrame(can_id, data_zero, 2));
	}

	// Test 3: Different data lengths up to 8 bytes
	{
		uint32_t can_id = 0x200;
		
		int8_t data1[1] = {50};
		ASSERT_NO_THROW(can.sendFrame(can_id, data1, 1));

		int8_t data2[2] = {50, 25};
		ASSERT_NO_THROW(can.sendFrame(can_id, data2, 2));

		int8_t data8[8] = {1, 2, 3, 4, 5, 6, 7, 8};
		ASSERT_NO_THROW(can.sendFrame(can_id, data8, 8));
	}

	// Test 4: Different CAN IDs
	{
		int8_t data[2] = {50, 0};
		
		ASSERT_NO_THROW(can.sendFrame(0x100, data, 2));
		ASSERT_NO_THROW(can.sendFrame(0x200, data, 2));
		ASSERT_NO_THROW(can.sendFrame(0x7FF, data, 2)); // Max standard ID
	}

	// Test 5: Empty data
	{
		uint32_t can_id = 0x100;
		ASSERT_NO_THROW(can.sendFrame(can_id, nullptr, 0));
	}

	// Test 6: Sequential sends
	{
		uint32_t can_id = 0x300;
		
		for (int i = 0; i < 50; i++) {
			int8_t data[2] = {static_cast<int8_t>(i % 121), 
							  static_cast<int8_t>((i % 201) - 100)};
			ASSERT_NO_THROW(can.sendFrame(can_id, data, 2));
		}
	}

	// Test 7: Normal driving values
	{
		uint32_t can_id = 0x100;

		int8_t data_idle[2] = {0, 0};
		ASSERT_NO_THROW(can.sendFrame(can_id, data_idle, 2));

		int8_t data_cruise[2] = {60, 0};
		ASSERT_NO_THROW(can.sendFrame(can_id, data_cruise, 2));

		int8_t data_full[2] = {120, 100};
		ASSERT_NO_THROW(can.sendFrame(can_id, data_full, 2));

		int8_t data_reverse[2] = {60, -100};
		ASSERT_NO_THROW(can.sendFrame(can_id, data_reverse, 2));
	}

	// Test 8: Send with data length exceeding CAN limits (> 8 bytes)
	{
		CANController can(validInterface);
		
		int8_t data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
		// Should truncate to 8 bytes
		ASSERT_NO_THROW(can.sendFrame(0x100, data, 16));
	}
}

//Classical CAN sendFrame - Should fail
TEST_F(CANControllerTest, InvalidSendFrame) {
	
	// Test 1: Send on uninitialized controller
	{
		CANController can(validInterface);
		can.cleanup();
		EXPECT_FALSE(can.isInitialized());
		
		int8_t data[2] = {50, 0};
		EXPECT_THROW(can.sendFrame(0x100, data, 2), CANController::CANException);
	}
	
	// Test 2: Send with invalid socket (moved-from object)
	{
		CANController original(validInterface);
		CANController moved(std::move(original));
		
		int8_t data[2] = {50, 0};
		EXPECT_THROW(original.sendFrame(0x100, data, 2), CANController::CANException);
	}
	
	// Test 3: Send on closed socket
	{
		CANController can(validInterface);
		int socket = can.getSocket();
		close(socket);
		
		int8_t data[2] = {50, 0};
		EXPECT_THROW(can.sendFrame(0x100, data, 2), CANController::CANException);
	}
	
	// Test 4: Invalid CAN ID
	{
		CANController can(validInterface);
		int8_t data[2] = {50, 0};
	
		uint32_t invalid_id = 0xFFFFFFFF;
		EXPECT_THROW(can.sendFrame(invalid_id, data, 2), CANController::CANException);
	
		uint32_t just_over_limit = 0x800;
		EXPECT_THROW(can.sendFrame(just_over_limit, data, 2), CANController::CANException);
	}
}

//CAN FD sendFrameFD - Valid tests
TEST_F(CANControllerTest, ValidSendFrameFD) {
	CANController can(validInterface);
	
	// Test 1: Random valid data
	{
		int8_t data[2];
		uint32_t can_id = 0x100;
		std::srand(std::time(nullptr));

		for (int i = 0; i < 100; i++) {
			data[0] = std::rand() % 121;         // 0-120
			data[1] = (std::rand() % 201) - 100; // -100-100
			ASSERT_NO_THROW(can.sendFrameFD(can_id, data, 2));
		}
	}

	// Test 2: Boundary values
	{
		uint32_t can_id = 0x100;
		
		int8_t data_min[2] = {0, -100};
		ASSERT_NO_THROW(can.sendFrameFD(can_id, data_min, 2));

		int8_t data_max[2] = {120, 100};
		ASSERT_NO_THROW(can.sendFrameFD(can_id, data_max, 2));

		int8_t data_zero[2] = {0, 0};
		ASSERT_NO_THROW(can.sendFrameFD(can_id, data_zero, 2));
	}

	// Test 3: Different data lengths up to 64 bytes
	{
		uint32_t can_id = 0x200;
		
		//low
		int8_t data1[1] = {50};
		ASSERT_NO_THROW(can.sendFrameFD(can_id, data1, 1));

		int8_t data8[8] = {1, 2, 3, 4, 5, 6, 7, 8};
		ASSERT_NO_THROW(can.sendFrameFD(can_id, data8, 8));

		int8_t data12[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
		ASSERT_NO_THROW(can.sendFrameFD(can_id, data12, 12));

		int8_t data16[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
		ASSERT_NO_THROW(can.sendFrameFD(can_id, data16, 16));

		//medium
		int8_t data32[32];
		for (int i = 0; i < 32; i++) data32[i] = i;
		ASSERT_NO_THROW(can.sendFrameFD(can_id, data32, 32));

		//high
		int8_t data64[64];
		for (int i = 0; i < 64; i++) data64[i] = i;
		ASSERT_NO_THROW(can.sendFrameFD(can_id, data64, 64));
	}

	// Test 4: Different CAN IDs
	{
		int8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
		
		ASSERT_NO_THROW(can.sendFrameFD(0x100, data, 8));
		ASSERT_NO_THROW(can.sendFrameFD(0x200, data, 8));
		ASSERT_NO_THROW(can.sendFrameFD(0x7FF, data, 8)); // Max standard ID
	}

	// Test 5: Empty data
	{
		uint32_t can_id = 0x100;
		ASSERT_NO_THROW(can.sendFrameFD(can_id, nullptr, 0));
	}

	// Test 6: Sequential sends with varying lengths
	{
		uint32_t can_id = 0x300;
		
		for (int i = 0; i < 50; i++) {
			size_t len = (i % 8) + 1; // 1-8 bytes varying
			int8_t data[8];
			for (size_t j = 0; j < len; j++) {
				data[j] = static_cast<int8_t>((i + j) % 128);
			}
			ASSERT_NO_THROW(can.sendFrameFD(can_id, data, len));
		}
	}

	// Test 7: Large payload
	{
		uint32_t can_id = 0x400;

		int8_t large_data[48];
		for (int i = 0; i < 48; i++) {
			large_data[i] = static_cast<int8_t>(i);
		}
		ASSERT_NO_THROW(can.sendFrameFD(can_id, large_data, 48));
	}

	// Test 8: Typical driving values with extended data
	{
		uint32_t can_id = 0x100;
		
		// Extended telemetry: throttle, steering, speed, battery, temp, etc.
		int8_t telemetry[16] = {
			60,   // throttle
			0,    // steering
			80,   // speed
			95,   // battery %
			45,   // temperature
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // reserved
		};
		ASSERT_NO_THROW(can.sendFrameFD(can_id, telemetry, 16));
	}

	// Test 9: Exceeding max length
	{
		uint32_t can_id = 0x100;
		
		int8_t oversized[100];
		for (int i = 0; i < 100; i++) oversized[i] = i;
		
		// Should truncate to 64 bytes, not throw
		ASSERT_NO_THROW(can.sendFrameFD(can_id, oversized, 100));
	}
}

//CAN_FD sendFrame - Should fail
TEST_F(CANControllerTest, InvalidSendFrameFD) {
	
	// Test 1: Send on uninitialized controller
	{
		CANController can(validInterface);
		can.cleanup();
		EXPECT_FALSE(can.isInitialized());
		
		int8_t data[2] = {50, 0};
		EXPECT_THROW(can.sendFrameFD(0x100, data, 2), CANController::CANException);
	}
	
	// Test 2: Send with invalid socket (moved-from object)
	{
		CANController original(validInterface);
		CANController moved(std::move(original));
		
		int8_t data[2] = {50, 0};
		EXPECT_THROW(original.sendFrameFD(0x100, data, 2), CANController::CANException);
	}
	
	// Test 3: Send on closed socket
	{
		CANController can(validInterface);
		int socket = can.getSocket();
		close(socket);
		
		int8_t data[2] = {50, 0};
		EXPECT_THROW(can.sendFrameFD(0x100, data, 2), CANController::CANException);
	}
	
	// Test 4: Invalid CAN ID
	{
		CANController can(validInterface);
		int8_t data[2] = {50, 0};
	
		uint32_t invalid_id = 0xFFFFFFFF;
		EXPECT_THROW(can.sendFrameFD(invalid_id, data, 2), CANController::CANException);
	
		uint32_t just_over_limit = 0x800;
		EXPECT_THROW(can.sendFrameFD(just_over_limit, data, 2), CANController::CANException);
	}
}

//Getters
TEST_F(CANControllerTest, Getters) {
	
	// Test 1: Getters on initialized controller
	{
		CANController can(validInterface);
		
		EXPECT_TRUE(can.isInitialized());
		EXPECT_EQ(can.getInterface(), validInterface);
		EXPECT_GE(can.getSocket(), 0);
	}
	
	// Test 2: Getters after cleanup
	{
		CANController can(validInterface);
		can.cleanup();

		EXPECT_FALSE(can.isInitialized());
		EXPECT_EQ(can.getInterface(), validInterface);
		EXPECT_LT(can.getSocket(), 0);
	}
	
	// Test 3: Getters on moved-from object
	{
		CANController original(validInterface);
		CANController moved(std::move(original));
		
		EXPECT_FALSE(original.isInitialized());
		EXPECT_LT(original.getSocket(), 0);
		
		EXPECT_TRUE(moved.isInitialized());
		EXPECT_EQ(moved.getInterface(), validInterface);
		EXPECT_GE(moved.getSocket(), 0);
	}
	
	// Test 4: Multiple calls to getters return consistent values
	{
		CANController can(validInterface);
		
		bool init1 = can.isInitialized();
		bool init2 = can.isInitialized();
		EXPECT_EQ(init1, init2);
		
		int socket1 = can.getSocket();
		int socket2 = can.getSocket();
		EXPECT_EQ(socket1, socket2);
		
		const std::string& if1 = can.getInterface();
		const std::string& if2 = can.getInterface();
		EXPECT_EQ(if1, if2);
	}
	
	// Test 5: Socket value doesn't change during operation
	{
		CANController can(validInterface);
		int socket_before = can.getSocket();
		
		// Send some frames
		int8_t data[2] = {50, 0};
		can.sendFrame(0x100, data, 2);
		can.sendFrame(0x200, data, 2);
		
		int socket_after = can.getSocket();
		EXPECT_EQ(socket_before, socket_after);
	}
}
