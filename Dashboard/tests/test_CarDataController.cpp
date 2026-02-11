#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QtMath>
#include "../incs/CarDataController.hpp"

// Data structures for temperature conversion tests
struct TempCase { double inputC; double expectedF; };
class TemperatureTest : public ::testing::TestWithParam<TempCase> { };

// Data structures for distance conversion tests
struct DistanceCase { double inputKm; double expectedMiles; };
class DistanceTest : public ::testing::TestWithParam<DistanceCase> { };

// Data structure for basic int setter tests
struct IntCase { int input; int expected; };
class IntPropertyTest : public ::testing::TestWithParam<IntCase> { };

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


// Check default property values and unit defaults
TEST(CarDataControllerTest, DefaultValues) {
    CarDataController ctrl;
	ctrl.setDistanceUnit("Km");
	ctrl.setTemperatureUnit("Celsius");
    EXPECT_EQ(ctrl.speed(), 0);
    EXPECT_EQ(ctrl.speedLimit(), 120);
	EXPECT_EQ(ctrl.totalDistance(), 0.0);
	EXPECT_EQ(ctrl.distanceUnit(), "Km");
	EXPECT_EQ(ctrl.temperatureUnit(), "Celsius");
	EXPECT_EQ(ctrl.batteryLevel(), 100);
    EXPECT_DOUBLE_EQ(ctrl.temperature(), 20.0);
}

TEST_P(DistanceTest, DistanceUnitMilesConvertsTotalDistance) {
	auto p = GetParam();
	CarDataController ctrl;
	// set distance in km (test-only setter)
	ctrl.testSetDistance(p.inputKm);
	ctrl.setDistanceUnit("miles");
	EXPECT_NEAR(ctrl.totalDistance(), p.expectedMiles, 0.01);
}

// Verify total distance conversion (Km -> Miles) behaves as expected
TEST_P(DistanceTest, DistanceUnitMilesConvertsSpeed) {
	auto p = GetParam();
	CarDataController ctrl;
	// set distance in km (test-only setter)
	ctrl.testSetSpeed(int(qRound(p.inputKm)));
	ctrl.setDistanceUnit("miles");
	EXPECT_NEAR(ctrl.speed(), qRound(p.expectedMiles), 1);
}

// Verify speed conversion (Km/h -> Miles/h) with rounding
TEST_P(DistanceTest, DistanceUnitMilesConvertsSpeedLimit) {
	auto p = GetParam();
	CarDataController ctrl;
	// set distance in km (test-only setter)
	ctrl.testSetSpeedLimit(int(qRound(p.inputKm)));
	ctrl.setDistanceUnit("miles");
	EXPECT_NEAR(ctrl.speedLimit(), qRound(p.expectedMiles), 1);
}

// Verify speed limit conversion (Km/h -> Miles/h) for representative inputs
INSTANTIATE_TEST_SUITE_P (
	BasicDistances,
	DistanceTest,
	::testing::Values(
		DistanceCase{0.0, 0.0},
		DistanceCase{1.0, 0.621371},
		DistanceCase{10.0, 6.21371},
		DistanceCase{50.0, 31.0686},
		DistanceCase{80.0, 49.7097},
		DistanceCase{100.0, 62.1371},
		DistanceCase{123.45, 76.7127}
	)
);

TEST_P(TemperatureTest, ConvertsCorrectly) {
    auto p = GetParam();
    CarDataController ctrl;
    // if you need to set internal temperature value, add a test-only setter or subclass
	ctrl.testSetTemperature(p.inputC); // directly set internal value for testing
    ctrl.setTemperatureUnit("fahrenheit");
    EXPECT_DOUBLE_EQ(ctrl.temperature(), p.expectedF);
}

INSTANTIATE_TEST_SUITE_P(
    BasicTemps,
    TemperatureTest,
    ::testing::Values(
        TempCase{20.0, 68.0},
        TempCase{0.0, 32.0},
        TempCase{-40.0, -40.0},
        TempCase{100.0, 212.0},
        TempCase{36.6, 97.88}
    )
);
// Verify battery level update via test setter
TEST_P(IntPropertyTest, BatteryLevelUpdates)
{
	auto p = GetParam();
	CarDataController ctrl;
	ctrl.updateTestBatteryLevel(p.input);
	EXPECT_EQ(ctrl.batteryLevel(), p.expected);
}

// Verify battery range update via test setter
TEST_P(IntPropertyTest, TestBatteryRangeUpdates)
{
	auto p = GetParam();
	CarDataController ctrl;
	ctrl.updateTestBatteryRange(p.input);
	EXPECT_EQ(ctrl.batteryRange(), p.expected);
} 

// Verify motor power update via test setter
TEST_P(IntPropertyTest, TestMotorPowerUpdates)
{
	auto p = GetParam();
	CarDataController ctrl;
	ctrl.updateTestMotorPower(p.input);
	EXPECT_EQ(ctrl.motorPower(), p.expected);
} 

INSTANTIATE_TEST_SUITE_P(
	IntCases,
	IntPropertyTest,
	::testing::Values(
		IntCase{100, 100},
		IntCase{75, 75},
		IntCase{50, 50},
		IntCase{25, 25},
		IntCase{0, 0},
		IntCase{-10, -10}, // Test edge case of negative value
		IntCase{110, 110}  // Test edge case of value above 100
	)
);
// Verify motor active flag toggles correctly
TEST(CarDataControllerTest, MotorActiveUpdates)
{
	CarDataController ctrl;
	ctrl.updateTestMotorActive(true);
	EXPECT_TRUE(ctrl.motorActive());
	ctrl.updateTestMotorActive(false);
	EXPECT_FALSE(ctrl.motorActive());
}

// Verify showError flag toggles correctly
TEST(CarDataControllerTest, ErrorShowUpdates)
{
	CarDataController ctrl;
	ctrl.updateTestShowError(true);
	EXPECT_TRUE(ctrl.showError());
	ctrl.updateTestShowError(false);
	EXPECT_FALSE(ctrl.showError());
} 

// Verify errorMessage is updated and cleared
TEST(CarDataControllerTest, ErrorMessageUpdates)
{
	CarDataController ctrl;
	ctrl.updateTestErrorMessage("Test error");
	EXPECT_EQ(ctrl.errorMessage(), "Test error");
	ctrl.updateTestErrorMessage("");
	EXPECT_EQ(ctrl.errorMessage(), "");
} 

// Verify dismissError clears the error state and message
TEST(CarDataControllerTest, DismissErrorResetsState)
{
	CarDataController ctrl;
	ctrl.updateTestShowError(true);
	ctrl.updateTestErrorMessage("Test error");
	ctrl.dismissError();
	EXPECT_FALSE(ctrl.showError());
	EXPECT_EQ(ctrl.errorMessage(), "");
}
