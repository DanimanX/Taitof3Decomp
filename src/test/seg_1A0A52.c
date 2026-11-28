/* ROM 0x1A0A52 test-mode screen "CONFIGURATION" (menu entry 3 of the
 * Task_TestModeMenu table at 0x1A2350), in segments (src/kernel/seg.h). It
 * yields with jsr 0x001F62 (Sys_Yield call gate) at 0x1A0AC0 (frame loop),
 * 0x1A118A (leave the editor, clear the layers) and 0x1A11CE (the confirm
 * screen); the resumes are 0x1A0AC6, 0x1A1190 and 0x1A11D4. The screen ends at
 * the RTS at 0x1A1280.
 *
 * Entry: link.w a6,#-0x12, save D2-D7/A2-A4, A2 = the 0x80-byte settings block
 * 0x406C54, A4 = 0x5666, D4 = the text base 0x61C000, D6 = the P1 switch byte
 * 0x402224, D7 = the shared editor 0x19FFF0, twelve rows (-0xE(A6) = 0xC),
 * selection D3 = 0, previous selection -0xA(A6) = 0, the value copied from
 * 0x402240 at -0x6(A6), the exit flag byte -0x2(A6) = 0. It clears the layers,
 * reloads the settings with 0x4D06, loads the character set with 0x1A006C and
 * draws the title "CONFIGURATION" at 0x61C09C.
 *
 * Each 0x1A0AC6 frame walks D3 with the P2 switch byte 0x402225 (bit 1 down,
 * bit 0 up, wrapping through the row count) and draws the twelve rows at
 * 0x61C000 + 0x20E + row*0x100 (rows 8 and 10 continue the COIN-A/COIN-B lines
 * at 0x61C93C and 0x61CA3C): the label, then the value in colour 8 for the
 * selected row and colour 7 for the rest. The selected row first runs its
 * settings field through the editor at 0x19FFF0 and writes the result back
 * with bfins. The bit fields use the 68020 bitfield encoding: offset in
 * extension bits 11-6 counted from the MSB of the byte at the address, width in
 * bits 5-0 (so {0:1} is btst #7 of that byte). The row's "EXIT" line sets the
 * exit flag byte when bit 4 of 0x402225 is pressed. When the flag is set the
 * frame clears D3 and the flag, clears the layers and yields; the next segment
 * (0x1A1190) draws the confirm screen ("SAVE MODIFIED DATA" / "ARE YOU SURE?")
 * and yields again, and 0x1A11D4 runs the YES/NO editor. YES writes the block
 * back through 0x4DEE, which cannot succeed without the serial device and
 * enters the reset loop, so that arm returns SEG_LEAVE; NO reloads through
 * 0x4D06 and both arms clear the layers, restore and return. */
#include "../kernel/seg.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (ROM 0x005666) */
#include "../video/sprite.h"        /* Gfx_ClearAllLayers (ROM 0x005A12) */
#include "../auto/fn_auto.h"        /* Fn_1A006C, Fn_004C78, Fn_004C12 */
#include "../auto/eeprom_block.h"   /* settings_refresh_4DEE (ROM 0x004DEE) */

#define SEG_YIELD  0x001A0AC0u      /* jsr 0x001F62, the frame loop */
#define SEG_YIELD2 0x001A118Au      /* jsr 0x001F62, leave the editor */
#define SEG_YIELD3 0x001A11CEu      /* jsr 0x001F62, the confirm screen */
#define SETTINGS   0x00406C54u      /* the 0x80-byte settings block */
#define P1_SWITCH  0x00402224u
#define P2_SWITCH  0x00402225u

/* 0x1A0A52 link.w a6,#-0x12 ; 0x1A0A56 movem.l d2-d7/a2-a4,-(SP) */
static void save_frame(const f3mem_t *m, f3regs_t *r)
{
    r->a[7] -= 4u;
    wr32(m, r->a[7], r->a[6]);
    r->a[6] = r->a[7];
    r->a[7] -= 0x12u;
    r->a[7] -= 36u;
    for (unsigned i = 0; i < 6u; ++i)
        wr32(m, r->a[7] + 4u * i, r->d[2u + i]);
    for (unsigned i = 0; i < 3u; ++i)
        wr32(m, r->a[7] + 24u + 4u * i, r->a[2u + i]);
}

