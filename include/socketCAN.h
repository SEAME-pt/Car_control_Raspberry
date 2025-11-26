#ifndef SOCKETCAN_H
#define SOCKETCAN_H

#include <unistd.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/bcm.h>
#include <linux/can/error.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#define U64_DATA(p) (*(unsigned long long*)(p)->data)

// Broadcast Manager
typedef struct s_bcm_msg_head {
	__u32				opcode;         // command 
	__u32				flags;          // special flags 
	__u32				count;          // run 'count' times with ival1 
	struct timeval		ival1, ival2;   // count and subsequent interval 
	canid_t				can_id;         // unique can_id for task 
	__u32				nframes;        // number of can_frames following
	struct canfd_frame	frames[];

} bcm_msg_head;

typedef struct s_mytxmsg {
	bcm_msg_head		msg_head;
	struct canfd_frame	frame[1];

} t_mytxmsg;

#endif