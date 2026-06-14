#define LIBC_NO_START
#include "./libc.h"
#include "netd.h"

netd_state_t g_net;

int driver_open(const char *path) {
    g_net.fd = open(path, O_RDWR, 0);
    return g_net.fd;
}

/* Send a command, read reply until null terminator.
 * Returns reply status, fills rep_data/rep_len if provided. */
int driver_transact(uint32_t cmd, const uint8_t *payload, uint32_t plen,
                    uint8_t *rep_data, uint32_t *rep_len)
{
    /* build cmd packet: cmd(4) + len(4) + payload */
    uint8_t pkt[8 + 1500];
    memset(pkt, 0, 8);
    *(uint32_t*)(pkt+0) = cmd;
    *(uint32_t*)(pkt+4) = plen;
    if (payload && plen) memcpy(pkt+8, payload, plen);

    if (write(g_net.fd, pkt, 8+plen) < 0) return -1;

    /* read reply until null terminator */
    uint8_t rep[1520];
    memset(rep, 0, sizeof(rep));
    int got = 0;
    while (got < (int)sizeof(rep)-1) {
        ssize_t n = read(g_net.fd, rep+got, sizeof(rep)-1-got);
        if (n < 0)  return -2;
        if (n == 0) { yield(); continue; }
        got += (int)n;
        if (rep[got-1] == 0) break;
    }
    if (got < 12) return -3;

    int32_t  status = *(int32_t*) (rep+4);
    uint32_t len    = *(uint32_t*)(rep+8);

    if (rep_data && rep_len) {
        uint32_t copy = len < *rep_len ? len : *rep_len;
        memcpy(rep_data, rep+12, copy);
        *rep_len = copy;
    }
    return (int)status;
}

int driver_tx(const uint8_t *frame, uint32_t len) {
    return driver_transact(NET_CMD_TX_FRAME, frame, len, NULL, NULL);
}

/* Returns frame length or 0 if nothing ready */
int driver_rx(uint8_t *frame, uint32_t *len) {
    uint8_t pkt[8];
    memset(pkt, 0, 8);
    *(uint32_t*)pkt = NET_CMD_RX_POLL;
    if (write(g_net.fd, pkt, 8) < 0) return -1;

    uint8_t rep[1520];
    memset(rep, 0, sizeof(rep));
    int got = 0;
    while (got < (int)sizeof(rep)-1) {
        ssize_t n = read(g_net.fd, rep+got, sizeof(rep)-1-got);
        if (n < 0)  return -1;
        if (n == 0) { *len = 0; return 0; } /* nothing ready */
        got += (int)n;
        if (rep[got-1] == 0) break;
    }
    if (got < 12) return -1;

    uint32_t flen = *(uint32_t*)(rep+8);
    if (flen == 0) { *len = 0; return 0; }

    uint32_t copy = flen < *len ? flen : *len;
    memcpy(frame, rep+12, copy);
    *len = copy;
    return (int)copy;
}