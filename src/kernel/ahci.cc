// ----------------------------------------------------------------------------
// \file  ahci.cc
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------

# include "config.h"

# include "stdint.h"
# include "proto.h"
# include "iso9660.h"

# define DESKTOP
# include "vga.h"

#define HBA_PxCMD_ST   (1  <<  0)
#define HBA_PxCMD_FRE  (1  <<  4)
#define HBA_PxCMD_FR   (1  << 14)
#define HBA_PxCMD_CR   (1  << 15)
#define HBA_PxIS_TFES  (1u << 30)

#define ATA_CMD_READ_DMA_EXT  0x25  // 48-bit LBA; für <=28bit tuts auch 0x25/0xC8

// ATAPI
# define ATAPI_SECTOR_SIZE 2048

typedef struct {
    // 0x00
    uint8_t  cfl:5;    // Command FIS Length
    uint8_t  a:1;
    uint8_t  w:1;      // 1 = write, 0 = read
    uint8_t  p:1;
    uint8_t  r:1;
    uint8_t  b:1;
    uint8_t  c:1;
    uint8_t  rsv0:1;
    uint8_t  pmp:4;
    uint16_t prdtl;    // Physical Region Descriptor Table Length
    // 0x04
    volatile uint32_t prdbc;
    // 0x08
    uint32_t ctba;
    uint32_t ctbau;
    // 0x10
    uint32_t rsv1[4];
} hba_cmd_header_t;

typedef struct {
    uint32_t dba;
    uint32_t dbau;
    uint32_t rsv0;
    uint32_t dbc:22;
    uint32_t rsv1:9;
    uint32_t i:1;
} hba_prdt_entry_t;

typedef volatile struct {
    uint32_t clb;       // 0x00: Command List Base (low)
    uint32_t clbu;      // 0x04: Command List Base (high)
    uint32_t fb;        // 0x08: FIS Base (low)
    uint32_t fbu;       // 0x0C: FIS Base (high)
    uint32_t is;        // 0x10: Interrupt Status
    uint32_t ie;        // 0x14: Interrupt Enable
    uint32_t cmd;       // 0x18: Command and Status
    uint32_t rsv0;      // 0x1C
    uint32_t tfd;       // 0x20: Task File Data
    uint32_t sig;       // 0x24: Signature
    uint32_t ssts;      // 0x28: SATA Status (SStatus)
    uint32_t sctl;      // 0x2C: SATA Control
    uint32_t serr;      // 0x30: SATA Error
    uint32_t sact;      // 0x34: SATA Active
    uint32_t ci;        // 0x38: Command Issue
    uint32_t sntf;      // 0x3C
    uint32_t fbs;       // 0x40
    uint32_t rsv1[11];  // 0x44..0x6F
    uint32_t vendor[4]; // 0x70..0x7F
} hba_port_t;

typedef volatile struct {
    uint32_t cap;       // 0x00: Host Capabilities
    uint32_t ghc;       // 0x04: Global Host Control
    uint32_t is;        // 0x08: Interrupt Status
    uint32_t pi;        // 0x0C: Ports Implemented
    uint32_t vs;        // 0x10: Version
    uint32_t ccc_ctl;   // 0x14
    uint32_t ccc_pts;   // 0x18
    uint32_t em_loc;    // 0x1C
    uint32_t em_ctl;    // 0x20
    uint32_t cap2;      // 0x24
    uint32_t bohc;      // 0x28
    uint8_t  rsv[0xA0 - 0x2C];
    uint8_t  vendor[0x100 - 0xA0];
    hba_port_t ports[32]; // ab 0x100
} hba_mem_t;

typedef volatile struct {
    uint8_t cfis[64];
    uint8_t acmd[16];    // ATAPI Command
    uint8_t rsv[48];
    hba_prdt_entry_t prdt[1]; // für Demo: ein Eintrag
} hba_cmd_tbl_t;

typedef struct {
    hba_port_t *port;
    int dev_type;
} ahci_device_t;

typedef struct {
    uint32_t abar;  // physische Adresse der HBA-MMIO-Struktur
} ahci_controller_t;

// statische Puffer (aligned nötig)
static uint8_t g_clb[1024] __attribute__((aligned(1024)));
static uint8_t g_fis[256]  __attribute__((aligned(256)));
static uint8_t g_ct[sizeof(hba_cmd_tbl_t)] __attribute__((aligned(128)));
enum {
    AHCI_DEV_NULL = 0,
    AHCI_DEV_SATA = 1,
    AHCI_DEV_SATAPI = 4,
};

