/* ROM 0x13E82C: 3D camera projection update (no arguments; A0/D0-D2 preserved).
 * Picks the coefficient pair 0x13E9C8 (flat) or 0x13E9D0 (when A5-0x5D58 is
 * set), scales A5+0x32EA/A5+0x32E2 through 0x13E994 into the 16.16 camera X
 * A5-0x5DA8 (+ word A5+0x32E6), the A5+0x32EC/A5+0x32E4 pair through 0x13E9B0
 * into camera Y A5-0x5DA4 (+ word A5+0x32E8), stores the packed
 * {A5+0x32EA.b, A5+0x32EC.b} word at 0x628000 + A5+0x3300 and copies both
 * camera longs to the mirror A5+0x4204 / A5+0x4208. */
#include "fn_auto.h"

#define CAM_TBL_A 0x0013E9C8u   /* flat:  0x0017, 0x0002, -0x0008, -0x0001 */
#define CAM_TBL_B 0x0013E9D0u   /* 3D:    0x00A9, 0x0116,  0x0157,  0x00E8 */

#define FLAT_FLAG (F3_A5 - 0x5D58u)
#define CAM_X     (F3_A5 - 0x5DA8u)
#define CAM_Y     (F3_A5 - 0x5DA4u)
#define MIR_X     (F3_A5 + 0x4204u)
#define MIR_Y     (F3_A5 + 0x4208u)
#define IN_XA     (F3_A5 + 0x32EAu)
#define IN_XB     (F3_A5 + 0x32E2u)
#define IN_XOFF   (F3_A5 + 0x32E6u)
#define IN_YA     (F3_A5 + 0x32ECu)
#define IN_YB     (F3_A5 + 0x32E4u)
#define IN_YOFF   (F3_A5 + 0x32E8u)
#define PROJ_OFS  (F3_A5 + 0x3300u)
#define PROJ_BASE 0x00628000u

/* ROM 0x13E994: D0 = swap((0x100 - d0) * ((tbl+4) - d1) << 8) with (tbl) added
 * to the swapped value's low word, then swapped back. */
static uint32_t sub_13E994(const f3mem_t *m, uint32_t tbl, uint16_t d0w, uint16_t d1w)
{
    uint16_t d2 = (uint16_t)(0x0100u - d0w);            /* 0x13E994..0x13E998 */
    uint16_t d1 = (uint16_t)(0u - d1w);                 /* 0x13E99A neg.w */
    d1 = (uint16_t)(d1 + rd16(m, tbl + 4u));            /* 0x13E99C */
    uint32_t prod = (uint32_t)((int32_t)(int16_t)d1 * (int32_t)(int16_t)d2); /* 0x13E9A0 */
    prod <<= 8;                                         /* 0x13E9A2 */
    uint32_t sw = (prod << 16) | (prod >> 16);          /* 0x13E9A6 move.l/swap */
    sw = (sw & 0xFFFF0000u) | (uint16_t)(sw + rd16(m, tbl)); /* 0x13E9A8 */
    return (sw << 16) | (sw >> 16);                     /* 0x13E9AC swap */
}

/* ROM 0x13E9B0: same for Y; the scale is (d0 + 1) * ((tbl+6) - d1) << 9 and
 * (tbl+2) is the addend. */
static uint32_t sub_13E9B0(const f3mem_t *m, uint32_t tbl, uint16_t d0w, uint16_t d1w)
{
    uint16_t a = (uint16_t)(d0w + 1u);                  /* 0x13E9B0 addq.w */
    uint16_t b = (uint16_t)(0u - d1w);                  /* 0x13E9B2 neg.w */
    b = (uint16_t)(b + rd16(m, tbl + 6u));              /* 0x13E9B4 */
    uint32_t prod = (uint32_t)((int32_t)(int16_t)a * (int32_t)(int16_t)b); /* 0x13E9B8 */
    prod <<= 1;                                         /* 0x13E9BA */
    prod <<= 8;                                         /* 0x13E9BC */
    uint32_t sw = (prod << 16) | (prod >> 16);          /* 0x13E9BE swap */
    sw = (sw & 0xFFFF0000u) | (uint16_t)(sw + rd16(m, tbl + 2u)); /* 0x13E9C0 */
    return (sw << 16) | (sw >> 16);                     /* 0x13E9C4 swap */
}

int Fn_13E82C(const f3mem_t *m, f3regs_t *r)
{
    (void)r;                                            /* 0x13E82C/0x13E89C movem */

    uint32_t tbl = (rd8(m, FLAT_FLAG) != 0u) ? CAM_TBL_B : CAM_TBL_A;
    uint32_t res = sub_13E994(m, tbl, rd16(m, IN_XA), rd16(m, IN_XB)); /* 0x13E848.. */
    wr32(m, CAM_X, res);                                /* 0x13E854 */
    wr16(m, CAM_X, (uint16_t)(rd16(m, CAM_X) + rd16(m, IN_XOFF))); /* 0x13E858 */

    res = sub_13E9B0(m, tbl, rd16(m, IN_YA), rd16(m, IN_YB)); /* 0x13E860.. */
    wr32(m, CAM_Y, res);                                /* 0x13E86C */
    wr16(m, CAM_Y, (uint16_t)(rd16(m, CAM_Y) + rd16(m, IN_YOFF))); /* 0x13E870 */

    /* 0x13E878..0x13E88E: bfins D0,D1{0x10:0x8} packs the two bytes, and the
     * word lands at 0x628000 + A5+0x3300. */
    uint16_t packed = (uint16_t)((rd16(m, IN_YA) & 0x00FFu)
                               | ((rd16(m, IN_XA) & 0x00FFu) << 8));
    wr16(m, PROJ_BASE + (uint32_t)(int32_t)(int16_t)rd16(m, PROJ_OFS), packed);

    wr32(m, MIR_X, rd32(m, CAM_X));                     /* 0x13E890 */
    wr32(m, MIR_Y, rd32(m, CAM_Y));                     /* 0x13E896 */
    return 0;
}