/* 0x1A127A movem.l (SP)+,d2-d7/a2-a4 ; 0x1A127E unlk a6 ; 0x1A1280 rts */
static uint32_t restore_frame(const f3mem_t *m, f3regs_t *r)
{
    for (unsigned i = 0; i < 6u; ++i)
        r->d[2u + i] = rd32(m, r->a[7] + 4u * i);
    for (unsigned i = 0; i < 3u; ++i)
        r->a[2u + i] = rd32(m, r->a[7] + 24u + 4u * i);
    r->a[7] += 36u;
    r->a[7] = r->a[6];                      /* unlk a6 */
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return Seg_Run(m, r, seg_pop32(m, r));  /* rts */
}

/* jsr (A4) -> 0x5666: the three arguments stay on the stack until the
 * caller's lea, the callee's frame and return PC are written below them, and
 * only D0 (the end of the line) survives. The colour is the pushed immediate
 * (D2 for the menu rows). jsr_pc is the two-byte jsr, so its return PC is
 * jsr_pc+2. */
static void draw_text1(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc,
                       uint32_t dst, uint32_t str, uint16_t col)
{
    uint32_t sp = r->a[7] - 10u;
    r->a[7] = sp;
    wr32(m, sp, str);
    wr32(m, sp + 4u, dst);
    wr16(m, sp + 8u, col);
    wr32(m, sp - 4u, jsr_pc + 2u);
    wr32(m, sp - 8u, r->a[6]);
    wr32(m, sp - 20u, r->d[7]);
    wr32(m, sp - 16u, r->a[0]);
    wr32(m, sp - 12u, r->a[1]);
    r->d[0] = Text_DrawString(m, str, dst, col);
    r->a[7] = sp;
}

/* jsr 0x1A1BA6.l (six bytes) with the colour in D2 and one long argument: the
 * caller pushes the argument first, then fmt/col/dst and the return PC (see
 * seg_1A1A26). */
static void draw_fmt1(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc,
                      uint32_t dst, uint32_t fmt)
{
    r->a[7] -= 4u;
    wr32(m, r->a[7], fmt);
    r->a[7] -= 2u;
    wr16(m, r->a[7], (uint16_t)r->d[2]);
    r->a[7] -= 4u;
    wr32(m, r->a[7], dst);
    r->a[7] -= 4u;
    wr32(m, r->a[7], jsr_pc + 6u);
    Fn_1A1BA6(m, r);                        /* leaves SP at dst */
}

/* 0x19FFF0: the shared value editor (see seg_1A1944.c). The five argument
 * words and the callee's frame stay on the stack for the caller's
 * lea 0xA(SP); D0.w is the new value. jsr_pc is the two-byte jsr (A0) the row
 * does after loading D7 into A0, so its return PC is jsr_pc+2. */
static void edit_value(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc,
                       uint16_t in, uint16_t cur, uint16_t lo, uint16_t hi,
                       uint16_t step)
{
    uint32_t sp = r->a[7] - 10u;
    r->a[7] = sp;
    wr16(m, sp, in);
    wr16(m, sp + 2u, cur);
    wr16(m, sp + 4u, lo);
    wr16(m, sp + 6u, hi);
    wr16(m, sp + 8u, step);
    wr32(m, sp - 4u, jsr_pc + 2u);
    wr32(m, sp - 8u, r->d[5]);              /* movem.l d3-d5,-(SP) */
    wr32(m, sp - 12u, r->d[4]);
    wr32(m, sp - 16u, r->d[3]);

    uint16_t d3;
    r->d[5] = (r->d[5] & 0xFFFF0000u) | in;
    r->d[1] = (r->d[1] & 0xFFFF0000u) | cur;
    r->a[0] = 0x0040CBD0u;

    r->d[0] = (r->d[0] & 0xFFFF0000u) | in;         /* 0x1A0002 move.w D5w,D0w */
    if ((r->d[0] & 4u) != 0u) {                     /* andi.w #4,D0w */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | in;     /* 0x1A000A */
        if ((r->d[0] & 8u) != 0u) {                 /* andi.w #8,D0w */
            wr16(m, 0x0040CBD0u, 0u);               /* 0x1A0062 clr.w (A0) */
            goto done;
        }
    }
    if (rd16(m, 0x0040CBD0u) != 0u && rd16(m, 0x0040CBD0u) <= 0x14u)
        goto bump;                                  /* 0x1A0016 ble 0x1A0058 */
    d3 = rd16(m, 0x0040CBD0u) >= 0x78u ? 0xAu : 1u; /* 0x1A001C cmpi.w #0x78 */
    for (uint32_t d4 = 0;; ) {
        if ((r->d[5] & 8u) != 0u)
            r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)((r->d[1] - step) & 0xFFFFu);
        else
            r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)((r->d[1] + step) & 0xFFFFu);
        if ((int16_t)r->d[1] > (int16_t)hi)         /* 0x1A003E ble */
            r->d[1] = (r->d[1] & 0xFFFF0000u) | hi;
        if ((int16_t)r->d[1] < (int16_t)lo)         /* 0x1A0048 bge */
            r->d[1] = (r->d[1] & 0xFFFF0000u) | lo;
        d4 += 1u;                                   /* 0x1A0052 addq.l #1,D4 */
        if ((int32_t)d4 >= (int32_t)d3)             /* 0x1A0054 cmp.l D3,D4 */
            break;
    }
