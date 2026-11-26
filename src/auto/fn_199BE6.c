/* ROM 0x199BE6: boss palette cycling and flash-ramp controller (A0 = object).
 * Transitions to 0x17A85C (+0x06=2) when A5+0x4B38 == -1. Otherwise uploads
 * phase palettes at 0x582C/0x585C if A5+0x0879 bit 0 is clear (selecting set
 * by bit 1), uploads cycling 3-entry palette from 0x199EB0[(A5+0x0878)%3] to
 * 0x5874, runs palette ramp/flash state machine on +0x48/+0x44 when A5+0x4BA8
 * bit 7 is set, runs periodic 0x3C-frame palette ramp on +0x4C when A5+0x4BA8
 * bit 6 is set, and re-arms 0x199BE6. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../video/palette.h"

/* ROM 0x00183C wrapper returning 0xFFFFFFFF on slot exhaustion (0x0017C8). */
static uint32_t pal_ramp_regs(const f3mem_t *m, uint32_t a0, uint32_t a1,
                              uint32_t d0, uint32_t d1, uint32_t d2)
{
    uint32_t slot = Pal_QueueRamp(m, a0, a1, d0, d1, d2);
    return (slot == 0u) ? 0xFFFFFFFFu : slot;
}

int Fn_199BE6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {          /* 0x199BE6 */
        wr16(m, obj + 0x06u, 2u);                           /* 0x17A84C */
        wr32(m, obj + 0x00u, 0x0017A85Cu);                  /* 0x17A854 */
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x0879u) & 0x01u) == 0u) {          /* 0x199BF2 */
        if ((rd8(m, F3_A5 + 0x0879u) & 0x02u) == 0u) {      /* 0x199BFC */
            Pal_QueueUpload(m, 0x00199E00u, 0x582Cu, 4u);   /* 0x199C18 */
            Pal_QueueUpload(m, 0x00199E80u, 0x585Cu, 6u);   /* 0x199C34 */
        } else {
            Pal_QueueUpload(m, 0x00199E10u, 0x582Cu, 4u);   /* 0x199C54 */
            Pal_QueueUpload(m, 0x00199E98u, 0x585Cu, 6u);   /* 0x199C70 */
        }
    }

    {
        uint32_t num = rd16(m, F3_A5 + 0x0878u);            /* 0x199C7A */
        uint16_t q = (uint16_t)(num / 3u);                  /* 0x199C80 */
        uint16_t rem = (uint16_t)(num % 3u);
        uint32_t d0 = ((uint32_t)q << 16) | rem;            /* 0x199C84 */
        uint32_t a1 = 0x00199EB0u + (uint32_t)rem * 4u;     /* 0x199C86..0x199C8C */
        Pal_QueueUpload(m, a1, 0x5874u, 3u);                /* 0x199C9E */
        r->d[0] = d0;
        r->a[1] = a1;
    }

    if ((rd8(m, F3_A5 + 0x4BA8u) & 0x80u) != 0u) {          /* 0x199CA8 */
        int16_t t48 = (int16_t)rd16(m, obj + 0x48u);        /* 0x199CB2 */
        if (t48 == 0) {
            Pal_QueueUpload(m, 0x00199E30u, 0x583Cu, 4u);   /* 0x199CFA */
        } else if (t48 < 0) {
            uint32_t v = rd32(m, obj + 0x44u);              /* 0x199D08 */
            uint32_t q = v / 0x18u;                         /* 0x199D0C */
            uint16_t rem;
            uint32_t d0;
            if (q <= 0xFFFFu) {
                rem = (uint16_t)(v % 0x18u);
                d0 = ((uint32_t)(uint16_t)q << 16) | rem;
            } else {
                d0 = v;
                rem = (uint16_t)(d0 >> 16);
            }
            r->d[0] = d0;
            if (rem == 0u) {                                /* 0x199D12 */
                if ((d0 & 0x00010000u) != 0u) {             /* 0x199D18: bftst D0{15:1} */
                    r->d[0] = pal_ramp_regs(m, 0x00199E20u, 0x00199E30u, 0x583Cu, 0x18u, 4u); /* 0x199D70 */
                } else {
                    r->d[0] = pal_ramp_regs(m, 0x00199E30u, 0x00199E20u, 0x583Cu, 0x18u, 4u); /* 0x199D3E */
                    wr16(m, obj + 0x48u, 0x001Cu);          /* 0x199D48 */
                }
            }
            wr32(m, obj + 0x44u, v + 1u);                   /* 0x199D7A */
        } else if (t48 == 4) {                              /* 0x199CBE */
            Pal_QueueUpload(m, 0x00199E40u, 0x583Cu, 4u);   /* 0x199CDA */
        }
        wr16(m, obj + 0x48u, (uint16_t)(rd16(m, obj + 0x48u) - 1u)); /* 0x199D7E */
    }

    if ((rd8(m, F3_A5 + 0x4BA8u) & 0x40u) != 0u) {          /* 0x199D82 */
        uint32_t v = rd32(m, obj + 0x4Cu);                  /* 0x199D8C */
        uint32_t q = v / 0x3Cu;                             /* 0x199D90 */
        uint16_t rem;
        uint32_t d0;
        if (q <= 0xFFFFu) {
            rem = (uint16_t)(v % 0x3Cu);
            d0 = ((uint32_t)(uint16_t)q << 16) | rem;
        } else {
            d0 = v;
            rem = (uint16_t)(d0 >> 16);
        }
        r->d[0] = d0;
        if (rem == 0u) {                                    /* 0x199D96 */
            if ((d0 & 0x00010000u) != 0u)                   /* 0x199D9C: bftst D0{15:1} */
                r->d[0] = pal_ramp_regs(m, 0x00199E60u, 0x00199E70u, 0x584Cu, 0x3Cu, 4u); /* 0x199DEE */
            else
                r->d[0] = pal_ramp_regs(m, 0x00199E70u, 0x00199E60u, 0x584Cu, 0x3Cu, 4u); /* 0x199DC2 */
        }
        wr32(m, obj + 0x4Cu, v + 1u);                       /* 0x199DF8 */
    }

    wr32(m, obj + 0x00u, 0x00199BE6u);                      /* 0x199DFC -> 0x199BDE */
    return 0;
}
