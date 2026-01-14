#include <gtest/gtest.h>
#include <sys/resource.h>
#include <socketCAN.h>
#include "CANController.hpp"

class socketCANTest : public ::testing::Test {

protected:
	const char *validInterface = "vcan0";
	const char *invalidInterface = "invalid_can99";
	const char *emptyInterface = "";

	void SetUp() override {
		// Setup vcan0 with loopback enabled
		system("sudo ip link add dev vcan0 type vcan");
		system("sudo ip link set vcan0 mtu 72");
		system("sudo ip link set vcan0 type vcan loopback on");
		system("sudo ip link set up vcan0");
	}

	void TearDown() override {
		// Cleanup: remove virtual CAN interface
		system("sudo ip link delete vcan0 2>/dev/null");
	}
};

//check_mtu_support
TEST_F(socketCANTest, ValidCheckMTU) {

	// test normal mtu being = to 72
	{
		int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
		struct ifreq ifr;
		ASSERT_NE(s, -1);
	
		strncpy(ifr.ifr_name, validInterface, IFNAMSIZ);
		ASSERT_EQ(ioctl(s, SIOCGIFINDEX, &ifr), 0);
	
		int result = check_mtu_support(s, &ifr);
		EXPECT_EQ(result, 0);
		EXPECT_EQ(ifr.ifr_mtu, CANFD_MTU); // PASS = 72
	
		close(s);
	}
	// test mtu for classical can = 16
	{
		system("sudo ip link delete vcan0 2>/dev/null");
		system("sudo ip link add dev vcan0 type vcan");
		system("sudo ip link set vcan0 down");
		system("sudo ip link set vcan0 mtu 16");
		system("sudo ip link set up vcan0");

		int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
		struct ifreq ifr;
		ASSERT_NE(s, -1);
	
		strncpy(ifr.ifr_name, validInterface, IFNAMSIZ);
		ASSERT_EQ(ioctl(s, SIOCGIFINDEX, &ifr), 0);
	
		int result = check_mtu_support(s, &ifr);

		EXPECT_EQ(result, 0);
		EXPECT_EQ(ifr.ifr_mtu, CAN_MTU); // PASS = 16
	
		close(s);
	}
}

//check_mtu_support
TEST_F(socketCANTest, InvalidCheckMTU) {

	int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	struct ifreq ifr;
	ASSERT_NE(s, -1);

	// invalid interface names
	strncpy(ifr.ifr_name, invalidInterface, IFNAMSIZ);
	EXPECT_LT(ioctl(s, SIOCGIFINDEX, &ifr), 0);
	EXPECT_EQ(check_mtu_support(s, &ifr), -1);

	strncpy(ifr.ifr_name, emptyInterface, IFNAMSIZ);
	EXPECT_LT(ioctl(s, SIOCGIFINDEX, &ifr), 0);
	EXPECT_EQ(check_mtu_support(s, &ifr), -1);

	//Force Classical CAN
	system("sudo ip link set vcan0 mtu 16");
	strncpy(ifr.ifr_name, validInterface, IFNAMSIZ);
	ASSERT_EQ(ioctl(s, SIOCGIFINDEX, &ifr), 0);

	close(s);
}

// socketCan_init
TEST_F(socketCANTest, ValidSocketInit) {

	// Test 1: Normal init with valid interface
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);

		struct ifreq ifr;
		strncpy(ifr.ifr_name, validInterface, IFNAMSIZ);
		EXPECT_EQ(ioctl(s, SIOCGIFINDEX, &ifr), 0);
		
		close(s);
	}
	// Test 2: Multiple sockets on same interface
	{
		int s1 = socketCan_init(validInterface);
		ASSERT_GE(s1, 0);
		
		int s2 = socketCan_init(validInterface);
		ASSERT_GE(s2, 0);
		
		// Should be different sockets
		EXPECT_NE(s1, s2);
		
		close(s1);
		close(s2);
	}
	// Test 3: Sending frames
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);
		
		// Try sending a frame to verify socket works
		int16_t data[2] = {50, 0};
		int result = can_send_frame(s, 0x100, data, 2);
		EXPECT_EQ(result, 0);
		
		close(s);
	}
	// Test 4: Sending frames CAN_FD
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);

		int16_t data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
		int result = can_send_frame_fd(s, 0x200, data, 16);
		EXPECT_EQ(result, 0);
		
		close(s);
	}
}

