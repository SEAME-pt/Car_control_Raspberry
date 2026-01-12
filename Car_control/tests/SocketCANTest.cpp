#include <gtest/gtest.h>
#include <sys/resource.h>
#include <socketCAN.h>

class socketCANTest : public ::testing::Test {

protected:
	const char *validInterface = "vcan0";
	const char *invalidInterface = "invalid_can99";
	const char *emptyInterface = "";

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

		EXPECT_EQ(result, 1);
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
		int8_t data[2] = {50, 0};
		int result = can_send_frame(s, 0x100, data, 2);
		EXPECT_EQ(result, 0);
		
		close(s);
	}
	// Test 4: Sending frames CAN_FD
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);

		int8_t data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
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
		int8_t data[2] = {50, 0};
		int result = can_send_frame(-1, 0x100, data, 2);
		EXPECT_LT(result, 0);
	}

	// Test 2: Send frames on closed socket
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);
		close(s);
		
		int8_t data[2] = {50, 0};
		int result = can_send_frame(s, 0x100, data, 2);
		EXPECT_LT(result, 0);
	}

	// Test 3: Invalid CAN ID
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);
		
		int8_t data[2] = {50, 0};
		
		// Just over limit
		int result1 = can_send_frame(s, 0x800, data, 2);
		EXPECT_LT(result1, 0);
		
		// Way over limit
		int result2 = can_send_frame(s, 0xFFFFFFFF, data, 2);
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
		int8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
		int result = can_send_frame_fd(-1, 0x100, data, 8);
		EXPECT_LT(result, 0);
	}

	// Test 2: Send frames on closed socket
	{
		int s = socketCan_init(validInterface);
		ASSERT_GE(s, 0);
		close(s);
		
		int8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
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
		
		int8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
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