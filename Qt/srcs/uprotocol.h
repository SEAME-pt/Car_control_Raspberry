#pragma once
#include <cstdint>
#include <arpa/inet.h>
#include <cstring>

// Simple uProtocol framing
// Header: 1 byte type | 3 bytes reserved | 4 bytes payload_len (network order)
// Payload: bytes (message specific)

// Message types
static constexpr uint8_t MSG_SPEED = 1;

// Header size
static constexpr size_t UPROTO_HEADER_SIZE = 1 + 3 + 4;

#pragma pack(push,1)
struct UProtoHeader {
    uint8_t type;
    uint8_t reserved[3];
    uint32_t payload_len_be; // network byte order
};
#pragma pack(pop)

// helpers for 64-bit hton/ntoh
static inline uint64_t htonll(uint64_t v) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return (((uint64_t)htonl((uint32_t)(v & 0xffffffffULL))) << 32) |
           (uint64_t)htonl((uint32_t)(v >> 32));
#else
    return v;
#endif
}

static inline uint64_t ntohll(uint64_t v) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return (((uint64_t)ntohl((uint32_t)(v & 0xffffffffULL))) << 32) |
           (uint64_t)ntohl((uint32_t)(v >> 32));
#else
    return v;
#endif
}

static inline void pack_header(UProtoHeader &h, uint8_t type, uint32_t payload_len) {
    h.type = type;
    h.reserved[0] = h.reserved[1] = h.reserved[2] = 0;
    h.payload_len_be = htonl(payload_len);
}
static inline uint32_t header_payload_len(const UProtoHeader &h) {
    return ntohl(h.payload_len_be);
}