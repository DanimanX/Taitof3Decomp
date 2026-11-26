/* ROM 0x101E68: scrolling breakable / multi-phase obstacle tick (A0 = object).
 * Dies (clears +0x06 and calls Obj_StartDying 0x0CEE74) if A5-0x144B bit 5 is
 * set or after subtracting stage scroll (A5+0x4224, A5+0x4228) from (+0x3A,
 * +0x3E) when +0x3A < -0x50. Otherwise copies (+0x3A, +0x3E) to (+0x14, +0x18),
 * dispatches byte state (+0x06) via jump table 0x101EB8 (1: collision list +
 * break on +0x24 <= 0 with sound 0x15, spawns 0x102010 & 0x102278, score 0x3E8,
 * frame 0x101FD2, +0x23=0xC8, state=2; 2: countdown +0x23 with 0x0CF898 orbit
 * and +0x3E += 0x9C40, transitioning to state 3 with frame 0x101FDC at 0;
 * 3: no-op), and applies Obj_BlinkColour (0x0CBE34). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D0DC2 with A1 register tracking. */
static void add_coll_regs(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint32_t p = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, p, obj);
    wr32(m, F3_A5 + 0x08FEu, p + 4u);
    p = rd32(m, F3_A5 + 0x0906u);
    wr32(m, p, obj);
    p += 4u;
    wr32(m, F3_A5 + 0x0906u, p);
    *a1_io = p;
}

int Fn_101E68(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {      /* 0x101E68 */
        wr8(m, obj + 0x06u, 0u);                         /* 0x101EAE */
        Obj_StartDying(m, obj);                          /* 0x101EB2 -> 0x0CEE74 */
        return 0;
    }

    uint32_t d0 = rd32(m, F3_A5 + 0x4224u);              /* 0x101E72 */
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);     /* 0x101E76 */
    d0 = rd32(m, F3_A5 + 0x4228u);                       /* 0x101E7A */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);     /* 0x101E7E */
    r->d[0] = d0;

    if ((int16_t)rd16(m, obj + 0x3Au) < -0x0050) {       /* 0x101E82..0x101E88 */
        wr8(m, obj + 0x06u, 0u);                         /* 0x101EAE */
        Obj_StartDying(m, obj);                          /* 0x101EB2 -> 0x0CEE74 */
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));          /* 0x101E8C */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));          /* 0x101E92 */

    uint16_t state = rd8(m, obj + 0x06u);                /* 0x101E98..0x101E9A */
    r->d[0] = setw(d0, state);

    switch (state) {                                     /* 0x101E9E table 0x101EB8 */
    case 1u:                                             /* 0x101EC8 */
        add_coll_regs(m, obj, &r->a[1]);
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {        /* 0x101ECE..0x101ED2 */
            Sound_PlayHooked(m, 0x0015u);                /* 0x101ED6..0x101EE6 */
            Obj_Spawn(m, obj, 0x00102010u, 9u, 2u, 0u);  /* 0x101EEA..0x101F02 */
            r->d[0] = Obj_Spawn(m, obj, 0x00102278u, 9u, 2u, 0u); /* 0x101F06..0x101F1E */
            Obj_0D0E04(m, obj, 0x000003E8u);             /* 0x101F22..0x101F28 */
            Obj_SetFrame(m, obj, 0x00101FD2u);           /* 0x101F2E..0x101F32 */
            r->d[0] = setw(r->d[0], rd16(m, 0x00101FD2u));
            r->a[1] = 0x00101FD8u;
            wr8(m, obj + 0x23u, 0xC8u);                  /* 0x101F38 */
            wr8(m, obj + 0x06u, 2u);                     /* 0x101F3E */
        }
        break;

    case 2u: {                                           /* 0x101F46 */
        uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, c);
        if (c != 0u) {                                   /* 0x101F4A */
            wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + 0x00009C40u); /* 0x101F4E */
            uint32_t dx = 0u, dy = 0u;
            if (Obj_0CF898(m, obj, &dx, &dy) < 0)        /* 0x101F56 */
                return -1;
            r->d[0] = dx;
            r->d[1] = dy;
            r->a[1] = rd32(m, obj + 0x0Cu);
            wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + dx); /* 0x101F5C */
            wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy); /* 0x101F60 */
        } else {                                         /* 0x101F66 */
            wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 0x000Eu));
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 0x000Eu));
            wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
            wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
            Obj_SetFrame(m, obj, 0x00101FDCu);           /* 0x101F7E..0x101F82 */
            r->d[0] = setw(r->d[0], rd16(m, 0x00101FDCu));
            r->a[1] = 0x00101FE2u;
            wr8(m, obj + 0x06u, 3u);                     /* 0x101F88 */
        }
        break;
    }

    case 3u:                                             /* 0x101F8E: rts */
        break;

    default:
        return -1;
    }

    Obj_BlinkColour(m, obj);                             /* 0x101EA6 */
    return 0;
}
