#include "../../include/socketCAN.h"

/*
struct sockaddr_can {
        sa_family_t can_family;
        int         can_ifindex;
        union {
                // transport protocol class address info (e.g. ISOTP)
                struct { canid_t rx_id, tx_id; } tp;

                // J1939 address information
                struct {
                        //8 byte name when using dynamic addressing
                        __u64 name;

                        //pgn:
                        ///8 bit: PS in PDU2 case, else 0
                        //8 bit: PF
                        //1 bit: DP
                        //1 bit: reserved
                        
                        __u32 pgn;

                        // 1 byte address
                        __u8 addr;
                } j1939;

                // reserved for future CAN protocols address information
        } can_addr;
};
*/

/*struct canfd_frame {
	canid_t can_id;  // 32 bit CAN_ID + EFF/RTR/ERR flags
	 __u8    len;     // frame payload length in byte (0 .. 64)
	__u8    flags;   // additional flags for CAN FD
	__u8    __res0;  // reserved / padding
	__u8    __res1;  // reserved / padding
	__u8    data[64] __attribute__((aligned(8)));
};

/*
In this case the driver flag IFF_ECHO has to be set to prevent the PF_CAN core from locally echoing sent frames 
dev->flags = (IFF_NOARP | IFF_ECHO);

SIOCGIFINDEX -> refers to network interface index
*/

int main(void) {

    struct sockaddr_can	addr;
    struct ifreq		ifr;
	t_mytxmsg			mytxmsg = {0};
    int 				s;

	//Socket creation
	s = socket(PF_CAN, SOCK_DGRAM, CAN_BCM);
	if (s < 0) {
		perror("socket");
		return (1);
	}

	strcpy(ifr.ifr_name, "vcan0");

	// Get MTU
	if (ioctl(s, SIOCGIFMTU, &ifr) < 0) {
        perror("ioctl SIOCGIFMTU");
		close(s);
        return (1);
    }

	// Check CAN FD support
	if (ifr.ifr_mtu == CANFD_MTU)
		printf("Device supports CAN FD\n");
	else if (ifr.ifr_mtu == CAN_MTU)
		printf("Device only supports Classical CAN\n");
	else
		printf("Unkown MTU: %d\n", ifr.ifr_mtu);

	// Get interface index
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl SIOCGIFINDEX");
		close(s);
        return (1);
    }

	//setup adrees
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	//connect to can interface
	if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("connect");
		close(s);
		return (1);
	}

	printf("Socket connected to vcan0\n");

	//setup BCM message
	mytxmsg.msg_head.opcode  = TX_SETUP;
	mytxmsg.msg_head.flags   = SETTIMER | STARTTIMER | TX_CP_CAN_ID; //| CAN_FD_FRAME;
	mytxmsg.msg_head.ival1.tv_sec  = 0;
    mytxmsg.msg_head.ival1.tv_usec = 100000;
	mytxmsg.msg_head.can_id  = 0x100;
	mytxmsg.msg_head.nframes = 1;

	// Set frame data
    mytxmsg.frame[0].can_id = 0x100;
    mytxmsg.frame[0].len = 8;
    mytxmsg.frame[0].flags = 0; //CANFD_BRS;

	uint64_t data = 0xFF00000000000000ULL;
	data = htobe64(data);
	memcpy(mytxmsg.frame[0].data, &data, 8);

	size_t msg_size = sizeof(bcm_msg_head) + (mytxmsg.msg_head.nframes * sizeof(struct can_frame));

	// Send BCM configuration
	if (write(s, &mytxmsg, msg_size) < 0) {
		perror("write BCM message");
		close(s);
		return (1);
	}

	printf("BCM message configured - sending every 100ms\n");
    printf("Press Ctrl+C to stop\n");

	sleep(10);

	mytxmsg.msg_head.opcode = TX_DELETE;
	mytxmsg.msg_head.flags = 0;

    write(s, &mytxmsg, sizeof(bcm_msg_head));

	printf("Transmission stopped\n");

	close(s);
	return 0;

}
