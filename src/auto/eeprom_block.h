#ifndef FN_EEPROM_BLOCK_H
#define FN_EEPROM_BLOCK_H
/* Shared translations of the main-CPU serial settings-device helpers used by
 * the boot/configuration routines 0x004C42 and 0x00509A (and by 0x0050DC via
 * 0x004DEE):
 *   0x0047C0  eeprom_xfer        clock out a command word, receive 16 bits
 *   0x0046C0  eeprom_block_write write+verify the 0x80-byte block
 *   0x00466E  eeprom_block_read  read the 0x80-byte block into the RAM image
 *   0x004DEE  settings_refresh   both checksums, then the write-back
 *   0x01014E  the reset sequence that the write-back failure enters
 * Latch 0x406C52 (Work RAM), status 0x406C50, command port 0x4A0013 and reply
 * bit 0x4A0001. The harness has no I/O space, so a reply read returns the
 * zeroed unmapped byte (f3mem.h). */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_1194C2.h"
#include "../video/sprite.h"

#define EE_LATCH  0x00406C52u
#define EE_SR     0x00406C50u
#define EE_PORT   0x004A0013u
#define EE_IN     0x004A0001u
#define EE_SETTINGS 0x00406C54u

/* ROM 0x0047C0: clock D1+1 command bits of D0 out of the latch byte 0x406C52
 * and, when the two shifted copies of the command set bit 31, receive 16 reply
 * bits into D0. SR is modelled as 0x2000 (supervisor, IPL 0). */
static inline uint32_t ee_xfer(const f3mem_t *m, uint32_t d0, uint16_t *d1p)
{
    uint8_t latch = rd8(m, EE_LATCH);
    uint16_t d1 = *d1p;
    int carry;

    wr16(m, EE_SR, 0x2000u);                      /* 0x47C0 move.w SR,0x406C50 */
    for (;;) {
        if (d0 & (1u << (d1 & 31u))) {            /* 0x47CA btst.l D1,D0 */
            latch = (uint8_t)(latch & (uint8_t)~0x08u);   /* 0x47EA bclr.b #3 */
            wr8(m, EE_LATCH, latch);
            latch = (uint8_t)(latch | 0x04u);             /* 0x47F2 bset.b #2 */
            wr8(m, EE_LATCH, latch);
        } else {
            latch = (uint8_t)(latch & (uint8_t)~0x08u);   /* 0x47CE bclr.b #3 */
            wr8(m, EE_LATCH, latch);
            latch = (uint8_t)(latch & (uint8_t)~0x04u);   /* 0x47D6 bclr.b #2 */
            wr8(m, EE_LATCH, latch);
        }
        /* 0x47DE / 0x47FA: move.b 0x406C52,0x4A0013 (write-only port). */
        latch = (uint8_t)(latch | 0x10u);         /* 0x4804 bset.b #4 */
        wr8(m, EE_LATCH, latch);
        /* 0x480C: port write. */
        latch = (uint8_t)(latch | 0x08u);         /* 0x481C bset.b #3 */
        wr8(m, EE_LATCH, latch);
        /* 0x4824: port write. */
        d1 = (uint16_t)(d1 - 1u);                 /* 0x482E dbf D1 */
        if (d1 != 0xFFFFu)
            continue;
        break;
    }
    *d1p = d1;

    carry = (int)((d0 >> 31) & 1u);
    d0 <<= 1;                                     /* 0x4832 add.l D0,D0 */
    if (!carry) {
        carry = (int)((d0 >> 31) & 1u);
        d0 <<= 1;                                 /* 0x4886 add.l D0,D0 */
        if (carry) {
            latch = (uint8_t)(latch & (uint8_t)~0x08u);  /* 0x488A bclr.b #3 */
            wr8(m, EE_LATCH, latch);
            latch = (uint8_t)(latch & (uint8_t)~0x04u);  /* 0x4892 bclr.b #2 */
            wr8(m, EE_LATCH, latch);
            latch = (uint8_t)(latch & (uint8_t)~0x10u);  /* 0x489A bclr.b #4 */
            wr8(m, EE_LATCH, latch);
            /* 0x48A2: port write. */
            latch = (uint8_t)(latch | 0x10u);            /* 0x48AC bset.b #4 */
            wr8(m, EE_LATCH, latch);
            /* 0x48B4: port write. */
            d1 = 0x270Fu;                                /* 0x48BE move.w #0x270F */
            while (!(rd8(m, EE_IN) & 0x01u)) {           /* 0x48C2 btst.b #0 */
                d1 = (uint16_t)(d1 - 1u);
                if (d1 == 0xFFFFu)
                    break;
            }
            *d1p = d1;
        } else {
            /* 0x48D4: clock burst without a receive. Port write after each. */
            latch = (uint8_t)(latch & (uint8_t)~0x10u); wr8(m, EE_LATCH, latch); /* 0x48D4 */
            latch = (uint8_t)(latch & (uint8_t)~0x08u); wr8(m, EE_LATCH, latch); /* 0x48E6 */
            latch = (uint8_t)(latch | 0x08u);           wr8(m, EE_LATCH, latch); /* 0x4902 */
            latch = (uint8_t)(latch & (uint8_t)~0x08u); wr8(m, EE_LATCH, latch); /* 0x491E */
            latch = (uint8_t)(latch | 0x08u);           wr8(m, EE_LATCH, latch); /* 0x493A */
            latch = (uint8_t)(latch & (uint8_t)~0x08u); wr8(m, EE_LATCH, latch); /* 0x4956 */
            latch = (uint8_t)(latch | 0x08u);           wr8(m, EE_LATCH, latch); /* 0x4972 */
        }
        goto tail;
    }

    /* 0x4836: receive path - D0 = 0x00010000, clock in one bit per shift. */
    latch = (uint8_t)(latch & (uint8_t)~0x08u);   /* 0x4836 bclr.b #3 */
    wr8(m, EE_LATCH, latch);
    /* 0x483E: port write. */
    d0 = 0x00010000u;                             /* 0x4848 moveq #1 ; 0x484A swap */
    for (;;) {
        latch = (uint8_t)(latch | 0x08u);         /* 0x484C bset.b #3 */
        wr8(m, EE_LATCH, latch);
        /* 0x4854: port write. */
        latch = (uint8_t)(latch & (uint8_t)~0x08u); /* 0x485E bclr.b #3 */
        wr8(m, EE_LATCH, latch);
        /* 0x4866: port write. */
        if (rd8(m, EE_IN) & 0x01u)                /* 0x4870 btst.b #0 */
            d0 += 1u;                             /* 0x487A addq.l #1 */
        carry = (int)((d0 >> 31) & 1u);
        d0 <<= 1;                                 /* 0x487C add.l D0,D0 */
        if (!carry)
            continue;
        d0 = (uint32_t)((int32_t)d0 >> 1);        /* 0x4880 asr.l #1 */
        break;
    }

tail:
    latch = (uint8_t)(latch & (uint8_t)~0x10u);   /* 0x498E bclr.b #4 */
    wr8(m, EE_LATCH, latch);
    /* 0x4996: port write. */
    wr16(m, EE_SR, 0x2000u);                      /* 0x49A0 move.w 0x406C50,SR */
    return d0;
}

