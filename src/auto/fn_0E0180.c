/* ROM 0x0E0180: trail node tick (A0 = object) — follow +0x22 (child of a
 * living parent).
 * While +0x22 is set the grandparent ((+0x08)->+0x08) is looked up: if its
 * +0x06 counter is 0 the flag is cleared, else +0x18 is replaced by the
 * grandparent's +0x3E and offset by 0x0D4A12(7, grandparent+0x1E) — the same
 * polar table as 0x0D4A0C. Then +0x0C is subtracted from +0x14, the script at
 * +0x48 is stepped (0x0D0690) and, unless +0x4C is still running, the node
 * unlinks itself from the +0x50/+0x54 list (both ways when the next pointer is
 * a real address) and enters the dying chain (0x0CEE74). */
#include "fn_auto.h"
#include "../objects/obj_common.h"     /* Obj_StartDying */

/* ROM 0x0D4A12: D0 = swap(table[d1w] * d0w), D1 = d0w (see 0x0D4A0C). */
static uint32_t sub_0D4A12(const f3mem_t *m, uint16_t d0w, uint16_t d1w)
{
    uint32_t v = rd32(m, 0x000D4A5Au + (uint32_t)(int16_t)d1w * 4u);
    uint32_t prod = v * (uint32_t)d0w;
    return (prod << 16) | (prod >> 16);
}

/* ROM 0x0D0690 with the D0/A1 register outputs the callers observe. */
static void step_script_regs(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)                                      /* 0x0D0696 beq/rts */
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;         /* 0x0D069A..0x0D069E */
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);                          /* 0x0D06A0 */
    a1 += 2u;
    if ((int16_t)d0 < 0) {                              /* 0x0D06A2 bmi 0x0D06B8 */
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0) {                                /* 0x0D06B8 tst.w/beq */
            *d0_io = (*d0_io & 0xFFFF0000u) | d0;
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;                  /* 0x0D06BE suba.w */
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = (*d0_io & 0xFFFF0000u) | pal;
        wr8(m, obj + 0x1Cu, (uint8_t)pal);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        *a1_io = a1;
        return;
    }

    wr16(m, obj + 0x4Cu, d0);                           /* 0x0D06A6 */
    uint16_t pal = rd16(m, a1);                         /* 0x0D06AA */
    a1 += 2u;
    *d0_io = (*d0_io & 0xFFFF0000u) | pal;
    wr8(m, obj + 0x1Cu, (uint8_t)pal);                  /* 0x0D06AC */
    wr32(m, obj + 0x10u, rd32(m, a1));                  /* 0x0D06B0 */
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));                  /* 0x0D06B4 */
    *a1_io = a1;
}

int Fn_0E0180(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if (rd8(m, obj + 0x22u) != 0u) {                    /* 0x0E0180 tst.b/beq */
        a1 = rd32(m, obj + 0x08u);                      /* 0x0E0188 */
        a1 = rd32(m, a1 + 0x08u);                       /* 0x0E018C */
        if (rd8(m, a1 + 0x06u) == 0u) {                 /* 0x0E0190 */
            wr8(m, obj + 0x22u, 0u);                    /* 0x0E0198 */
        } else {
            wr32(m, obj + 0x18u, rd32(m, a1 + 0x3Eu));  /* 0x0E01A0 */
            d0 = sub_0D4A12(m, 0x0007u, (uint16_t)rd8(m, a1 + 0x1Eu)); /* 0x0E01A6.. */
            d1 = 0x0007u;                               /* 0x0D4A12 leaves D1 = d0w */
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x0E01B4 */
        }
    }

    d0 = rd32(m, obj + 0x0Cu);                          /* 0x0E01B8 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);    /* 0x0E01BC */
    step_script_regs(m, obj, &d0, &a1);                 /* 0x0E01C0 */
    if (rd16(m, obj + 0x4Cu) != 0u) {                   /* 0x0E01C6 beq */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;                                       /* 0x0E01CE rts */
    }

    a1 = rd32(m, obj + 0x50u);                          /* 0x0E01D0 */
    uint32_t a2 = rd32(m, obj + 0x54u);                 /* 0x0E01D4 */
    wr32(m, a1 + 0x54u, a2);                            /* 0x0E01D8 */
    if ((int32_t)a2 >= 0)                               /* 0x0E01DC bmi */
        wr32(m, a2 + 0x50u, a1);                        /* 0x0E01E2 */
    Obj_StartDying(m, obj);                             /* 0x0E01E6 -> 0x0CEE74 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
