#include "socketCAN.h"
#include <sys/time.h>

int	check_mtu_support(int s, struct ifreq *ifr) {

	// Get MTU
	if (ioctl(s, SIOCGIFMTU, ifr) < 0) {
		perror("ioctl SIOCGIFMTU");
		return (-1);
	}

	// Check CAN_FD support
	if (ifr->ifr_mtu == CANFD_MTU)
		printf("Device supports CAN_FD.\n");
	else if (ifr->ifr_mtu == CAN_MTU)
		printf("Device supports Classical CAN.\n");
	else {
		printf("ERROR! Unkown MTU suported by device...");
		return (-1);
	}
	return (0);
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
int	can_send_frame(int socket, uint16_t can_id, 
		const int8_t* data, uint8_t len) {

	struct can_frame	frame;

	// Validate standard CAN ID (11-bit: 0x000 - 0x7FF)
	if (can_id > 0x7FF) {
		fprintf(stderr, "Invalid CAN ID: 0x%X (too large)\n", can_id);
		return (-1);
	}

	memset(&frame, 0, sizeof(frame));
	if (len > 8) 
		len = 8;

	if (data && len > 0)
		memcpy(frame.data, data, len);

	frame.len = len;
	frame.can_id = can_id;

	if (can_id == 0x100) {  // Only log emergency brake
    	struct timeval tv;
    	gettimeofday(&tv, NULL);
    	long long t_write = tv.tv_sec * 1000000LL + tv.tv_usec;
    
    	FILE* log = fopen("Car_control_Raspberry/latency_test.log", "a");
    	if (log) {
        	fprintf(log, "CAN_WRITE,0x%X,%lld\n", can_id, t_write);
        	fclose(log);
    	}
	}

	if (write(socket, &frame, sizeof(struct can_frame)) < 0) {
		perror("write CAN frame");
		return (-1);
	}
	return (0);
}

// CAN_FD (64 bytes)
int	can_send_frame_fd(int socket, uint16_t can_id, 
					  const int16_t *data, uint8_t len) {

	struct canfd_frame	frame;

	// Validate standard CAN ID (11-bit: 0x000 - 0x7FF)
	if (can_id > 0x7FF) {
		fprintf(stderr, "Invalid CAN ID: 0x%X (too large)\n", can_id);
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

// Reads incoming messages and stores all information on struct
// -1 returned if no message was read
int	can_try_receive(int socket, struct can_frame *frame)
{
	struct pollfd	pfd;

	pfd.fd 		= socket;
	pfd.events	= POLLIN;

	if (poll(&pfd, 1, 0) <= 0)
		return (-1);

	if (read(socket, frame, sizeof(*frame)) < 0)
		return (-1);
	
	return (0);
}

// Same as previous function but for can-fd
int	canfd_try_receive(int socket, struct canfd_frame *frame) {

	struct pollfd	pfd;
	pfd.fd 		= 	socket;
	pfd.events	= 	POLLIN;

	if (poll(&pfd, 1, 0) <= 0)
		return (-1);

	if (read(socket, frame, sizeof(*frame)) < 0)
		return (-1);
	
	return (0);
}

// Close Socket
void	can_close(int socket) {

	if (socket >= 0)
		close(socket);
}
