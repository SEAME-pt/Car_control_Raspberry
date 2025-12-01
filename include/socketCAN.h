#ifndef SOCKETCAN_H
#define SOCKETCAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int		socketCan_init(const char *interface);

int		can_send_frame(int socket, uint32_t can_id, 
					const int8_t *data, uint8_t len);

int		can_send_frame_fd(int socket, uint32_t can_id, 
					const int8_t *data, uint8_t len);

void	can_close(int socket);

#ifdef __cplusplus
}
#endif

#endif

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