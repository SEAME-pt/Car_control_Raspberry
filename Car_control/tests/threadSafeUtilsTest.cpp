#include <gtest/gtest.h>
#include "carControl.h"
#include <thread>
#include <vector>
#include <chrono>

/********************************/
/*   THREAD-SAFE UTILS TESTS    */
/********************************/

class ThreadSafeUtilsTest : public ::testing::Test {
protected:
	t_CANReceiver receiver;

	void SetUp() override {
		// Initialize receiver with empty queues
		receiver.can = nullptr;
	}

	void TearDown() override {
		// Clear queues
		while (!receiver.speedQueue.empty()) {
			receiver.speedQueue.pop();
		}
		while (!receiver.batteryQueue.empty()) {
			receiver.batteryQueue.pop();
		}
	}
};

/********************************/
/*   getSpeedData TESTS         */
/********************************/

// Test getting speed data from empty queue
TEST_F(ThreadSafeUtilsTest, GetSpeedDataEmptyQueue) {
	t_speedData data;
	
	bool result = getSpeedData(&receiver, &data);
	
	EXPECT_FALSE(result);
}

// Test getting speed data from queue with one element
TEST_F(ThreadSafeUtilsTest, GetSpeedDataSingleElement) {
	t_speedData expectedData = {1000, 50};
	receiver.speedQueue.push(expectedData);
	
	t_speedData actualData;
	bool result = getSpeedData(&receiver, &actualData);
	
	EXPECT_TRUE(result);
	EXPECT_EQ(actualData.rpm, expectedData.rpm);
	EXPECT_EQ(actualData.speedMps, expectedData.speedMps);
	EXPECT_TRUE(receiver.speedQueue.empty());
}

// Test getting speed data with multiple elements (FIFO order)
TEST_F(ThreadSafeUtilsTest, GetSpeedDataMultipleElements) {
	t_speedData data1 = {1000, 50};
	t_speedData data2 = {2000, 100};
	t_speedData data3 = {3000, 150};
	
	receiver.speedQueue.push(data1);
	receiver.speedQueue.push(data2);
	receiver.speedQueue.push(data3);
	
	t_speedData result;
	
	// Should get first element
	ASSERT_TRUE(getSpeedData(&receiver, &result));
	EXPECT_EQ(result.rpm, 1000);
	EXPECT_EQ(result.speedMps, 50);
	
	// Should get second element
	ASSERT_TRUE(getSpeedData(&receiver, &result));
	EXPECT_EQ(result.rpm, 2000);
	EXPECT_EQ(result.speedMps, 100);
	
	// Should get third element
	ASSERT_TRUE(getSpeedData(&receiver, &result));
	EXPECT_EQ(result.rpm, 3000);
	EXPECT_EQ(result.speedMps, 150);
	
	// Queue should now be empty
	EXPECT_FALSE(getSpeedData(&receiver, &result));
}

// Test with boundary values
TEST_F(ThreadSafeUtilsTest, GetSpeedDataBoundaryValues) {
	// Test zero values
	t_speedData zeroData = {0, 0};
	receiver.speedQueue.push(zeroData);
	
	t_speedData result;
	ASSERT_TRUE(getSpeedData(&receiver, &result));
	EXPECT_EQ(result.rpm, 0);
	EXPECT_EQ(result.speedMps, 0);
	
	// Test max values
	t_speedData maxData = {UINT16_MAX, UINT16_MAX};
	receiver.speedQueue.push(maxData);
	
	ASSERT_TRUE(getSpeedData(&receiver, &result));
	EXPECT_EQ(result.rpm, UINT16_MAX);
	EXPECT_EQ(result.speedMps, UINT16_MAX);
}

