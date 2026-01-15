// Unit tests for init.cpp:initCarControl
#include <gtest/gtest.h>
#include <stdexcept>
#include <net/if.h>
#include "carControl.h"

class InitTest : public ::testing::Test {

protected:
	const std::string validInterface = "vcan0";
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

// Test that initCarControl returns early and sets exit when --help is used
TEST_F(InitTest, ParsingHelpCausesEarlyExit) {
    char* argv[] = { (char*)"prog", (char*)"--help" };
    int argc = 2;
    t_carControl cfg = initCarControl(argc, argv);
    EXPECT_TRUE(cfg.exit);
}

// Test that if CAN init throws, initCarControl sets exit=true
TEST_F(InitTest, CanInitThrowsSetsExitTrue) {
    // Inject a CAN init that throws
    char* argv[] = { (char*)"prog" };
    int argc = 1;
    t_carControl cfg = initCarControl(argc, argv);
    EXPECT_TRUE(cfg.exit);
}

// Test that providing a valid CAN interface (vcan0) results in successful init
TEST_F(InitTest, ValidCanInitSucceeds) {
	std::string valid_device = "--can=" + validInterface;
	char* argv[] = { (char*)"prog", (char*)valid_device.c_str() };
	int argc = 2;
	t_carControl cfg = initCarControl(argc, argv);
	EXPECT_FALSE(cfg.exit);
	EXPECT_TRUE(cfg.can != nullptr);
}

// Test that providing a valid CAN interface (vcan0) and false manual mode results in successful init
TEST_F(InitTest, ValidFalseManualModeInitSucceeds) {
	std::string valid_device = "--can=" + validInterface;
	char* argv[] = { (char*)"prog", (char*)valid_device.c_str(), (char*)"--manual=false" };
	int argc = 3;
	t_carControl cfg = initCarControl(argc, argv);
	EXPECT_FALSE(cfg.exit);
	EXPECT_FALSE(cfg.controller != nullptr);
}

// Test that providing a valid CAN interface (vcan0) and true manual mode results in successful init
TEST_F(InitTest, ValidTrueManualModeInitSucceeds) {
	t_carControl cfg;
	try {
		std::string valid_device = "--can=" + validInterface;
		char* argv[] = { (char*)"prog", (char*)valid_device.c_str(), (char*)"--manual=true" };
		int argc = 3;

		std::cout << "\n=== Interactive Joystick Test ===" << std::endl;
		std::cout << "Either disconnect the controller or skip the test" << std::endl;
		std::cout << "Press ENTER to start the test or 's' to skip: ";
		
		std::string input;
		std::getline(std::cin, input);
		
		if (input == "s" || input == "S") {
			GTEST_SKIP() << "Interactive test skipped by user";
			return;
		}
		cfg = initCarControl(argc, argv);

	} catch (const std::filesystem::filesystem_error& e) {
	}
	EXPECT_TRUE(cfg.exit);
	EXPECT_TRUE(cfg.controller != nullptr);
}