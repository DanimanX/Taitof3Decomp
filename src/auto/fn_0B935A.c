/* ROM 0x0B935A: UNVERIFIED (no capture): Task_0B935A (kernel slot 2), the
 * attract-mode stage task. The pre-yield setup (0x0B935A..0x0B93E2) spawns the
 * object-pool runner, reloads sound bank 0, starts effect B(7), clears line
 * RAM, sleeps 5 frames, fades two palette blocks out and plays hooked sound
 * 0x42. The loop after the yield (0x0B93E6..0x0B9680) installs the stage-2
 * background script (three ROM-scripted dispatchers), runs the "STAGE CLEAR"
 * line-RAM fade, draws the banner tilemaps and finally exits via TRAP #3.
 *
 * Two ROM script tables drive the background through function pointers:
 *   A5+0x338A -> 0x0BE39C, 8 entries {frame.w, fn.l} (driver 0x145464);
 *   A5+0x3BB6 -> 0x0BE2B8, 37 entries  (driver 0x145638).
 * Every target of the 0x338A table (0x0B934E, 0x0B984A, 0x0B9948, and the
 * 0x0B9964 it arms, which re-arms itself) is translated below, as are the
 * first twelve targets of the 0x3BB6 table. The remaining 0x3BB6 targets --
 * 0x0BA446, 0x0BA520, 0x0BA700, 0x0BA75E, 0x0BA7DA, 0x0BA7F4, 0x0BA862,
 * 0x0BA9E0, 0x0BAA0A, 0x0BAB34, 0x0BAB9C, 0x0BABB8, 0x0BACC0, 0x0BACEC,
 * 0x0BADDA, 0x0BAE10, 0x0BAE7E, 0x0BAED0, 0x0BAF5A, 0x0BAF70, 0x0BB16A,
 * 0x0BB23E, 0x0BB3A0, 0x0BB528 -- are separate ROM functions (the first is
 * reached once the frame counter passes 0x48E) that are not decompiled yet,
 * so the dispatcher returns -1 there before their side effects (the same
 * contract stage_timeline.c's bg_run_frame_events uses). The routine is
 * therefore PARTIAL, not PASS. */
#include "fn_auto.h"
#include "../common/fn_leaf1.h"
#include "../kernel/fn_010420.h"
#include "../stage/stage_timeline.h"
#include "../video/sprite.h"
#include "../video/palette.h"
#include "../video/linefx_176390.h"
#include "../objects/obj_pool.h"
#include "../enemy/obj_1194C2.h"
#include "../objects/obj_common.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_shared_ext.h"

/* ---- kernel TRAP services --------------------------------------------- */
static void sys_spawn_req(const f3mem_t *m, uint16_t slot, uint32_t entry)
{
    uint32_t id = (uint32_t)(slot & 0x0Fu);
    uint32_t parent = (uint32_t)(rd16(m, F3_A5 - 0x1906u) & 0xFFu);
    (void)entry;
    wr32(m, F3_A5 - 0x1904u, rd32(m, F3_A5 - 0x1904u) | (1u << id));
    wr32(m, F3_A5 - 0x18F8u + id * 4u, parent << 16);
}

static void sys_sleep(const f3mem_t *m, uint16_t frames)
{
    uint32_t cur = (uint32_t)(rd16(m, F3_A5 - 0x1906u) & 0x0Fu);
    wr32(m, F3_A5 - 0x18F8u + cur * 4u,
         ((uint32_t)(0xA000u | cur) << 16) | (uint32_t)frames);
    wr32(m, F3_A5 - 0x18B8u + cur * 4u, 0u);
}

static void sys_exit(const f3mem_t *m)
{
    uint32_t cur = (uint32_t)(rd16(m, F3_A5 - 0x1906u) & 0x1Fu);
    wr32(m, F3_A5 - 0x1904u, rd32(m, F3_A5 - 0x1904u) & ~(1u << cur));
}

/* Fill the 0xE8 words at base + (A5-0x5D56) with val. */
static void fill_e8_words(const f3mem_t *m, uint32_t base, uint16_t val)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (unsigned i = 0; i < 0xE8u; i++)
        wr16(m, a0 + i * 2u, val);
}

/* divs.w src,Dn: 32/16 signed; on overflow Dn is left unchanged. */
static uint32_t divs_w(uint32_t dn, int16_t src)
{
    if (src == 0)
        return dn;                                      /* would trap */
    int32_t q = (int32_t)dn / src, r = (int32_t)dn % src;
    if (q < -32768 || q > 32767)
        return dn;
    return ((uint32_t)(uint16_t)r << 16) | (uint16_t)q;
}

/* divu.w src,Dn: 32/16 unsigned; on overflow Dn is left unchanged. */
static uint32_t divu_w(uint32_t dn, uint16_t src)
{
    if (src == 0u)
        return dn;                                      /* would trap */
    uint32_t q = dn / src, r = dn % src;
    if (q > 0xFFFFu)
        return dn;
    return (r << 16) | q;
}

/* ROM 0x16AB26: write D0 to (A0), D1 to (A0). */
static void pair_write_16AB26(const f3mem_t *m, uint32_t a0, uint32_t d0, uint32_t d1)
{
    wr32(m, a0, d0);
    wr32(m, a0 + 4u, d1);
}

/* ROM 0x1768CA: line-RAM gradient. base+(A5-0x5D56) is the target; each word
 * gets its low 9 bits replaced from the running 0x4298 accumulator, advancing
 * index d7 toward d6 by d5 and accumulating d3 by d4. */
