#include <gtest/gtest.h>
#include "carControl.h"


/********************************/
/*			JOYSTICK			*/
/********************************/
class JoystickTest : public ::testing::Test {

protected:

	void TearDown() override {

		if (SDL_WasInit(SDL_INIT_JOYSTICK)) {
			SDL_Quit();
		}
	}

};

// Test when no joystick is connected
TEST_F(JoystickTest, NoJoystickConnected) {

	EXPECT_THROW({
		SDL_Joystick* joy = initJoystick();
	}, InitException);
}

// Test SDL initialization works
TEST_F(JoystickTest, SDLInit) {
	// Test that SDL can at least initialize
	EXPECT_GE(SDL_Init(SDL_INIT_JOYSTICK), 0);

	int numJoysticks = SDL_NumJoysticks();
	EXPECT_GE(numJoysticks, 0);
	
	SDL_Quit();
}

TEST_F(JoystickTest, InitWithJoystick) {
	// Skip if no joystick connected
	if (SDL_Init(SDL_INIT_JOYSTICK) < 0 || SDL_NumJoysticks() <= 0) {
		GTEST_SKIP() << "No joystick connected, skipping test";
	}
	SDL_Quit();

	SDL_Joystick* joy = nullptr;
	ASSERT_NO_THROW({
		joy = initJoystick();
	});
	
	ASSERT_NE(joy, nullptr);
	EXPECT_TRUE(SDL_JoystickGetAttached(joy));

	SDL_JoystickClose(joy);
	SDL_Quit();
}

TEST_F(JoystickTest, JoyErrorHandling) {

	EXPECT_THROW({
		initJoystick();
	}, InitException);

	try {
		initJoystick();
		FAIL() << "Expected InitException";
	} catch (const InitException& e) {
		std::string msg = e.what();
		EXPECT_FALSE(msg.empty());
	}
}

TEST_F(JoystickTest, JoystickSteeringAndThrottle) {
	
	// Skip if no joystick connected
	if (SDL_Init(SDL_INIT_JOYSTICK) < 0 || SDL_NumJoysticks() <= 0) {
		GTEST_SKIP() << "No joystick connected, skipping test";
	}
	
	SDL_Joystick* joy = SDL_JoystickOpen(0);
	if (joy == nullptr) {
		SDL_Quit();
		GTEST_SKIP() << "Failed to open joystick";
	}

	// Test steering range 0-120
	{
		int8_t steering = joystickSteering(joy);
		EXPECT_GE(steering, 0);
		EXPECT_LE(steering, 120);
	}

	// Test throttle range -100 to 100
	{
		int8_t throttle = joystickThrottle(joy);
		EXPECT_GE(throttle, -100);
		EXPECT_LE(throttle, 100);
	}

	// Read multiple times
	{
		int8_t steering1 = joystickSteering(joy);
		int8_t steering2 = joystickSteering(joy);

		EXPECT_NEAR(steering1, steering2, 5);
		
		int8_t throttle1 = joystickThrottle(joy);
		int8_t throttle2 = joystickThrottle(joy);
		
		EXPECT_NEAR(throttle1, throttle2, 5);
	}

	SDL_JoystickClose(joy);
	SDL_Quit();
}