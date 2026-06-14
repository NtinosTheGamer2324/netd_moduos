#define LIBC_NO_START
#include "./libc.h"
#include "netd.h"

#define DNS_MAX_WAIT_MS 3000

static int dns_build_query(uint8_t *buf, int max,
                           uint16_t txid, const char *name)
{
    if (max < 12) return -1;

    *(uint16_t*)(buf+0) = htons(txid);
    *(uint16_t*)(buf+2) = htons(0x0100); /* QR=0 RD=1 */
    *(uint16_t*)(buf+4) = htons(1);      /* QDCOUNT=1 */
    *(uint16_t*)(buf+6) = 0;
    *(uint16_t*)(buf+8) = 0;
    *(uint16_t*)(buf+10)= 0;

    uint8_t *p = buf + 12;
    const char *s = name;
    while (*s) {
        const char *dot = s;
        while (*dot && *dot != '.') dot++;
        int label_len = (int)(dot - s);
        if ((p - buf) + 1 + label_len + 5 >= max) return -1;
        *p++ = (uint8_t)label_len;
        memcpy(p, s, label_len);
        p += label_len;
        s = dot;
        if (*s == '.') s++;
    }
    *p++ = 0;                        /* root label */
    *(uint16_t*)p = htons(1); p+=2;  /* QTYPE A */
    *(uint16_t*)p = htons(1); p+=2;  /* QCLASS IN */

    return (int)(p - buf);
}

/* Called from udp_rx when src_port == DNS_PORT */
void dns_rx(uint32_t src_ip, const uint8_t *data, int len) {
    (void)src_ip;
    if (len < 12) return;

    uint16_t txid    = ntohs(*(uint16_t*)(data+0));
    uint16_t flags   = ntohs(*(uint16_t*)(data+2));
    uint16_t ancount = ntohs(*(uint16_t*)(data+6));

    if (txid != g_net.dns_txid) return;
    if (!(flags & 0x8000)) return; /* not a response */
    if (ancount == 0) { g_net.dns_done = -1; return; }

    /* skip question section */
    const uint8_t *p = data + 12;
    const uint8_t *end = data + len;

    /* skip QNAME */
    while (p < end && *p) {
        if ((*p & 0xC0) == 0xC0) { p += 2; goto answers; }
        p += 1 + *p;
    }
    p += 1 + 4; /* null label + QTYPE + QCLASS */

answers:
    for (int i = 0; i < (int)ancount && p < end; i++) {
        /* skip name (may be pointer) */
        if (p >= end) break;
        if ((*p & 0xC0) == 0xC0) p += 2;
        else { while (p < end && *p) { p += 1+*p; } p++; }

        if (p+10 > end) break;
        uint16_t type  = ntohs(*(uint16_t*)(p+0));
        uint16_t rdlen = ntohs(*(uint16_t*)(p+8));
        p += 10;

        if (type == 1 && rdlen == 4 && p+4 <= end) {
            g_net.dns_result = ntohl(*(uint32_t*)p);
            g_net.dns_done   = 1;
            return;
        }
        p += rdlen;
    }
    g_net.dns_done = -1;
}

int dns_resolve(const char *name, uint32_t *ip_out) {
    if (!g_net.dns || !g_net.ip) return -1;

    g_net.dns_txid  = (uint16_t)(time_ms() & 0xFFFF);
    g_net.dns_done  = 0;
    g_net.dns_result= 0;

    uint8_t query[256];
    int qlen = dns_build_query(query, sizeof(query), g_net.dns_txid, name);
    if (qlen < 0) return -1;

    udp_tx(g_net.dns, 1053, DNS_PORT, query, (uint16_t)qlen);

    uint64_t deadline = time_ms() + DNS_MAX_WAIT_MS;
    while (time_ms() < deadline) {
        /* pump rx */
        uint8_t frame[1518];
        uint32_t flen = sizeof(frame);
        if (driver_rx(frame, &flen) > 0) {
            uint16_t et = (uint16_t)((frame[12]<<8)|frame[13]);
            if (et == ETHERTYPE_IP)  ip_rx(frame, (int)flen);
            else if (et == ETHERTYPE_ARP) arp_rx(frame, (int)flen);
        }
        if (g_net.dns_done != 0) break;
        yield();
    }

    if (g_net.dns_done == 1) {
        *ip_out = g_net.dns_result;
        return 0;
    }
    return -1;
}