/* ROM 0x18098C: boss-part placement/first-frame initializer (A0 = object).
 * If A5+0x1D28 is zero the object is freed at once. Otherwise 0x17BD68 places
 * it relative to its parent, and if +0x50/+0x54 leave the cmp2.w windows at
 * 0x1849F8 / 0x1849FC it is freed too. On screen it plays sound 0x3C, loads the
 * stats record 0x180A26 (0x184888), starts animation index 7 (0x180A3C ->
 * 0x179A5C), installs script 0x180A5C (+0x48 timer word, +0x4A = 0), sets the
 * speed +0x42 = -0x100, draws one 0x1794F0 random of range 0x200 and negates it
 * into +0x5C, and re-arms the tick at 0x17C05E. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"      /* Sound_PlayHooked */
#include "../objects/obj_anim.h"       /* Obj_AnimInit */
#include "../objects/obj_pool.h"       /* Obj_Free */
#include "../objects/obj_shared_ext.h" /* Obj_184888 */

#define STATS_180A26 0x00180A26u
#define ANIM_180A3C  0x00180A3Cu
#define SCRIPT_180A5C 0x00180A5Cu
#define XBOUND_LO 0x001849F8u
#define YBOUND_LO 0x001849FCu
#define SPAWN_SEED (F3_A5 + 0x1D28u)

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x1794F0: bounded pseudo-random word. D0 holds the range; the seed at
 * A5+0x4B3C is multiplied by 5 and decremented each call, and the result is the
 * masked seed reduced by the range. Returns D0 (D1/D2 preserved). */
static uint32_t sub_1794F0(const f3mem_t *m, uint32_t d0)
{
    uint16_t d0w = (uint16_t)d0;
    uint16_t d1 = 0x0020u;                              /* bfffo d0{0:32},D1 */
    for (uint16_t i = 0u; i < 32u; i++) {
        if (d0 & (0x80000000u >> i)) { d1 = i; break; }
    }
    if (d1 != 0u) {                                     /* 0x1794F8 beq */
        d1 = (uint16_t)(d1 - 0x0010u);                  /* 0x1794FC */
        uint16_t mask = rd16(m, 0x00179532u + (uint16_t)((int16_t)d1 * 2)); /* 0x179500 */
        uint16_t s = (uint16_t)(rd16(m, F3_A5 + 0x4B3Cu) & mask); /* 0x179506..0x17950A */
        if ((int16_t)d0w < (int16_t)s)                  /* 0x17950C cmp/bge */
            s = (uint16_t)(s - d0w);                    /* 0x179512 */
        d0w = s;                                        /* 0x179514 */
    }
    uint32_t seed = rd32(m, F3_A5 + 0x4B3Cu);           /* 0x179516..0x179524 */
    seed = (seed * 5u) - 1u;
    wr32(m, F3_A5 + 0x4B3Cu, (seed << 16) | (seed >> 16));
    return (d0 & 0xFFFF0000u) | d0w;
}

/* ROM 0x17BD68: place relative to the parent with the signed bytes +0x0C/+0x0D. */
static void sub_17BD68(const f3mem_t *m, uint32_t obj, uint32_t *d0_io,
                       uint32_t *d1_io, uint32_t *a4_io)
{
    uint32_t d0 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Cu); /* bfexts {0:8} */
    uint32_t d1 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Du); /* bfexts {8:8} */
    wr16(m, obj + 0x16u, (uint16_t)d0);
    wr16(m, obj + 0x1Au, (uint16_t)d1);
    uint32_t a4 = rd32(m, obj + 0x08u);
    d0 = setw(d0, rd16(m, a4 + 0x50u));
    d1 = setw(d1, rd16(m, a4 + 0x54u));
    d0 = setw(d0, (uint16_t)(d0 + rd16(m, obj + 0x16u)));
    d1 = setw(d1, (uint16_t)(d1 + rd16(m, obj + 0x1Au)));
    wr16(m, obj + 0x50u, (uint16_t)d0);
    wr16(m, obj + 0x54u, (uint16_t)d1);
    wr16(m, obj + 0x14u, (uint16_t)d0);
    wr16(m, obj + 0x18u, (uint16_t)d1);
    *d0_io = d0;
    *d1_io = d1;
    *a4_io = a4;
}

int Fn_18098C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];
    uint32_t a4 = r->a[4];

    uint32_t v = rd32(m, SPAWN_SEED);                   /* 0x18098C */
    wr32(m, obj + 0x4Cu, v);
    if (v == 0u)                                        /* 0x180992 beq */
        goto free_obj;

    sub_17BD68(m, obj, &d0, &d1, &a4);                  /* 0x180996 bsr */
    d0 = setw(d0, rd16(m, obj + 0x50u));                /* 0x18099A */
    {
        int16_t x = (int16_t)d0;
        if (x < (int16_t)rd16(m, XBOUND_LO) ||          /* 0x18099E cmp2.w */
            x > (int16_t)rd16(m, XBOUND_LO + 2u))       /* 0x1809A6 bcs */
            goto free_obj;
    }
    d0 = setw(d0, rd16(m, obj + 0x54u));                /* 0x1809AA */
    {
        int16_t y = (int16_t)d0;
        if (y < (int16_t)rd16(m, YBOUND_LO) ||          /* 0x1809AE cmp2.w */
            y > (int16_t)rd16(m, YBOUND_LO + 2u))       /* 0x1809B6 bcs */
            goto free_obj;
    }

    Sound_PlayHooked(m, 0x003Cu);                       /* 0x1809BA..0x1809CA */
    Obj_184888(m, obj, STATS_180A26);                   /* 0x1809CE..0x1809D4 */

    a1 = ANIM_180A3C;                                   /* 0x1809DA */
    uint16_t anim = rd16(m, a1);
    a1 += 2u;
    d0 = setw(d0, anim);
    wr16(m, obj + 0x40u, (uint16_t)d0);                 /* 0x1809E2 */
    wr32(m, obj + 0x3Au, a1);                           /* 0x1809E6 */
    if (Obj_AnimInit(m, obj, (uint16_t)d0) < 0)         /* 0x1809EA..0x1809F4 */
        return -1;

    a1 = SCRIPT_180A5C;                                 /* 0x1809F6 */
    wr32(m, obj + 0x44u, a1);                           /* 0x1809FC */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));             /* 0x180A00 */
    wr16(m, obj + 0x4Au, 0u);                           /* 0x180A06 */
    wr16(m, obj + 0x42u, 0xFF00u);                      /* 0x180A0A move.w #-0x100 */
    d0 = sub_1794F0(m, setw(d0, 0x0200u));              /* 0x180A10..0x180A16 */
    d0 = setw(d0, (uint16_t)(0u - (uint16_t)d0));       /* 0x180A1C neg.w */
    wr16(m, obj + 0x5Cu, (uint16_t)d0);                 /* 0x180A1E */
    wr32(m, obj + 0x00u, 0x0017C05Eu);                  /* 0x180A22 -> 0x17C056 */
    goto out;

free_obj:
    Obj_Free(m, obj);                                   /* 0x17C78E */

out:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    r->a[4] = a4;
    return 0;
}
