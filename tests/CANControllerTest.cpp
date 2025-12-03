#include <gtest/gtest.h>
#include "CANController.hpp"

class CANControllerTest : public ::testing::Test {

protected:
	const std::string validInterface = "vcan0";
    const std::string invalidInterface = "invalid_can99";

	void SetUp() override {
        // Setup vcan0 if needed (in real tests)
        system("sudo ip link add dev vcan0 type vcan");
        system("sudo ip link set vcan0 mtu 72");
        system("sudo ip link set up vcan0");
    }

	void TearDown() override {
        // Cleanup
    }
};

TEST_F(CANControllerTest, ConstructorValidInterface) {
    ASSERT_NO_THROW({
        CANController can(validInterface);
        EXPECT_TRUE(can.isInitialized());
        EXPECT_EQ(can.getInterface(), validInterface);
        EXPECT_GE(can.getSocket(), 0);
    });
}