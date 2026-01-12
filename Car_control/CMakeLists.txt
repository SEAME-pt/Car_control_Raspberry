cmake_minimum_required(VERSION 3.16)
project(car)

# Standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -Wextra -Werror")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find LIBEVDEV
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
	${LIBEVDEV_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    srcs/main.cpp
	#utils
	srcs/utils/parsing.cpp
	srcs/utils/signal.cpp
	srcs/utils/temp_print_can_rx.cpp
	#init
	srcs/init/init_can.cpp
	srcs/init/init.cpp
	#core
	srcs/core/autonomous_mode.cpp
	srcs/core/manual_mode.cpp
	#controller
    srcs/controller/Joystick.cpp
	#can
    srcs/can/socketCAN.c
    srcs/can/CANController.cpp
)

# Executable
add_executable(car ${SOURCES})

# Link libraries
target_link_libraries(car PRIVATE ${LIBEVDEV_LIBRARIES})

# Summary
message(STATUS "================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Libevdev found: ${LIBEVDEV_FOUND}")
message(STATUS "Libevdev version: ${LIBEVDEV_VERSION}")
message(STATUS "Libevdev libraries: ${LIBEVDEV_LIBRARIES}")
message(STATUS "Libevdev include: ${LIBEVDEV_INCLUDE_DIRS}")
message(STATUS "================================")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## ================================
## Testing with Google Test
## ================================
#option(BUILD_TESTS "Build the tests" ON)
#
#if(BUILD_TESTS)
#    enable_testing()
#
#    # Find Google Test
#    find_package(GTest REQUIRED)
#    include_directories(${GTEST_INCLUDE_DIRS})
#
#    # Test source files (excluding main.cpp from main project)
#    set(TEST_SOURCES
#        srcs/utils/parsing.cpp
#        srcs/init/init_can.cpp
#        srcs/init/init.cpp
#        srcs/controller/Joystick.cpp
#        srcs/can/socketCAN.c
#        srcs/can/CANController.cpp
#    )
#
#    # Test files
#    set(TEST_FILES
#		tests/CANControllerTest.cpp
#		tests/CANInitTest.cpp
#		tests/CANProtocolTest.cpp
#		tests/SocketCANTest.cpp
#		#tests/JoystickTest.cpp
#    )
#
#    # Create test executable
#    add_executable(tests ${TEST_SOURCES} ${TEST_FILES})
#
#    # Link libraries for tests
#    target_link_libraries(tests 
#        PRIVATE 
#		${LIBEVDEV_LIBRARIES}
#        GTest::GTest
#        GTest::Main
#        pthread
#    )
#
#    # Add tests to CTest
#    include(GoogleTest)
#    gtest_discover_tests(tests)
#
#    message(STATUS "Tests enabled")
#    message(STATUS "GTest found: ${GTest_FOUND}")
#endif()
#
## ================================
## Code Coverage
## ================================
## option(ENABLE_COVERAGE "Enable code coverage" OFF)
#
#if(ENABLE_COVERAGE)
#    message(STATUS "Code coverage enabled")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
#
#    # Optional: Add coverage target
#    add_custom_target(coverage
#        COMMAND ${CMAKE_BINARY_DIR}/tests
#        COMMAND lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch,inconsistent 2>/dev/null
#        COMMAND lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage_filtered.info --ignore-errors mismatch,unused,inconsistent 2>/dev/null
#        COMMAND genhtml coverage_filtered.info --output-directory coverage_html --quiet
#        COMMAND echo "✅ Coverage report generated: coverage_html/index.html"
#        COMMAND xdg-open coverage_html/index.html
#        COMMAND stty sane
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#        DEPENDS tests
#        COMMENT "Generating coverage report..."
#    )
#endif()
#