bump:
    if (rd16(m, 0x0040CBD0u) < 0x78u)
        wr16(m, 0x0040CBD0u, (uint16_t)(rd16(m, 0x0040CBD0u) + 1u));
done:
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)r->d[1];
    r->d[5] = rd32(m, sp - 8u);
    r->d[4] = rd32(m, sp - 12u);
    r->d[3] = rd32(m, sp - 16u);
    r->a[7] = sp;
}

/* One row of the editor (0x1A0B00..0x1A116E). The selected row (D5 == D3)
 * runs its field through the editor; every row then draws its label and value
 * in colour D2 (8 for the selected row). */
static void row_body(const f3mem_t *m, f3regs_t *r, uint32_t row)
{
    const int sel = r->d[5] == r->d[3];
    const uint32_t t = 0x0061C000u;             /* D4 */
    uint16_t in = (uint16_t)rd8(m, P1_SWITCH);

    switch (row) {
    case 0u:                                    /* 0x1A0B3A GAME STYLE */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0xAu, 11u, 3u);
            edit_value(m, r, 0x001A0B5Au, in, cur, 0u, 1u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0]; /* ext.l */
            ee_bfput(m, SETTINGS + 0xAu, 11u, 3u, r->d[0]);
        }
        r->a[0] = r->d[4];            /* movea.l D4,A0 */
        draw_text1(m, r, 0x001A0B76u, t + 0x20Eu, 0x001A2153u, (uint16_t)r->d[2]); /* GAME STYLE   : */
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];                      /* movea.l D0,A3 */
        {
            /* 0x1A0B7E: subq.l #1 / bhi 0x1A116C skips values above 1, so only
             * 0 and 1 reach the two-entry table at 0x1A0B94. */
            uint32_t value = ee_bfget(m, SETTINGS + 0xAu, 11u, 3u);
            r->d[0] = value;
            r->d[0] -= 1u;
            if (value > 1u)
                break;
            uint32_t str = (value == 0u) ? 0x001A2162u : 0x001A216Eu;
            r->d[0] = str;                      /* D (1P + 2P) / E (1P ONLY) */
            draw_text1(m, r, 0x001A1164u, r->a[3], str, (uint16_t)r->d[2]);
            r->a[7] += 0xAu;
            r->a[3] = r->d[0];
        }
        break;

    case 1u:                                    /* 0x1A0BB4 MONITOR MODE */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0xAu, 0u, 1u);
            edit_value(m, r, 0x001A0BDAu, in, cur, 0u, 1u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            ee_bfput(m, SETTINGS + 0xAu, 0u, 1u, r->d[0]);
        }
        r->a[0] = r->d[4];            /* movea.l D4,A0 */
        draw_text1(m, r, 0x001A0BF6u, t + 0x30Eu, 0x001A217Au, (uint16_t)r->d[2]); /* MONITOR MODE : */
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, SETTINGS + 0xAu) & 0x80u);
        draw_text1(m, r, 0x001A1164u, r->a[3],
                   (r->d[0] & 0xFFu) ? 0x001A2191u : 0x001A2189u,
                   (uint16_t)r->d[2]);          /* INVERT/NORMAL */
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];
        break;

    case 2u:                                    /* 0x1A0C22 ATTRACT SOUND */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0xAu, 1u, 1u);
            edit_value(m, r, 0x001A0C48u, in, cur, 0u, 1u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            ee_bfput(m, SETTINGS + 0xAu, 1u, 1u, r->d[0]);
        }
        r->a[0] = r->d[4];            /* movea.l D4,A0 */
        draw_text1(m, r, 0x001A0C64u, t + 0x40Eu, 0x001A2199u, (uint16_t)r->d[2]); /* ATTRACT SOUND: */
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, SETTINGS + 0xAu) & 0x40u);
        draw_text1(m, r, 0x001A1164u, r->a[3],
                   (r->d[0] & 0xFFu) ? 0x001A1F18u : 0x001A1F1Cu,
                   (uint16_t)r->d[2]);          /* ON / OFF */
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];
        break;

    case 3u:                                    /* 0x1A0C84 DIFFICULTY */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0xAu, 14u, 2u);
            edit_value(m, r, 0x001A0CA4u, in, cur, 0u, 3u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            ee_bfput(m, SETTINGS + 0xAu, 14u, 2u, r->d[0]);
        }
        r->a[0] = r->d[4];            /* movea.l D4,A0 */
        draw_text1(m, r, 0x001A0CC0u, t + 0x50Eu, 0x001A21A8u, (uint16_t)r->d[2]); /* DIFFICULTY   : */
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];
        {
            static const uint32_t str[4] = { 0x001A21B7u, 0x001A21C2u,
                                             0x001A21CDu, 0x001A21D8u };
            uint32_t value = ee_bfget(m, SETTINGS + 0xAu, 14u, 2u);
            r->d[0] = value;
            r->d[0] -= 3u;                      /* subq.l #3 / bhi 0x1A116C */
            if (value > 3u)
                break;
            r->d[0] = str[value];               /* EASY/NORMAL/HARD/SUPER HARD */
            draw_text1(m, r, 0x001A1164u, r->a[3], r->d[0], (uint16_t)r->d[2]);
            r->a[7] += 0xAu;
            r->a[3] = r->d[0];
        }
        break;

    case 4u:                                    /* 0x1A0D1E PLAYER STOCK */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0x18u, 2u, 3u);
            edit_value(m, r, 0x001A0D3Eu, in, cur, 0u, 4u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            ee_bfput(m, SETTINGS + 0x18u, 2u, 3u, r->d[0]);
        }
        /* 0x1A0D4C: the label and the value in one formatted line */
        r->d[0] = ee_bfget(m, SETTINGS + 0x18u, 2u, 3u);
        r->a[7] -= 4u;
        wr32(m, r->a[7], r->d[0]);
        r->a[0] = r->d[4];            /* movea.l D4,A0 */
        draw_fmt1(m, r, 0x001A0D62u, t + 0x60Eu, 0x001A21E3u); /* PLAYER STOCK :%1d */
        r->a[7] += 0xEu;
        break;

    case 5u:                                    /* 0x1A0D70 CONTINUE MODE */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0xAu, 2u, 1u);
            edit_value(m, r, 0x001A0D96u, in, cur, 0u, 1u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            ee_bfput(m, SETTINGS + 0xAu, 2u, 1u, r->d[0]);
        }
        r->a[0] = r->d[4];            /* movea.l D4,A0 */
        draw_text1(m, r, 0x001A0DB2u, t + 0x70Eu, 0x001A21F5u, (uint16_t)r->d[2]); /* CONTINUE MODE: */
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, SETTINGS + 0xAu) & 0x20u);
        draw_text1(m, r, 0x001A1164u, r->a[3],
                   (r->d[0] & 0xFFu) ? 0x001A1F18u : 0x001A1F1Cu,
                   (uint16_t)r->d[2]);
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];
        break;

    case 6u:                                    /* 0x1A0DD0 FREE PLAY */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0x18u, 0u, 1u);
            edit_value(m, r, 0x001A0DF6u, in, cur, 0u, 1u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            ee_bfput(m, SETTINGS + 0x18u, 0u, 1u, r->d[0]);
        }
        r->a[0] = r->d[4];            /* movea.l D4,A0 */
        draw_text1(m, r, 0x001A0E12u, t + 0x80Eu, 0x001A2204u, (uint16_t)r->d[2]); /* FREE PLAY    : */
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, SETTINGS + 0x18u) & 0x80u);
        draw_text1(m, r, 0x001A1164u, r->a[3],
                   (r->d[0] & 0xFFu) ? 0x001A1F18u : 0x001A1F1Cu,
                   (uint16_t)r->d[2]);
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];
        break;

    case 7u:                                    /* 0x1A0E3E COIN-A */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0xCu, 0u, 4u);
            edit_value(m, r, 0x001A0E60u, in, cur, 1u, 4u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            ee_bfput(m, SETTINGS + 0xCu, 0u, 4u, r->d[0]);
        }
        r->d[0] = ee_bfget(m, SETTINGS + 0xCu, 0u, 4u);
        r->a[7] -= 4u;
        wr32(m, r->a[7], r->d[0]);
        if (rd16(m, 0x001FFFFEu) == 2u) {       /* 0x1A0E6E table, a0.w = 0 */
            r->a[0] = r->d[4];            /* movea.l D4,A0 */
            draw_fmt1(m, r, 0x001A0EA0u, t + 0x90Eu, 0x001A2213u);
            r->a[7] += 0xEu;
        } else {
            r->a[0] = r->d[4];            /* movea.l D4,A0 */
            draw_fmt1(m, r, 0x001A0EC4u, t + 0x90Eu, 0x001A222Au);
            r->a[7] += 0xEu;
        }
        break;

    case 8u:                                    /* 0x1A0ED2 COIN-A credits */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0xCu, 4u, 4u);
            edit_value(m, r, 0x001A0EF4u, in, cur, 1u, 6u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            ee_bfput(m, SETTINGS + 0xCu, 4u, 4u, r->d[0]);
        }
        r->d[0] = ee_bfget(m, SETTINGS + 0xCu, 4u, 4u);
        r->a[7] -= 4u;
        wr32(m, r->a[7], r->d[0]);
        /* 0x1A0F02: version 2 and the default both land on 0x1A0F1E. */
        r->a[0] = r->d[4];            /* movea.l D4,A0 */
        draw_fmt1(m, r, 0x001A0F34u, t + 0x93Cu, 0x001A2241u); /* %1d CREDIT */
        r->a[7] += 0xEu;
        break;

    case 9u:                                    /* 0x1A0F42 COIN-B */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0xCu, 8u, 4u);
            edit_value(m, r, 0x001A0F64u, in, cur, 1u, 4u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            ee_bfput(m, SETTINGS + 0xCu, 8u, 4u, r->d[0]);
        }
        if (rd16(m, 0x001FFFFEu) == 2u) {
            /* 0x1A0F90: clamp the two COIN-B fields against each other. */
            r->d[0] = ee_bfget(m, SETTINGS + 0xCu, 8u, 4u);
            r->d[1] = ee_bfget(m, SETTINGS + 0xCu, 0u, 4u);
            if ((int32_t)r->d[0] > (int32_t)r->d[1]) {      /* 0x1A0F9E bls */
                r->d[0] = ee_bfget(m, SETTINGS + 0xCu, 0u, 4u);
                ee_bfput(m, SETTINGS + 0xCu, 8u, 4u, r->d[0]);
            }
            r->d[0] = ee_bfget(m, SETTINGS + 0xCu, 4u, 4u);
            r->d[0] -= 1u;
            if (r->d[0] != 0u && r->d[0] != 0xFFFFFFFFu) {  /* 0x1A0FB4 bls */
                r->d[0] = ee_bfget(m, SETTINGS + 0xCu, 0u, 4u);
                ee_bfput(m, SETTINGS + 0xCu, 8u, 4u, r->d[0]);
            }
            r->d[0] = ee_bfget(m, SETTINGS + 0xCu, 0u, 4u);
            r->d[1] = ee_bfget(m, SETTINGS + 0xCu, 18u, 4u);
            if (r->d[0] == r->d[1]) {                       /* 0x1A0FD0 bne */
                r->a[0] = r->d[4];            /* movea.l D4,A0 */
                draw_fmt1(m, r, 0x001A0FE0u, t + 0xA0Eu, 0x001A224Cu);
                r->a[7] += 0xAu;
            } else {
                r->d[0] = ee_bfget(m, SETTINGS + 0xCu, 8u, 4u);
                r->a[7] -= 4u;
                wr32(m, r->a[7], r->d[0]);
                r->a[0] = r->d[4];            /* movea.l D4,A0 */
                draw_fmt1(m, r, 0x001A1004u, t + 0xA0Eu, 0x001A226Du);
                r->a[7] += 0xEu;
            }
        } else {
            r->d[0] = ee_bfget(m, SETTINGS + 0xCu, 8u, 4u);
            r->a[7] -= 4u;
            wr32(m, r->a[7], r->d[0]);
            r->a[0] = r->d[4];            /* movea.l D4,A0 */
            draw_fmt1(m, r, 0x001A1028u, t + 0xA0Eu, 0x001A228Eu);
            r->a[7] += 0xEu;
        }
        break;

    case 10u:                                   /* 0x1A1036 COIN-B credits */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0xCu, 12u, 4u);
            edit_value(m, r, 0x001A1058u, in, cur, 1u, 6u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            ee_bfput(m, SETTINGS + 0xCu, 12u, 4u, r->d[0]);
        }
        if (rd16(m, 0x001FFFFEu) == 2u) {
            /* 0x1A108E: the COIN-B credit counter follows the selection. */
            if ((int32_t)r->d[5] > (int32_t)rd32(m, r->a[6] - 0xAu))
                r->d[3] += 1u;                  /* 0x1A1094 addq.l #1,D3 */
            if (r->d[5] != r->d[3])
                break;                          /* 0x1A1098 bne 0x1A116C */
            if ((int32_t)r->d[5] >= (int32_t)rd32(m, r->a[6] - 0xAu))
                break;                          /* 0x1A10A0 bge 0x1A116C */
            r->d[3] -= 1u;                      /* 0x1A10A4 subq.l #1,D3 */
        } else {
            r->d[0] = ee_bfget(m, SETTINGS + 0xCu, 12u, 4u);
            r->a[7] -= 4u;
            wr32(m, r->a[7], r->d[0]);
            r->a[0] = r->d[4];            /* movea.l D4,A0 */
            draw_fmt1(m, r, 0x001A10C0u, t + 0xA3Cu, 0x001A2241u);
            r->a[7] += 0xEu;
        }
        break;

    case 11u:                                   /* 0x1A10CE EXIT */
        r->a[0] = r->d[4];            /* movea.l D4,A0 */
        draw_text1(m, r, 0x001A10DCu, t + 0xB0Eu, 0x001A20C7u, (uint16_t)r->d[2]); /* EXIT */
        r->a[7] += 0xAu;
        if (r->d[5] != r->d[3])                 /* 0x1A10E2 cmp.l D3,D5 */
            break;
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x10u);
        if (r->d[0] & 0xFFu)
            break;                              /* 0x1A10F2 bne 0x1A116C */
        wr8(m, r->a[6] - 2u, 1u);               /* 0x1A10F4 move.b #1,-0x2(A6) */
        break;

    default:                                    /* 0x1A10FC GAME MODE (row 0xC,
                                                 * past the twelve-row count) */
        if (sel) {
            uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0x18u, 1u, 1u);
            edit_value(m, r, 0x001A1122u, in, cur, 0u, 1u, 1u);
            r->a[7] += 0xAu;
            r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            ee_bfput(m, SETTINGS + 0x18u, 1u, 1u, r->d[0]);
        }
        r->a[0] = r->d[4];            /* movea.l D4,A0 */
        draw_text1(m, r, 0x001A113Eu, t + 0xC0Eu, 0x001A22A5u, (uint16_t)r->d[2]); /* GAME MODE    : */
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, SETTINGS + 0x18u) & 0x40u);
        draw_text1(m, r, 0x001A1164u, r->a[3],
                   (r->d[0] & 0xFFu) ? 0x001A22BBu : 0x001A22B4u,
                   (uint16_t)r->d[2]);
        r->a[7] += 0xAu;
        r->a[3] = r->d[0];
        break;
    }
    r->d[5] += 1u;                              /* 0x1A116C addq.l #1,D5 */
}

