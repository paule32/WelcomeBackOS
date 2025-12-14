#include "stdint.h"
#include "proto.h"

// Primary ATA
# define ATA_BASE        0x1F0
# define ATA_REG_DATA    (ATA_BASE + 0)
# define ATA_REG_ERROR   (ATA_BASE + 1)
# define ATA_REG_FEATURE (ATA_BASE + 1)
# define ATA_REG_SECCNT  (ATA_BASE + 2)
# define ATA_REG_LBA_LO  (ATA_BASE + 3)
# define ATA_REG_LBA_MID (ATA_BASE + 4)
# define ATA_REG_LBA_HI  (ATA_BASE + 5)
# define ATA_REG_DRIVE   (ATA_BASE + 6)
# define ATA_REG_STATUS  (ATA_BASE + 7)
# define ATA_REG_COMMAND (ATA_BASE + 7)

// Control / Alternate Status
# define ATA_CTRL_BASE   0x3F6
# define ATA_REG_DEVCTRL (ATA_CTRL_BASE + 0)
# define ATA_REG_ALTSTAT (ATA_CTRL_BASE + 0)

// Status-Bits
# define ATA_SR_BSY   0x80
# define ATA_SR_DRDY  0x40
# define ATA_SR_DRQ   0x08
# define ATA_SR_ERR   0x01

// Device-Selektion
# define ATA_DRIVE_MASTER 0xA0
# define ATA_DRIVE_SLAVE  0xB0

// Kommandos
# define ATA_CMD_IDENTIFY_PACKET 0xA1
# define ATA_CMD_PACKET          0xA0

// ATAPI
# define ATAPI_SECTOR_SIZE 2048

typedef struct {
    uint16_t base;    // z.B. 0x1F0 oder 0x170
    uint16_t ctrl;    // z.B. 0x3F6 oder 0x376
} ata_channel_t;

typedef struct {
    ata_channel_t *chan;
    uint8_t drive_sel;   // 0xA0 (Master) oder 0xB0 (Slave)
    int present;
} atapi_device_t;

static ata_channel_t g_channels[2] = {
    { 0x1F0, 0x3F6 }, // primary
    { 0x170, 0x376 }  // secondary
};

static atapi_device_t g_cd = { 0 };

static inline uint16_t ata_reg_data    (ata_channel_t *c) { return c->base + 0; }
static inline uint16_t ata_reg_error   (ata_channel_t *c) { return c->base + 1; }
static inline uint16_t ata_reg_feature (ata_channel_t *c) { return c->base + 1; }
static inline uint16_t ata_reg_seccnt  (ata_channel_t *c) { return c->base + 2; }
static inline uint16_t ata_reg_lba_lo  (ata_channel_t *c) { return c->base + 3; }
static inline uint16_t ata_reg_lba_mid (ata_channel_t *c) { return c->base + 4; }
static inline uint16_t ata_reg_lba_hi  (ata_channel_t *c) { return c->base + 5; }
static inline uint16_t ata_reg_drive   (ata_channel_t *c) { return c->base + 6; }
static inline uint16_t ata_reg_status  (ata_channel_t *c) { return c->base + 7; }
static inline uint16_t ata_reg_command (ata_channel_t *c) { return c->base + 7; }

static inline uint16_t ata_reg_devctrl (ata_channel_t *c) { return c->ctrl + 0; }
static inline uint16_t ata_reg_altstat (ata_channel_t *c) { return c->ctrl + 0; }

int cd_test_iso9660(void);

static void ata_delay(ata_channel_t *c) {
    inb(ata_reg_altstat(c));
    inb(ata_reg_altstat(c));
    inb(ata_reg_altstat(c));
    inb(ata_reg_altstat(c));
}

static void ata_select_drive(ata_channel_t *c, uint8_t drive_sel) {
    outb(ata_reg_drive(c), drive_sel);
    ata_delay(c);
}

static int ata_wait_not_busy(ata_channel_t *c) {
    uint8_t st;
    do {
        st = inb(ata_reg_status(c));
    }   while (st & ATA_SR_BSY);
    return 0;
}

static int ata_wait_drq_or_err(ata_channel_t *c) {
    for (;;) {
        uint8_t st = inb(ata_reg_status(c));
        if (st & ATA_SR_ERR) return -1;
        if ((st & (ATA_SR_BSY | ATA_SR_DRQ)) == ATA_SR_DRQ)
            return 0;
    }
}

