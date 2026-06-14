#define LIBC_NO_START
#include "./libc.h"
#include "netd.h"

static const uint8_t bcast[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

void arp_init(void) {
    memset(g_net.arp_cache, 0, sizeof(g_net.arp_cache));
}

void arp_learn(uint32_t ip, const uint8_t *mac) {
    /* update existing */
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (g_net.arp_cache[i].valid && g_net.arp_cache[i].ip == ip) {
            memcpy(g_net.arp_cache[i].mac, mac, 6);
            g_net.arp_cache[i].ts = time_ms();
            return;
        }
    }
    /* find empty or oldest slot */
    int slot = 0;
    uint64_t oldest = 0xFFFFFFFFFFFFFFFFULL;
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (!g_net.arp_cache[i].valid) { slot = i; break; }
        if (g_net.arp_cache[i].ts < oldest) {
            oldest = g_net.arp_cache[i].ts;
            slot = i;
        }
    }
    g_net.arp_cache[slot].ip    = ip;
    g_net.arp_cache[slot].ts    = time_ms();
    g_net.arp_cache[slot].valid = 1;
    memcpy(g_net.arp_cache[slot].mac, mac, 6);
}

int arp_lookup(uint32_t ip, uint8_t *mac_out) {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        arp_entry_t *e = &g_net.arp_cache[i];
        if (e->valid && e->ip == ip) {
            if (time_ms() - e->ts < ARP_TIMEOUT_MS) {
                memcpy(mac_out, e->mac, 6);
                return 0;
            }
            e->valid = 0; /* expired */
        }
    }
    return -1;
}

static void arp_send(uint16_t op, const uint8_t *dst_mac,
                     uint32_t src_ip, uint32_t dst_ip)
{
    uint8_t frame[42];
    memset(frame, 0, sizeof(frame));

    /* Ethernet header */
    memcpy(frame+0, dst_mac, 6);
    memcpy(frame+6, g_net.mac, 6);
    *(uint16_t*)(frame+12) = htons(ETHERTYPE_ARP);

    /* ARP body */
    *(uint16_t*)(frame+14) = htons(1);       /* hw type Ethernet */
    *(uint16_t*)(frame+16) = htons(0x0800);  /* proto IPv4 */
    frame[18] = 6; frame[19] = 4;
    *(uint16_t*)(frame+20) = htons(op);
    memcpy(frame+22, g_net.mac, 6);
    *(uint32_t*)(frame+28) = htonl(src_ip);
    memcpy(frame+32, dst_mac, 6);
    *(uint32_t*)(frame+38) = htonl(dst_ip);

    driver_tx(frame, 42);
}

void arp_announce(void) {
    /* gratuitous ARP: tell everyone our IP/MAC */
    arp_send(ARP_OP_REQUEST, bcast, g_net.ip, g_net.ip);
}

void arp_request(uint32_t ip) {
    arp_send(ARP_OP_REQUEST, bcast, g_net.ip, ip);
}

void arp_rx(const uint8_t *frame, int len) {
    if (len < 42) return;
    const uint8_t *arp = frame + 14;

    uint16_t op      = ntohs(*(uint16_t*)(arp+6));
    uint32_t src_ip  = ntohl(*(uint32_t*)(arp+14));
    uint32_t dst_ip  = ntohl(*(uint32_t*)(arp+24));
    const uint8_t *src_mac = arp + 8;

    /* always learn sender */
    if (src_ip != 0) arp_learn(src_ip, src_mac);

    if (op == ARP_OP_REQUEST && dst_ip == g_net.ip && g_net.ip != 0) {
        /* someone wants our MAC — reply */
        arp_send(ARP_OP_REPLY, src_mac, g_net.ip, src_ip);
    }
}