#include "../../include/socketCAN.h"

static	check_mtu_support(int s, struct ifreq ifr) {

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
}

int	socketCan_init(const char *interface) {

	struct sockaddr_can	addr;
    struct ifreq		ifr;
    int 				s;

	s = socket(PF_CAN, SOCK_DGRAM, CAN_BCM);
    if (s < 0) {
		perror("socket");
		return (-1);
	}

	strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	check_mtu_support(s, ifr);

	// Get interface index
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl SIOCGIFINDEX");
		close(s);
        return (-1);
    }

	//setup adrees
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	//connect to can interface
	if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("connect");
		close(s);
		return (-1);
	}

	return (s);
}

int	can_bcm_send(int s, uint32_t can_id, const uint64_t *data, 
	uint64_t len, uint32_t interval_us) {
	
	t_mytxmsg	msg = {0};
	size_t		msg_size;

	//setup BCM message
	msg.msg_head.opcode  = TX_SETUP;
	msg.msg_head.flags   = SETTIMER | STARTTIMER | TX_CP_CAN_ID; //| CAN_FD_FRAME;
	msg.msg_head.ival1.tv_sec  = interval_us / 1000000;
    msg.msg_head.ival1.tv_usec = interval_us % 1000000;
	msg.msg_head.can_id  = can_id;
	msg.msg_head.nframes = 1;

	msg.frame[0].can_id = can_id;
	msg.frame[0].len = len;
	memcpy(msg.frame[0].data, data, len);

	msg_size = sizeof(bcm_msg_head) + sizeof(struct can_frame);

		// Send BCM configuration
	if (write(s, &msg, msg_size) < 0) {
		perror("write BCM message");
		return (-1);
	}
	return (0);
}

int	can_stop(int s, uint32_t can_id) {

	t_mytxmsg msg = {0};

	msg.msg_head.opcode = TX_DELETE;
	msg.msg_head.can_id = can_id;

	if (write(s, &msg, sizeof(bcm_msg_head)) < 0) {
		perror("write TX_DELETE");
		return (-1);
	}
	return (0);
}

void	can_close(int socket) {

	if (socket >= 0) {
		close(socket);
	}
}