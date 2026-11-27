#include "fn_auto.h"
/* ROM 0x1203FE: verified on a capture from the Y04 re-survey states: Spawn an effect, move and draw the part, then re-arm or free it. */
#include "fn_sprite_regs.h"
#include "../player/player_move.h"
#include "../objects/obj_shared_ext.h"

/* 0x13E4D0 packs the first two sprite dimensions into D0's top bytes. */
static void pack_dimensions(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[1]);
    push_long(m, r, r->d[1]);
    r->d[0] = Obj_13E4D0(m, r->a[0], r->d[0]);
    r->a[7] += 8u;
}

int Fn_1203FE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (!(rd8(m, r->a[5] + 0x879u) & 1u)) { /* 0x120404 -> 0x12042A */
        r->d[0] = 2u;
        push_long(m, r, 0x120410u);
        pack_dimensions(m, r);
        r->a[7] += 4u;
        call_spawn(m, r, 0x120426u, 0x13E4EEu, 5u, 2u, r->d[0]);
    }

    uint16_t speed = rd16(m, obj + 0x42u) + 1u;
    wr16(m, obj + 0x42u, speed);
    r->d[0] = set_word(r->d[0], 0x40u);
    push_long(m, r, 0x120438u);
    Obj_MoveByAngle(m, obj, 0x40u); /* 0x0CBE12 */
    r->a[3] = 0x027E40u;
    r->d[1] = (uint32_t)((int32_t)(int16_t)speed * (int16_t)rd16(m, 0x027F40u));
    r->d[2] = (uint32_t)((int32_t)(int16_t)speed * (int16_t)rd16(m, 0x027EC0u));
    r->a[7] += 4u;

    r->a[1] = 0x12043Au + 0x2Cu;
    uint16_t dx = rd16(m, r->a[1] + 2 * (int32_t)(int16_t)rd16(m, obj + 0x22u));
    r->d[0] = set_word(r->d[0], dx);
    wr16(m, obj + 0x14u, rd16(m, obj + 0x14u) + dx);
    call_draw_sprite(m, r, 0x120450u);
    if ((int16_t)rd16(m, obj + 0x18u) < 0xE8) /* 0x120456 -> 0x1203F6 */
        wr32(m, obj, 0x1203FEu);
    else
        call_free(m, r, 0x120460u);
    r->a[7] += 4u;
    return 0;
}