/* ROM 0x0046C0: write the 0x80 bytes ending at arg (0x40 words, walked
 * backwards) and verify every word with a read-back. D0 = 0 when all 0x40
 * words read back, 0xFF after the first that does not; the closing 0x100
 * command is always clocked out. D1 is left as the receive loop leaves it. */
static inline uint32_t ee_block_write(const f3mem_t *m, uint32_t arg,
                                      uint32_t *d1_io)
{
    uint32_t a0 = arg + 0x80u;                    /* 0x46D6..0x46DA */
    int16_t d2 = 0x3F;                            /* 0x46DE moveq #0x3F */
    uint32_t d0 = 0;
    uint16_t d1;

    for (;;) {
        d1 = 9;                                   /* 0x46E8 moveq #9 */
        d0 = ee_xfer(m, 0x80000180u | (uint16_t)d2, &d1); /* 0x46E0..0x46EE */
        a0 -= 2u;                                 /* 0x46F4 cmp.w -(A0),D0 */
        if ((uint16_t)d0 != rd16(m, a0)) {
            d1 = 9;                               /* 0x46F8/0x4700 */
            (void)ee_xfer(m, 0x400001C0u | (uint16_t)d2, &d1);
            d0 = ((uint32_t)(uint16_t)d2 << 16) | (uint32_t)rd16(m, a0); /* 0x470C..0x4710 */
            d0 |= 0x41400000u;                    /* 0x4712 ori.l #0x41400000 */
            d1 = 0x19;                            /* 0x4718 moveq #0x19 */
            (void)ee_xfer(m, d0, &d1);            /* 0x471A..0x4722 */
            d1 = 9;                               /* 0x472C */
            d0 = ee_xfer(m, 0x80000180u | (uint16_t)d2, &d1); /* 0x4724..0x4734 */
            if ((uint16_t)d0 != rd16(m, a0)) {    /* 0x4738 cmp.w (A0),D0 */
                d0 = 0xFFu;                       /* 0x473C moveq #0xFF */
                break;
            }
        }
        d2 = (int16_t)(d2 - 1);                   /* 0x4740 dbra D2 */
        if (d2 != -1)
            continue;
        d0 = 0;                                   /* 0x4744 moveq #0 */
        break;
    }

    uint32_t result = d0;                         /* 0x4746 movea.l D0,A0 */
    d1 = 9;                                       /* 0x474E moveq #9 */
    (void)ee_xfer(m, 0x100u, &d1);                /* 0x4748..0x4756 */
    *d1_io = d1;                                  /* dbf leaves 0xFFFF */
    return result;                                /* 0x475A move.l A0,D0 */
}