/* 0x1A1176: the exit flag test both the frame loop and the confirm screen use.
 * Returns the yield PC, SEG_LEAVE, or 0 when the screen has to close. */
static uint32_t exit_flag(const f3mem_t *m, f3regs_t *r)
{
    if (rd8(m, r->a[6] - 2u) == 0u)
        return SEG_YIELD;                       /* 0x1A117A / 0x1A125A beq */
    r->d[3] = 0u;                               /* 0x1A117E */
    wr8(m, r->a[6] - 2u, 0u);                   /* move.b D3,-0x2(A6) */
    Gfx_ClearAllLayers(m);                      /* 0x1A1184 */
    return SEG_YIELD2;                          /* 0x1A118A */
}

/* 0x1A1256 (confirm): the flag leaves the screen, otherwise the frame yields. */
static uint32_t confirm_done(const f3mem_t *m, f3regs_t *r)
{
    if (rd8(m, r->a[6] - 2u) == 0u)
        return SEG_YIELD3;                      /* 0x1A125A beq 0x1A11CE */
    r->d[0] = r->d[3];                          /* 0x1A125E tst.l D3 */
    if (r->d[3] == 0u) {
        /* 0x1A1262 jsr 0x004DEE: the checksums and the serial write-back,
         * which enters the reset loop (0x01014E) when the device does not
         * answer - the harness has no I/O space, so this arm never returns. */
        push_long(m, r, 0x001A1268u);
        (void)settings_refresh_4DEE(m, r);
        return SEG_LEAVE;
    }
    r->d[3] -= 1u;                              /* 0x1A126A subq.l #1,D3 */
    if (r->d[3] == 0u) {                        /* 0x1A126C bne 0x1A1274 */
        r->a[7] -= 4u;                          /* 0x1A126E jsr 0x004D06 */
        wr32(m, r->a[7], 0x001A1274u);
        settings_apply_4D06(m, r);
    }
    Gfx_ClearAllLayers(m);                      /* 0x1A1274 */
    return restore_frame(m, r);
}

