/* ROM 0x10E9E2: mid-boss/large-enemy tick (A0 = object), twin of 0x110328.
 * When HP (+0x24) > 0: dies on bomb flag (A5-0x144B bit 5); otherwise counts
 * down +0x23 and alternates palette ramps (0x10ECEE <-> 0x10ECF2 on 0x5704)
 * via 0x00183C; applies scroll deltas (A5+0x4224/4228) to +0x14/+0x18; dies
 * if X < -0x40; registers in collision/hazard lists (0x0D0DC2), blinks
 * hit-flash colour (0x0CBE34), and draws sprite (0x001282). When HP <= 0:
 * adds 0x12C score timer (0x0D0E04), spawns 6 debris/explosion children
 * (0x10ECF6, +0x10=0..5, +0x60=0x0F) + 1 snake/worm spawner (0x10F90A),
 * plays hooked sound 0x1A, draws sprite, and starts dying (0x0CEE74). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setl_w(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

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
        r->d[1] = setl_w(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0D0E04 with A1 register tracking. */
static void add_score_timer(const f3mem_t *m, f3regs_t *r, uint32_t obj, uint32_t d0)
{
    uint8_t k = rd8(m, obj + 0x35u);
    r->d[0] = d0;
    if (k == 1u)
        r->a[1] = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
    else if (k == 2u)
        r->a[1] = rd32(m, obj + 0x36u);
    Obj_0D0E04(m, obj, d0);
}

/* ROM 0x10EA96: death burst when HP (+0x24) <= 0. */
static void death_burst(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    static const struct { int16_t dx, dy; uint16_t f58, f10; } parts[6] = {
        {  0x10, 0x13, 0x004Bu, 0u },
        { -0x11, 0x11, 0x0069u, 1u },
        {  0x06, 0x1F, 0x005Fu, 2u },
        { -0x01, 0x20, 0x0064u, 3u },
        {  0x12, 0x23, 0x0050u, 4u },
        {  0x10, 0x0E, 0x0055u, 5u },
    };

    add_score_timer(m, r, obj, 0x0000012Cu);                      /* 0x10EA96..0x10EA9C */
    uint32_t d1 = rd32(m, obj + 0x14u);                           /* 0x10EAA2 */
    uint32_t d2 = rd32(m, obj + 0x18u);                           /* 0x10EAA6 */
    r->d[1] = d1;

    for (int i = 0; i < 6; i++) {
        uint32_t child = Obj_Spawn(m, obj, 0x0010ECF6u, 9u, 2u, 0u);
        r->d[0] = child;
        if ((int32_t)child < 0)
            goto done_spawn;
        r->a[1] = child;
        wr32(m, child + 0x14u, d1);
        wr16(m, child + 0x14u, (uint16_t)((int16_t)rd16(m, child + 0x14u) + parts[i].dx));
        wr32(m, child + 0x18u, d2);
        wr16(m, child + 0x18u, (uint16_t)((int16_t)rd16(m, child + 0x18u) + parts[i].dy));
        wr16(m, child + 0x60u, 0x000Fu);
        wr16(m, child + 0x58u, parts[i].f58);
        wr16(m, child + 0x10u, parts[i].f10);
    }

    {
        uint32_t child = Obj_Spawn(m, obj, 0x0010F90Au, 5u, 2u, 0u); /* 0x10EC68 */
        r->d[0] = child;
        if ((int32_t)child >= 0) {
            r->a[1] = child;
            wr32(m, child + 0x14u, d1);                           /* 0x10EC7A */
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 1u)); /* 0x10EC7E */
            wr32(m, child + 0x18u, d2);                           /* 0x10EC82 */
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0016u)); /* 0x10EC86 */
        }
    }

done_spawn:
    Sound_PlayHooked(m, 0x001Au);                                 /* 0x10EC8C..0x10EC9C */
    draw_sprite_regs(m, r);                                       /* 0x10ECA0 */
    Obj_StartDying(m, obj);                                       /* 0x10ECA6 */
}

int Fn_10E9E2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {                     /* 0x10E9E2..0x10E9E6 */
        death_burst(m, r, obj);
        return 0;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {                /* 0x10E9EA..0x10E9F0 */
        Obj_StartDying(m, obj);
        return 0;
    }

    uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u);            /* 0x10E9F6 */
    wr8(m, obj + 0x23u, cnt);
    if (cnt == 0u) {                                              /* 0x10E9FA */
        uint8_t tog = (uint8_t)(~rd8(m, obj + 0x22u));            /* 0x10E9FE */
        wr8(m, obj + 0x22u, tog);
        uint32_t slot;
        if (tog == 0u) {                                          /* 0x10EA02 */
            wr8(m, obj + 0x23u, 0x14u);                           /* 0x10EA06 */
            slot = Pal_QueueRamp(m, 0x0010ECEEu, 0x0010ECF2u, 0x5704u, 0x0Fu, 1u); /* 0x10EA2A */
        } else {
            wr8(m, obj + 0x23u, 0x1Eu);                           /* 0x10EA38 */
            slot = Pal_QueueRamp(m, 0x0010ECF2u, 0x0010ECEEu, 0x5704u, 0x19u, 1u); /* 0x10EA5C */
        }
        r->d[0] = (slot == 0u) ? 0xFFFFFFFFu : slot;
    }

    uint32_t dx = rd32(m, F3_A5 + 0x4224u);                       /* 0x10EA66 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - dx);              /* 0x10EA6A */
    uint32_t dy = rd32(m, F3_A5 + 0x4228u);                       /* 0x10EA6E */
    r->d[0] = dy;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - dy);              /* 0x10EA72 */

    if ((int16_t)rd16(m, obj + 0x14u) < -0x40) {                  /* 0x10EA76..0x10EA7C */
        Obj_StartDying(m, obj);
        return 0;
    }

    Obj_0D0DC2(m, obj);                                           /* 0x10EA82 */
    r->a[1] = rd32(m, F3_A5 + 0x0906u);
    Obj_BlinkColour(m, obj);                                      /* 0x10EA88 */
    draw_sprite_regs(m, r);                                       /* 0x10EA8E */
    return 0;
}