// Test thread safety with concurrent reads
TEST_F(ThreadSafeUtilsTest, GetSpeedDataConcurrentReads) {
	// Push 100 elements
	for (uint16_t i = 0; i < 100; i++) {
		t_speedData data = {i, static_cast<uint16_t>(i * 2)};
		receiver.speedQueue.push(data);
	}
	
	std::atomic<int> successCount{0};
	std::vector<std::thread> threads;
	
	// Create 10 threads trying to read
	for (int i = 0; i < 10; i++) {
		threads.emplace_back([&]() {
			for (int j = 0; j < 10; j++) {
				t_speedData data;
				if (getSpeedData(&receiver, &data)) {
					successCount++;
				}
			}
		});
	}
	
	for (auto& thread : threads) {
		thread.join();
	}
	
	// Exactly 100 reads should succeed
	EXPECT_EQ(successCount.load(), 100);
	EXPECT_TRUE(receiver.speedQueue.empty());
}

// Test concurrent writes and reads
TEST_F(ThreadSafeUtilsTest, GetSpeedDataConcurrentWritesAndReads) {
	std::atomic<int> readCount{0};
	std::atomic<bool> stopReading{false};
	
	// Writer thread
	std::thread writer([&]() {
		for (int i = 0; i < 100; i++) {
			std::lock_guard<std::mutex> lock(receiver.speedMutex);
			t_speedData data = {static_cast<uint16_t>(i), static_cast<uint16_t>(i * 2)};
			receiver.speedQueue.push(data);
		}
	});
	
	// Reader thread
	std::thread reader([&]() {
		while (!stopReading.load() || !receiver.speedQueue.empty()) {
			t_speedData data;
			if (getSpeedData(&receiver, &data)) {
				readCount++;
			}
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}
	});
	
	writer.join();
	stopReading.store(true);
	reader.join();
	
	EXPECT_EQ(readCount.load(), 100);
}

/********************************/
/*   getBatteryData TESTS       */
/********************************/

// Test getting battery data from empty queue
TEST_F(ThreadSafeUtilsTest, GetBatteryDataEmptyQueue) {
	t_batteryData data;
	
	bool result = getBatteryData(&receiver, &data);
	
	EXPECT_FALSE(result);
}

// Test getting battery data from queue with one element
TEST_F(ThreadSafeUtilsTest, GetBatteryDataSingleElement) {
	t_batteryData expectedData = {12500, 85};
	receiver.batteryQueue.push(expectedData);
	
	t_batteryData actualData;
	bool result = getBatteryData(&receiver, &actualData);
	
	EXPECT_TRUE(result);
	EXPECT_EQ(actualData.voltage, expectedData.voltage);
	EXPECT_EQ(actualData.percentage, expectedData.percentage);
	EXPECT_TRUE(receiver.batteryQueue.empty());
}

// Test getting battery data with multiple elements (FIFO order)
TEST_F(ThreadSafeUtilsTest, GetBatteryDataMultipleElements) {
	t_batteryData data1 = {12000, 50};
	t_batteryData data2 = {12500, 75};
	t_batteryData data3 = {13000, 100};
	
	receiver.batteryQueue.push(data1);
	receiver.batteryQueue.push(data2);
	receiver.batteryQueue.push(data3);
	
	t_batteryData result;
	
	// Should get first element
	ASSERT_TRUE(getBatteryData(&receiver, &result));
	EXPECT_EQ(result.voltage, 12000);
	EXPECT_EQ(result.percentage, 50);
	
	// Should get second element
	ASSERT_TRUE(getBatteryData(&receiver, &result));
	EXPECT_EQ(result.voltage, 12500);
	EXPECT_EQ(result.percentage, 75);
	
	// Should get third element
	ASSERT_TRUE(getBatteryData(&receiver, &result));
	EXPECT_EQ(result.voltage, 13000);
	EXPECT_EQ(result.percentage, 100);
	
	// Queue should now be empty
	EXPECT_FALSE(getBatteryData(&receiver, &result));
}

// Test with boundary values
TEST_F(ThreadSafeUtilsTest, GetBatteryDataBoundaryValues) {
	// Test zero values
	t_batteryData zeroData = {0, 0};
	receiver.batteryQueue.push(zeroData);
	
	t_batteryData result;
	ASSERT_TRUE(getBatteryData(&receiver, &result));
	EXPECT_EQ(result.voltage, 0);
	EXPECT_EQ(result.percentage, 0);
	
	// Test max values
	t_batteryData maxData = {UINT16_MAX, UINT8_MAX};
	receiver.batteryQueue.push(maxData);
	
	ASSERT_TRUE(getBatteryData(&receiver, &result));
	EXPECT_EQ(result.voltage, UINT16_MAX);
	EXPECT_EQ(result.percentage, UINT8_MAX);
}