static void linram_1768CA(const f3mem_t *m, uint32_t base, uint16_t d7, uint16_t d6,
                          uint32_t v4298, uint32_t d3, uint32_t d4)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);

    wr32(m, F3_A5 + 0x4298u, v4298);
    if (rd16(m, F3_A5 - 0x5D58u) != 0u) {
        wr32(m, F3_A5 + 0x4298u, (uint32_t)(-(int32_t)rd32(m, F3_A5 + 0x4298u)));
        d3 = (uint32_t)(-(int32_t)d3);
        d4 = (uint32_t)(-(int32_t)d4);
    }
    int16_t d5 = ((int16_t)d7 > (int16_t)d6) ? -1 : 1;
    for (;;) {
        uint16_t d2 = rd16(m, F3_A5 + 0x4298u);
        uint16_t d0 = rd16(m, a0 + (uint16_t)(d7 * 2u));
        d0 = (uint16_t)((d0 & (uint16_t)~0x1FFu) | (d2 & 0x1FFu)); /* bfins {23:9} */
        wr16(m, a0 + (uint16_t)(d7 * 2u), d0);
        d3 = (uint32_t)((int32_t)d3 + (int32_t)d4);
        wr32(m, F3_A5 + 0x4298u,
             (uint32_t)((int32_t)rd32(m, F3_A5 + 0x4298u) + (int32_t)d3));
        d7 = (uint16_t)(d7 + (uint16_t)d5);
        if (d5 >= 0) {
            if ((int16_t)d7 <= (int16_t)d6) continue;
            break;
        }
        if ((int16_t)d6 <= (int16_t)d7) continue;
        break;
    }
}

/* ROM 0x176832: line-RAM gradient with a divide/multiply shading curve.
 * a0 gets the shaded word, a1 the shade byte, from index d7 toward d6. */
static void linram_176832(const f3mem_t *m, uint32_t base_a0, uint32_t base_a1,
                          uint16_t d7, uint16_t d6, uint32_t v4298, uint32_t d5,
                          uint32_t d3, uint16_t v42a0)
{
    uint32_t a0 = base_a0 + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint32_t a1 = base_a1 + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);

    wr32(m, F3_A5 + 0x4298u, v4298);
    wr16(m, F3_A5 + 0x42A0u, v42a0);
    int16_t d4 = ((int16_t)d7 > (int16_t)d6) ? -1 : 1;
    for (;;) {
        uint16_t d2 = rd16(m, F3_A5 + 0x4298u);
        int32_t d0 = (int32_t)0xFFFF6000;
        uint16_t d1w = (uint16_t)(0x00A0u + d2);
        if (d1w == 0u)                       /* the ROM would trap on divs.w */
            break;
        uint32_t q = divs_w(d0, (int16_t)d1w);                 /* divs.w */
        uint16_t d0w = (uint16_t)((uint16_t)q + 0x0100u);      /* addi.w #0x100 */
        int32_t t = (int16_t)(0x0024 - (int16_t)v42a0);
        int32_t d2l = t * (int32_t)(int16_t)d0w;               /* muls.w */
        d2l = (int32_t)((uint32_t)d2l << 6);
        uint16_t res = (uint16_t)((uint16_t)divs_w((uint32_t)d2l, 0x32) ^ 0x003Fu);
        wr16(m, a0 + (uint16_t)(d7 * 2u), res);
        wr8(m, a1 + (uint16_t)(d7 * 2u), (uint8_t)(d0w & 0xFFu)); /* move.b d0 */
        d5 = (uint32_t)((int32_t)d5 + (int32_t)d3);
        wr32(m, F3_A5 + 0x4298u,
             (uint32_t)((int32_t)rd32(m, F3_A5 + 0x4298u) + (int32_t)d5));
        d7 = (uint16_t)(d7 + (uint16_t)d4);
        if (d4 >= 0) {
            if ((int16_t)d7 <= (int16_t)d6) continue;
            break;
        }
        if ((int16_t)d6 <= (int16_t)d7) continue;
        break;
    }
}

/* ROM 0x176992: fill line RAM rows [d7..d6] of a0 from the sine table 0x27E40.
 * The index is byte 1 of the A5+0x4298 accumulator; every row advances the
 * accumulator by d1 and writes (table[idx]*d2 asr 8) ^ 0x3F. */
static void linram_176992(const f3mem_t *m, uint32_t base, uint32_t d0, uint32_t d1,
                          uint16_t d2w, uint16_t d6w, uint16_t d7w)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u)
                  + (uint32_t)(int32_t)(int16_t)d7w * 2u;

    wr32(m, F3_A5 + 0x4298u, d0);
    for (;;) {
        uint32_t idx = (uint32_t)rd8(m, F3_A5 + 0x4299u);            /* move.b 0x4299 */
        int16_t s = (int16_t)rd16(m, 0x00027E40u + idx * 2u);
        int32_t prod = (int32_t)s * (int32_t)(int16_t)d2w;           /* muls.w d2 */
        int32_t shifted = (prod < 0)                                 /* asr.l #8 */
            ? (int32_t)~(uint32_t)((uint32_t)(~prod) >> 8)
            : (int32_t)((uint32_t)prod >> 8);
        wr16(m, a0, (uint16_t)((uint16_t)(uint32_t)shifted ^ 0x003Fu));
        a0 += 2u;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d1);
        d7w = (uint16_t)(d7w + 1u);
        if ((int16_t)d7w > (int16_t)d6w)
            break;
    }
}

/* ROM 0x176296: line-RAM perspective band, the descending twin of 0x176390.
 * Same four 0xE8-row tables, but the active band runs from the first row DOWN
 * to first - count, and the step accumulator is not negated after the divide. */
