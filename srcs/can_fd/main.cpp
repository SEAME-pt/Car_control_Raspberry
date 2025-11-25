#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>

int main(void) {
	
	//test
	int	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
}
