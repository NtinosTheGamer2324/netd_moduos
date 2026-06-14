#include "./libc.h"
#include "netd.h"

#define NIC_PATH "$/dev/net/net0"
#define DHCP_TIMEOUT_MS 10000

static void rx_once(void) {
    uint8_t frame[1518];
    uint32_t flen = sizeof(frame);
    int n = driver_rx(frame, &flen);
    if (n <= 0) return;

    uint16_t et = (uint16_t)((frame[12]<<8)|frame[13]);
    if      (et == ETHERTYPE_IP)  ip_rx(frame, (int)flen);
    else if (et == ETHERTYPE_ARP) arp_rx(frame, (int)flen);
}

int md_main(long argc, char **argv) {
    (void)argc; (void)argv;

    printf("[netd] starting\n");

    /* ── open NIC ─────────────────────────────────────────────────────── */
    if (driver_open(NIC_PATH) < 0) {
        printf("[netd] FATAL: cannot open %s\n", NIC_PATH);
        return 1;
    }

    /* ── get link info ────────────────────────────────────────────────── */
    {
        uint32_t rep_len;
        uint32_t val = 0;

        /* MAC */
        rep_len = 6;
        if (driver_transact(NET_CMD_GET_MAC, NULL, 0,
                            g_net.mac, &rep_len) != 0) {
            printf("[netd] FATAL: GET_MAC failed\n");
            return 1;
        }
        printf("[netd] MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
               g_net.mac[0],g_net.mac[1],g_net.mac[2],
               g_net.mac[3],g_net.mac[4],g_net.mac[5]);

        /* MTU */
        rep_len = 4;
        driver_transact(NET_CMD_GET_MTU, NULL, 0, (uint8_t*)&val, &rep_len);
        g_net.mtu = val ? val : 1500;

        /* link */
        rep_len = 4; val = 0;
        driver_transact(NET_CMD_GET_LINK_UP, NULL, 0, (uint8_t*)&val, &rep_len);
        g_net.link_up = (int)val;
        printf("[netd] link: %s  mtu: %u\n",
               g_net.link_up ? "UP" : "DOWN", g_net.mtu);
    }

    /* ── init subsystems ──────────────────────────────────────────────── */
    arp_init();
    memset(g_net.udp_binds, 0, sizeof(g_net.udp_binds));

    /* ── DHCP ─────────────────────────────────────────────────────────── */
    printf("[netd] starting DHCP\n");
    dhcp_start();

    uint64_t dhcp_deadline = time_ms() + DHCP_TIMEOUT_MS;
    while (!dhcp_bound() && time_ms() < dhcp_deadline) {
        rx_once();
    }

    if (!dhcp_bound()) {
        printf("[netd] DHCP failed, falling back to 10.0.2.15/24\n");
        g_net.ip      = MAKE_IP(10,0,2,15);
        g_net.netmask = MAKE_IP(255,255,255,0);
        g_net.gateway = MAKE_IP(10,0,2,1);
        g_net.dns     = MAKE_IP(8,8,8,8);
        arp_announce();
    }

    printf("[netd] IP: %u.%u.%u.%u / %u.%u.%u.%u  GW: %u.%u.%u.%u  DNS: %u.%u.%u.%u\n",
           (g_net.ip>>24)&0xFF,      (g_net.ip>>16)&0xFF,
           (g_net.ip>>8)&0xFF,        g_net.ip&0xFF,
           (g_net.netmask>>24)&0xFF,  (g_net.netmask>>16)&0xFF,
           (g_net.netmask>>8)&0xFF,    g_net.netmask&0xFF,
           (g_net.gateway>>24)&0xFF,  (g_net.gateway>>16)&0xFF,
           (g_net.gateway>>8)&0xFF,    g_net.gateway&0xFF,
           (g_net.dns>>24)&0xFF,      (g_net.dns>>16)&0xFF,
           (g_net.dns>>8)&0xFF,        g_net.dns&0xFF);

    printf("[netd] ready\n");

    /* ── register API ─────────────────────────────────────────────────── */
    api_init();
    /* ── main rx loop ─────────────────────────────────────────────────── */
    for (;;) {
        printf("LOOP\n");
        rx_once();
        sleep(1);
    }

    return 0;
}