/* ROM 0x1608EE: stage horizon line-scroll fill and twin boundary hazard spawner.
 * Fills line RAM 0x62A600 + (-0x5D56,A5) up to A5+0x3BBA+0x40 with 0x3F (via
 * 0x176992, D2=0), then from max(0, A5+0x3BBA+0x41)..0xE7 with sine wobble
 * (D2=5, step A5+0x3384). Computes perspective bounds via 0x16099A and checks
 * objects 0x41B190 / 0x41B21A via 0x1609EC, spawning 0x1113AC (group 0xB) and
 * playing hooked sound 0x24 when crossed, storing offsets in A5+0x3BC2/0x3BC6. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x176992: fill line RAM entries [d7w..d6w] from sine table 0x027E40 */
static void sub_176992(const f3mem_t *m, f3regs_t *r, uint32_t a0,
                       uint32_t d0, uint32_t d1, uint16_t d2w,
                       uint16_t d6w, uint16_t d7w)
{
    uint32_t a1 = 0x00027E40u;                           /* 0x176992 */
    wr32(m, F3_A5 + 0x4298u, d0);                        /* 0x176998 */
    uint16_t d5w = 0x003Fu;                              /* 0x17699C */
    uint32_t d3 = 0u;                                    /* 0x1769A0 */
    a0 += (uint32_t)(int32_t)(int16_t)d7w * 2u;          /* 0x1769A2 */
    for (;;) {
        d3 = (d3 & 0xFFFFFF00u) | rd8(m, F3_A5 + 0x4299u); /* 0x1769A6 */
        int16_t s = (int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)(uint16_t)d3 * 2u); /* 0x1769AA */
        int32_t prod = (int32_t)s * (int32_t)(int16_t)d2w; /* 0x1769AE */
        int32_t shifted = (prod < 0)
            ? ~(int32_t)((uint32_t)(~prod) >> 8)
            : (int32_t)((uint32_t)prod >> 8);            /* 0x1769B0 asr.l #8 */
        uint16_t outw = (uint16_t)((uint16_t)(uint32_t)shifted ^ d5w); /* 0x1769B2 */
        d0 = ((uint32_t)shifted & 0xFFFF0000u) | outw;
        wr16(m, a0, outw);                               /* 0x1769B4 */
        a0 += 2u;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d1); /* 0x1769B6 */
        d7w = (uint16_t)(d7w + 1u);                      /* 0x1769BA */
        if ((int16_t)d7w > (int16_t)d6w)                 /* 0x1769BC..0x1769BE */
            break;
    }
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[0] = a0;
    r->a[1] = a1;
    r->d[2] = setw(r->d[2], d2w);
    r->d[3] = d3;
    r->d[5] = setw(r->d[5], d5w);
    r->d[6] = setw(r->d[6], d6w);
    r->d[7] = setw(r->d[7], d7w);
}

/* ROM 0x16099A: perspective boundary Y from A5+0x4218 scroll */
static void sub_16099A(const f3mem_t *m, f3regs_t *r)
{
    int16_t d0w = (int16_t)(-(int16_t)rd16(m, F3_A5 + 0x4218u)); /* 0x16099A..0x16099E */
    uint16_t u0 = (uint16_t)((d0w + 0x007F) & 0x00FF);            /* 0x1609A0..0x1609A4 */
    uint32_t d1;
    if ((int16_t)u0 <= 0x0073) {                                  /* 0x1609A8..0x1609AC */
        uint16_t v = (uint16_t)(-(int16_t)u0 + 0x0074);
        d1 = (uint32_t)v << 6;
        uint32_t q = d1 / 0x28u;
        uint32_t rem = d1 % 0x28u;
        if (q <= 0xFFFFu)
            d1 = (rem << 16) | q;
        d1 = (uint32_t)(uint16_t)d1 * 0xE000u;                    /* 0x1609C0 mulu.w #-0x2000,D1 */
        d1 <<= 1;
        d1 = (d1 << 16) | (d1 >> 16);
        u0 = (uint16_t)(u0 - (uint16_t)d1);
    } else {
        uint16_t v = (uint16_t)((int16_t)u0 - 0x0073);
        d1 = (uint32_t)v << 6;
        uint32_t q = d1 / 0x28u;
        uint32_t rem = d1 % 0x28u;
        if (q <= 0xFFFFu)
            d1 = (rem << 16) | q;
        d1 = (uint32_t)(uint16_t)d1 * 0xE000u;                    /* 0x1609DC mulu.w #-0x2000,D1 */
        d1 <<= 1;
        d1 = (d1 << 16) | (d1 >> 16);
        u0 = (uint16_t)(u0 + (uint16_t)d1);
    }
    u0 = (uint16_t)(u0 - 0x000Cu);                                /* 0x1609E6 */
    r->d[0] = setw(r->d[0], u0);
    r->d[1] = d1;
}

