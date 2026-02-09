#include <gtest/gtest.h>
#include <CANProtocol.hpp>

/********************************/
/*		CAN PROTOCOL			*/
/********************************/

class CANProtocolTest : public ::testing::Test {
protected:
    const std::string validInterface = "vcan0";

    void SetUp() override {
        system("sudo ip link add dev vcan0 type vcan");
        system("sudo ip link set vcan0 mtu 72");
        system("sudo ip link set up vcan0");
    }

    void TearDown() override {
        system("sudo ip link delete vcan0 2>/dev/null");
    }
};

