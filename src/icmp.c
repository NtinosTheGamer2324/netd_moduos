#define LIBC_NO_START
#include "./libc.h"
#include "netd.h"

void icmp_rx(uint32_t src, const uint8_t *data, int len) {
    if (len < 8) return;
    uint8_t type = data[0];
    uint16_t id  = ntohs(*(uint16_t*)(data+4));
    uint16_t seq = ntohs(*(uint16_t*)(data+6));

    if (type == ICMP_ECHO_REQ) {
        /* reply with same payload */
        uint8_t reply[1500];
        if (len > (int)sizeof(reply)) len = sizeof(reply);
        memcpy(reply, data, len);
        reply[0] = ICMP_ECHO_REP;
        reply[1] = 0;
        *(uint16_t*)(reply+2) = 0;
        *(uint16_t*)(reply+2) = htons(inet_cksum(reply, len));
        ip_tx(src, IPPROTO_ICMP, reply, (uint16_t)len);
        return;
    }

    if (type == ICMP_ECHO_REP) {
        if (id == g_net.ping_id && seq == g_net.ping_seq) {
            g_net.ping_reply_ip = src;
            g_net.ping_replied  = 1;
        }
        return;
    }
}

int icmp_ping(uint32_t dst, uint16_t id, uint16_t seq) {
    uint8_t pkt[16];
    memset(pkt, 0, sizeof(pkt));
    pkt[0] = ICMP_ECHO_REQ;
    pkt[1] = 0;
    *(uint16_t*)(pkt+4) = htons(id);
    *(uint16_t*)(pkt+6) = htons(seq);
    /* 8 bytes payload "NETDPING" */
    memcpy(pkt+8, "NETDPING", 8);
    *(uint16_t*)(pkt+2) = htons(inet_cksum(pkt, 16));

    g_net.ping_id      = id;
    g_net.ping_seq     = seq;
    g_net.ping_replied = 0;

    return ip_tx(dst, IPPROTO_ICMP, pkt, 16);
}