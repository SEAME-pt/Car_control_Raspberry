#include <gtest/gtest.h>
#include "carControl.h"

/********************************/
/*			CAN INIT			*/
/********************************/

class CANInitTest : public ::testing::Test {
protected:
	const std::string validInterface = "vcan0";
	const std::string invalidInterface = "invalid_can99";
	const std::string emptyInterface = "";

	void SetUp() override {

		system("sudo ip link add dev vcan0 type vcan");
		system("sudo ip link set vcan0 mtu 72");
		system("sudo ip link set up vcan0");
	}

	void TearDown() override {

		system("sudo ip link delete vcan0 2>/dev/null");
	}
};

// Test valid initialization
TEST_F(CANInitTest, ValidInit) {
	
	std::unique_ptr<CANController> can;
	
	ASSERT_NO_THROW({
		can = init_can(validInterface);
	});

	// if pointer is valid
	ASSERT_NE(can, nullptr);
	
	// Verify if is initialized
	EXPECT_TRUE(can->isInitialized());
	EXPECT_EQ(can->getInterface(), validInterface);
	EXPECT_GE(can->getSocket(), 0);
}

// unique_ptr ownership
TEST_F(CANInitTest, UniquePointerOwnership) {
	
	auto can = init_can(validInterface);
	ASSERT_NE(can, nullptr);
	
	// Move ownership
	auto can2 = std::move(can);
	
	// Original should be null
	EXPECT_EQ(can, nullptr);
	
	// New should be valid
	ASSERT_NE(can2, nullptr);
	EXPECT_TRUE(can2->isInitialized());
}

// Test if can send frames after init
TEST_F(CANInitTest, CanSendFramesAfterInit) {
	
	auto can = init_can(validInterface);
	ASSERT_NE(can, nullptr);
	
	// Sending a frame
	int16_t data[2] = {50, 0};
	ASSERT_NO_THROW({
		can->sendFrame(0x100, data, 2);
	});
}

// Invalid interface
TEST_F(CANInitTest, InvalidInit) {

	EXPECT_THROW({
		auto can = init_can(invalidInterface);
	}, CANController::CANException);

	EXPECT_THROW({
		auto can = init_can(emptyInterface);
	}, CANController::CANException);
}

// Test exception message
TEST_F(CANInitTest, ExceptionMessage) {

	try {
		auto can = init_can(invalidInterface);
		FAIL() << "Expected CANException";
	} catch (const CANController::CANException& e) {
		std::string msg = e.what();
		EXPECT_NE(msg.find("CAN Error"), std::string::npos);
		EXPECT_NE(msg.find(invalidInterface), std::string::npos);
	}
	EXPECT_THROW({
		auto can = init_can(emptyInterface);
	}, CANController::CANException);
}

// Test multiple initializations
TEST_F(CANInitTest, MultipleInits) {
	
	auto can1 = init_can(validInterface);
	ASSERT_NE(can1, nullptr);
	
	auto can2 = init_can(validInterface);
	ASSERT_NE(can2, nullptr);
	
	// Both should be valid
	EXPECT_TRUE(can1->isInitialized());
	EXPECT_TRUE(can2->isInitialized());
	
	// Should have different sockets
	EXPECT_NE(can1->getSocket(), can2->getSocket());
}