static void linram_176296(const f3mem_t *m, uint32_t d0, uint32_t d1, uint32_t d7,
                          uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);

    wr32(m, F3_A5 + 0x4298u, d0);                       /* 0x176296 */
    wr16(m, F3_A5 + 0x429Cu, (uint16_t)d7);
    uint16_t d5 = (uint16_t)d1;                         /* move.w d1,d5 */
    wr32(m, F3_A5 + 0x42A0u, d0 << 7);                  /* 0x1762A0 lsl.l #7 */

    uint32_t h = rd32(m, F3_A5 + 0x4298u) >> 1;         /* 0x1762A6 */
    if (h != 0u)
        d1 = (uint32_t)(-(int32_t)(0x80000000u / h)) + 0x10000u; /* divu.l;neg;addi */
    else
        d1 = 0xFFFFFFFFu;                               /* 0x1762C6 moveq #-1 */
    wr32(m, F3_A5 + 0x42A4u, d1);                       /* 0x1762C8 (no second neg) */

    {
        uint32_t q = divu_w(0x00010000u, rd16(m, F3_A5 + 0x42A0u)); /* 0x1762D6 */
        wr32(m, F3_A5 + 0x42A8u, (uint32_t)(uint16_t)q * 0x280u);   /* mulu.w */
    }
    wr32(m, F3_A5 + 0x42ACu, 0u);
    wr32(m, F3_A5 + 0x42B0u, 0u);
    if (rd16(m, F3_A5 - 0x5D58u) != 0u)                 /* 0x1762E8 */
        wr32(m, F3_A5 + 0x42A4u, (uint32_t)(-(int32_t)rd32(m, F3_A5 + 0x42A4u)));

    /* pass 1 (0x1762F4): bit 13 is set strictly inside (first-count, first) */
    {
        int16_t hi = (int16_t)rd16(m, F3_A5 + 0x429Cu);
        int16_t lo = (int16_t)(uint16_t)(hi - rd16(m, F3_A5 + 0x42A0u));
        for (int16_t r = 0xE7; r >= 0; r--) {
            uint32_t pp = a3 + off + (uint32_t)(int32_t)r * 2u;
            uint16_t v = rd16(m, pp);
            if (r > lo && r < hi)                       /* cmp.w d1,d7 ble ; cmp.w d0,d7 bge */
                v |= 0x2000u;                           /* bset #13 of the register */
            else
                v &= (uint16_t)~0x2000u;                /* bclr #13 */
            wr16(m, pp, v);
        }
    }

    /* pass 2 (0x176326): rows first down to first-count */
    {
        uint16_t row = rd16(m, F3_A5 + 0x429Cu);
        int16_t end = (int16_t)(uint16_t)(row - rd16(m, F3_A5 + 0x42A0u));
        for (;;) {
            uint16_t acc = rd16(m, F3_A5 + 0x42ACu);
            uint32_t q = divs_w(0xFFFF6000u, (int16_t)(uint16_t)(0x00A0u + acc));
            uint16_t qw = (uint16_t)((uint16_t)q + 0x0100u);        /* addi.w #0x100 */
            int32_t z = (int32_t)(int16_t)(uint16_t)(0x0024u - d5) *
                        (int32_t)(int16_t)qw;                       /* muls.w */
            z = (int32_t)((uint32_t)z << 6);                        /* asl.l #6 */
            uint32_t zz = divs_w((uint32_t)z, 0x32);                /* divs.w #0x32 */
            uint32_t o = (uint32_t)(int32_t)(int16_t)row * 2u;
            wr16(m, a0 + off + o, (uint16_t)((uint16_t)zz ^ 0x003Fu));
            wr8(m, a1 + off + o, (uint8_t)(qw & 0xFFu));            /* move.b d0 */
            uint16_t x = rd16(m, a2 + off + o);
            x = (uint16_t)((x & 0xFE00u) | (rd16(m, F3_A5 + 0x42B0u) & 0x01FFu));
            wr16(m, a2 + off + o, x);
            wr32(m, F3_A5 + 0x42ACu,
                 rd32(m, F3_A5 + 0x42ACu) + rd32(m, F3_A5 + 0x42A8u));
            wr32(m, F3_A5 + 0x42B0u,
                 rd32(m, F3_A5 + 0x42B0u) + rd32(m, F3_A5 + 0x42A4u));
            row = (uint16_t)(row - 1u);                            /* subq.w #1 */
            if ((int16_t)row < 0)
                break;                                             /* blt */
            if (end <= (int16_t)row)
                continue;                                          /* cmp.w d7,d6 ble */
            break;
        }
    }
}

/* ROM 0x0B975E: the "STAGE CLEAR" line-RAM fade. d0 = (0x7F - frame)*2 is
 * used as the start index; three gradient passes build the fade. */
static void stage_clear_fx_0B975E(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)((uint16_t)(0x7Fu - rd16(m, F3_A5 + 0x3384u)) << 1);

    /* 0x0B9776..0x0B9798: 0x624400, d7=0, d6=d0, 0x4298=d0<<16, d3=0xFFFF0000 */
    linram_1768CA(m, 0x00624400u, 0u, d0, (uint32_t)d0 << 16, 0xFFFF0000u, 0u);

    /* 0x0B979C..0x0B97E2: table 0x27E40[(0x7F-frame)&0xFF]*0x20 as d5 */
    {
        uint16_t idx = (uint16_t)(0x7Fu - rd16(m, F3_A5 + 0x3384u));
        idx = (uint16_t)((idx & 0xFFu) * 2u);
        int32_t d1 = (int32_t)(int16_t)rd16(m, 0x00027E40u + (uint16_t)idx);
        d1 *= 0x20;                                   /* muls.w #0x20 */
        linram_176832(m, 0x0062A400u, 0x00628400u, d0, 0u, 0u,
                      (uint32_t)d1, 0u, 2u);
    }

    d0 = (uint16_t)(d0 + 1u);
    /* 0x0B97E6..0x0B980A: clear from d0+1 to 0xE7 */
    linram_1768CA(m, 0x00624400u, d0, 0x00E7u, 0u, 0u, 0u);
    /* 0x0B9814..0x0B9844 */
    linram_176832(m, 0x0062A400u, 0x00628400u, d0, 0x00E7u, 0u, 0u, 0u, 2u);
}

/* ROM 0x0B968C: redraw the "STAGE CLEAR" banner tilemaps. */
static void stage_clear_tiles_0B968C(const f3mem_t *m)
{
    static const uint32_t dst[8] = {
        0x00612084u, 0x0061231Cu, 0x00613000u, 0x00613400u,
        0x00613020u, 0x00613420u, 0x00613040u, 0x00613440u
    };
    static const uint32_t src[8] = {
        0x000679FCu, 0x00067A74u, 0x00067A32u, 0x00067A3Au,
        0x00067A48u, 0x00067A50u, 0x00067A5Eu, 0x00067A66u
    };
    for (unsigned i = 0; i < 8u; i++)
        Gfx_DrawCompressedTilemap(m, src[i], dst[i], 0u);
}

/* ---- ROM-scripted background drivers ---------------------------------- */

/* ROM 0x145484: 4-byte {frame.w, value.w} line-RAM entry; when the frame has
 * been reached, fill 0xE8 words of the 0x626200 table with the derived value
 * and advance the script pointer. */