/*
static int atapi_check_device(ata_channel_t *c, uint8_t drive_sel) {
    ata_select_drive(c, drive_sel);

    ata_delay(c);

    // "Device presence" / Signatur-Check
    outb(ata_reg_seccnt(c), 0);
    outb(ata_reg_lba_lo(c), 0);
    outb(ata_reg_lba_mid(c), 0);
    outb(ata_reg_lba_hi(c), 0);

    ata_delay(c);

    uint8_t mid = inb(ata_reg_lba_mid(c));
    uint8_t hi  = inb(ata_reg_lba_hi(c));

    // ATAPI-Signatur: 0x14 / 0xEB
    if (mid == 0x14 && hi == 0xEB) {
        printformat("ATAPI gefunden auf %s %s (MID=0x%x HI=0x%x)\n",
                (c->base == 0x1F0) ? "Primary" : "Secondary",
                (drive_sel == ATA_DRIVE_MASTER) ? "Master" : "Slave",
                mid, hi);
        return 0;
    }

    return -1;
}*/

static int atapi_try_identify(ata_channel_t *c, uint8_t drive_sel) {
    uint16_t buf[256];

    ata_select_drive(c, drive_sel);
    ata_delay(c);

    // Register auf 0
    outb(ata_reg_seccnt(c), 0);
    outb(ata_reg_lba_lo(c), 0);
    outb(ata_reg_lba_mid(c), 0);
    outb(ata_reg_lba_hi(c), 0);

    // IDENTIFY PACKET (ATAPI)
    outb(ata_reg_command(c), 0xA1);
    ata_delay(c);

    uint8_t st = inb(ata_reg_status(c));
    if (st == 0x00 || st == 0xFF) {
        // kein Device an diesem Port
        printformat("IDENTIFY PACKET: kein Device (status=0x%x)\n", st);
        return -1;
    }

    // Warten auf DRQ oder ERR
    if (ata_wait_drq_or_err(c) < 0) {
        printformat("IDENTIFY PACKET: Fehler (kein DRQ, status=0x%x)\n", inb(ata_reg_status(c)));
        return -1;
    }

    // 256 Wörter lesen
    for (int i = 0; i < 256; ++i)
        buf[i] = inw(ata_reg_data(c));

    uint16_t cfg = buf[0];
    uint8_t devtype = (cfg >> 8) & 0x1F;
    
    printformat("IDENTIFY PACKET: cfg=0x%x devtype=%x\n", cfg, devtype);
    
    // Bit 15: 0 = Device ok, 1 = kein Device
    if (cfg & 0x8000) {
        printformat("IDENTIFY PACKET: kein gueltiges Device (cfg bit15=1)\n");
        return -1;
    }
    
    // devtype 5 = CDROM (typisch bei QEMU)
    if (devtype == 5) {
        printformat("ATAPI-CDROM erkannt!\n");
        return 0;
    }
    
    // zur Not: alles, was auf 0xA1 sauber antwortet, akzeptieren
    printformat("Gerät antwortet auf IDENTIFY PACKET, devtype=%u (trotzdem als ATAPI akzeptiert)\n",
            devtype);
    return 0;
}

static int cd_detect_any(void)
{
    for (int ch = 0; ch < 2; ++ch) {
        ata_channel_t *c = &g_channels[ch];

        printformat("Pruefe Channel %d (base=0x%x ctrl=0x%x)\n", ch, c->base, c->ctrl);

        // Master
        printformat("  -> Master\n");
        if (atapi_try_identify(c, ATA_DRIVE_MASTER) == 0) {
            g_cd.chan = c;
            g_cd.drive_sel = ATA_DRIVE_MASTER;
            g_cd.present = 1;
            return 0;
        }

        // Slave
        printformat("  -> Slave\n");
        if (atapi_try_identify(c, ATA_DRIVE_SLAVE) == 0) {
            g_cd.chan = c;
            g_cd.drive_sel = ATA_DRIVE_SLAVE;
            g_cd.present = 1;
            return 0;
        }
    }

    printformat("Kein ATAPI-Geraet gefunden.\n");
    g_cd.present = 0;
    return -1;
}

static int atapi_identify(void) {
    if (!g_cd.present) return -1;

    ata_channel_t *c = g_cd.chan;
    uint16_t buffer[256];

    ata_select_drive(c, g_cd.drive_sel);
    ata_wait_not_busy(c);

    outb(ata_reg_command(c), 0xA1);  // IDENTIFY PACKET
    ata_delay(c);

    uint8_t st = inb(ata_reg_status(c));
    if (st == 0) {
        printformat("IDENTIFY PACKET: kein Device antwortet\n");
        return -1;
    }

    if (ata_wait_drq_or_err(c) < 0) {
        printformat("IDENTIFY PACKET: Fehler (kein DRQ)\n");
        return -1;
    }

    for (int i = 0; i < 256; ++i) {
        buffer[i] = inw(ata_reg_data(c));
    }

    printformat("IDENTIFY PACKET: ok (erste Woerter: 0x%x 0x%x)\n",
            buffer[0], buffer[1]);
    return 0;
}