uint32_t Seg_1A0A52(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x1A0A52u:
        save_frame(m, r);
        r->a[2] = SETTINGS;
        r->a[4] = 0x00005666u;      /* jsr (A4) draws text */
        r->d[4] = 0x0061C000u;      /* text RAM base */
        r->d[6] = P1_SWITCH;
        r->d[7] = 0x0019FFF0u;      /* the shared editor */
        r->d[0] = 0xCu;
        wr32(m, r->a[6] - 0xEu, r->d[0]);       /* 0x1A0A7A: twelve rows */
        r->d[3] = 0u;
        wr32(m, r->a[6] - 0xAu, 0u);            /* clr.l -0xA(A6) */
        wr32(m, r->a[6] - 0x6u, 0u);            /* clr.l -0x6(A6) */
        wr8(m, r->a[6] - 2u, 0u);               /* clr.b -0x2(A6) */
        Gfx_ClearAllLayers(m);                  /* 0x1A0A8C */
        r->a[7] -= 4u;                          /* 0x1A0A92 jsr 0x004D06 */
        wr32(m, r->a[7], 0x001A0A98u);
        settings_apply_4D06(m, r);
        r->a[0] = (uint32_t)(int32_t)(int16_t)rd16(m, 0x00402240u); /* movea.w */
        wr32(m, r->a[6] - 0x6u, r->a[0]);       /* move.l A0,-0x6(A6) */
        r->a[7] -= 4u;                          /* 0x1A0AA2 jsr 0x1A006C */
        wr32(m, r->a[7], 0x001A0AA6u);
        Fn_1A006C(m, r);
        r->a[0] = r->d[4];                      /* movea.l D4,A0 */
        draw_text1(m, r, 0x001A0AB6u, r->a[0] + 0x9Cu, 0x001A2145u, 0xAu); /* CONFIGURATION */
        r->a[7] += 0xAu;                        /* 0x1A0AB8 lea 0xA(SP) */
        return exit_flag(m, r);                 /* 0x1A0ABC bra 0x1A1176 */

    case 0x1A0AC6u:
        /* 0x1A0AC6: the selection walks with the P2 switch byte */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x2u);
        if (!(r->d[0] & 0xFFu)) {
            wr32(m, r->a[6] - 0xAu, r->d[3]);   /* move.l D3,-0xA(A6) */
            r->d[3] += 1u;
            if ((int32_t)r->d[3] >= (int32_t)rd32(m, r->a[6] - 0xEu))
                r->d[3] = 0u;                   /* 0x1A0ADE */
        }
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x1u);
        if (!(r->d[0] & 0xFFu)) {
            wr32(m, r->a[6] - 0xAu, r->d[3]);
            r->d[3] -= 1u;
            if ((int32_t)r->d[3] < 0)           /* 0x1A0AF2 bpl */
                r->d[3] = rd32(m, r->a[6] - 0xEu) - 1u;
        }
        for (r->d[5] = 0u; (int32_t)r->d[5] < (int32_t)rd32(m, r->a[6] - 0xEu); ) {
            /* 0x1A0B00: colour 8 for the selected row, 7 for the rest */
            if (r->d[5] == r->d[3])
                r->d[2] = 8u;
            else
                r->d[2] = 7u;
            r->d[0] = r->d[5];                  /* 0x1A0B0C cmpi.l #0xC */
            if (r->d[0] > 0xCu) {               /* bhi 0x1A116C */
                r->d[5] += 1u;
                continue;
            }
            row_body(m, r, r->d[5]);
        }
        return exit_flag(m, r);                 /* 0x1A1176 */

    case 0x1A1190u:
        r->a[0] = r->d[4];
        draw_text1(m, r, 0x001A11A0u, r->a[0] + 0x9Cu, 0x001A2145u, 0xAu);  /* CONFIGURATION */
        draw_text1(m, r, 0x001A11B2u, r->a[0] + 0x316u, 0x001A20CCu, 0x7u); /* SAVE MODIFIED DATA */
        draw_text1(m, r, 0x001A11C4u, r->a[0] + 0x41Cu, 0x001A20DFu, 0x7u); /* ARE YOU SURE? */
        r->a[7] += 0x1Eu;                       /* 0x1A11C6 lea 0x1E(SP) */
        return confirm_done(m, r);              /* 0x1A11CA bra 0x1A1256 */

    case 0x1A11D4u:
        /* 0x1A11D4: the YES/NO editor; its arguments are dropped together with
         * the following draw's (lea 0x14(SP) at 0x1A1202). */
        edit_value(m, r, 0x001A11EAu, (uint16_t)rd8(m, P1_SWITCH),
                   (uint16_t)r->d[3], 0u, 1u, 1u);
        r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];  /* ext.l */
        r->d[3] = r->d[0];
        r->a[0] = r->d[4];
        draw_text1(m, r, 0x001A1200u, r->a[0] + 0x79Cu, 0x001A20EDu, 0x7u); /* YES       NO */
        r->a[7] += 0x14u;                       /* 0x1A1202 lea 0x14(SP) */
        r->d[0] = r->d[3];
        r->d[0] -= 1u;                          /* 0x1A1208 subq.l #1,D0 */
        if (r->d[3] <= 1u) {                    /* 0x1A120A bhi 0x1A1244 */
            r->a[0] = r->d[4];
            if (r->d[3] == 0u)
                draw_text1(m, r, 0x001A123Eu, r->a[0] + 0x79Cu, 0x001A20FAu, 0x8u); /* YES */
            else
                draw_text1(m, r, 0x001A123Eu, r->a[0] + 0x7B0u, 0x001A20FEu, 0x8u); /* NO */
            r->a[7] += 0xAu;                    /* 0x1A123E jsr + lea 0xA */
        }
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x10u);
        if (!(r->d[0] & 0xFFu))
            wr8(m, r->a[6] - 2u, 1u);           /* 0x1A1250 move.b #1,-0x2(A6) */
        return confirm_done(m, r);              /* 0x1A1256 */
    }
    return SEG_LEAVE;
}
