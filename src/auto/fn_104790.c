/* ROM 0x104790: multi-jointed/turret enemy angle-tracking tick (A0 = object,
 * +0x08 = parent). If parent->+0x06 == 0, spawns 0x0DC49A and starts dying
 * (0x0CEE74); if parent->+0x06 == 9, starts dying. Otherwise copies +0x1D and
 * +0x08.w (+0x5C) from parent, refreshes target angle +0x4C on timer +0x23
 * (when +0x3E.w == 0) or inherits +0x58/+0x4C from +0x50, and steps three
 * 16.16 angles (+0x44, +0x48, +0x0C) toward the clamped target via 0x1048B2 +
 * 0x0D0C20 using speed table 0x10491A[+0x3E.w]. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../enemy/obj_E8B72.h"

static uint32_t swap32(uint32_t v) { return (v << 16) | (v >> 16); }
static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x1048B2: clamp target angle +0x4C to within +/- +0x5C of +0x58 (mod 360) */
static uint16_t sub_1048B2(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0w = rd16(m, obj + 0x58u);                 /* 0x1048B2 */
    uint16_t d1w = d0w;                                  /* 0x1048B6 */
    uint16_t d2w = rd16(m, obj + 0x4Cu);                 /* 0x1048B8 */
    d0w = (uint16_t)(d0w + rd16(m, obj + 0x5Cu));        /* 0x1048BC */
    if (d0w >= 0x0168u)                                  /* 0x1048C0..0x1048C4 bcs */
        d0w = (uint16_t)(d0w - 0x0168u);                 /* 0x1048C8 */
    d1w = (uint16_t)(d1w - rd16(m, obj + 0x5Cu));        /* 0x1048CC */
    if ((int16_t)d1w < 0)                                /* 0x1048D0 bpl */
        d1w = (uint16_t)(d1w + 0x0168u);                 /* 0x1048D4 */

    if (d1w > d0w) {                                     /* 0x1048D8..0x1048DA bhi */
        if (d2w <= d0w || d2w >= d1w)                    /* 0x1048EC..0x1048F2 */
            return d2w;
    } else {
        if (d2w <= d0w && d2w >= d1w)                    /* 0x1048DE..0x1048E6 */
            return d2w;
    }

    uint16_t diff = (uint16_t)(d2w - rd16(m, obj + 0x58u)); /* 0x1048F4 */
    if ((int16_t)diff < 0)                               /* 0x1048F8 bpl */
        diff = (uint16_t)(diff + 0x0168u);               /* 0x1048FC */
    return (diff < 0x00B4u) ? d0w : d1w;                 /* 0x104900..0x10490C */
}

/* ROM 0x0D0C20: step 16.16 angle d0 toward d2w by step d1 (16.16) mod 360.0 */
static uint32_t sub_0D0C20(uint32_t d0, uint32_t *d1_io, uint16_t d2w)
{
    uint32_t d1 = swap32(*d1_io);                        /* 0x0D0C22 */
    d0 = swap32(d0);                                     /* 0x0D0C20 */
    uint16_t d3w = (uint16_t)(d2w - (uint16_t)d0);       /* 0x0D0C24..0x0D0C26 */
    if ((int16_t)d3w < 0)                                /* 0x0D0C28 bpl */
        d3w = (uint16_t)(d3w + 0x0168u);                 /* 0x0D0C2C */

    if (d3w > 0x00B4u) {                                 /* 0x0D0C30..0x0D0C34 bls */
        d3w = (uint16_t)(0x0168u - d3w);                 /* 0x0D0C38..0x0D0C3A */
        if (d3w <= (uint16_t)d1) {                       /* 0x0D0C3E..0x0D0C40 bls */
            *d1_io = d1;
            return swap32(setw(d0, d2w));                /* 0x0D0C74..0x0D0C78 */
        }
        d0 = swap32(d0);                                 /* 0x0D0C44 */
        d1 = swap32(d1);                                 /* 0x0D0C46 */
        d0 = d0 - d1;                                    /* 0x0D0C48 */
        if ((int32_t)d0 < 0)                             /* 0x0D0C4A bpl */
            d0 += 0x01680000u;                           /* 0x0D0C4E */
        *d1_io = d1;
        return d0;
    }

    if (d3w <= (uint16_t)d1) {                           /* 0x0D0C56..0x0D0C58 bls */
        *d1_io = d1;
        return swap32(setw(d0, d2w));                    /* 0x0D0C74..0x0D0C78 */
    }
    d0 = swap32(d0);                                     /* 0x0D0C5C */
    d1 = swap32(d1);                                     /* 0x0D0C5E */
    d0 = d0 + d1;                                        /* 0x0D0C60 */
    if (d0 >= 0x01680000u)                               /* 0x0D0C62..0x0D0C68 bcs */
        d0 -= 0x01680000u;                               /* 0x0D0C6C */
    *d1_io = d1;
    return d0;
}