static void bg_line_145484(const f3mem_t *m)
{
    uint32_t a0 = rd32(m, F3_A5 + 0x3BAEu);
    int16_t frame = (int16_t)rd16(m, F3_A5 + 0x3384u);
    if ((int16_t)rd16(m, a0) > frame)                   /* cmp.w ; bgt 0x1454FA */
        return;
    uint16_t d0 = rd16(m, a0 + 2u);
    uint16_t n1 = 8u, n0 = (uint16_t)(d0 & 0x0Fu);
    if (n0 > 8u) n0 = 8u;
    uint16_t v1 = (uint16_t)(((n1 << 8) + (8u - n1)) << 4);
    uint16_t v0 = (uint16_t)((n0 << 8) + (8u - n0));
    uint16_t v = (uint16_t)~(v1 | v0);
    fill_e8_words(m, 0x00626200u, v);
    wr32(m, F3_A5 + 0x3BAEu, a0 + 4u);
}

/* Forward declaration: the handler dispatcher. */
static int handler_is_known(uint32_t addr);
static int run_handler(const f3mem_t *m, uint32_t addr);

/* ROM 0x145638: 6-byte {frame.w, fn.l} list; run every entry whose frame has
 * been reached and advance by 6. */
static int bg_frame_list_145638(const f3mem_t *m)
{
    for (;;) {
        uint32_t a0 = rd32(m, F3_A5 + 0x3BB6u);
        int16_t frame = (int16_t)rd16(m, F3_A5 + 0x3384u);
        if ((int16_t)rd16(m, a0) > frame)               /* cmp.w ; bgt 0x145652 */
            return 0;
        uint32_t fn = rd32(m, a0 + 2u);
        if (!handler_is_known(fn))
            return -1;
        if (run_handler(m, fn) < 0)
            return -1;
        wr32(m, F3_A5 + 0x3BB6u, a0 + 6u);              /* addq.l #6 */
    }
}

/* ROM 0x145464: 6-byte {frame.w, fn.l} script that latches the handler at
 * A5+0x3386 and calls it every frame. */
static int bg_handler_script_145464(const f3mem_t *m)
{
    uint32_t a0 = rd32(m, F3_A5 + 0x338Au);
    int16_t frame = (int16_t)rd16(m, F3_A5 + 0x3384u);
    uint32_t fn;

    if ((int16_t)rd16(m, a0) <= frame) {                /* cmp.w ; bgt skip */
        fn = rd32(m, a0 + 2u);
        if (!handler_is_known(fn))
            return -1;
        wr32(m, F3_A5 + 0x3386u, fn);
        wr32(m, F3_A5 + 0x338Au, a0 + 6u);              /* addq.l #6 */
    } else {
        fn = rd32(m, F3_A5 + 0x3386u);
        if (!handler_is_known(fn))
            return -1;
    }
    return run_handler(m, fn);
}

/* ---- translated script handlers --------------------------------------- */

static int h_9BCC(const f3mem_t *m)
{
    static const uint32_t dst[16] = {
        0x00613000u, 0x00613400u, 0x00613800u, 0x00613C00u,
        0x00613020u, 0x00613420u, 0x00613820u, 0x00613C20u,
        0x00613040u, 0x00613440u, 0x00613840u, 0x00613C40u,
        0x00613060u, 0x00613460u, 0x00613860u, 0x00613C60u
    };
    f3regs_t s;

    for (unsigned i = 0; i < 16u; i++)
        Gfx_DrawCompressedTilemap(m, (i & 1u) ? 0x0006766Au : 0x00067590u, dst[i], 0u);
    Gfx_ClearPf1AndLineRam(m);                          /* 0x0B9D6C */
    Gfx_DrawCompressedTilemap(m, 0x00090956u, 0x00612218u, 0u); /* 0x0B9D72 */

    for (unsigned i = 0; i < 8u; i++) {
        s.d[i] = 0u;
        s.a[i] = 0u;
    }
    (void)Fn_16A310(m, &s);                             /* 0x0B9D8C */
    Leaf_16AB62(m, &s);                                 /* 0x0B9D92 */
    for (unsigned i = 0; i < 4u; i++) {                 /* 0x0B9D98..0x0B9DCA */
        s.a[0] = F3_A5 + 0x4284u + i;
        s.d[0] = 0u;
        Leaf_16AB58(m, &s);
    }
    pair_write_16AB26(m, F3_A5 + 0x4224u, 0u, 0u);       /* 0x0B9DD0 */
    pair_write_16AB26(m, F3_A5 + 0x422Cu, 0u, 0u);       /* 0x0B9DEA */
    pair_write_16AB26(m, F3_A5 + 0x4234u, 0u, 0x00003000u); /* 0x0B9E04 */
    pair_write_16AB26(m, F3_A5 + 0x423Cu, 0x00000C00u, 0x00001800u); /* 0x0B9E1E */
    Mem_WritePair64(m, F3_A5 + 0x4214u, F3_A5 - 0x5D98u, 0u, 0xFFCA0000u); /* 0x0B9E38 */
    return 0;
}

static int h_9E58(const f3mem_t *m)
{
    Fx_StartA(m, 5u);                                   /* 0x0B9E58 */
    (void)Obj_Spawn(m, 0x000B9E58u, 0x000BE248u, 0u, 1u, 0x000BE534u);
    return 0;
}

static int h_9E80(const f3mem_t *m)
{
    (void)Obj_Spawn(m, 0x000B9E80u, 0x000BD248u, 0u, 1u, 0u);
    return 0;
}

static int h_9EA0(const f3mem_t *m)
{
    Gfx_ClearTextLayer(m);                              /* 0x0B9EA0 jsr 0x5880 */
    (void)Obj_Spawn(m, 0x000B9EA0u, 0x000BD282u, 0u, 1u, 0u);
    (void)Obj_Spawn(m, 0x000B9EA0u, 0x000BD398u, 0u, 1u, 0u);
    return 0;
}

static int h_9EE4(const f3mem_t *m)
{
    (void)Obj_Spawn(m, 0x000B9EE4u, 0x000BD47Eu, 0u, 1u, 0u);
    return 0;
}

