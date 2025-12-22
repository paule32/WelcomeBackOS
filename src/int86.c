// int86.c
# include "int86.h"
# include "stdint.h"

#define INT86_MAILBOX_ADDR 0x00006000u
#define INT86_BLOB_ADDR    0x00007000u

typedef struct Int86Mailbox {
    uint8_t  intno;
    uint8_t  status;     // 0=ok, 1=unsupported int
    uint16_t _pad;

    REGS16   in;
    REGS16   out;
} Int86Mailbox;

static volatile Int86Mailbox *const mb = (volatile Int86Mailbox*)INT86_MAILBOX_ADDR;

extern uint8_t _int86_blob_start[];
extern uint8_t _int86_blob_end[];

static void memcpy8(void *dst, const void *src, uint32_t n) {
    uint8_t *d = (uint8_t*)dst;
    const uint8_t *s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
}

static inline void int86_enter_blob(void) {
    void (*blob)(void) = (void(*)(void))INT86_BLOB_ADDR;
    blob();
}

int int86(uint8_t intno, const REGS16 *in, REGS16 *out)
{
    mb->intno = intno;
    mb->status = 0;
    mb->in = *in;

    uint32_t sz = (uint32_t)(_int86_blob_end - _int86_blob_start);
    memcpy8((void*)INT86_BLOB_ADDR, _int86_blob_start, sz);

    int86_enter_blob();

    *out = mb->out;

    // Carry flag (bit0) signalisiert meist Fehler
    // zusätzlich: unsupported int => status!=0
    if (mb->status != 0) return -2;
    return (out->flags & 0x0001) ? -1 : 0;
}
