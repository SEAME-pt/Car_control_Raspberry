#include <gtest/gtest.h>
#include "carControl.h"
#include "CANController.hpp"
#include <thread>
#include <chrono>

extern std::atomic<bool> g_running;

class AutonomousModeTest : public ::testing::Test {
protected:
    void SetUp() override {
        system("sudo ip link add dev vcan0 type vcan");
        system("sudo ip link set vcan0 mtu 72");
        system("sudo ip link set up vcan0");
        g_running.store(true);
    }
    
    void TearDown() override {
        g_running.store(false);
        system("sudo ip link delete vcan0 2>/dev/null");
    }
};

// Test that autonomousLoop exits promptly when carControl.exit is true
TEST_F(AutonomousModeTest, AutonomousLoopExitsWhenCarControlExitIsTrue) {
    t_carControl carControl;
    try {
        carControl.can = std::make_unique<CANController>("vcan0");
    } catch (const CANController::CANException&) {
        GTEST_SKIP() << "vcan0 interface not available";
    }
    
    carControl.exit = true;
    
    // Measure execution time to ensure it exits quickly
    auto start = std::chrono::high_resolution_clock::now();
    
    autonomousLoop(carControl);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // Expect the loop to exit almost immediately (within 50 ms)
    EXPECT_LT(duration.count(), 50);
}

// Test that autonomousLoop runs until g_running is set to false
TEST_F(AutonomousModeTest, AutonomousLoopRunsUntilGlobalRunningIsFalse) {
    t_carControl carControl;
    try {
        carControl.can = std::make_unique<CANController>("vcan0");
    } catch (const CANController::CANException&) {
        GTEST_SKIP() << "vcan0 interface not available";
    }
    
    carControl.exit = false;  // Loop should continue
    
    // Start the autonomous loop in a separate thread
    std::atomic<bool> loop_started{false};
    std::thread loop_thread([&carControl, &loop_started]() {
        loop_started.store(true);
        autonomousLoop(carControl);
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
TEST_F(AutonomousModeTest, AutonomousLoopHandlesCANFailure) {
    // Temporarily remove vcan0 to force CAN initialization failure
    system("sudo ip link delete vcan0 2>/dev/null");
    
    t_carControl carControl;
    carControl.exit = true;  // Set exit true so it won't loop indefinitely
    
    // This should throw an exception because vcan0 doesn't exist
    EXPECT_THROW({
        carControl.can = std::make_unique<CANController>("vcan0");
        autonomousLoop(carControl);
    }, CANController::CANException);
    
    // Recreate vcan0 for cleanup in TearDown
    system("sudo ip link add dev vcan0 type vcan 2>/dev/null");
    system("sudo ip link set vcan0 mtu 72 2>/dev/null");
    system("sudo ip link set up vcan0 2>/dev/null");
}

TEST_F(AutonomousModeTest, AutonomousLoopHandlesNoCANController) {
    t_carControl carControl;
    carControl.can = nullptr;  // No CAN controller
    carControl.exit = true;    // So it doesn't loop indefinitely
    
    // Expect no exceptions - loop should exit immediately
    EXPECT_NO_THROW({
        autonomousLoop(carControl);
    });
}

TEST_F(AutonomousModeTest, HandlesSocketDisconnect) {
    t_carControl carControl;
    try {
        carControl.can = std::make_unique<CANController>("vcan0");
    } catch (const CANController::CANException&) {
        GTEST_SKIP() << "vcan0 interface not available";
    }
    
    carControl.exit = false;
    
    // Start autonomous loop in separate thread
    std::atomic<bool> loop_started{false};
    std::thread loop_thread([&carControl, &loop_started]() {
        loop_started.store(true);
        try {
            autonomousLoop(carControl);  // Call the REAL function
        } catch (...) {
            // Stop the loop
            g_running.store(false);
        }
    });
    
    // Wait for loop to start
    while (!loop_started.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Let it run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Simulate socket disconnect by cleaning up CAN controller
    carControl.can->cleanup();
    
    // Let it run a bit more to observe behavior
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    loop_thread.join();
    
    // If we reach here without crashing, the test passes
    SUCCEED();
}

TEST_F(AutonomousModeTest, MaxSizeCanFrame)
{
    t_carControl carControl;
    try {
        carControl.can = std::make_unique<CANController>("vcan0");
    } catch (const CANController::CANException&) {
        GTEST_SKIP() << "vcan0 interface not available";
    }
    
    carControl.exit = false;
    
    // Check if CAN FD is supported
    {
        int16_t test_data[1] = {0};
        try {
            carControl.can->sendFrameFD(0x100, test_data, 1);
        } catch (const CANController::CANException&) {
            GTEST_SKIP() << "CAN FD not supported on this interface";
        }
    }
    
    // Start autonomous loop in separate thread
    std::atomic<bool> loop_started{false};
    std::thread loop_thread([&carControl, &loop_started]() {
        loop_started.store(true);
        autonomousLoop(carControl);  // Call the REAL function
    });
    
    // Wait for loop to start
    while (!loop_started.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Let it run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Send maximum size CAN FD frame
    int16_t max_data[64];
    for (int i = 0; i < 64; ++i) {
        max_data[i] = static_cast<int16_t>(i);
    }
    carControl.can->sendFrameFD(0x7FF, max_data, 64); // Max DLC for CAN FD
    
    // Let it run a bit more to observe behavior
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Stop the loop
    g_running.store(false);
    loop_thread.join();
    
    // If we reach here without crashing, the test passes
    SUCCEED();
}
