#include <gtest/gtest.h>
#include "joyStick.hpp"
#include <cstdlib>
#include <thread>
#include <chrono>

TEST(JoyStickTest, PassNullPtr)
{
	EXPECT_THROW({
			try {
				joyStick controller(nullptr);
			} catch (const std::exception &err) {
				EXPECT_STREQ("joyStick: Device cant be Null", err.what());
				throw ;
			}
	}, std::exception);
}

TEST(JoyStickTest, RunPythonSimulation)
{
	// Start Python script
	pid_t python_pid = system("python3 /home/afogonca/seame/Car_control_Raspberry/tests/main.py A &");
	// Wait for device to be created
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	try {
		joyStick controller("/dev/input/event25");
		// Test reading while Python script runs
		for (int i = 0; i < 3; i++) {
			__u16 button_code = controller.readPress();
			if (button_code != 0) {
				EXPECT_EQ(button_code, 304);
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
		SUCCEED();
	} catch (const std::exception &err) {
		GTEST_SKIP() << "Cannot test: " << err.what();
	}	
	// Kill Python script
	system("sudo pkill -f main.py");
	python_pid = system("python3 /home/afogonca/seame/Car_control_Raspberry/tests/main.py B &");
	// Wait for device to be created
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	try {
		joyStick controller("/dev/input/event25");
		
		// Test reading while Python script runs
		for (int i = 0; i < 3; i++) {
			__u16 button_code = controller.readPress();
			if (button_code != 0) {
				EXPECT_EQ(button_code, 305);
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
		SUCCEED();
	} catch (const std::exception &err) {
		GTEST_SKIP() << "Cannot test: " << err.what();
	}	
	// Kill Python script
	system("sudo pkill -f main.py");
}