static int h_9F04(const f3mem_t *m)
{
    Gfx_ClearPf2AndLineRam(m);                          /* 0x0B9F04 jsr 0x59B4 */
    (void)Obj_Spawn(m, 0x000B9F04u, 0x000BD630u, 0u, 1u, 0u);
    return 0;
}

static int h_9F2A(const f3mem_t *m)
{
    (void)Obj_Spawn(m, 0x000B9F2Au, 0x000BD8C0u, 0u, 1u, 0u);
    return 0;
}

static int h_9F4A(const f3mem_t *m)
{
    (void)Pal_QueueRamp(m, 0x00026940u, 0x000267C0u, 0x1000u, 0x60u, 0x20u);
    (void)Pal_QueueRamp(m, 0x000BA026u, 0x00020340u, 0x0880u, 0x60u, 0x20u);
    return 0;
}

static int h_9F9C(const f3mem_t *m)
{
    pair_write_16AB26(m, F3_A5 + 0x422Cu, 0x00008000u, 0u); /* 0x0B9F9C */
    Pal_QueueUpload(m, 0x00026D40u, 0x1400u, 0x20u);    /* 0x0B9FB6 */
    Gfx_DrawCompressedTilemap(m, 0x0009098Eu, 0x00611058u, 0u); /* 0x0B9FD2 */
    wr32(m, F3_A5 + 0x08ACu,
         Obj_Spawn(m, 0x000B9F9Cu, 0x000BB654u, 0u, 1u, 0u)); /* 0x0B9FEC..0x0BA00A */
    return 0;
}

static int h_A010(const f3mem_t *m)
{
    Obj_Free(m, rd32(m, F3_A5 + 0x08ACu));              /* 0x0BA010 */
    Gfx_ClearPf0b(m);                                   /* 0x0BA01E jsr 0x5918 */
    return 0;
}

static int h_B530(const f3mem_t *m)
{
    Pal_FadeOut001866(m, 0x0001E080u, 0x6B80u, 0x4Bu, 0x20u); /* 0x0BB530 */
    (void)Obj_Spawn(m, 0x000BB530u, 0x000BB572u, 0u, 1u, 0u);
    return 0;
}

/* ROM 0x0B9964: zoom/scroll driver for the attract stage. Rebuilds A5+0x3BBA
 * from the A5-0x5D94 depth value, picks one of two perspective bands
 * (0x176296 or 0x176390) by the screen position, then repaints the sine bands
 * of 0x62A600 with 0x176992. Re-arms itself at 0x0B995A. */