static hba_mem_t *g_hba = 0;

static ahci_controller_t g_ahci;
static ahci_device_t g_sata_dev; // wir nehmen das erste gefundene für Demo

static int ahci_check_port(hba_port_t *p)
{
    uint32_t ssts = p->ssts;
    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    if (det != 3 || ipm != 1)
        return AHCI_DEV_NULL; // kein Device aktiv

    switch (p->sig) {
        case 0x00000101: return AHCI_DEV_SATA;   // SATA
        case 0xEB140101: return AHCI_DEV_SATAPI; // SATAPI (CD/DVD)
        default:         return AHCI_DEV_NULL;
    }
}

static inline uint32_t pci_config_read32(
    uint8_t bus, uint8_t slot,
    uint8_t func, uint8_t offset) {
        
    uint32_t address = (1U << 31)        // enable bit
        | ((uint32_t)bus  << 16)
        | ((uint32_t)slot << 11)
        | ((uint32_t)func << 8)
        | (offset & 0xFC);

    outl(0xCF8, address);
    return inl(0xCFC);
}

int ahci_find_controller(void)
{
    for (int bus = 0; bus < 256; ++bus) {
        for (uint8_t slot = 0; slot < 32; ++slot) {
            uint32_t vendor_device = pci_config_read32(bus, slot, 0, 0x00);
            if (vendor_device == 0xFFFFFFFF)
                continue; // kein Device

            uint32_t class_regs = pci_config_read32(bus, slot, 0, 0x08);
            uint8_t _class      = (class_regs >> 24) & 0xFF;
            uint8_t subclass    = (class_regs >> 16) & 0xFF;
            uint8_t prog_if     = (class_regs >> 8)  & 0xFF;

            if (_class == 0x01 && subclass == 0x06 && prog_if == 0x01) {
                gfx_printf("AHCI-Controller gefunden: bus=0x%x slot=0x%x\n", bus, slot);

                uint32_t bar5 = pci_config_read32(bus, slot, 0, 0x24);
                g_ahci.abar = bar5 & ~0x0F; // untere Bits sind Flags

                gfx_printf("ABAR (BAR5) = 0x%x\n", g_ahci.abar);
                return 0;
            }
        }
    }
    gfx_printf("Kein AHCI-Controller gefunden.\n");
    return -1;
}

static void ahci_stop_port(hba_port_t *p)
{
    p->cmd &= ~HBA_PxCMD_ST;
    p->cmd &= ~HBA_PxCMD_FRE;
    // warten bis FR und CR 0 sind
    while (p->cmd & (HBA_PxCMD_FR | HBA_PxCMD_CR));
}

static void ahci_start_port(hba_port_t *p)
{
    while (p->cmd & HBA_PxCMD_CR);
    p->cmd |= HBA_PxCMD_FRE;
    p->cmd |= HBA_PxCMD_ST;
}

void ahci_port_init(hba_port_t *p)
{
    ahci_stop_port(p);

    p->clb  = (uint32_t)(uintptr_t)g_clb;
    p->clbu = 0;
    p->fb   = (uint32_t)(uintptr_t)g_fis;
    p->fbu  = 0;

    for (int i = 0; i < sizeof(g_clb); ++i) g_clb[i] = 0;
    for (int i = 0; i < sizeof(g_fis); ++i) g_fis[i] = 0;
    for (int i = 0; i < sizeof(g_ct);  ++i) g_ct[i]  = 0;

    ahci_start_port(p);
}