static void atapi_build_read12(uint8_t *packet, uint32_t lba, uint32_t count)
{
    for (int i = 0; i < 12; ++i) packet[i] = 0;
    packet[0] = 0xA8;   // READ(12) Opcode

    // LBA
    packet[2] = (lba >> 24) & 0xFF;
    packet[3] = (lba >> 16) & 0xFF;
    packet[4] = (lba >> 8)  & 0xFF;
    packet[5] = (lba)       & 0xFF;

    // Transfer length (in Blöcken)
    packet[6] = (count >> 24) & 0xFF;
    packet[7] = (count >> 16) & 0xFF;
    packet[8] = (count >> 8)  & 0xFF;
    packet[9] = (count)       & 0xFF;
}

int atapi_read_sector(uint32_t lba, void *buffer) {
    if (!g_cd.present) return -1;

    ata_channel_t *c = g_cd.chan;
    uint8_t packet[12];

    ata_select_drive(c, g_cd.drive_sel);
    ata_wait_not_busy(c);

    // Byte Count = 2048
    outb(ata_reg_feature(c), 0x00);
    outb(ata_reg_lba_mid(c), (uint8_t)(ATAPI_SECTOR_SIZE & 0xFF));
    outb(ata_reg_lba_hi(c),  (uint8_t)((ATAPI_SECTOR_SIZE >> 8) & 0xFF));

    // PACKET
    outb(ata_reg_command(c), 0xA0);
    ata_delay(c);

    if (ata_wait_drq_or_err(c) < 0) {
        printformat("cd_read_sector: DRQ kam nicht (Packet Phase)\n");
        return -1;
    }

    atapi_build_read12(packet, lba, 1);

    uint16_t *pw = (uint16_t *)packet;
    for (int i = 0; i < 6; ++i) {
        outw(ata_reg_data(c), pw[i]);
    }

    if (ata_wait_drq_or_err(c) < 0) {
        printformat("cd_read_sector: DRQ kam nicht (Data Phase)\n");
        return -1;
    }

    uint16_t *bufw = (uint16_t *)buffer;
    for (int i = 0; i < ATAPI_SECTOR_SIZE / 2; ++i) {
        bufw[i] = inw(ata_reg_data(c));
    }

    uint8_t st = inb(ata_reg_status(c));
    if (st & ATA_SR_ERR) {
        printformat("cd_read_sector: Status ERR=1\n");
        return -1;
    }

    return 0;
}

int atapi_read_sectors(uint32_t lba, uint32_t count, void *buffer) {
    uint8_t *p = (uint8_t *)buffer;
    for (uint32_t i = 0; i < count; ++i) {
        if (atapi_read_sector(lba + i, p + i * ATAPI_SECTOR_SIZE) != 0) {
            return -1;
        }
    }
    return 0;
}

int check_atapi(void)
{
    settextcolor(14,0);
    printformat("cd_init: suche ATAPI-Laufwerk...\n");

    if (cd_detect_any() != 0) {
        return -1;
    }
    
    if (atapi_identify() != 0) {
        printformat("Warnung: IDENTIFY PACKET fehlgeschlagen, versuche trotzdem Reads\n");
    }

    printformat("cd_init: ATAPI-CD auf base=0x%X drive_sel=0x%X\n",
            g_cd.chan->base, g_cd.drive_sel);
    printformat("cd_init: fertig\n");
    return 0;
}

int cd_test_iso9660(void) {
    uint8_t sector[ATAPI_SECTOR_SIZE];

    settextcolor(14,0);
    printformat("cd_test_iso9660: lese LBA 16...\n");
    // todo: atapi or ahci check
    if (atapi_read_sector(16, sector) != 0) {
        printformat("cd_test_iso9660: lesen fehlgeschlagen\n");
        return -1;
    }   else {
        printformat("cd_test_iso: lesen ok\n");
        //return 0;
    }

    // PVD: sector[0] = Type (1 = Primary Volume Descriptor)
    //       sector[1..5] = 'C','D','0','0','1'
    uint8_t type = sector[0];
    char id[6];
    for (int i = 0; i < 5; ++i) {
        id[i] = sector[1 + i];
    }
    id[5] = '\0';

    printformat("cd_test_iso9660: type=%u id=\"%s\"\n", (unsigned)type, id);

    if (type == 1 &&
        id[0] == 'C' && id[1] == 'D' && id[2] == '0' &&
        id[3] == '0' && id[4] == '1') {

        printformat("cd_test_iso9660: ISO9660-CD erkannt!\n");
        return 0;
    }

    printformat("cd_test_iso9660: kein ISO9660-Primary-Descriptor gefunden\n");
    return -1;
}
