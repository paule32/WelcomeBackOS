// ----------------------------------------------------------------------------
// \file  rtc.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"

namespace tvision
{
    static inline void outb(uint16_t port, uint8_t val) {
        __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
    }
    static inline uint8_t inb(uint16_t port) {
        uint8_t ret;
        __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }

    # define CMOS_ADDR 0x70
    # define CMOS_DATA 0x71

    static inline uint8_t cmos_read(uint8_t reg) {
        // NMI disable (bit7=1) + reg
        outb(CMOS_ADDR, (uint8_t)(0x80 | reg));
        return inb(CMOS_DATA);
    }

    static inline bool rtc_update_in_progress(void) {
        return (cmos_read(0x0A) & 0x80) != 0;
    }

    static inline uint8_t bcd_to_bin(uint8_t bcd) {
        return (uint8_t)((bcd & 0x0F) + ((bcd >> 4) * 10));
    }

    typedef struct {
        uint8_t h, m, s;
    } rtc_time_t;

    static rtc_time_t rtc_read_time_once(void) {
        rtc_time_t t;

        // warten bis RTC nicht gerade updated
        while (rtc_update_in_progress()) { /* spin */ }

        uint8_t sec  = cmos_read(0x00);
        uint8_t min  = cmos_read(0x02);
        uint8_t hour = cmos_read(0x04);
        uint8_t regB = cmos_read(0x0B);

        bool is_binary = (regB & 0x04) != 0;
        bool is_24h    = (regB & 0x02) != 0;

        // Wenn BCD: konvertieren (Achtung: hour hat ggf. PM-Bit bei 12h)
        if (!is_binary) {
            sec = bcd_to_bin(sec);
            min = bcd_to_bin(min);

            // Bei 12h hat hour Bit7 = PM-Flag, die unteren Bits sind BCD
            uint8_t pm = hour & 0x80;
            hour = bcd_to_bin((uint8_t)(hour & 0x7F));
            hour |= pm; // PM-Flag wieder dran, falls 12h
        }

        // 12h -> 24h umrechnen, falls nötig
        if (!is_24h) {
            bool pm = (hour & 0x80) != 0;
            hour = (uint8_t)(hour & 0x7F);

            if (pm) {
                // 12 PM -> 12, 1..11 PM -> +12
                if (hour != 12) hour = (uint8_t)(hour + 12);
            } else {
                // 12 AM -> 0
                if (hour == 12) hour = 0;
            }
        }

        t.h = hour;
        t.m = min;
        t.s = sec;
        return t;
    }

    static bool rtc_time_equal(rtc_time_t a, rtc_time_t b) {
        return a.h == b.h && a.m == b.m && a.s == b.s;
    }

    // Stabile Zeit lesen: zweimal lesen und vergleichen
    rtc_time_t rtc_read_time_stable(void) {
        rtc_time_t t1 = rtc_read_time_once();
        rtc_time_t t2 = rtc_read_time_once();
        while (!rtc_time_equal(t1, t2)) {
            t1 = t2;
            t2 = rtc_read_time_once();
        }
        return t2;
    }

    // Format "HH:mm:ss" (buf muss mind. 9+1 = 10 Bytes haben)
    void rtc_format_hhmmss(char *buf10) {
        rtc_time_t t = rtc_read_time_stable();

        buf10[0] = (char)('0' + (t.h / 10));
        buf10[1] = (char)('0' + (t.h % 10));
        buf10[2] = ':';
        buf10[3] = (char)('0' + (t.m / 10));
        buf10[4] = (char)('0' + (t.m % 10));
        buf10[5] = ':';
        buf10[6] = (char)('0' + (t.s / 10));
        buf10[7] = (char)('0' + (t.s % 10));
        buf10[8] = '\0';
    }
}   // namespace: tvision
