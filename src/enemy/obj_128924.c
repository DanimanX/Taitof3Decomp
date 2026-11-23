#include "obj_128924.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"

/* ROM 0x0CBE12: pos += speed * {cos,sin}[angle], where the speed is the word
 * at (0x42,A0) and the 0x27E40 table is indexed by the low byte of the angle.
 * (Obj_MoveByAngle in player_move.h reads its speed from A0+0x20, i.e. the
 * player struct layout, so the object form is spelled out here.) */
static void sub_0CBE12(const f3mem_t *m, uint32_t obj, uint16_t angle)
{
    int16_t spd = (int16_t)rd16(m, obj + 0x42u);
    uint32_t idx = (uint32_t)(uint8_t)angle * 2u;
    int16_t cos_v = (int16_t)rd16(m, 0x027E40u + 0x80u + idx);
    int16_t sin_v = (int16_t)rd16(m, 0x027E40u + idx);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)(int32_t)((int32_t)spd * (int32_t)cos_v));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)(int32_t)((int32_t)spd * (int32_t)sin_v));
}

/* ROM 0x128924: bullet spawner. Clears the per-frame fields, aims with a random
 * speed and angle, records its start position and frame list, then hands over
 * to 0x1289A6 (the ballistic mover). */
int Obj_128924_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);                             /* 0x128924 jsr 0xCBD86 */
    wr32(m, obj + 0x14u, 0u);                            /* 0x12892A clr.l */
    wr32(m, obj + 0x18u, 0u);                            /* 0x12892E clr.l */

    /* 0x128932: jsr 0x1D8A; andi.w #0x7,D0w; addi.w #0x18,D0w; move.w D0w,(0x42,A0) */
    uint16_t d0 = (uint16_t)((Rng_NextMem(m) & 0x0007u) + 0x0018u);
    wr16(m, obj + 0x42u, d0);

    /* 0x128944: jsr 0x1D8A; andi.w #0x3f,D0w; addi.b #-0x60,D0b; jsr 0xCBE12.
     * The byte add leaves D0w = 0xFFA0..0xFFDF: the angle byte is A0..DF with
     * the high byte all ones, and 0xCBE12 uses the low byte only. */
    d0 = (uint16_t)(Rng_NextMem(m) & 0x003Fu);
    d0 = (uint16_t)(d0 + 0xFFA0u);
    sub_0CBE12(m, obj, d0);                              /* ROM 0x0CBE12 */

    wr32(m, obj + 0x44u, rd32(m, obj + 0x14u));          /* 0x128958 */
    wr32(m, obj + 0x48u, rd32(m, obj + 0x18u));          /* 0x12895E */

    /* 0x128964: jsr 0x1D8A; andi.w #0x3f,D0w; addi.w #0x80,D0w; move.w D0w,(0x14,A0) */
    d0 = (uint16_t)((Rng_NextMem(m) & 0x003Fu) + 0x0080u);
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, 0x00E6u);                       /* 0x128976 */

    /* 0x12897C: jsr 0x1D8A; andi.w #0x3,D0w;
     * move.l (0x1289da,PC,D0w*4),(0x10,A0) — the words at 0x1289DA. */
    d0 = (uint16_t)(Rng_NextMem(m) & 0x0003u);
    wr32(m, obj + 0x10u, rd32(m, 0x001289DAu + 4u * (uint32_t)d0));

    /* 0x128990: jsr 0x1D8A; andi.w #0x1,D0w; move.b D0b,(0x1c,A0) */
    d0 = (uint16_t)(Rng_NextMem(m) & 0x0001u);
    wr8(m, obj + 0x1Cu, (uint8_t)d0);

    /* 0x12899E */
    wr32(m, obj + 0x00u, 0x001289A6u);
    return 0;
}