// Test thread safety with concurrent reads
TEST_F(ThreadSafeUtilsTest, GetBatteryDataConcurrentReads) {
	// Push 100 elements
	for (uint16_t i = 0; i < 100; i++) {
		t_batteryData data = {i, static_cast<uint8_t>(i % 101)};
		receiver.batteryQueue.push(data);
	}
	
	std::atomic<int> successCount{0};
	std::vector<std::thread> threads;
	
	// Create 10 threads trying to read
	for (int i = 0; i < 10; i++) {
		threads.emplace_back([&]() {
			for (int j = 0; j < 10; j++) {
				t_batteryData data;
				if (getBatteryData(&receiver, &data)) {
					successCount++;
				}
			}
		});
	}
	
	for (auto& thread : threads) {
		thread.join();
	}
	
	// Exactly 100 reads should succeed
	EXPECT_EQ(successCount.load(), 100);
	EXPECT_TRUE(receiver.batteryQueue.empty());
}

// Test concurrent writes and reads
TEST_F(ThreadSafeUtilsTest, GetBatteryDataConcurrentWritesAndReads) {
	std::atomic<int> readCount{0};
	std::atomic<bool> stopReading{false};
	
	// Writer thread
	std::thread writer([&]() {
		for (int i = 0; i < 100; i++) {
			std::lock_guard<std::mutex> lock(receiver.batteryMutex);
			t_batteryData data = {static_cast<uint16_t>(12000 + i), static_cast<uint8_t>(i % 101)};
			receiver.batteryQueue.push(data);
		}
	});
	
	// Reader thread
	std::thread reader([&]() {
		while (!stopReading.load() || !receiver.batteryQueue.empty()) {
			t_batteryData data;
			if (getBatteryData(&receiver, &data)) {
				readCount++;
			}
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}
	});
	
	writer.join();
	stopReading.store(true);
	reader.join();
	
	EXPECT_EQ(readCount.load(), 100);
}

/********************************/
/*   MIXED OPERATIONS TESTS     */
/********************************/

// Test that speed and battery operations are independent
TEST_F(ThreadSafeUtilsTest, IndependentQueues) {
	// Add data to both queues
	t_speedData speedData = {1000, 50};
	t_batteryData batteryData = {12500, 85};
	
	receiver.speedQueue.push(speedData);
	receiver.batteryQueue.push(batteryData);
	
	// Get battery data shouldn't affect speed queue
	t_batteryData resultBattery;
	ASSERT_TRUE(getBatteryData(&receiver, &resultBattery));
	EXPECT_FALSE(receiver.speedQueue.empty());
	
	// Get speed data shouldn't affect battery queue
	t_speedData resultSpeed;
	ASSERT_TRUE(getSpeedData(&receiver, &resultSpeed));
	EXPECT_TRUE(receiver.batteryQueue.empty());
}

// Test concurrent access to both queues
TEST_F(ThreadSafeUtilsTest, ConcurrentMixedOperations) {
	std::atomic<int> speedReads{0};
	std::atomic<int> batteryReads{0};
	
	// Writer threads
	std::thread speedWriter([&]() {
		for (int i = 0; i < 50; i++) {
			std::lock_guard<std::mutex> lock(receiver.speedMutex);
			t_speedData data = {static_cast<uint16_t>(i), static_cast<uint16_t>(i * 2)};
			receiver.speedQueue.push(data);
		}
	});
	
	std::thread batteryWriter([&]() {
		for (int i = 0; i < 50; i++) {
			std::lock_guard<std::mutex> lock(receiver.batteryMutex);
			t_batteryData data = {static_cast<uint16_t>(12000 + i), static_cast<uint8_t>(i % 101)};
			receiver.batteryQueue.push(data);
		}
	});
	
	// Wait for writers to finish
	speedWriter.join();
	batteryWriter.join();
	
	// Reader threads
	std::thread speedReader([&]() {
		t_speedData data;
		while (getSpeedData(&receiver, &data)) {
			speedReads++;
		}
	});
	
	std::thread batteryReader([&]() {
		t_batteryData data;
		while (getBatteryData(&receiver, &data)) {
			batteryReads++;
		}
	});
	
	speedReader.join();
	batteryReader.join();
	
	EXPECT_EQ(speedReads.load(), 50);
	EXPECT_EQ(batteryReads.load(), 50);
}