static int h_9964(const f3mem_t *m)
{
    uint32_t v5d94 = rd32(m, F3_A5 - 0x5D94u);
    uint16_t d0;

    /* 0x0B9968: A5+0x3BBA = -depth + 0x7FFFFF */
    wr32(m, F3_A5 + 0x3BBAu, (uint32_t)(-(int32_t)v5d94 + 0x7FFFFF));

    d0 = (uint16_t)v5d94;                               /* 0x0B997A move.w -0x5D94 */
    if (d0 == rd16(m, F3_A5 + 0x41F8u))                 /* 0x0B997E beq 0x0B9AF8 */
        goto tail;
    d0 = (uint16_t)(-(int16_t)d0);                      /* 0x0B9986 neg.w */
    d0 = (uint16_t)(d0 + 0x0080u);                      /* 0x0B9988 */
    if ((int16_t)d0 > 0x0074)                           /* 0x0B998C bgt 0x0B9A2E */
        goto path_b;

    {   /* path A: 0x0B9994..0x0B9A2A */
        uint32_t v = (uint32_t)(-(int32_t)rd32(m, F3_A5 + 0x3BBAu) + 0x740000);
        v >>= 5;
        v = divu_w(v, 5u);                              /* divu.w #5 */
        v = (uint32_t)(uint16_t)v * 0x1C00u;            /* mulu.w #0x1c00 */
        v >>= 8;
        v >>= 3;
        if ((int32_t)v > 0x10000)                       /* 0x0B99AE ble */
            v = 0x10000u;
        uint32_t d1 = rd32(m, F3_A5 + 0x3BBAu) + 0x1000u; /* 0x0B99BE */
        d1 = (d1 << 16) | (d1 >> 16);                   /* 0x0B99C8 swap */
        linram_176296(m, v, 2u, (uint16_t)d1,
                      0x0062A400u, 0x00628400u, 0x00624400u, 0x0062B400u);
        /* 0x0B9A04: A5-0x5D8C = (A5+0x4238 + 0x200000) & 0xFFFF0000 */
        wr32(m, F3_A5 - 0x5D8Cu,
             (rd32(m, F3_A5 + 0x4238u) + 0x200000u) & 0xFFFF0000u);
        /* 0x0B9A18: A5+0x4250 = -((A5+0x4238 << 5) - 0x20000) */
        wr32(m, F3_A5 + 0x4250u,
             (uint32_t)(-(int32_t)((rd32(m, F3_A5 + 0x4238u) << 5) - 0x20000u)));
    }
    goto tail;

path_b:
    {   /* path B: 0x0B9A2E..0x0B9AF4 */
        uint32_t v = rd32(m, F3_A5 + 0x3BBAu) - 0x730000u; /* 0x0B9A2E subi.l */
        uint32_t v4238 = rd32(m, F3_A5 + 0x4238u);
        uint32_t d1;
        v >>= 5;
        v = divu_w(v, 5u);
        v = (uint32_t)(uint16_t)v * 0x1C00u;
        v >>= 8;
        v >>= 3;
        if ((int32_t)v > 0x10000)                       /* 0x0B9A46 */
            v = 0x10000u;
        d1 = rd32(m, F3_A5 + 0x3BBAu);                  /* 0x0B9A56 */
        if (rd16(m, F3_A5 - 0x5D58u) != 0u) {           /* 0x0B9A5A tst.w */
            if ((int16_t)(uint16_t)(v4238 >> 16) != 0) {/* 0x0B9A62 tst.w 0x4238 */
                d1 += v4238;                            /* 0x0B9A6A..0x0B9A72 */
                d1 += v4238;
                d1 += v4238;
            }
        } else {
            d1 += 0x10000u;                             /* 0x0B9A7A */
            d1 += v4238;
            d1 += v4238;
            d1 += v4238;
            /* 0x0B9A90 tst.w 0x4238 ; bne 0x0B9A98 -- both arms reach 0x0B9A98 */
        }
        d1 = (d1 << 16) | (d1 >> 16);                   /* 0x0B9A98 swap */
        {   /* 0x0B9A9A..0x0B9ABE: the ROM adds (A5-0x5D56) to A0..A3 first */
            uint32_t off = (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
            LineFx_176390(m, v, 2u, (uint16_t)d1,
                          0x0062A400u + off, 0x00628400u + off,
                          0x00624400u + off, 0x0062B400u + off);
        }
        /* 0x0B9AD4: A5-0x5D8C = (A5-0x5D94 + 0x200000) & 0xFFFF0000 */
        wr32(m, F3_A5 - 0x5D8Cu,
             (rd32(m, F3_A5 - 0x5D94u) + 0x200000u) & 0xFFFF0000u);
        /* 0x0B9AE8: A5+0x4250 = (A5+0x4238 << 5) + 0x200000 */
        wr32(m, F3_A5 + 0x4250u, (v4238 << 5) + 0x200000u);
    }

tail:
    {   /* 0x0B9AF8: sine band above the horizon */
        uint16_t v = (uint16_t)((rd32(m, F3_A5 + 0x3BBAu) >> 16) + 0x0050u);
        if ((int16_t)v >= 0) {
            linram_176992(m, 0x0062A600u, 0u, 0u, 0u, v, 0u);
        }
    }
    {   /* 0x0B9B32..0x0B9B76: sine band below it, wobbling with 0x4238 */
        uint16_t v = (uint16_t)((rd32(m, F3_A5 + 0x3BBAu) >> 16) + 0x0051u);
        if ((int16_t)v < 0x00E7) {                      /* 0x0B9B3A bge 0x0B9B76 */
            if ((int16_t)v < 0)                         /* 0x0B9B42 tst/bge */
                v = 0u;
            /* 0x0B9B5A reads the LONG at A5+0x3384 ({frame.w, fn hi word}) */
            linram_176992(m, 0x0062A600u, rd32(m, F3_A5 + 0x3384u),
                          0x00080000u, 5u, 0x00E7u, v);
        }
    }
    wr32(m, F3_A5 + 0x3386u, 0x000B9964u);              /* 0x0B9B76 bra 0x0B995A */
    return 0;
}

/* ROM 0x0BA0A6: rebuild every attract-stage line table and palette block. */
static int h_A0A6(const f3mem_t *m)
{
    static const uint32_t src[12] = {
        0x0006D7C4u, 0x0006DC7Eu, 0x0006DA3Eu, 0x0006D846u,
        0x0006DD04u, 0x0006DB04u, 0x0006D8F2u, 0x0006DD6Cu,
        0x0006DA3Eu, 0x0006D8F2u, 0x0006DDC2u, 0x0006DB04u
    };
    static const uint32_t dst[12] = {
        0x00613000u, 0x00613400u, 0x00613800u, 0x00613020u,
        0x00613420u, 0x00613820u, 0x00613040u, 0x00613440u,
        0x00613840u, 0x00613060u, 0x00613460u, 0x00613860u
    };
    static const uint32_t pal_src[12] = {
        0x00022040u, 0x00022740u, 0x00022340u, 0x000226C0u,
        0x00022640u, 0x000224C0u, 0x000220C0u, 0x000219C0u,
        0x000223C0u, 0x00021940u, 0x000225C0u, 0x00022240u
    };
    static const uint16_t pal_dst[12] = {
        0x1500u, 0x1C00u, 0x1800u, 0x1B80u, 0x1B00u, 0x1980u,
        0x1580u, 0x0E80u, 0x1880u, 0x0E00u, 0x1A80u, 0x1700u
    };

    fill_e8_words(m, 0x0062B200u, 0x3007u);             /* 0x0BA0A6 jsr 0x176198 */
    linram_176832(m, 0x0062A200u, 0x00628200u, 0u, 0x00E7u, 0u, 0u, 0u, 1u);
    linram_176832(m, 0x0062A600u, 0x00628600u, 0u, 0x00E7u, 0u, 0u, 0u, 3u);
    linram_1768CA(m, 0x00624600u, 0u, 0x00E7u, 0u, 0u, 0u);

    pair_write_16AB26(m, F3_A5 + 0x422Cu, 0u, 0u);       /* 0x0BA14C */
    pair_write_16AB26(m, F3_A5 + 0x423Cu, 0u, 0u);       /* 0x0BA166 */
    Mem_WritePair64(m, F3_A5 + 0x420Cu, F3_A5 - 0x5DA0u,
                    0x00800000u, 0xFF400000u);          /* 0x0BA180 */
    Mem_WritePair64(m, F3_A5 + 0x421Cu, F3_A5 - 0x5D90u,
                    0x00800000u, 0u);                   /* 0x0BA19E */

    for (unsigned i = 0; i < 12u; i++)
        Gfx_DrawCompressedTilemap(m, src[i], dst[i], 0u);
    for (unsigned i = 0; i < 12u; i++)
        Pal_QueueUpload(m, pal_src[i], pal_dst[i], 0x0020u);
    return 0;
}

static int h_934E(const f3mem_t *m)
{
    wr32(m, F3_A5 + 0x3386u, 0x000B9358u);              /* 0x0B934E / 0x0B9358 */
    return 0;
}

static int h_984A(const f3mem_t *m)
{
    wr32(m, F3_A5 + 0x3386u, 0x000B9854u);              /* 0x0B984A */
    return 0;
}

static int h_9854(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd16(m, F3_A5 + 0x3384u) & 0x007Fu);

    if (d0 == 0u) {                                     /* 0x0B9854..0x0B985C */
        Pal_FadeIn001890(m, 0x00026D94u, 0x1454u, 0x3Fu, 4u);
        Pal_FadeIn001890(m, 0x00026DA4u, 0x1464u, 0x1Fu, 6u);
    }
    if (d0 == 0x20u)                                    /* 0x0B98A4 */
        Pal_FadeOut001866(m, 0x00026DA4u, 0x1464u, 0x10u, 6u);
    if (d0 == 0x40u) {                                  /* 0x0B98CE */
        Pal_FadeOut001866(m, 0x00026D94u, 0x1454u, 0x3Fu, 4u);
        Pal_FadeIn001890(m, 0x00026DA4u, 0x1464u, 0x1Fu, 6u);
    }
    if (d0 == 0x60u)                                    /* 0x0B991A */
        Pal_FadeOut001866(m, 0x00026DA4u, 0x1464u, 0x1Fu, 6u);
    wr32(m, F3_A5 + 0x3386u, 0x000B9854u);              /* 0x0B9944 bra 0x0B984A */
    return 0;
}

