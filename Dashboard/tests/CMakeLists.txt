cmake_minimum_required(VERSION 3.16)
project(DashboardTests LANGUAGES CXX)

# Minimal, test-only CMakeLists. This file intentionally avoids touching
# global compile flags or building the main application.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Always build tests from this CMakeLists (test-only). Do not modify global build flags.

enable_testing()

# Find Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Prefer Qt6, fall back to Qt5
# Request all modules we may link so CMake reports missing modules early
find_package(Qt6 COMPONENTS Core Network Quick Gui Qml Test QUIET)
if(Qt6_FOUND)
    set(QT_LIBS
        Qt6::Core
        Qt6::Network
        Qt6::Quick
        Qt6::Gui
        Qt6::Qml
        Qt6::Test
    )
    set(CMAKE_AUTOMOC ON)
else()
    find_package(
        Qt5
        COMPONENTS Core Network Quick Gui Qml Test
        REQUIRED
    )
    set(QT_LIBS
        Qt5::Core
        Qt5::Network
        Qt5::Quick
        Qt5::Gui
        Qt5::Qml
        Qt5::Test
    )
    set(CMAKE_AUTOMOC ON)
endif()

# Ensure automoc can find headers included with relative paths and generated moc files
# are compiled into the test target
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Test source files (implementation parts to compile into the test binary)
# Add all implementation files (except application entry `main.cpp`) so coverage can see them
set(TEST_SOURCES
    ../srcs/CarDataController.cpp
    ../srcs/CoverArtProvider.cpp
    ../srcs/Dashboard.cpp
)

# Test files
set(TEST_FILES
    test_CarDataController.cpp
    test_ServerCommunication.cpp
	test_CoverArtProvider.cpp
	test_dashboard.cpp
)

# Create test executable
add_executable(tests ${TEST_SOURCES} ${TEST_FILES})

# Include project's public headers for tests
target_include_directories(
    tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../incs
)

# Ensure headers that contain Q_OBJECT are explicitly part of the target so AUTOMOC runs
# Add public headers for all sources we compile into tests
target_sources(
    tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../incs/CarDataController.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../incs/CoverArtProvider.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../incs/Dashboard.hpp
)
# Make sure AUTOMOC is enabled for this target
set_target_properties(tests PROPERTIES AUTOMOC ON)
# Define UNIT_TEST so test-only APIs are available
target_compile_definitions(tests PRIVATE UNIT_TEST=1)

# Link libraries for tests
target_link_libraries(tests PRIVATE GTest::GTest GTest::Main ${QT_LIBS} pthread)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(tests)

message(STATUS "Tests enabled")
message(STATUS "GTest found: ${GTest_FOUND}")

# Optional: generate code coverage report (requires lcov + genhtml)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(SKIP_HARDWARE_TESTS "Skip hardware tests during coverage run" ON)

if(ENABLE_COVERAGE)
    message(STATUS "Code coverage enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

    find_program(LCOV_EXEC lcov)
    find_program(GENHTML_EXEC genhtml)

    if(NOT LCOV_EXEC)
        message(WARNING "lcov not found: coverage target may fail")
    endif()
    if(NOT GENHTML_EXEC)
        message(WARNING "genhtml not found: coverage target may fail")
    endif()

    if(SKIP_HARDWARE_TESTS)
        set(SKIP_HW_VAL 1)
    else()
        set(SKIP_HW_VAL 0)
    endif()

    add_custom_target(
        coverage
        COMMAND env SKIP_HARDWARE_TESTS=${SKIP_HW_VAL} ${CMAKE_BINARY_DIR}/tests
        COMMAND ${LCOV_EXEC} --capture --directory . --output-file coverage.info
        COMMAND ${LCOV_EXEC} --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage_filtered.info
        COMMAND ${GENHTML_EXEC} coverage_filtered.info --output-directory coverage_html --quiet
        COMMAND ${CMAKE_COMMAND} -E echo "✅ Coverage report generated: coverage_html/index.html"
    	#COMMAND xdg-open coverage_html/index.html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS tests
        COMMENT "Generating coverage report..."
    )
endif()

