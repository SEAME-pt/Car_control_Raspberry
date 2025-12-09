#include <gtest/gtest.h>
#include "carControl.h"

/********************************/
/*			CarControl INIT		*/
/********************************/

class CarControlInitTest : public ::testing::Test {
protected:
	const std::string validInterface = "vcan0";

	void SetUp() override {
		// Setup vcan0
		system("sudo ip link add dev vcan0 type vcan");
		system("sudo ip link set vcan0 mtu 72");
		system("sudo ip link set up vcan0");
	}

	void TearDown() override {
		// Cleanup
		system("sudo ip link delete vcan0 2>/dev/null");
	}
};

// Test valid arguments (no joystick, valid CAN)
TEST_F(CarControlInitTest, MinimalValidInit) {

	const char* argv[] = {"car", "--joy=false", "--can=vcan0"};
	int argc = 3;

	t_carControl ctrl = initCarControl(argc, const_cast<char**>(argv));

	EXPECT_FALSE(ctrl.exit);
	EXPECT_FALSE(ctrl.useJoystick);
	EXPECT_EQ(ctrl.joystick, nullptr);
	EXPECT_NE(ctrl.can, nullptr);
	EXPECT_EQ(ctrl.canInterface, validInterface);

	if (ctrl.can)
		EXPECT_TRUE(ctrl.can->isInitialized());
}

// Test with invalid interface
TEST_F(CarControlInitTest, InvalidCANInterface) {

	const char* argv[] = {"car", "--joy=false", "--can=invalid_can99"};
	int argc = 3;
	
	t_carControl ctrl = initCarControl(argc, const_cast<char**>(argv));
	
	// Should fail and set exit flag
	EXPECT_TRUE(ctrl.exit);
	EXPECT_EQ(ctrl.can, nullptr);
}

// Test with joystick enabled but no joystick connected
TEST_F(CarControlInitTest, JoystickEnabledButNotConnected) {

	const char* argv[] = {"car", "--can=vcan0"};
	int argc = 2;
	
	t_carControl ctrl = initCarControl(argc, const_cast<char**>(argv));
	if (ctrl.joystick)
		GTEST_SKIP();
	// Should fail due to no joystick
	EXPECT_TRUE(ctrl.exit);
	EXPECT_TRUE(ctrl.useJoystick);
	EXPECT_EQ(ctrl.joystick, nullptr);
}

// Test default values
TEST_F(CarControlInitTest, DefaultValues) {

	//test some input being equal to false
	{
		const char* argv[] = {"car", "--joy=false"};
		int argc = 2;
		
		t_carControl ctrl = initCarControl(argc, const_cast<char**>(argv));
	
		EXPECT_FALSE(ctrl.useJoystick);
		EXPECT_FALSE(ctrl.debug);
		EXPECT_EQ(ctrl.canInterface, "can0");
	}
	//test some input being equal to true
	{
		const char* argv[] = {"car", "--joy=weirdBool"};
		int argc = 2;
	
		t_carControl ctrl = initCarControl(argc, const_cast<char**>(argv));

		EXPECT_TRUE(ctrl.useJoystick);
	}
	// test a valid bool but in a diff format
	{
		const char* argv[] = {"car", "--joy=TRUE"};
		int argc = 2;
	
		t_carControl ctrl = initCarControl(argc, const_cast<char**>(argv));

		EXPECT_TRUE(ctrl.useJoystick);
	}
	// test "--help" input flag
	{
		const char* argv[] = {"car", "--help"};
		int argc = 2;
		
		t_carControl ctrl = initCarControl(argc, const_cast<char**>(argv));
	
		EXPECT_TRUE(ctrl.exit);
	}
}

// Test with debug flag
TEST_F(CarControlInitTest, DebugFlag) {

	const char* argv[] = {"car", "--can=vcan0", "--joy=false", "--debug"};
	int argc = 4;
	
	t_carControl ctrl = initCarControl(argc, const_cast<char**>(argv));
	
	EXPECT_FALSE(ctrl.exit);
	EXPECT_TRUE(ctrl.debug);
}

// Test invalid arguments
TEST_F(CarControlInitTest, InvalidArguments) {

	const char* argv[] = {"car", "--invalid=true"};
	int argc = 2;
	
	t_carControl ctrl = initCarControl(argc, const_cast<char**>(argv));
	
	EXPECT_TRUE(ctrl.exit);
}

// Test CAN initialization with joystick
TEST_F(CarControlInitTest, WithJoystickIfAvailable) {
	
	// Skip if no joystick
	if (SDL_Init(SDL_INIT_JOYSTICK) < 0 || SDL_NumJoysticks() <= 0) {
		SDL_Quit();
		GTEST_SKIP() << "No joystick connected";
	}
	SDL_Quit();

	const char* argv[] = {"car", "--can=vcan0"};
	int argc = 2;

	t_carControl ctrl = initCarControl(argc, const_cast<char**>(argv));

	EXPECT_FALSE(ctrl.exit);
	EXPECT_TRUE(ctrl.useJoystick);
	EXPECT_NE(ctrl.joystick, nullptr);
	EXPECT_NE(ctrl.can, nullptr);

	cleanExit(ctrl.joystick);
}

// cleanExit
TEST_F(CarControlInitTest, CleanExit) {
	
	// NULL joystick
	ASSERT_NO_THROW({
		cleanExit(nullptr);
	});

	// Valid joystick (if available)
	if (SDL_Init(SDL_INIT_JOYSTICK) >= 0 && SDL_NumJoysticks() > 0) {
		SDL_Joystick* joy = SDL_JoystickOpen(0);
		if (joy) {
			ASSERT_NO_THROW({
				cleanExit(joy);
			});
		}
	}
}