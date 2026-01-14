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
    carControl.useJoystick = false;
    carControl.debug = false;
    
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
    carControl.useJoystick = false;
    carControl.debug = false;
    
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
    carControl.useJoystick = false;
    carControl.debug = false;
    
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

// Test CAN frame reception and stdout output in autonomousLoop
TEST_F(AutonomousModeTest, AutonomousLoopReceivesAndPrintsCAN) {
    t_carControl carControl;
    try {
        carControl.can = std::make_unique<CANController>("vcan0");
    } catch (const CANController::CANException&) {
        GTEST_SKIP() << "vcan0 interface not available";
    }
    
    carControl.exit = false;
    carControl.useJoystick = false;
    carControl.debug = false;
    
    // Create separate CAN controller to send frames from outside
    std::unique_ptr<CANController> sender_can;
    try {
        sender_can = std::make_unique<CANController>("vcan0");
    } catch (const CANController::CANException&) {
        GTEST_SKIP() << "Could not create sender CAN controller";
    }
    
    // Capture stdout to verify the CAN frame printing
    testing::internal::CaptureStdout();
    
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
    
    // Let the loop start and send some emergency brake frames
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    // Send additional test frames from external controller
    int16_t test_data[3] = {0x12, 0x34, 0xAB};
    sender_can->sendFrame(0x456, test_data, 3);
    
    // Give time to process
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    // Stop the loop
    g_running.store(false);
    loop_thread.join();
    
    // Get the captured output
    std::string output = testing::internal::GetCapturedStdout();
    
    // Print output for debugging
    if (output.empty()) {
        std::cout << "No output captured!" << std::endl;
    } else {
        std::cout << "Captured output: " << output << std::endl;
    }
    
    // The autonomous loop should receive its own emergency brake frames
    // or the external frames due to CAN loopback behavior
    EXPECT_TRUE(output.find("ID: 0x") != std::string::npos);
    EXPECT_TRUE(output.find("DLC:") != std::string::npos);
    EXPECT_TRUE(output.find("Data:") != std::string::npos);
}

TEST_F(AutonomousModeTest, AutonomousLoopHandlesNoCANController) {
    t_carControl carControl;
    carControl.can = nullptr;  // No CAN controller
    carControl.exit = true;    // So it doesn't loop indefinitely
    carControl.useJoystick = false;
    carControl.debug = false;
    
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
    carControl.useJoystick = false;
    carControl.debug = false;
    
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

TEST_F(AutonomousModeTest, CorruptedCanFrames)
{
    t_carControl carControl;
    try {
        carControl.can = std::make_unique<CANController>("vcan0");
    } catch (const CANController::CANException&) {
        GTEST_SKIP() << "vcan0 interface not available";
    }
    
    carControl.exit = false;
    carControl.useJoystick = false;
    carControl.debug = false;
    
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
    
    // Send corrupted CAN frame (invalid length)
    int16_t corrupted_data[8] = {0xFF}; // Data larger than DLC
    carControl.can->sendFrame(0x123, corrupted_data, 20); // Invalid DLC
    
    // Let it run a bit more to observe behavior
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Stop the loop
    g_running.store(false);
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
    carControl.useJoystick = false;
    carControl.debug = false;
    
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
