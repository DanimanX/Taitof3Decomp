/* ROM 0x0F8D54: multi-part boss/enemy tick (A0 = object).
 * When state (+0x06) != 2 and HP (+0x24) <= 0, switches to state 2, plays
 * sound 0x33, and spawns child objects 0x0F8F52 and 0x0F9108. Scrolls
 * +0x3A/+0x3E, runs state handler (0x0F8E8A[state]), steps script
 * (0x0D0690), and either transitions to 0x0F95FE when timer (+0x4C) hits 0
 * or applies 0x0CF898 orbit offset, flashes (0x0CBE34), and draws (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0F8D54(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if (rd8(m, obj + 0x06u) != 2u) {                          /* 0x0F8D54 */
        Obj_0D0DC2(m, obj);                                   /* 0x0F8D5E */
        a1 = rd32(m, F3_A5 + 0x0906u);
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {             /* 0x0F8D64 */
            a1 = 0x000F8F0Au;                                 /* 0x0F8D6C */
            Obj_StartBehaviour(m, obj, a1);                   /* 0x0F8D70 */
            d0 = (d0 & 0xFFFF0000u) | rd16(m, a1 + 2u);
            a1 += 8u;
            wr8(m, obj + 0x06u, 2u);                          /* 0x0F8D76 */
            Sound_PlayHooked(m, 0x0033u);                     /* 0x0F8D7C..0x0F8D8C */
            d0 = Obj_Spawn(m, obj, 0x000F8F52u, 9u, 2u, 0u);  /* 0x0F8D90..0x0F8DA8 */
            if ((int32_t)d0 >= 0) {                           /* 0x0F8DAC */
                a1 = d0;                                      /* 0x0F8DB2 */
                wr32(m, a1 + 0x14u, rd32(m, obj + 0x3Au));    /* 0x0F8DB4 */
                wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x000Au)); /* 0x0F8DBA */
                wr32(m, a1 + 0x18u, rd32(m, obj + 0x3Eu));    /* 0x0F8DC0 */
                wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0016u)); /* 0x0F8DC6 */
                d0 = Obj_Spawn(m, obj, 0x000F9108u, 9u, 2u, 0u); /* 0x0F8DCC..0x0F8DE4 */
                if ((int32_t)d0 >= 0) {                       /* 0x0F8DE8 */
                    a1 = d0;                                  /* 0x0F8DEE */
                    wr32(m, a1 + 0x14u, rd32(m, obj + 0x3Au)); /* 0x0F8DF0 */
                    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0016u)); /* 0x0F8DF6 */
                    wr32(m, a1 + 0x18u, rd32(m, obj + 0x3Eu)); /* 0x0F8DFC */
                    wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0026u)); /* 0x0F8E02 */
                }
            }
        }
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {            /* 0x0F8E08 */
        Obj_StartDying(m, obj);                               /* 0x0F8E0E -> 0x0CEE74 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    d0 = rd32(m, F3_A5 + 0x4224u);                            /* 0x0F8E14 */
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);          /* 0x0F8E18 */
    d0 = rd32(m, F3_A5 + 0x4228u);                            /* 0x0F8E1C */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);          /* 0x0F8E20 */
    Obj_FlagOffscreen(m, obj);                                /* 0x0F8E24 -> 0x0CF85A */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {                /* 0x0F8E2A */
        Obj_StartDying(m, obj);                               /* 0x0F8E30 -> 0x0CEE74 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));               /* 0x0F8E36 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));               /* 0x0F8E3C */

    uint16_t state = rd8(m, obj + 0x06u);                     /* 0x0F8E42..0x0F8E44 */
    d0 = (d0 & 0xFFFF0000u) | state;
    if (state == 0u) {                                        /* 0x0F8E48 -> 0x0F8E96 */
        Obj_ArmOnScreen(m, obj);                              /* 0x0F8E96 -> 0x0CF7FE */
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {            /* 0x0F8E9C */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u)); /* 0x0F8EA6 */
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u)); /* 0x0F8EAC */
            wr8(m, obj + 0x06u, 1u);                          /* 0x0F8EB2 */
        }
    }
    /* state 1, 2 -> 0x0F8EB8 (rts) */

    Obj_StepScript(m, obj);                                   /* 0x0F8E50 -> 0x0D0690 */
    if (rd16(m, obj + 0x4Cu) == 0u) {                         /* 0x0F8E56 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x001Au)); /* 0x0F8E7A */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0014u)); /* 0x0F8E80 */
        d0 = Obj_Spawn(m, obj, 0x000F8692u, 5u, 2u, 0u);      /* 0x0F9596..0x0F95AE */
        if ((int32_t)d0 < 0) {                                /* 0x0F95B2 */
            Obj_StartDying(m, obj);                           /* 0x0F95B4 -> 0x0CEE74 */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
        Obj_SeedFromParams(m, obj, 0x000F986Cu);              /* 0x0F95BA..0x0F95BE */
        Obj_SetScript(m, obj, 0x000F98A4u);                   /* 0x0F95C4..0x0F95C8 */
        a1 = 0x000F9A54u;                                     /* 0x0F95CE */
        Obj_StartBehaviour(m, obj, a1);                       /* 0x0F95D2 */
        d0 = (d0 & 0xFFFF0000u) | rd16(m, a1 + 2u);
        a1 += 8u;
        wr8(m, obj + 0x4Fu, 0x7Cu);                           /* 0x0F95D8 */
        wr8(m, obj + 0x1Du, 0x7Cu);                           /* 0x0F95DE */
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u)); /* 0x0F95E4 */
        wr16(m, obj + 0x0Cu, 0x0384u);                        /* 0x0F95EA */
        wr8(m, obj + 0x06u, 1u);                              /* 0x0F95F0 */
        wr32(m, obj, 0x000F95FEu);                            /* 0x0F95F6 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    if (Obj_0CF898(m, obj, &d0, &d1) < 0)                     /* 0x0F8E5E */
        return -1;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);          /* 0x0F8E64 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);          /* 0x0F8E68 */
    Obj_HitFlash(m, obj);                                     /* 0x0F8E6C -> 0x0CBE34 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    draw_sprite_regs(m, r);                                   /* 0x0F8E72 -> 0x001282 */
    return 0;
}
