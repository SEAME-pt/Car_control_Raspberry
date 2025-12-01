#include "../../include/socketCAN.h"

static int	check_mtu_support(int s, struct ifreq *ifr) {

	// Get MTU
	if (ioctl(s, SIOCGIFMTU, ifr) < 0) {
		perror("ioctl SIOCGIFMTU");
		return (-1);
	}

	// Check CAN FD support
	if (ifr->ifr_mtu == CANFD_MTU) {
		printf("Device supports CAN FD\n");
		return (0);
	}
	else if (ifr->ifr_mtu == CAN_MTU)
		printf("Device only supports Classical CAN\n");
	else
		printf("Unkown MTU: %d\n", ifr->ifr_mtu);
	return (1);
}

int	socketCan_init(const char *interface) {

	struct sockaddr_can	addr;
	struct ifreq		ifr;
	int 				s;
	int enable_canfd = 1;

	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0) {
		perror("socket");
		return (-1);
	}

	strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	// // Check MTU
	if (check_mtu_support(s, &ifr) < 0) {
		close(s);
		return (-1);
	}

	if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, 
					&enable_canfd, sizeof(enable_canfd)) < 0) {
        perror("setsockopt CAN_RAW_FD_FRAMES");
        close(s);
        return -1;
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

	//bind to interface
	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
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
	memset(&frame, 0, sizeof(frame));

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

void	can_close(int socket) {

	if (socket >= 0)
		close(socket);
}