static int satapi_read(hba_port_t *p, uint32_t lba, uint32_t count, void *buffer)
{
    if (!count) return 0;

    ahci_port_init(p);

    int slot = 0;
    hba_cmd_header_t *cmd_hdr = (hba_cmd_header_t *)(uintptr_t)p->clb;
    cmd_hdr += slot;
    
    cmd_hdr->cfl   = 3;   // 20 bytes / 4
    cmd_hdr->a     = 1;   // ATAPI
    cmd_hdr->w     = 0;   // Read
    cmd_hdr->c     = 1;
    cmd_hdr->prdtl = 1; // ein PRDT-Eintrag

    hba_cmd_tbl_t *cmd_tbl = (hba_cmd_tbl_t *)(uintptr_t)g_ct;
    cmd_hdr->ctba  = (uint32_t)(uintptr_t)cmd_tbl;
    cmd_hdr->ctbau = 0;

    // PRDT auf Zielbuffer
    uint32_t bytes = count * ATAPI_SECTOR_SIZE;

    cmd_tbl->prdt[0].dba  = (uint32_t)(uintptr_t)buffer;
    cmd_tbl->prdt[0].dbau = 0;
    cmd_tbl->prdt[0].dbc  = bytes - 1;   // dbc = Anzahl Bytes -1
    cmd_tbl->prdt[0].i    = 1;           // Interrupt (für später, hier egal)

    // CFIS: Register – Host to Device FIS
    for (int i = 0; i < 64; ++i)
    cmd_tbl->cfis[i] = 0;

    volatile uint8_t *cfis = cmd_tbl->cfis;
    
    cfis[0] = 0x27;      // FIS Type = Register H2D
    cfis[1] = 1 << 7;    // C = 1 (Command)
    cfis[2] = 0xA0;      // ATA_CMD_PACKET
    cfis[3] = 0x05;      // Features (low) – optional
    cfis[7]  = 0;        // device = 0 (Master, LBA=0bits)
    
    // ATAPI-Kommando: READ(12)
    volatile uint8_t *acmd = cmd_tbl->acmd;
    for (int i = 0; i < 16; ++i) acmd[i] = 0;

    acmd[0] = 0xA8;      // READ(12)
    acmd[1] = 0;         // Flags

    // LBA (Big Endian)
    acmd[2] = (uint8_t)(lba >> 24);
    acmd[3] = (uint8_t)(lba >> 16);
    acmd[4] = (uint8_t)(lba >> 8);
    acmd[5] = (uint8_t)(lba);

    // Transfer Length (Anzahl Sektoren, Big Endian)
    acmd[6] = (uint8_t)(count >> 24);
    acmd[7] = (uint8_t)(count >> 16);
    acmd[8] = (uint8_t)(count >> 8);
    acmd[9] = (uint8_t)(count);

    // ggf. alte Interrupts löschen
    p->is   = (uint32_t)-1;
    p->serr = (uint32_t)-1;

    // Task File darf nicht busy sein
    while (p->tfd & (0x80 | 0x08)) {
        // BSY oder DRQ noch gesetzt
    }
    
    p->ci = 1U << slot;

    // Auf Abschluss warten
    while (1) {
        if ((p->ci & (1U << slot)) == 0)
            break; // fertig

        // Fehler?
        if (p->is & HBA_PxIS_TFES) { // Task File Error
            gfx_printf("satapi_read: Task File Error (p->is=0x%x)\n", p->is);
            gfx_printf("TFES: tfd=0x%x, serr=0x%x, ssts=0x%x\n",
                        p->tfd, p->serr, p->ssts);
            return -1;
        }
    }

    if (p->is & HBA_PxIS_TFES) {
        gfx_printf("satapi_read: TFES nach Abschluss, tfd=0x%x, serr=0x%x, ssts=0x%x\n",
                p->tfd, p->serr, p->ssts);
        return -1;
    }

    return 0;
}

int ahci_probe_ports(void)
{
    uint32_t pi = g_hba->pi;
    gfx_printf("ahci_probe_ports: pi=0x%x\n", pi);

    for (int i = 0; i < 32; ++i) {
        if (!(pi & (1U << i)))
            continue;

        hba_port_t *p = &g_hba->ports[i];
        int type = ahci_check_port(p);

        if (type == AHCI_DEV_SATA) {
            gfx_printf("Port %d: SATA-Device\n", i);
            g_sata_dev.port = p;
            g_sata_dev.dev_type = type;
            return 0;
        } else if (type == AHCI_DEV_SATAPI) {
            gfx_printf("Port %d: SATAPI-Device (CD/DVD)\n", i);
            // hier könntest du später AHCI+PACKET (CD) implementieren
            g_sata_dev.port = p;
            g_sata_dev.dev_type = type;
            return 0;
        }
    }

    gfx_printf("Kein SATA/SATAPI-Gerät gefunden.\n");
    return -1;
}

int sata_read_sectors(uint32_t lba, uint32_t count, void *buffer)
{
    if (!g_hba || !g_sata_dev.port)
        return -1;

    if (g_sata_dev.dev_type != AHCI_DEV_SATAPI) {
        gfx_printf("cd_read_sectors: Device ist nicht SATAPI (Typ=%d)\n",
                g_sata_dev.dev_type);
        return -1;
    }

    return satapi_read(g_sata_dev.port, lba, count, buffer);
}

