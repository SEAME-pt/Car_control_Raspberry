#include <gtest/gtest.h>
#include "carControl.h"
#include "CANController.hpp"
#include <thread>
#include <chrono>

extern std::atomic<bool> g_running;

class ManualModeTest : public ::testing::Test {
protected:
    void SetUp() override {

        // Try to create vcan0 without sudo; skip if it cannot be created
        system("modprobe vcan 2>/dev/null || true");
        system("ip link add dev vcan0 type vcan 2>/dev/null || true");
        system("ip link set vcan0 mtu 72 2>/dev/null || true");
        system("ip link set up vcan0 2>/dev/null || true");
        g_running.store(true);
    }
    
    void TearDown() override {
        g_running.store(false);
        system("ip link delete vcan0 2>/dev/null || true");
    }
};

// Test that manualLoop exits promptly when carControl.exit is true
TEST_F(ManualModeTest, ManualLoopExitsWhenCarControlExitIsTrue) {
    t_carControl carControl;
    try {
        carControl.can = std::make_unique<CANController>("vcan0");
    } catch (const CANController::CANException&) {
        GTEST_SKIP() << "vcan0 interface not available";
    }
    
    carControl.exit = true;
    
    // Measure execution time to ensure it exits quickly
    auto start = std::chrono::high_resolution_clock::now();
    
    manualLoop(&carControl);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // Expect the loop to exit almost immediately (within 50 ms)
    EXPECT_LT(duration.count(), 50);
}

// Test that manualLoop runs until g_running is set to false
TEST_F(ManualModeTest, ManualLoopRunsUntilGlobalRunningIsFalse) {
    t_carControl carControl;
    try {
        carControl.can = std::make_unique<CANController>("vcan0");
    } catch (const CANController::CANException&) {
        GTEST_SKIP() << "vcan0 interface not available";
    }
    
    carControl.exit = false;  // Loop should continue
    
    // Start the manual loop in a separate thread
    std::atomic<bool> loop_started{false};
    std::thread loop_thread([&carControl, &loop_started]() {
        loop_started.store(true);
        manualLoop(&carControl);
    });
    
    // Wait for the loop to start
    while (!loop_started.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Let it run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Stop the loop by setting g_running to false
    g_running.store(false);
    
    // Wait for the thread to finish
    loop_thread.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should exit quickly after g_running is set to false (within 200ms)
    EXPECT_LT(duration.count(), 200);
}

// Test behavior when CAN interface fails to initialize
TEST_F(ManualModeTest, ManualLoopHandlesCANFailure) {
    // Temporarily remove vcan0 to force CAN initialization failure
    system("sudo ip link delete vcan0 2>/dev/null");
    
    t_carControl carControl;
    carControl.exit = true;  // Set exit true so it won't loop indefinitely
    
    // This should throw an exception because vcan0 doesn't exist
    EXPECT_THROW({
        carControl.can = std::make_unique<CANController>("vcan0");
        manualLoop(&carControl);
    }, CANController::CANException);
    
    // Recreate vcan0 for cleanup in TearDown
    system("sudo ip link add dev vcan0 type vcan 2>/dev/null");
    system("sudo ip link set vcan0 mtu 72 2>/dev/null");
    system("sudo ip link set up vcan0 2>/dev/null");
}

TEST_F(ManualModeTest, ManualLoopWithJoystick)
{
	try {

		if (std::getenv("SKIP_HARDWARE_TESTS")) {
			GTEST_SKIP() << "Skipping interactive joystick test (SKIP_HARDWARE_TESTS set)";
		}
		t_carControl carControl;
		try {
			carControl.can = std::make_unique<CANController>("vcan0");
		} catch (const CANController::CANException&) {
			GTEST_SKIP() << "vcan0 interface not available";
		}
		carControl.controller = std::make_unique<Joystick>();
    	carControl.exit = false;

    	std::atomic<bool> loop_started{false};
    	std::thread loop_thread([&carControl, &loop_started]() {
        	loop_started.store(true);
			manualLoop(&carControl);
    	});
	
    	// Wait for loop to start
    	while (!loop_started.load()) {
        	std::this_thread::sleep_for(std::chrono::milliseconds(1));
    	}

	    g_running.store(false);

    	// Stop the loop
    	loop_thread.join();
    
    	// If we reach here without crashing, the test passes
    	SUCCEED();
	}
	catch (const std::runtime_error& e) {
		GTEST_SKIP() << "No joystick available for testing: " << e.what();
	}
}