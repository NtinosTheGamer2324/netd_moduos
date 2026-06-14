#define LIBC_NO_START
#include "./libc.h"
#include "netd.h"

int udp_bind(uint16_t port) {
    for (int i = 0; i < UDP_MAX_BINDS; i++) {
        if (g_net.udp_binds[i].used && g_net.udp_binds[i].port == port)
            return 0; /* already bound */
    }
    for (int i = 0; i < UDP_MAX_BINDS; i++) {
        if (!g_net.udp_binds[i].used) {
            memset(&g_net.udp_binds[i], 0, sizeof(udp_bind_t));
            g_net.udp_binds[i].port = port;
            g_net.udp_binds[i].used = 1;
            return 0;
        }
    }
    return -1; /* table full */
}

void udp_unbind(uint16_t port) {
    for (int i = 0; i < UDP_MAX_BINDS; i++) {
        if (g_net.udp_binds[i].used && g_net.udp_binds[i].port == port) {
            g_net.udp_binds[i].used = 0;
            return;
        }
    }
}

int udp_tx(uint32_t dst, uint16_t src_port, uint16_t dst_port,
           const uint8_t *data, uint16_t len)
{
    uint16_t udp_len = (uint16_t)(8 + len);
    uint8_t  pkt[8 + 1472];

    if (len > 1472) return -1;

    *(uint16_t*)(pkt+0) = htons(src_port);
    *(uint16_t*)(pkt+2) = htons(dst_port);
    *(uint16_t*)(pkt+4) = htons(udp_len);
    *(uint16_t*)(pkt+6) = 0; /* checksum optional for UDP */
    memcpy(pkt+8, data, len);

    return ip_tx(dst, IPPROTO_UDP, pkt, udp_len);
}

void udp_rx(uint32_t src_ip, const uint8_t *data, int len) {
    if (len < 8) return;
    uint16_t src_port = ntohs(*(uint16_t*)(data+0));
    uint16_t dst_port = ntohs(*(uint16_t*)(data+2));
    uint16_t udp_len  = ntohs(*(uint16_t*)(data+4));
    int payload_len   = (int)udp_len - 8;

    if (payload_len <= 0) return;
    const uint8_t *payload = data + 8;

    /* route to DHCP / DNS first */
    if (dst_port == DHCP_PORT_CLIENT) {
        dhcp_rx(src_ip, payload, payload_len);
        return;
    }
    if (src_port == DNS_PORT) {
        dns_rx(src_ip, payload, payload_len);
        return;
    }

    /* deliver to bound port */
    for (int i = 0; i < UDP_MAX_BINDS; i++) {
        udp_bind_t *b = &g_net.udp_binds[i];
        if (b->used && b->port == dst_port) {
            int copy = payload_len < UDP_RXBUF_SIZE ? payload_len : UDP_RXBUF_SIZE;
            memcpy(b->buf, payload, copy);
            b->len      = copy;
            b->src_ip   = src_ip;
            b->src_port = src_port;
            return;
        }
    }
}

int udp_recv(uint16_t port, uint8_t *buf, int max,
             uint32_t *src_ip, uint16_t *src_port)
{
    for (int i = 0; i < UDP_MAX_BINDS; i++) {
        udp_bind_t *b = &g_net.udp_binds[i];
        if (b->used && b->port == port && b->len > 0) {
            int copy = b->len < max ? b->len : max;
            memcpy(buf, b->buf, copy);
            if (src_ip)   *src_ip   = b->src_ip;
            if (src_port) *src_port = b->src_port;
            b->len = 0;
            return copy;
        }
    }
    return 0;
}