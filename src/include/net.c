# include "stdint.h"
# include "net.h"

uint8_t  net_mac[6]    = {0};
uint32_t net_ip        = 0;
uint32_t net_gateway   = 0;
uint32_t net_netmask   = 0;

void net_config_set(
    const uint8_t mac[6],
    uint32_t ip,
    uint32_t gateway,
    uint32_t netmask) {

    for (int i = 0; i < 6; ++i) net_mac[i] = mac[i];
    net_ip      = ip;
    net_gateway = gateway;
    net_netmask = netmask;
}

void net_init(void) {
    // Vorläufig feste Werte:
    uint8_t mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
    uint32_t ip        = 0xC0A80A64; // 192.168.1.100
    uint32_t gateway   = 0xC0A80A01; // 192.168.1.1
    uint32_t netmask   = 0xFFFFFF00; // 255.255.255.0
    
    net_config_set(mac, ip, gateway, netmask);
    
    e1000_init();    // NIC initialisieren
    arp_init();      // ARP-Cache, etc.
    ipv4_init();     // eigene IP, Routing (meist nur default gateway)
    udp_init();      // UDP-Ports, Sockets, etc.
}

void pci_enumerate(void) {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t dev = 0; dev < 32; dev++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint16_t vendor = pci_read_vendor_id(bus, dev, func);
                if (vendor == 0xFFFF) continue;

                uint16_t device = pci_read_device_id(bus, dev, func);

                if (vendor == 0x8086 && (device == 0x100E /* E1000 */)) {
                    // passende Funktion im NIC-Treiber
                    e1000_pci_attach(bus, dev, func);
                }
            }
        }
    }
}
