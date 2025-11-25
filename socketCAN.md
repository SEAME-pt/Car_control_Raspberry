# Overview

## What's SocketCAN

SocketCAN is an implementation of the CAN protocol (Controller Area Network) for Linux. CAN is a widely used networking technology in automation, embedded systems, and automotive applications.

SocketCAN leverages the Berkeley socket API and the Linux networking stack, implementing CAN device drivers as standard network interfaces. Its socket API is designed to closely resemble TCP/IP, making it easy for programmers familiar with network programming to work with CAN sockets.

By integrating CAN into the Linux networking stack, CAN interfaces appear as regular network devices. Applications can use standard socket functions to send and receive CAN frames. The framework provides features such as queuing, filtering, and multi-process support, and allows transport protocols like ISO-TP to integrate seamlessly.

Overall, SocketCAN offers a unified, flexible, hardware-independent, and kernel-native method to use CAN on Linux, providing a significant improvement over traditional character-device-based drivers.

## SocketCAN Concept

SocketCAN provides a socket-based API for CAN communication using the Linux networking stack. Unlike Ethernet, CAN is a broadcast-only bus, so the CAN ID works like a source identifier, and must be unique per sender.

To allow multiple applications to receive the same CAN IDs, SocketCAN implements efficient receive lists. Applications subscribe to specific CAN IDs (or ranges), and the core handles filtering and routing of frames.

SocketCAN additionally supports network problem notifications. CAN drivers can generate Error Message Frames when physical or bus-level errors occur. Applications can subscribe to these error frames via filters to monitor issues such as arbitration errors, transceiver faults, or error frames. Reception of error frames is disabled by default.

## How to Use SocketCAN

SocketCAN uses the Linux socket API to communicate over CAN networks.
You open a CAN socket just like TCP/UDP, but using the PF_CAN protocol family:

Raw CAN frames:
```shell
socket(PF_CAN, SOCK_RAW, CAN_RAW)
```

Broadcast Manager (BCM):
```shell
socket(PF_CAN, SOCK_DGRAM, CAN_BCM)
```

After creating the socket, you must bind it to a CAN interface (e.g., can0) using bind().
To find the interface index, use ioctl(SIOCGIFINDEX) on an ifreq structure.

Once bound, you can read and write CAN frames using standard system calls (read, write, sendto, recvfrom, etc.).

### Source

For the full and extended documentation and more complete instrunctions on how to use it, please refer to the this link:

https://docs.kernel.org/networking/can.html