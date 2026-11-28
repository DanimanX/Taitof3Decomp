/* ROM 0x184322: scripted sub-object initializer (A0 = object).
 * Stats/hitboxes come from 0x1844F8 (0x184888); 0x17BD68 then reads the two
 * signed offset bytes at +0x0C/+0x0D as +0x16/+0x1A and places +0x14/+0x18
 * (and +0x50/+0x54) at the parent's (+0x08) +0x50/+0x54 plus those offsets.
 * The zoom bytes +0x0D -> +0x1F and +0x1E, +0x0E -> +0x20 (via bfextu), the
 * movement speed +0x42 = (0xFF - +0x1E) << 8 / 0x64, and the top nibble of
 * +0x1E is trimmed off the hitboxes +0x2E/+0x32. Animation index 0 (0x18450E)
 * runs through 0x179A5C, script 0x184516 goes to +0x44 (+0x48 timer word,
 * +0x4A = 0) and the tick is armed at 0x1843AE. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"      /* Obj_AnimInit */
#include "../objects/obj_shared_ext.h"/* Obj_184888 */

#define STATS_1844F8  0x001844F8u
#define ANIM_18450E   0x0018450Eu     /* {index.w = 0, record...} */
#define SCRIPT_184516 0x00184516u

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x17BD68: place the object relative to its parent with the signed
 * offsets at +0x0C/+0x0D. Leaves D0/D1 sign-extended from those bytes and
 * A4 = the parent (+0x08). */
static void sub_17BD68(const f3mem_t *m, uint32_t obj, uint32_t *d0_io,
                       uint32_t *d1_io, uint32_t *a4_io)
{
    uint32_t d0 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Cu); /* bfexts clears the rest */
    uint32_t d1 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Du);
    wr16(m, obj + 0x16u, (uint16_t)d0);                 /* 0x17BD74 */
    wr16(m, obj + 0x1Au, (uint16_t)d1);                 /* 0x17BD78 */
    uint32_t a4 = rd32(m, obj + 0x08u);                 /* 0x17BD7C */
    d0 = setw(d0, rd16(m, a4 + 0x50u));                 /* 0x17BD80 */
    d1 = setw(d1, rd16(m, a4 + 0x54u));                 /* 0x17BD84 */
    d0 = setw(d0, (uint16_t)(d0 + rd16(m, obj + 0x16u))); /* 0x17BD88 */
    d1 = setw(d1, (uint16_t)(d1 + rd16(m, obj + 0x1Au))); /* 0x17BD8C */
    wr16(m, obj + 0x50u, (uint16_t)d0);                 /* 0x17BD90 */
    wr16(m, obj + 0x54u, (uint16_t)d1);                 /* 0x17BD94 */
    wr16(m, obj + 0x14u, (uint16_t)d0);                 /* 0x17BD98 */
    wr16(m, obj + 0x18u, (uint16_t)d1);                 /* 0x17BD9C */
    *d0_io = d0;
    *d1_io = d1;
    *a4_io = a4;
}

int Fn_184322(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a4 = r->a[4];

    Obj_184888(m, obj, STATS_1844F8);                   /* 0x184322..0x184328 */
    sub_17BD68(m, obj, &d0, &d1, &a4);                  /* 0x18432E bsr */

    d0 = rd8(m, obj + 0x0Eu);                           /* 0x184334 bfextu {0x10:8} */
    wr8(m, obj + 0x1Fu, (uint8_t)d0);                   /* 0x18433A */
    wr8(m, obj + 0x1Eu, (uint8_t)d0);                   /* 0x18433E */
    d0 = rd8(m, obj + 0x0Fu);                           /* 0x184342 bfextu {0x18:8} */
    wr8(m, obj + 0x20u, (uint8_t)d0);                   /* 0x184348 */

    d0 = rd8(m, obj + 0x1Eu);                           /* 0x18434C bfextu {0:8} */
    d0 = setw(d0, (uint16_t)((uint16_t)d0 - 0x00FFu));  /* 0x184352 subi.w */
    if ((int16_t)d0 < 0)                                /* 0x184356 bge */
        d0 = setw(d0, (uint16_t)(0u - (uint16_t)d0));   /* 0x18435C neg.w */
    d0 = setw(d0, (uint16_t)(d0 << 8));                 /* 0x18435E lsl.w #8 */
    {
        uint32_t num = d0 & 0xFFFFu;                    /* 0x184360 divu.w #0x64 */
        d0 = ((uint32_t)(num % 0x0064u) << 16) | (uint16_t)(num / 0x0064u);
    }
    wr16(m, obj + 0x42u, (uint16_t)d0);                 /* 0x184364 */

    d0 = (uint16_t)(rd8(m, obj + 0x1Eu) >> 4);           /* 0x184368 bfextu {0:4} */
    wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) - (uint16_t)d0)); /* 0x18436E */
    wr16(m, obj + 0x32u, (uint16_t)(rd16(m, obj + 0x32u) - (uint16_t)d0)); /* 0x184372 */

    uint32_t a1 = ANIM_18450E;                          /* 0x184376 movea.l */
    uint16_t anim = rd16(m, a1);                        /* 0x18437C move.w (A1)+,D0w */
    a1 += 2u;
    d0 = setw(d0, anim);
    wr16(m, obj + 0x40u, (uint16_t)d0);                 /* 0x18437E */
    wr32(m, obj + 0x3Au, a1);                           /* 0x184382 */
    if (Obj_AnimInit(m, obj, (uint16_t)d0) < 0)         /* 0x184386..0x184390 */
        return -1;

    a1 = SCRIPT_184516;                                 /* 0x184392 */
    wr32(m, obj + 0x44u, a1);                           /* 0x184398 */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));             /* 0x18439C */
    wr16(m, obj + 0x4Au, 0u);                           /* 0x1843A2 */
    wr32(m, obj + 0x00u, 0x001843AEu);                  /* 0x1843A6 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    r->a[4] = a4;
    return 0;
}
