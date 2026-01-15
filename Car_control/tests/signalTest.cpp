#include <gtest/gtest.h>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include "carControl.h"

// Helper to reset g_running between tests
class SignalTest : public ::testing::Test {
protected:
	void SetUp() override { g_running.store(true); }
	void TearDown() override { g_running.store(true); }
};


// Test that signalHandler sets g_running to false when SIGINT is received
TEST_F(SignalTest, SignalHandlerSetsRunningFalse) {
	g_running.store(true);
	signalHandler(SIGINT);
	EXPECT_FALSE(g_running.load());
}


// Test that signalManager installs signal handlers without throwing
TEST_F(SignalTest, SignalManagerInstallsHandlers) {
	EXPECT_NO_THROW(signalManager());
}
// Test that signalHandler sets g_running to false when SIGTERM is received
TEST_F(SignalTest, SignalHandlerSetsRunningFalseForSIGTERM) {
	g_running.store(true);
	signalHandler(SIGTERM);
	EXPECT_FALSE(g_running.load());
}

// Test that signalHandler sets g_running to false when SIGUSR1 is received
TEST_F(SignalTest, SignalHandlerSetsRunningFalseForSIGUSR1) {
	g_running.store(true);
	signalHandler(SIGUSR1);
	EXPECT_FALSE(g_running.load());
}

// Test that g_running remains true if signalHandler is not called
TEST_F(SignalTest, RunningRemainsTrueIfHandlerNotCalled) {
	g_running.store(true);
	EXPECT_TRUE(g_running.load());
}

// Test that calling signalManager multiple times does not throw and does not affect g_running
TEST_F(SignalTest, MultipleSignalManagerCallsNoThrow) {
	EXPECT_NO_THROW(signalManager());
	EXPECT_NO_THROW(signalManager());
	EXPECT_TRUE(g_running.load());
}