int Fn_104790(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t d2 = r->d[2];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x104790 */
    r->a[1] = a1;

    uint8_t state = rd8(m, a1 + 0x06u);                  /* 0x104794 */
    d0 = (d0 & 0xFFFFFF00u) | state;
    if (state == 0u) {                                   /* 0x104798 */
        uint32_t child = Obj_Spawn(m, obj, 0x000DC49Au, 3u, 2u, 0u); /* 0x10485E..0x104870 */
        d0 = child;
        if ((int32_t)child >= 0) {                       /* 0x10487A..0x10487C */
            a1 = child;                                  /* 0x104882 */
            r->a[1] = a1;
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));   /* 0x104884 */
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 8u)); /* 0x10488A */
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));   /* 0x10488E */
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 8u)); /* 0x104894 */
            wr16(m, a1 + 0x60u, 5u);                     /* 0x104898 */
            uint16_t idx = rd16(m, obj + 0x3Au);         /* 0x10489E */
            d0 = setw(d0, idx);
            /* 0x1048A2: table at 0x1048A4 + 0x6C = 0x104910 */
            wr16(m, a1 + 0x58u, rd16(m, 0x00104910u + (uint32_t)(int32_t)(int16_t)idx * 2u));
        }
        Obj_StartDying(m, obj);                          /* 0x10487C / 0x1048AC */
        r->d[0] = d0;
        return 0;
    }

    if (state == 9u) {                                   /* 0x10479C..0x1047A0 */
        Obj_StartDying(m, obj);
        r->d[0] = d0;
        return 0;
    }

    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));             /* 0x1047A6 */
    wr16(m, obj + 0x5Cu, rd16(m, a1 + 0x08u));           /* 0x1047AC */

    uint16_t idx = rd16(m, obj + 0x3Eu);                 /* 0x1047B2 */
    d1 = setw(d1, idx);
    if (idx == 0u) {                                     /* 0x1047B6 */
        wr16(m, obj + 0x58u, rd16(m, obj + 0x44u));      /* 0x1047BA */
        uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u); /* 0x1047C0 */
        wr8(m, obj + 0x23u, cnt);
        if (cnt == 0u) {                                 /* 0x1047C4 */
            uint16_t r0 = Rng_Mix1D60(m);                /* 0x1047C8 */
            uint16_t sum = r0;                           /* 0x1047CE */
            sum = (uint16_t)(sum + (Rng_Mix1D60(m) & 0x003Fu)); /* 0x1047D0..0x1047DA */
            r0 = (uint16_t)(Rng_Mix1D60(m) & 0x001Fu);   /* 0x1047DC..0x1047E2 */
            sum = (uint16_t)(sum + r0);                  /* 0x1047E6 */
            d0 = setw(d0, r0);
            d2 = setw(d2, sum);
            wr16(m, obj + 0x4Cu, sum);                   /* 0x1047E8 */
            wr8(m, obj + 0x23u, 0x3Cu);                  /* 0x1047EC */
        }
    } else {
        uint32_t a2 = rd32(m, obj + 0x50u);              /* 0x1047F6 */
        wr16(m, obj + 0x58u, rd16(m, a2 + 0x58u));       /* 0x1047FA */
        wr16(m, obj + 0x4Cu, rd16(m, a2 + 0x4Cu));       /* 0x104800 */
    }

    /* 0x104806: table at 0x104808 + 0x112 = 0x10491A */
    uint32_t a2 = rd32(m, 0x0010491Au + (uint32_t)(int32_t)(int16_t)idx * 4u);

    d2 = setw(d2, sub_1048B2(m, obj));                   /* 0x10480E */
    d0 = rd32(m, obj + 0x44u);                           /* 0x104814 */
    d1 = rd32(m, a2); a2 += 4u;                          /* 0x104818 */
    d0 = sub_0D0C20(d0, &d1, (uint16_t)d2);              /* 0x10481A */
    wr32(m, obj + 0x44u, d0);                            /* 0x104820 */
    wr32(m, obj + 0x58u, d0);                            /* 0x104824 */

    d2 = setw(d2, sub_1048B2(m, obj));                   /* 0x104828 */
    d0 = rd32(m, obj + 0x48u);                           /* 0x10482E */
    d1 = rd32(m, a2); a2 += 4u;                          /* 0x104832 */
    d0 = sub_0D0C20(d0, &d1, (uint16_t)d2);              /* 0x104834 */
    wr32(m, obj + 0x48u, d0);                            /* 0x10483A */
    wr32(m, obj + 0x58u, d0);                            /* 0x10483E */

    d2 = setw(d2, sub_1048B2(m, obj));                   /* 0x104842 */
    d0 = rd32(m, obj + 0x0Cu);                           /* 0x104848 */
    d1 = rd32(m, a2);                                    /* 0x10484C */
    d0 = sub_0D0C20(d0, &d1, (uint16_t)d2);              /* 0x10484E */
    wr32(m, obj + 0x0Cu, d0);                            /* 0x104854 */
    wr32(m, obj + 0x58u, d0);                            /* 0x104858 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    return 0;                                            /* 0x10485C */
}
