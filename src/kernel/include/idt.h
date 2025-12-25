// idt.h
#pragma once
#include <stdint.h>

// Ein Eintrag in der IDT (Gate Descriptor)
typedef struct {
    uint16_t base_low;   // niedriges 16-bit der Handler-Adresse
    uint16_t sel;        // Code-Segment-Selector (z.B. 0x08)
    uint8_t  always0;    // immer 0
    uint8_t  flags;      // Typ + DPL + Present
    uint16_t base_high;  // hohes 16-bit der Handler-Adresse
}   __attribute__((packed)) idt_entry_t;

// Pointer auf die IDT (für lidt)
typedef struct {
    uint16_t limit;
    uint32_t base;
}   __attribute__((packed)) idt_ptr_t;

//extern void idt_init(void);
extern void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