static int h_9948(const f3mem_t *m)
{
    int32_t v = -(int32_t)rd32(m, F3_A5 - 0x5D94u) + 0x7FFFFF; /* 0x0B9948 */
    wr32(m, F3_A5 + 0x3BBAu, (uint32_t)v);
    wr32(m, F3_A5 + 0x3386u, 0x000B9964u);
    return 0;
}

static int h_9B7A(const f3mem_t *m)
{
    wr32(m, F3_A5 + 0x3386u, 0x000B9B84u);              /* 0x0B9B7A */
    return 0;
}

static int h_9B84(const f3mem_t *m)
{
    f3regs_t s;

    for (unsigned i = 0; i < 8u; i++) {
        s.d[i] = 0u;
        s.a[i] = 0u;
    }
    s.a[0] = F3_A5 + 0x4284u;
    s.d[0] = 0u;
    Leaf_16AB58(m, &s);                                 /* 0x0B9B84 */
    Mem_WritePair64(m, F3_A5 + 0x4204u, F3_A5 - 0x5DA8u, 0u, 0u); /* 0x0B9B92 */
    pair_write_16AB26(m, F3_A5 + 0x4224u, 0u, 0u);       /* 0x0B9BB0 */
    wr32(m, F3_A5 + 0x3386u, 0x000B9B84u);              /* 0x0B9BCA bra 0x0B9B7A */
    return 0;
}

static int handler_is_known(uint32_t addr)
{
    switch (addr) {
    case 0x000B934Eu: case 0x000B9358u:
    case 0x000B984Au: case 0x000B9854u:
    case 0x000B9948u: case 0x000B9964u:
    case 0x000B9B7Au: case 0x000B9B84u:
    case 0x000B9BCCu:
    case 0x000B9E58u: case 0x000B9E80u: case 0x000B9EA0u:
    case 0x000B9EE4u: case 0x000B9F04u: case 0x000B9F2Au:
    case 0x000B9F4Au: case 0x000B9F9Cu:
    case 0x000BA010u: case 0x000BA0A6u:
    case 0x000BB530u:
        return 1;
    default:
        return 0;                                       /* 0x0B9964/0x0BA0A6 not translated */
    }
}

static int run_handler(const f3mem_t *m, uint32_t addr)
{
    switch (addr) {
    case 0x000B934Eu: case 0x000B9358u: return h_934E(m);
    case 0x000B984Au: return h_984A(m);
    case 0x000B9854u: return h_9854(m);
    case 0x000B9948u: return h_9948(m);
    case 0x000B9964u: return h_9964(m);
    case 0x000B9B7Au: return h_9B7A(m);
    case 0x000B9B84u: return h_9B84(m);
    case 0x000B9BCCu: return h_9BCC(m);
    case 0x000B9E58u: return h_9E58(m);
    case 0x000B9E80u: return h_9E80(m);
    case 0x000B9EA0u: return h_9EA0(m);
    case 0x000B9EE4u: return h_9EE4(m);
    case 0x000B9F04u: return h_9F04(m);
    case 0x000B9F2Au: return h_9F2A(m);
    case 0x000B9F4Au: return h_9F4A(m);
    case 0x000B9F9Cu: return h_9F9C(m);
    case 0x000BA010u: return h_A010(m);
    case 0x000BA0A6u: return h_A0A6(m);
    case 0x000BB530u: return h_B530(m);
    default: return -1;
    }
}

/* ---- the task ---------------------------------------------------------- */

int Fn_0B935A(const f3mem_t *m, f3regs_t *r)
{
    (void)r;
    wr16(m, F3_A5 + 0x3384u, 0u);                       /* 0x0B935A clr.w */
    sys_spawn_req(m, 4u, 0x00010488u);                  /* 0x0B935E..0x0B936C */
    Kernel_SoundBank0_0014DA(m);                        /* 0x0B936E jsr 0x14DA */
    Fx_StartB(m, 7u);                                   /* 0x0B9374 moveq #7; jsr 0x15EC */
    Gfx_InitLineRam(m);                                 /* 0x0B937C jsr 0x5B0C */
    sys_sleep(m, 5u);                                   /* 0x0B9382 trap #4 */
    Pal_FadeOut001866(m, 0x000BA026u, 0x0880u, 0x96u, 0x20u); /* 0x0B938C */
    Pal_FadeOut001866(m, 0x00026940u, 0x1000u, 0x96u, 0x20u); /* 0x0B93AE */
    Sound_PlayHooked(m, 0x0042u);                       /* 0x0B93D0 */
    /* 0x0B93E4 trap #5: first yield */
    return 0;
}