// Test rapid alternating access
TEST_F(ThreadSafeUtilsTest, RapidAlternatingAccess) {
	for (int i = 0; i < 50; i++) {
		t_speedData speedData = {static_cast<uint16_t>(i), static_cast<uint16_t>(i * 2)};
		t_batteryData batteryData = {static_cast<uint16_t>(12000 + i), static_cast<uint8_t>(i % 101)};
		
		receiver.speedQueue.push(speedData);
		receiver.batteryQueue.push(batteryData);
	}
	
	// Alternate between reading speed and battery
	for (int i = 0; i < 50; i++) {
		t_speedData speed;
		t_batteryData battery;
		
		ASSERT_TRUE(getSpeedData(&receiver, &speed));
		ASSERT_TRUE(getBatteryData(&receiver, &battery));
		
		EXPECT_EQ(speed.rpm, i);
		EXPECT_EQ(battery.voltage, 12000 + i);
	}
	
	EXPECT_TRUE(receiver.speedQueue.empty());
	EXPECT_TRUE(receiver.batteryQueue.empty());
}

// Stress test with many concurrent operations
TEST_F(ThreadSafeUtilsTest, StressTestConcurrentOperations) {
	const int NUM_ITEMS = 1000;
	const int NUM_THREADS = 10;
	
	std::atomic<int> speedWrites{0};
	std::atomic<int> batteryWrites{0};
	std::atomic<int> speedReads{0};
	std::atomic<int> batteryReads{0};
	
	std::vector<std::thread> threads;
	
	// Speed writers
	for (int t = 0; t < NUM_THREADS; t++) {
		threads.emplace_back([&]() {
			for (int i = 0; i < NUM_ITEMS / NUM_THREADS; i++) {
				std::lock_guard<std::mutex> lock(receiver.speedMutex);
				t_speedData data = {static_cast<uint16_t>(i), static_cast<uint16_t>(i * 2)};
				receiver.speedQueue.push(data);
				speedWrites++;
			}
		});
	}
	
	// Battery writers
	for (int t = 0; t < NUM_THREADS; t++) {
		threads.emplace_back([&]() {
			for (int i = 0; i < NUM_ITEMS / NUM_THREADS; i++) {
				std::lock_guard<std::mutex> lock(receiver.batteryMutex);
				t_batteryData data = {static_cast<uint16_t>(12000 + i), static_cast<uint8_t>(i % 101)};
				receiver.batteryQueue.push(data);
				batteryWrites++;
			}
		});
	}
	
	// Wait for all writers
	for (auto& thread : threads) {
		thread.join();
	}
	threads.clear();
	
	// Speed readers
	for (int t = 0; t < NUM_THREADS; t++) {
		threads.emplace_back([&]() {
			t_speedData data;
			while (getSpeedData(&receiver, &data)) {
				speedReads++;
			}
		});
	}
	
	// Battery readers
	for (int t = 0; t < NUM_THREADS; t++) {
		threads.emplace_back([&]() {
			t_batteryData data;
			while (getBatteryData(&receiver, &data)) {
				batteryReads++;
			}
		});
	}
	
	for (auto& thread : threads) {
		thread.join();
	}
	
	EXPECT_EQ(speedWrites.load(), NUM_ITEMS);
	EXPECT_EQ(batteryWrites.load(), NUM_ITEMS);
	EXPECT_EQ(speedReads.load(), NUM_ITEMS);
	EXPECT_EQ(batteryReads.load(), NUM_ITEMS);
	EXPECT_TRUE(receiver.speedQueue.empty());
	EXPECT_TRUE(receiver.batteryQueue.empty());
}
