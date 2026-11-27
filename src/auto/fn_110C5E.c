/* ROM 0x110C5E: head/segment spawn init (A0 = object). Seeds the hitbox and
 * stat block from the parameter record 0x10C8E2 (0x0CEDE4), loads the sprite
 * definition {pal.w, spr.l, hit.l} at 0x110DC2 (0x0CFA86), sets the state byte
 * +0x06 = 1, queues the 0x20-word palette 0x17D00 at 0x5900 (0x001660) and
 * arms the tick 0x110C9C. */
#include "fn_auto.h"
#include "../video/palette.h"

/* ROM 0x0CEDE4: object template init from the record at A1 + 0x14 -- the HP
 * head (+0x24/+0x28), the sprite list at +0x2C, the accel/cap longs at +0x30,
 * flags +0x34 bit 1, and the position shadow at +0x3A/+0x3E. Leaves A1 at
 * A1+0x1C (its last read is a non-advancing (A1)); D0 is untouched. */
static void sub_0CEDE4(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint32_t a1 = *a1_io + 0x14u;              /* ROM 0x0CEDE4 lea (0x14,A1),A1 */

    wr32(m, obj + 0x1Cu, 0u);                  /* ROM 0x0CEDE8 clr.l (0x1C,A0) */
    wr32(m, obj + 0x24u, 0u);                  /* ROM 0x0CEDEC clr.l (0x24,A0) */
    wr16(m, obj + 0x24u, rd16(m, a1));         /* ROM 0x0CEDF0 move.w (A1)+,(0x24,A0) */
    a1 += 2u;
    wr32(m, obj + 0x28u, 0u);                  /* ROM 0x0CEDF4 clr.l (0x28,A0) */
    wr16(m, obj + 0x28u, rd16(m, a1));         /* ROM 0x0CEDF8 move.w (A1)+,(0x28,A0) */
    a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));         /* ROM 0x0CEDFC move.l (A1)+,(0x2C,A0) */
    a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));         /* ROM 0x0CEE00 move.l (A1),(0x30,A0) */
    wr16(m, obj + 0x34u, 0u);                  /* ROM 0x0CEE04 clr.w (0x34,A0) */
    wr8(m, obj + 0x34u,                     /* ROM 0x0CEE08 bset.b #1,(0x34,A0) */
        (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr16(m, obj + 0x06u, 0u);                  /* ROM 0x0CEE0E clr.w (0x6,A0) */
    wr16(m, obj + 0x20u, 0u);                  /* ROM 0x0CEE12 clr.w (0x20,A0) */
    wr16(m, obj + 0x22u, 0u);                  /* ROM 0x0CEE16 clr.w (0x22,A0) */
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));/* ROM 0x0CEE1A */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));/* ROM 0x0CEE20 */
    wr8(m, obj + 0x4Fu, 0u);                   /* ROM 0x0CEE26 clr.b (0x4F,A0) */

    *a1_io = a1;                               /* A1 ends at A1+0x1C */
}

/* ROM 0x0CFA86: load {pal.w, spr.l, hit.l} from A1 into +0x1C/+0x10/+0x68.
 * Leaves A1 at A1+6 and D0.w = pal. */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t *a1_io, uint32_t *d0_io)
{
    uint32_t a1 = *a1_io;
    uint16_t pal = rd16(m, a1);                /* ROM 0x0CFA86 move.w (A1)+,D0 */
    a1 += 2u;
    *d0_io = (*d0_io & 0xFFFF0000u) | pal;
    wr8(m, obj + 0x1Cu, (uint8_t)pal);         /* ROM 0x0CFA88 move.b D0,(0x1C,A0) */
    wr32(m, obj + 0x10u, rd32(m, a1));         /* ROM 0x0CFA8C move.l (A1)+,(0x10,A0) */
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));         /* ROM 0x0CFA90 move.l (A1),(0x68,A0) */
    *a1_io = a1;
}

int Fn_110C5E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1 = 0x0010C8E2u;                 /* ROM 0x110C5E lea (0x10C8E2,PC),A1 */

    sub_0CEDE4(m, obj, &a1);                   /* ROM 0x110C62 jsr 0x000CEDE4 */
    a1 = 0x00110DC2u;                          /* ROM 0x110C68 lea (0x110DC2,PC),A1 */
    sub_0CFA86(m, obj, &a1, &d0);              /* ROM 0x110C6C jsr 0x000CFA86 */
    wr8(m, obj + 0x06u, 0x01u);                /* ROM 0x110C72 move.b #1,(0x6,A0) */
    Pal_QueueUpload(m, 0x00017D00u, 0x5900u, 0x0020u); /* ROM 0x110C78..0x110C8A */
    wr32(m, obj, 0x00110C9Cu);                 /* ROM 0x110C94 move.l #0x110C9C,(A0) */

    r->d[0] = d0;
    r->a[1] = a1;

    /* ROM 0x110C9A rts -- D1/D2/A0 restored by 0x001660's movem, A0 = obj */
    (void)r;
    return 0;
}