int Task0B935A_Frame(const f3mem_t *m)
{
    /* 0x0B93E6: install the stage-2 background state */
    {
        f3regs_t s;
        for (unsigned i = 0; i < 8u; i++) {
            s.d[i] = 0u;
            s.a[i] = 0u;
        }
        (void)Fn_16A310(m, &s);                         /* 0x0B93E6 jsr 0x16A310 */
    }
    wr16(m, F3_A5 + 0x3BD6u, 0u);                       /* 0x0B93EC clr.w */
    fill_e8_words(m, 0x0062B000u, 0x300Cu);             /* 0x0B93F0 */
    fill_e8_words(m, 0x0062B200u, 0x300Au);             /* 0x0B9404 */
    fill_e8_words(m, 0x0062B400u, 0x3008u);             /* 0x0B9418 */
    fill_e8_words(m, 0x0062B600u, 0x3006u);             /* 0x0B942C */
    fill_e8_words(m, 0x00626200u, 0xBFB7u);             /* 0x0B9440..0x0B945A */
    wr32(m, F3_A5 + 0x3BAEu, 0x000BE3CCu);              /* 0x0B945E */
    wr32(m, F3_A5 + 0x3BB6u, 0x000BE2B8u);              /* 0x0B9466 */
    wr32(m, F3_A5 + 0x338Au, 0x000BE39Cu);              /* 0x0B946E..0x0B9474 */
    wr32(m, F3_A5 + 0x3386u, 0x000B934Eu);              /* 0x0B9478 rd32(0xBE3A0) */

    /* 0x0B947E loop: one pass, then yield (trap #5 at 0x0B9496) unless the
     * stage-clear flag A5+0x3BD6 was set by a handler. */
    for (;;) {
        bg_line_145484(m);                              /* 0x0B947E */
        if (bg_frame_list_145638(m) < 0)                /* 0x0B9484 */
            return -1;
        if (bg_handler_script_145464(m) < 0)            /* 0x0B948A */
            return -1;
        StageScroll_Step(m);                            /* 0x0B9490 jsr 0x16A3E0 */
        /* 0x0B9496 trap #5 */
        wr16(m, F3_A5 + 0x3384u,
             (uint16_t)(rd16(m, F3_A5 + 0x3384u) + 1u)); /* 0x0B9498 */
        if (rd16(m, F3_A5 + 0x3BD6u) != 0u)             /* 0x0B949C tst ; beq */
            break;
        return 1;                                       /* one frame yielded */
    }

    /* 0x0B94A2..0x0B956A: stage-clear scene */
    Gfx_ClearPf2AndLineRam(m);                          /* 0x0B94A2 */
    {
        f3regs_t s;
        for (unsigned i = 0; i < 8u; i++) {
            s.d[i] = 0u;
            s.a[i] = 0u;
        }
        (void)Fn_16A310(m, &s);                         /* 0x0B94A8 */
    }
    Mem_WritePair64(m, F3_A5 + 0x4214u, F3_A5 - 0x5D98u, 0u, 0x00080000u); /* 0x0B94AE */
    Mem_WritePair64(m, F3_A5 + 0x420Cu, F3_A5 - 0x5DA0u, 0x00080000u, 0xFFFC0000u); /* 0x0B94CC */
    (void)Pal_QueueRamp(m, 0x0000198Eu, 0x000207C0u, 0x0D00u, 0x7Fu, 0x20u); /* 0x0B94EA */
    Pal_FadeOut001866(m, 0x00020140u, 0x0680u, 0x7Fu, 0x20u); /* 0x0B950C */
    {   /* 0x0B952E: Sound_PreCmdHook + forced 0x43 */
        int16_t c = Sound_PreCmdHook(m, 0x0043);
        Sound_PlayCmdForced(m, c);
    }
    {   /* 0x0B9542: forced 0x2F */
        int16_t c = Sound_PreCmdHook(m, 0x002F);
        Sound_PlayCmdForced(m, c);
    }
    Sound_PlayHooked(m, 0x0014u);                       /* 0x0B9556 */
    /* 0x0B956A trap #5 */

    /* 0x0B956C..0x0B95D4: 0x7F-frame banner fade loop */
    wr16(m, F3_A5 + 0x3384u, 0u);                       /* 0x0B956C */
    /* 0x0B9570: move.w -0x1456(A5),D0 ; andi.w #0xFC,D0 ; move.w D0,0x3BBA(A5)
     * -- the mask is 0x00FC (bytes 02 40 00 FC) and only the high word of the
     * 0x3BBA long is written. */
    wr16(m, F3_A5 + 0x3BBAu,
         (uint16_t)(rd16(m, F3_A5 - 0x1456u) & 0x00FCu));
    stage_clear_fx_0B975E(m);                           /* 0x0B957C */
    for (;;) {
        if (rd16(m, F3_A5 + 0x3384u) == 0x007Fu)        /* 0x0B9580 */
            break;
        /* 0x0B958A trap #5 */
        if (rd16(m, F3_A5 + 0x3384u) == 1u)             /* 0x0B958C */
            stage_clear_tiles_0B968C(m);                /* 0x0B9596 */
        if (rd16(m, F3_A5 + 0x3384u) == 0x003Fu) {      /* 0x0B959A */
            (void)Pal_QueueRamp(m, 0x000BE434u, 0x000207C0u, 0x0D00u, 0x40u, 0x20u); /* 0x0B95A4 */
        }
        stage_clear_fx_0B975E(m);                       /* 0x0B95CC */
        wr16(m, F3_A5 + 0x3384u,
             (uint16_t)(rd16(m, F3_A5 + 0x3384u) + 1u)); /* 0x0B95D0 */
    }

    /* 0x0B95D6..0x0B9606 */
    Gfx_DrawCompressedTilemap(m, 0x00066F9Cu, 0x0061151Cu, 0u); /* 0x0B95D6 */
    Gfx_DrawCompressedTilemap(m, 0x00067A86u, 0x0061123Cu, 0u); /* 0x0B95F0 */
    if (rd16(m, 0x001FFFFEu) != 1u)                     /* 0x0B960A */
        Text_DrawString(m, 0x000B9682u, 0x0061C63Eu, 8u); /* 0x0B9616 */

    /* 0x0B9630..0x0B9640: wait until the frame counter reaches 0x8F */
    for (;;) {
        /* 0x0B9630 trap #5 */
        if (rd16(m, F3_A5 + 0x3384u) == 0x008Fu)
            break;
        wr16(m, F3_A5 + 0x3384u,
             (uint16_t)(rd16(m, F3_A5 + 0x3384u) + 1u)); /* 0x0B963C */
    }
    (void)Pal_QueueRamp(m, 0x000BE4B4u, 0x000207C0u, 0x0D00u, 0x40u, 0x20u); /* 0x0B9642 */
    Sound_PlayHooked(m, 0x004Eu);                       /* 0x0B966A */
    sys_exit(m);                                        /* 0x0B967E trap #3 */
    return 0;                                           /* 0x0B9680 rts (unreachable) */
}