int sata_test_iso9660(void)
{
    uint8_t sector[ATAPI_SECTOR_SIZE];

    gfx_printf("cd_test_iso9660: lese LBA 16...\n");
    if (sata_read_sectors(16, 1, sector) != 0) {
        gfx_printf("cd_test_iso9660: lesen fehlgeschlagen\n");
        return -1;
    }

    uint8_t type = sector[0];
    char id[6];
    
    for (int i = 0; i < 5; ++i)
    id[i] = sector[1 + i];
    id[5] = '\0';

    gfx_printf("cd_test_iso9660: type=%u id=\"%s\"\n", (unsigned)type, id);

    if (type == 1 &&
        id[0]=='C' && id[1]=='D' && id[2]=='0' &&
        id[3]=='0' && id[4]=='1') {
        gfx_printf("cd_test_iso9660: ISO9660-CD erkannt!\n");
        return 0;
    }

    gfx_printf("cd_test_iso9660: kein ISO9660 Primary Descriptor\n");
    return -1;
}

int ahci_init(void)
{
    if (ahci_find_controller() != 0)
    return -1;
// A
    //printformat("ABAR phys: 0x%x\n", g_ahci.abar);

    g_hba = (hba_mem_t *)mmio_map(g_ahci.abar, 4096); // mind. 4KB
//  g_hba = (hba_mem_t *)(uintptr_t)g_ahci.abar;
        
    if (graph_mode == 1) {
        printformat("g_hba Pointer (virt): 0x%x\n", (uint32_t)g_hba);
        printformat("AHCI-Version: 0x%x, Ports Implemented: 0x%x\n",
        g_hba->vs, g_hba->pi);
    }   else {
        gfx_printf("AHCI-Version: 0x%x, Ports Implemented: 0x%x\n",
        g_hba->vs, g_hba->pi);
    }
// B
    // HBA einschalten (GHC.AE = 1)
    g_hba->ghc |= (1U << 31);
    
    return 0;
}

// iso expects: void (*)(uint32_t lba, uint32_t count, void* dst)
static void iso_read_sectors_ahci(uint32_t lba, uint32_t count, void* dst)
{
    int rc = sata_read_sectors(lba, count, dst);
    if (rc != 0) {
        // Fehlerbehandlung: panic/log/flag setzen
        // z.B. kpanic("AHCI read failed");
    }
}

int check_ahci(void)
{
    if (ahci_init() != 0) {
        #if (ISOGUI == 0)
            #if (ISOLANG == LANG_ENU)
                printformat("AHCI init: failed.\n");
            #else
                printformat("AHCI init: Fehler.\n");
            #endif
        #else
            #if (ISOLANG == LANG_ENU)
                gfx_printf("AHCI init: failed.\n");
            #else
                gfx_printf("AHCI init: Fehler.\n");
            #endif
        #endif
        return -1;
    }
    if (ahci_probe_ports() != 0) {
        #if (ISOGUI == 0)
            #if (ISOLANG == LANG_ENU)
                printformat("AHCI probe: failed.\n");
            #else
                printformat("AHCI probe: Fehler.\n");
            #endif
        #else
            #if (ISOLANG == LANG_ENU)
                gfx_printf("AHCI probe failed\n");
            #else
                gfx_printf("AHCI probe: Fehler.\n");
            #endif
        #endif
        return -1;
    }

    iso_init(iso_read_sectors_ahci);
    
    uint8_t buf[512];
    if (sata_read_sectors(34, 1, buf) != 0) {
        #if (ISOGUI == 0)
            #if (ISOLANG == LANG_ENU)
                printformat("sata_read_sectors: failed.\n");
            #else
                printformat("sata_read_sectors: Fehler.\n");
            #endif
        #else
            #if (ISOLANG == LANG_ENU)
                gfx_printf("sata_read_sectors: failed.\n");
            #else
                gfx_printf("sata_read_sectors: Fehler.\n");
            #endif
        #endif
        return -1;
    }

    #if (ISOGUI == 0)
        #if (ISOLANG == LANG_ENU)
            printformat("First Byte of LBA 0: 0x%x\n", buf[0]);
        #else
            printformat("Erstes Byte von LBA 0: 0x%x\n", buf[0]);
        #endif
    #else
        #if (ISOLANG == LANG_ENU)
            gfx_printf("First Byte of LBA 0: 0x%x\n", buf[0]);
        #else
            gfx_printf("Erstes Byte von LBA 0: 0x%x\n", buf[0]);
        #endif
    #endif
    return 0;
}
