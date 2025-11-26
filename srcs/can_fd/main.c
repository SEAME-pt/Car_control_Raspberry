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

(..)
mytxmsg.msg_head.nframes = 4; */

/*
In this case the driver flag IFF_ECHO has to be set to prevent the PF_CAN core from locally echoing sent frames 
dev->flags = (IFF_NOARP | IFF_ECHO);

SIOCGIFINDEX -> refers to network interface index
*/

//create a struct to set up a sequence of four CAN frames

int main(void) {

	struct canfd_frame	cfd;
    struct sockaddr_can	addr;
    struct ifreq		ifr;
	bcm_msg_head		msg;
	t_mytxmsg			mytxmsg;
    int 				s;

	mytxmsg.msg_head = msg;
	mytxmsg.frame[0] = cfd;

	msg.opcode  = RX_SETUP;
	msg.flags   = CAN_FD_FRAME;
	msg.can_id  = 0x100;
	msg.nframes = 1;

	U64_DATA(mytxmsg.frame) = 0xFF00000000000000ULL;

	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0) {
		perror("socket");
		return (1);
	}

	strcpy(ifr.ifr_name, "can0");

	// Get MTU
	if (ioctl(s, SIOCGIFMTU, &ifr) < 0) {
        perror("ioctl SIOCGIFMTU");
		close(s);
        return (1);
    }

	// Get interface index
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl SIOCGIFINDEX");
		close(s);
        return (1);
    }

	if (ifr.ifr_mtu == CANFD_MTU)
		printf("Device supports CAN FD\n");
	else if (ifr.ifr_mtu == CAN_MTU)
		printf("Device only supports Classical CAN\n");

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("connect");
		close(s);
		return (1);
	}

	printf("Socket connected to can0\n");

    // Close socket when done
    close(s);
    return 0;

}
