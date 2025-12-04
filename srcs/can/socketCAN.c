#include "../../include/socketCAN.h"

int	check_mtu_support(int s, struct ifreq *ifr) {

	// Get MTU
	if (ioctl(s, SIOCGIFMTU, ifr) < 0) {
		perror("ioctl SIOCGIFMTU");
		return (-1);
	}

	// Check CAN_FD support
	if (ifr->ifr_mtu == CANFD_MTU) {
		printf("Device supports CAN FD\n");
		return (0);
	}
	else if (ifr->ifr_mtu == CAN_MTU)
		printf("Device only supports Classical CAN\n");
	else
		printf("Unknown MTU: %d\n", ifr->ifr_mtu);
	return (1);
}

int	socketCan_init(const char *interface) {

	struct sockaddr_can	addr;
	struct ifreq		ifr;
	int 				s;
	int 				enable_canfd = 1;

	if (!interface)
		return (-1);

	// SocketCAN initialization
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0) {
		perror("socket");
		return (-1);
	}

	// Assign interface name to ifr struct, which acts as a bridge to the kernel
	strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	// // Check MTU
	if (check_mtu_support(s, &ifr) < 0) {
		close(s);
		return (-1);
	}

	// Informs Kernel to accept CAN_FD and that allows on the same
	// socket send & receive CAN frames
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

	// Set up address
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	//bind socket to interface
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

	// Validate standard CAN ID (11-bit: 0x000 - 0x7FF)
	if (can_id > 0x7FF) {
		fprintf(stderr, "Invalid CAN ID: 0x%X (to large)\n", can_id);
		return (-1);
	}

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

// CAN_FD (64 bytes)
int	can_send_frame_fd(int socket, uint32_t can_id, 
					  const int8_t *data, uint8_t len) {

	struct canfd_frame frame;

	// Validate standard CAN ID (11-bit: 0x000 - 0x7FF)
	if (can_id > 0x7FF) {
		fprintf(stderr, "Invalid CAN ID: 0x%X (to large)\n", can_id);
		return (-1);
	}

	memset(&frame, 0, sizeof(frame));

	if (len > 64) len = 64;

	frame.can_id = can_id;
	frame.len = len;
	frame.flags = CANFD_BRS;  // Faster Bit Rate Switch
	
	if (data && len > 0)
		memcpy(frame.data, data, len);

	if (write(socket, &frame, sizeof(struct canfd_frame)) < 0) {
		perror("write CAN FD frame");
		return (-1);
	}
	return (0);
}

// Close Socket
void	can_close(int socket) {

	if (socket >= 0)
		close(socket);
}