// socketCan_init
TEST_F(socketCANTest, InvalidSocketInit) {

	{
		// invalid interfaces
		int s1 = socketCan_init(invalidInterface);
		EXPECT_LT(s1, 0);
		int s2 = socketCan_init(emptyInterface);
		EXPECT_LT(s2, 0);
		int s3 = socketCan_init(NULL);
		EXPECT_LT(s3, 0);
	}
	// socket creation failed
	{
		// get the max amount of fd on my system
		struct rlimit limit;
		getrlimit(RLIMIT_NOFILE, &limit);

		std::vector<int> fds;

		int max_attempts = limit.rlim_cur;

		for (int i = 0; i < max_attempts; i++) {
			int fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
			if (fd < 0)
				break;
			fds.push_back(fd);
		}

		int result = socketCan_init(validInterface);
		EXPECT_LT(result, 0);
		
		// Cleanup
		for (int fd : fds)
			close(fd);
	}
}

// can_send_frame - Invalid tests
TEST_F(socketCANTest, InvalidSendFrame) {

	// Test 1: invalid socket
	{
		int16_t data[2] = {50, 0};
		int result = can_send_frame(-1, 0x100, data, 2);
		EXPECT_LT(result, 0);
	}

	// Test 2: Send frames on closed socket
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);
		close(s);
		
		int16_t data[2] = {50, 0};
		int result = can_send_frame(s, 0x100, data, 2);
		EXPECT_LT(result, 0);
	}

	// Test 3: Invalid CAN ID
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);
		
		int16_t data[2] = {50, 0};
		
		// Just over limit
		int result1 = can_send_frame(s, 0x800, data, 2);
		EXPECT_LT(result1, 0);
		
		// Way over limit
		int result2 = can_send_frame(s, 65535, data, 2);
		EXPECT_LT(result2, 0);
		
		close(s);
	}

	// Test 4: NULL data with non-zero length
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);
		
		// NULL data but len > 0 should still work (writes zeros)
		int result = can_send_frame(s, 0x100, NULL, 2);
		EXPECT_EQ(result, 0);
		
		close(s);
	}
}

// can_send_frame_fd - Invalid tests
TEST_F(socketCANTest, InvalidSendFrameFD) {

	// Test 1: Send on invalid socket
	{
		int16_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
		int result = can_send_frame_fd(-1, 0x100, data, 8);
		EXPECT_LT(result, 0);
	}

	// Test 2: Send frames on closed socket
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);
		close(s);
		
		int16_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
		int result = can_send_frame_fd(s, 0x100, data, 8);
		EXPECT_LT(result, 0);
	}

	// Test 3: NULL data with non-zero length
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);

		int result = can_send_frame_fd(s, 0x100, NULL, 8);
		EXPECT_EQ(result, 0);
		
		close(s);
	}

	// Test 4: Very large CAN ID
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);
		
		int16_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
		int result = can_send_frame_fd(s, 0xFFFFFFFF, data, 8);

		EXPECT_LT(result, 0);
		
		close(s);
	}
}

// can_close
TEST_F(socketCANTest, InvalidClose) {

	// Test 1: Close invalid socket
	{
		can_close(-1);
		SUCCEED();
	}

	// Test 2: Double close
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);
		
		can_close(s);
		can_close(s);
		SUCCEED();
	}

	// Test 3: Close uninitialized socket
	{
		int s = -1;
		can_close(s);
		SUCCEED();
	}
}

// CANController::receiveFrame
TEST_F(socketCANTest, ValidReceiveFrame) {
	
	// Create TWO separate CANController instances: one sender, one receiver
	CANController sender(validInterface);
	CANController receiver(validInterface);
	
	// Test data
	uint8_t expected_bytes[4] = {0xAA, 0xBB, 0xCC, 0xDD};
	int16_t *test_data = (int16_t*)expected_bytes;
	uint16_t test_id = 0x123;

	// Send a frame using CANController
	sender.sendFrame(test_id, test_data, 4);
	std::cout << "Frame sent with ID: 0x" << std::hex << test_id << std::dec << std::endl;

	// Give some time for the frame to be available
	usleep(5000); // 5ms
	
	// Call CANController::receiveFrame - this should call can_try_receive internally
	struct can_frame received_frame;
	std::cout << "Calling receiveFrame..." << std::endl;
	int receive_result = receiver.receiveFrame(&received_frame);
	std::cout << "receiveFrame returned: " << receive_result << std::endl;
	
	// The function should be called regardless of success/failure
	if (receive_result == 0) {
		// Success case - verify the data
		EXPECT_EQ(received_frame.can_id, test_id);
		EXPECT_EQ(received_frame.can_dlc, 4);
		
		for (int i = 0; i < 4; i++) {
			EXPECT_EQ(received_frame.data[i], expected_bytes[i]);
		}
	} else {
		// Function was called but returned -1 (no data or error)
		FAIL() << "CANController::receiveFrame returned -1, but function was executed";
	}
}

