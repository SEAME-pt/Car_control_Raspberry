#ifndef SOCKETCAN_H
#define SOCKETCAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <poll.h>

#include <net/if.h>

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>


/**
 * @file socketCAN.h
 * @brief SocketCAN interface for CAN / CAN-FD communication
 */

/**
 * @brief Checks if the interface supports the required MTU
 * 
 * Determines if the interface supports classical CAN, CAN-FD or other unkown.
 * 
 * @param s CAN socket
 * @param ifr Pointer to the interface request struct
 * @return 0 if supported, -1 if not supported or error
 */
int		check_mtu_support(int s, struct ifreq *ifr);

/**
 * @brief Initializes a SocketCAN interface
 *
 * Creates a raw CAN socket, binds it to the interface, and enables CAN-FD if supported.
 *
 * @param interface Name of the CAN interface (e.g., "can0")
 * @return Socket file descriptor on success, -1 on failure
 */
int		socketCan_init(const char *interface);

/**
 * @brief Sends a standard CAN frame (8 bytes max)
 *
 * @param socket Socket returned by socketCan_init
 * @param can_id 11-bit CAN identifier
 * @param data Pointer to data buffer
 * @param len Number of bytes to send (max 8)
 * @return 0 if successful, -1 on error
 */
int		can_send_frame(int socket, uint16_t can_id, 
					const int16_t* data, uint8_t len);

/**
 * @brief Sends a CAN-FD frame (up to 64 bytes)
 *
 * @param socket Socket returned by socketCan_init
 * @param can_id 11-bit CAN identifier
 * @param data Pointer to data buffer
 * @param len Number of bytes to send (max 64)
 * @return 0 if successful, -1 on error
 */
int		can_send_frame_fd(int socket, uint16_t can_id, 
					const int16_t *data, uint8_t len);

/**
 * @brief Attempts to receive a standard CAN frame (non-blocking)
 *
 * @param socket CAN socket
 * @param frame Pointer to can_frame struct to store received data
 * @return 0 if a frame was read, -1 if no data or error
 */
int		can_try_receive(int socket, struct can_frame *frame);

/**
 * @brief Attempts to receive a CAN-FD frame (non-blocking)
 *
 * @param socket CAN socket
 * @param frame Pointer to canfd_frame struct to store received data
 * @return 0 if a frame was read, -1 if no data or error
 */
int		canfd_try_receive(int socket, struct canfd_frame *frame);

/**
 * @brief Closes a CAN socket
 *
 * @param socket Socket returned by socketCan_init
 */
void	can_close(int socket);

#ifdef __cplusplus
}
#endif

#endif

/*
struct canfd_frame {
	canid_t can_id;  // 32 bit CAN_ID + EFF/RTR/ERR flags
	 __u8    len;     // frame payload length in byte (0 .. 64)
	__u8    flags;   // additional flags for CAN FD
	__u8    __res0;  // reserved / padding
	__u8    __res1;  // reserved / padding
	__u8    data[64] __attribute__((aligned(8)));
};
*/