/* ROM 0x00466E: read the 0x80 bytes ending at arg from the device, storing the
 * 0x40 reply words backwards (0x4684..0x46A6), then clock the closing 0x100
 * command. Returns the last reply (D0). */
static inline uint32_t ee_block_read(const f3mem_t *m, uint32_t arg,
                                     uint32_t *d1_io)
{
    uint32_t a0 = arg + 0x80u;                    /* 0x4684..0x4688 */
    int16_t d2 = 0x3F;                            /* 0x468C moveq #0x3F */
    uint32_t d0;
    uint16_t d1 = 9;

    (void)ee_xfer(m, 0x130u, &d1);                /* 0x4672..0x467E header */
    for (;;) {
        d1 = 9;                                   /* 0x4696 */
        d0 = ee_xfer(m, 0x80000180u | (uint16_t)d2, &d1); /* 0x468E..0x469C */
        a0 -= 2u;                                 /* 0x46A2 move.w D0,-(A0) */
        wr16(m, a0, (uint16_t)d0);
        d2 = (int16_t)(d2 - 1);                   /* 0x46A4 dbra D2 */
        if (d2 == -1)
            break;
    }
    d1 = 9;                                       /* 0x46AE moveq #9 */
    d0 = ee_xfer(m, 0x100u, &d1);                 /* 0x46A8..0x46B4 */
    *d1_io = d1;
    return d0;                                    /* 0x46BA movem ; rts */
}

/* ROM 0x01014E: soft-reset sequence - clear the cache register and the two
 * control bytes, clear the sprite and layer memories, then spin forever at
 * 0x010170. The endless loop cannot be modelled here, so the caller gets -1. */
static inline int reset_loop_1014E(const f3mem_t *m)
{
    wr8(m, 0x004A0004u, 0u);                      /* 0x010158 */
    wr8(m, 0x004A0014u, 0u);                      /* 0x010160 */
    Sprite_ClearAll(m);                           /* 0x010168 -> 0x00117E */
    Gfx_ClearAllLayers(m);                        /* 0x01016C -> 0x005A12 */
    return -1;                                    /* 0x010170 bra.b 0x010170 */
}

/* ROM 0x004DEE: recompute the two settings checksums through 0x004BD2 and
 * write the 0x80-byte block back through 0x0046C0. A non-zero write result
 * clears all layers, draws the reset message and enters 0x01014E. */
static inline int settings_refresh_4DEE(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, 0x20u);                       /* 0x004DF6 pea (0x20).w */
    push_long(m, r, EE_SETTINGS);                 /* 0x004DFA move.l A2,-(SP) */
    push_long(m, r, 0x004E00u);                   /* 0x004DFC jsr 0x004BD2 */
    Fn_004BD2(m, r);
    push_long(m, r, 0x60u);                       /* 0x004E00 pea (0x60).w */
    push_long(m, r, EE_SETTINGS + 0x20u);         /* 0x004E04 pea (0x20,A2) */
    push_long(m, r, 0x004E0Cu);                   /* 0x004E08 jsr 0x004BD2 */
    Fn_004BD2(m, r);
    push_long(m, r, EE_SETTINGS);                 /* 0x004E0C move.l A2,-(SP) */
    r->d[0] = ee_block_write(m, EE_SETTINGS, &r->d[1]); /* 0x004E0E jsr 0x0046C0 */
    r->a[7] += 0x14u;                             /* 0x004E14 lea (0x14,SP),SP */
    r->a[0] = r->d[0];                            /* 0x4746 movea.l D0,A0 */

    if ((int32_t)r->d[0] != 0) {                  /* 0x004E18 tst.l D0 / beq */
        Gfx_ClearAllLayers(m);                    /* 0x004E1C jsr 0x005A12 */
        push_word(m, r, 8);                       /* 0x004E22 move.w #8,-(SP) */
        push_long(m, r, 0x0061C81Au);             /* 0x004E26 pea */
        push_long(m, r, 0x001A1C36u);             /* 0x004E2C pea */
        push_long(m, r, 0x004E38u);
        r->d[0] = Text_DrawString(m, 0x001A1C36u, 0x0061C81Au, 8u); /* 0x004E32 */
        r->a[7] += 0x0Au;                         /* 0x004E38 lea (0xa,SP),SP */
        return reset_loop_1014E(m);               /* 0x004E3C jsr 0x01014E */
    }
    return 0;                                     /* 0x004E42 movea.l (SP)+,A2 */
}

#endif
