#include "../../include/socketCAN.h"

static int	check_mtu_support(int s, struct ifreq *ifr) {

	// Get MTU
	if (ioctl(s, SIOCGIFMTU, &ifr) < 0) {
		perror("ioctl SIOCGIFMTU");
		return (-1);
	}

	// Check CAN FD support
	if (ifr.ifr_mtu == CANFD_MTU) {
		printf("Device supports CAN FD\n");
		return (0);
	}
	else if (ifr.ifr_mtu == CAN_MTU)
		printf("Device only supports Classical CAN\n");
	else
		printf("Unkown MTU: %d\n", ifr.ifr_mtu);
	return (1);
}

int	socketCan_init(const char *interface) {

	struct sockaddr_can	addr;
	struct ifreq		ifr;
	int 				s;

	if (!interface)
		interface = "can0";

	s = socket(PF_CAN, SOCK_DGRAM, CAN_BCM);
	if (s < 0) {
		perror("socket");
		return (-1);
	}

	strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	// Verificar suporte CAN FD
	if (check_mtu_support(s, &ifr) < 0) {
		close(s);
		return (-1);
	}

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

// Classical CAN Bus (8 bytes)
int	can_send_frame(int socket, uint32_t can_id, 
		const int8_t *data, uint8_t len) {

	struct can_frame frame;
	memset(&frame, 0, sizeof(frame));
	if (len > 8) len = 8;

	frame.can_id = can_id;
	frame.can_dlc = len;

	if (data && len > 0)
		memcpy(frame.data, data, len);

	if (write(socket, &frame, sizeof(struct can_frame)) < 0) {
		perror("write CAN frame");
		return (-1);
	}
	return (0);
}

// CAN FD (64 bytes)
int	can_send_frame_fd(int socket, uint32_t can_id, 
					  const int8_t *data, uint8_t len) {

	struct canfd_frame frame;

	if (len > 64) len = 64;

	frame.can_id = can_id;
	frame.len = len;
	frame.flags = CANFD_BRS;  // Bit Rate Switch
	
	if (data && len > 0)
		memcpy(frame.data, data, len);

	if (write(socket, &frame, sizeof(struct canfd_frame)) < 0) {
		perror("write CAN FD frame");
		return (-1);
	}
	return (0);
}

int	can_bcm_send(int s, uint32_t can_id, const int8_t *data, 
	uint8_t len, uint32_t interval_us) {
	
	t_mytxmsg	msg = {0};
	size_t		msg_size;

	if (len > 64) len = 64;

	//setup BCM message
	msg.msg_head.opcode  = TX_SETUP;
	msg.msg_head.flags   = SETTIMER | STARTTIMER | TX_CP_CAN_ID | CAN_FD_FRAME;
	msg.msg_head.ival1.tv_sec  = interval_us / 1000000;
	msg.msg_head.ival1.tv_usec = interval_us % 1000000;
	msg.msg_head.can_id  = can_id;
	msg.msg_head.nframes = 1;

	msg.frame[0].can_id = can_id;
	msg.frame[0].len = len;
	msg.frame[0].flags = CANFD_BRS;

	if (data && len > 0)
		memcpy(msg.frame[0].data, data, len);

	msg_size = sizeof(bcm_msg_head) + sizeof(struct canfd_frame);

	// Send BCM configuration
	if (write(s, &msg, msg_size) < 0) {
		perror("write BCM message");
		return (-1);
	}
	printf("BCM transmission started: ID=0x%03X, interval=%uµs, len=%u bytes (CAN FD)\n",
           can_id, interval_us, len);
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
	printf("BCM transmission stopped: ID=0x%03X\n", can_id);
	return (0);
}

void	can_close(int socket) {

	if (socket >= 0)
		close(socket);
}