// Direct test of can_try_receive function
TEST_F(socketCANTest, DirectReceiveTest) {
	
	// Create raw sockets to test can_try_receive directly
	int sender_socket = socketCan_init(validInterface);
	int receiver_socket = socketCan_init(validInterface);
	ASSERT_GE(sender_socket, 0);
	ASSERT_GE(receiver_socket, 0);

	// Test data
	uint8_t expected_bytes[4] = {0x11, 0x22, 0x33, 0x44};
	int16_t *test_data = (int16_t*)expected_bytes;
	uint16_t test_id = 0x456;

	// Send frame using raw function
	int send_result = can_send_frame(sender_socket, test_id, test_data, 4);
	EXPECT_EQ(send_result, 0);
	std::cout << "Direct test: Frame sent" << std::endl;

	// Wait for frame
	usleep(5000);
	
	// Call can_try_receive DIRECTLY
	struct can_frame received_frame;
	std::cout << "Direct test: Calling can_try_receive..." << std::endl;
	int receive_result = can_try_receive(receiver_socket, &received_frame);
	std::cout << "Direct test: can_try_receive returned: " << receive_result << std::endl;
	
	if (receive_result == 0) {
		EXPECT_EQ(received_frame.can_id, test_id);
		EXPECT_EQ(received_frame.can_dlc, 4);
		for (int i = 0; i < 4; i++) {
			EXPECT_EQ(received_frame.data[i], expected_bytes[i]);
		}
	} else {
		FAIL() << "can_try_receive returned -1";
	}

	can_close(sender_socket);
	can_close(receiver_socket);
}

// Test canfd_try_receive function
TEST_F(socketCANTest, DirectReceiveTestFD) {
	
	// Create raw sockets to test canfd_try_receive directly
	int sender_socket = socketCan_init(validInterface);
	int receiver_socket = socketCan_init(validInterface);
	ASSERT_GE(sender_socket, 0);
	ASSERT_GE(receiver_socket, 0);

	// Test data for CAN FD
	uint8_t expected_bytes[8] = {0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
	int16_t *test_data = (int16_t*)expected_bytes;
	uint16_t test_id = 0x789;

	// Send CAN FD frame
	int send_result = can_send_frame_fd(sender_socket, test_id, test_data, 8);
	EXPECT_EQ(send_result, 0);
	std::cout << "Direct FD test: Frame sent" << std::endl;

	// Wait for frame
	usleep(5000);
	
	// Call canfd_try_receive DIRECTLY
	struct canfd_frame received_frame;
	std::cout << "Direct FD test: Calling canfd_try_receive..." << std::endl;
	int receive_result = canfd_try_receive(receiver_socket, &received_frame);
	std::cout << "Direct FD test: canfd_try_receive returned: " << receive_result << std::endl;
	
	if (receive_result == 0) {
		EXPECT_EQ(received_frame.can_id, test_id);
		EXPECT_EQ(received_frame.len, 8);
		for (int i = 0; i < 8; i++) {
			EXPECT_EQ(received_frame.data[i], expected_bytes[i]);
		}
	} else {
		FAIL() << "canfd_try_receive returned -1";
	}

	can_close(sender_socket);
	can_close(receiver_socket);
}

// Test CANController::receiveFrameFD method
TEST_F(socketCANTest, CANControllerReceiveFrameFD) {
	
	// Create CANController instances
	CANController sender(validInterface);
	CANController receiver(validInterface);
	
	// Test data for CAN FD
	uint8_t expected_bytes[8] = {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90};
	int16_t *test_data = (int16_t*)expected_bytes;
	uint16_t test_id = 0x654;

	// Send CAN FD frame using CANController
	sender.sendFrameFD(test_id, test_data, 8);
	std::cout << "CANController FD test: Frame sent" << std::endl;

	// Wait for frame
	usleep(5000);
	
	// Call CANController::receiveFrameFD - this should call canfd_try_receive internally
	struct canfd_frame received_frame;
	std::cout << "CANController FD test: Calling receiveFrameFD..." << std::endl;
	int receive_result = receiver.receiveFrameFD(&received_frame);
	std::cout << "CANController FD test: receiveFrameFD returned: " << receive_result << std::endl;
	
	if (receive_result == 0) {
		EXPECT_EQ(received_frame.can_id, test_id);
		EXPECT_EQ(received_frame.len, 8);
		for (int i = 0; i < 8; i++) {
			EXPECT_EQ(received_frame.data[i], expected_bytes[i]);
		}
	} else {
		FAIL() << "CANController::receiveFrameFD returned -1";
	}
}