/* ROM 0x1609EC: check boundary crossing on A0 against D0w/D1w; spawn 0x1113AC. */
static void sub_1609EC(const f3mem_t *m, f3regs_t *r, uint32_t a0)
{
    if ((int16_t)rd16(m, F3_A5 - 0x5DA8u) >= 0x0F00)     /* 0x1609EC */
        return;
    if ((int16_t)rd16(m, F3_A5 - 0x5DA4u) >= 0x0100)     /* 0x1609F6 */
        return;

    int16_t diff = (int16_t)((uint16_t)r->d[0] - rd16(m, a0 + 0x18u)); /* 0x160A00 */
    r->d[0] = setw(r->d[0], (uint16_t)diff);
    int16_t d1w = (int16_t)(uint16_t)r->d[1];
    if (diff < 0) {
        if (d1w < 0)
            return;
    } else {
        if (d1w > 0)
            return;
    }

    if ((int16_t)rd16(m, a0 + 0x4Cu) < 0)                /* 0x160A18 */
        return;
    if ((int16_t)rd16(m, F3_A5 - 0x5DA4u) < 0)           /* 0x160A20 */
        return;

    uint32_t saved_d0 = r->d[0];                         /* 0x160A28 */
    (void)Obj_Spawn(m, a0, 0x001113ACu, 0x000Bu, 1u, a0); /* 0x160A2C..0x160A42 */
    Sound_PlayHooked(m, 0x0024u);                        /* 0x160A46..0x160A56 */
    r->d[0] = saved_d0;                                  /* 0x160A5A */
}

int Fn_1608EE(const f3mem_t *m, f3regs_t *r)
{
    int16_t d0w = (int16_t)(rd16(m, F3_A5 + 0x3BBAu) + 0x0040); /* 0x1608EE..0x1608F2 */
    r->d[0] = setw(r->d[0], (uint16_t)d0w);
    if (d0w >= 0) {                                      /* 0x1608F6 */
        uint32_t a0 = 0x0062A600u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
        sub_176992(m, r, a0, 0u, 0u, 0u, (uint16_t)d0w, 0u);
    }

    d0w = (int16_t)(rd16(m, F3_A5 + 0x3BBAu) + 0x0041);  /* 0x160928..0x16092C */
    r->d[0] = setw(r->d[0], (uint16_t)d0w);
    if (d0w < 0x00E7) {                                  /* 0x160930..0x160934 */
        if (d0w < 0)                                     /* 0x160938 */
            d0w = 0;
        uint32_t a0 = 0x0062A600u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
        uint32_t d0_init = rd32(m, F3_A5 + 0x3384u);     /* 0x160950 -> 0x160960 pop D0 */
        sub_176992(m, r, a0, d0_init, 0x00080000u, 5u, 0x00E7u, (uint16_t)d0w);
    }

    r->a[0] = 0x0041B190u;                               /* 0x16096C */
    sub_16099A(m, r);                                    /* 0x160972 */
    r->d[1] = setw(r->d[1], rd16(m, F3_A5 + 0x3BC2u));   /* 0x160976 */
    sub_1609EC(m, r, 0x0041B190u);                       /* 0x16097A */
    wr16(m, F3_A5 + 0x3BC2u, (uint16_t)r->d[0]);         /* 0x16097E */

    r->a[0] = 0x0041B21Au;                               /* 0x160982 */
    sub_16099A(m, r);                                    /* 0x160988 */
    r->d[1] = setw(r->d[1], rd16(m, F3_A5 + 0x3BC6u));   /* 0x16098C */
    sub_1609EC(m, r, 0x0041B21Au);                       /* 0x160990 */
    wr16(m, F3_A5 + 0x3BC6u, (uint16_t)r->d[0]);         /* 0x160994 */
    return 0;
}
