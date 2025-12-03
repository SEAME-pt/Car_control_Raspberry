#include <gtest/gtest.h>

#include "CANProtocol.hpp"
#include "carControl.h"
#include "socketCAN.h"
#include "exceptions.hpp"

int main(int argc, char **argv) {

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}