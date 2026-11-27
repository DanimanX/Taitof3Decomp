#include "fn_auto.h"
/* ROM 0x120174: verified on a capture from the Y04 re-survey states: Spawn a trail particle, move and draw the object, and free it outside the playfield. */
#include "batch_obj_regs.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"

/* ROM 0x12017E through 0x12019C. */
static void spawn_trail(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    wr32(m, sp - 4u, 0x120186u);
    wr32(m, sp - 8u, r->a[1]); /* 0x13E4D0 MOVEM */
    wr32(m, sp - 12u, r->d[1]);
    r->d[0] = Obj_13E4D0(m, r->a[0], 3u);
    wr32(m, sp - 4u, r->d[0]);
    wr16(m, sp - 6u, 2u);
    wr16(m, sp - 8u, 5u);
    wr32(m, sp - 12u, 0x13E4EEu);
    wr32(m, sp - 16u, 0x12019Cu);
    for (unsigned i = 3; i > 0; --i)
        wr32(m, sp - 20u - 4u * (3u - i), r->a[i - 1u]);
    for (unsigned i = 4; i > 0; --i)
        wr32(m, sp - 32u - 4u * (4u - i), r->d[i]);
    r->d[0] = Obj_Spawn(m, r->a[0], 0x13E4EEu, 5u, 2u, r->d[0]);
}

int Fn_120174(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if ((rd8(m, r->a[5] + 0x879u) & 1u) == 0)
        spawn_trail(m, r);

    uint16_t angle = rd16(m, obj + 0x20u);
    r->d[0] = batch_set_word(r->d[0], angle);
    wr32(m, r->a[7] - 4u, 0x1201AAu);
    r->a[3] = 0x027E40u;
    uint32_t index = r->a[3] + (int32_t)(int16_t)angle * 2;
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    r->d[1] = (uint32_t)((int32_t)speed * (int16_t)rd16(m, index + 0x80u));
    r->d[2] = (uint32_t)((int32_t)speed * (int16_t)rd16(m, index));
    if ((int16_t)angle >= 0) {
        Obj_MoveByAngle(m, obj, angle);
    } else {
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + r->d[1]);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + r->d[2]);
    }
    batch_draw_sprite(m, r, 0x1201B0u);

    /* CMP2.W uses signed bounds. Its carry is set outside the range. */
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    r->d[0] = batch_set_word(r->d[0], (uint16_t)x);
    if (x >= (int16_t)rd16(m, 0x0CC112u) && x <= (int16_t)rd16(m, 0x0CC114u)) {
        int16_t y = (int16_t)rd16(m, obj + 0x18u);
        r->d[0] = batch_set_word(r->d[0], (uint16_t)y);
        if (y >= (int16_t)rd16(m, 0x0CC116u) && y <= (int16_t)rd16(m, 0x0CC118u)) {
            wr32(m, obj, 0x120174u); /* 0x1201D0 -> 0x12016C */
            return 0;
        }
    }
    batch_free_object(m, r, 0x1201DAu);
    return 0;
}
