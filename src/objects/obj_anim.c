#include "obj_anim.h"
#include "obj_shared_ext.h"
#include "../enemy/obj_179550.h"
#include "../enemy/obj_179600.h"
#include "../enemy/obj_179614.h"
#include "../enemy/obj_1795F6.h"
#include "../enemy/obj_17960A.h"
#include "../enemy/obj_17955E.h"
#include "../enemy/obj_179588.h"
#include "../enemy/obj_179572.h"
#include "../enemy/obj_1795A4.h"

/* The 0x179A5C engine. Bodies are lifted from the ROM listings (see
 * work/disasm/bundle_179664.s and the copies in src/enemy/obj_17E05A.c); the
 * ticks for indices 0 and 9 are translated, the rest return -1 before any side
 * effect. */

/* ROM 0x179550 (index 0 init): plain frame record at +0x3A -- {duration.w,
 * sprite.l} into the countdown (+0x3E) and the sprite pointer (+0x10). Lives in
 * src/enemy/obj_179550.c; call it through its header rather than duplicating. */

/* ROM 0x179664 */
void Obj_AnimTick0(const f3mem_t *m, uint32_t obj)
{
    int16_t timer = (int16_t)rd16(m, obj + 0x3Eu);

    if (timer > 0) {
        /* 0x179668 bgt -> 0x1796AC: just count down */
    } else if (timer < 0) {
        return;                                     /* 0x17966C blt / 0x179670 rts */
    } else {
        /* 0x179672: zero duration -- take the next record */
        uint32_t a1 = rd32(m, obj + 0x3Au);
        a1 += 6u;                                   /* 0x179676 lea (0x6,A1),A1 */
        int16_t d = (int16_t)rd16(m, a1);
        if (d > 0) {
            /* 0x17967C bgt -> 0x1796A4: rewind to this record and reload */
            wr32(m, obj + 0x3Au, a1);
            Obj_179550_Tick(m, obj);                /* 0x1796A8 bsr 0x179550 */
        } else if (d < 0) {
            /* 0x179680 blt -> 0x1796AC */
        } else {
            /* 0x179684: jump the pointer record, fetch the next index */
            a1 = rd32(m, a1 + 2u);                  /* 0x179684 movea.l (0x2,A1),A1 */
            uint16_t d0w = rd16(m, a1);             /* 0x17968A move.w (A1)+,D0w */
            a1 += 2u;
            wr16(m, obj + 0x40u, d0w);              /* 0x17968C */
            wr32(m, obj + 0x3Au, a1);               /* 0x179690 */
            /* 0x17969A: movea.l (0x179A5C,D0w*0x8),A1; jsr (A1) -- the INIT
             * slot of the new index, through the same dispatch as the callers */
            if (Obj_AnimInit(m, obj, d0w) < 0)
                wr16(m, obj + 0x40u, d0w);
            /* 0x1796A0 bra.w 0x1796AC */
        }
    }
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));  /* 0x1796AC */
}

/* ROM 0x179664 again, for an init that pinned the countdown while its body
 * already decremented it: the entry's post value is one below the record's.
 * That only happens on the hand-over pass (0x17D9FE -> 0x17A3F4 -> 0x17A426),
 * where 0x179664 is called in place of the next index's init. */
static void walk_after_init(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, obj + 0x3Eu) == 0) {
        uint32_t a1 = rd32(m, obj + 0x3Au) + 6u;
        int16_t d = (int16_t)rd16(m, a1);
        if (d > 0) {
            wr32(m, obj + 0x3Au, a1);
            Obj_179550_Tick(m, obj);
        }
    }
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
}

/* ROM 0x179898 (index 9 tick, and the tail of the index 9 init 0x179600):
 * angle byte (+0x20) + 8 >> 4 picks one of 16 records of 6 bytes at +0x3A,
 * {spr.l, palette.w}. The init pins the countdown to 1 and so does this body,
 * which is why a 0x179898 tick shows +0x3E decremented by the index-0 walk
 * under it rather than by itself. */
static void anim9_tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4);
    uint32_t a1 = rd32(m, obj + 0x3Au) + ((uint32_t)d0 * 6u & 0xFFFFu);
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
}

/* ROM 0x179880 / 0x1798BA / 0x1798F8 / 0x179946 / 0x1799B2: directional
 * sprite (+hitbox/+offset) ticks indexed by +0x20 angle (16 or 32 dirs). */
static void anim_dir_tick(const f3mem_t *m, uint32_t obj, int dirs32, uint32_t stride, int copy_boxes, int add_xy)
{
    uint8_t ang = rd8(m, obj + 0x20u);
    uint16_t d0 = dirs32 ? (uint16_t)((uint8_t)(ang + 4u) >> 3)
                         : (uint16_t)((uint8_t)(ang + 8u) >> 4);
    uint32_t a1 = rd32(m, obj + 0x3Au) + ((uint32_t)d0 * stride & 0xFFFFu);
    wr32(m, obj + 0x10u, rd32(m, a1));
    if (copy_boxes) {
        wr32(m, obj + 0x2Cu, rd32(m, a1 + 4u));
        wr32(m, obj + 0x30u, rd32(m, a1 + 8u));
    }
    if (add_xy) {
        int16_t dx = (int16_t)(int8_t)rd8(m, a1 + 12u);
        int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 13u);
        wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + dx));
        wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + dy));
    }
}

/* ROM 0x1796B2 (index 8 tick): the index-0 walk with an 8-byte header; a
 * positive duration reloads through 0x17955E. */
static void anim8_tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, obj + 0x3Eu) > 0)
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
    else if ((int16_t)rd16(m, obj + 0x3Eu) == 0) {
        uint32_t a1 = rd32(m, obj + 0x3Au) + 8u;
        int16_t d = (int16_t)rd16(m, a1);
        if (d > 0) {
            wr32(m, obj + 0x3Au, a1);
            Obj_17955E_Tick(m, obj);
        } else if (d == 0) {
            a1 = rd32(m, a1 + 2u);
            uint16_t d0w = rd16(m, a1);
            a1 += 2u;
            wr16(m, obj + 0x40u, d0w);
            wr32(m, obj + 0x3Au, a1);
            if (Obj_AnimInit(m, obj, d0w) < 0)
                wr16(m, obj + 0x40u, d0w);
        }
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
    }
}

/* ROM 0x17979C (index 5 tick): 18-byte records with hitbox + dx/dy. */
static void anim5_tick(const f3mem_t *m, uint32_t obj)
{
    int16_t timer = (int16_t)rd16(m, obj + 0x3Eu);
    if (timer == 0) {
        uint32_t a1 = rd32(m, obj + 0x3Au) + 0x10u;
        int16_t d = (int16_t)rd16(m, a1);
        if (d > 0) {
            wr32(m, obj + 0x3Au, a1);
            Obj_1795A4_Tick(m, obj);
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return;
        } else if (d == 0) {
            a1 = rd32(m, a1 + 2u);
            uint16_t d0w = rd16(m, a1);
            a1 += 2u;
            wr16(m, obj + 0x40u, d0w);
            wr32(m, obj + 0x3Au, a1);
            if (Obj_AnimInit(m, obj, d0w) < 0)
                wr16(m, obj + 0x40u, d0w);
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return;
        }
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
    } else if (timer > 0) {
        wr16(m, obj + 0x3Eu, (uint16_t)(timer - 1));
    }
    uint32_t a1 = rd32(m, obj + 0x3Au);
    int16_t dx = (int16_t)(int8_t)rd8(m, a1 + 0x0Eu);
    int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 0x0Fu);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + dx));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + dy));
}

/* ROM 0x17997A (index 14 tick): 16-dir 16-byte record {spr.l, pal.w, 2c.l, 30.l, dx.b, dy.b}. */
static void anim14_tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4);
    uint32_t a1 = rd32(m, obj + 0x3Au) + ((uint32_t)d0 << 4);
    wr32(m, obj + 0x10u, rd32(m, a1));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 4u));
    wr32(m, obj + 0x2Cu, rd32(m, a1 + 6u));
    wr32(m, obj + 0x30u, rd32(m, a1 + 10u));
    int16_t dx = (int16_t)(int8_t)rd8(m, a1 + 14u);
    int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 15u);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + dx));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + dy));
}

int Obj_AnimTick(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    switch (idx) {
    case 0u: Obj_AnimTick0(m, obj);                  return 0;
    case 1u: anim_dir_tick(m, obj, 0,  4u, 0, 0);    return 0; /* ROM 0x179880 */
    case 2u: anim_dir_tick(m, obj, 1,  4u, 0, 0);    return 0; /* ROM 0x1798BA */
    case 4u: anim_dir_tick(m, obj, 0, 12u, 1, 0);    return 0; /* ROM 0x1798F8 */
    case 5u: anim5_tick(m, obj);                     return 0; /* ROM 0x17979C */
    case 6u: anim_dir_tick(m, obj, 0, 14u, 1, 1);    return 0; /* ROM 0x179946 */
    case 7u: anim_dir_tick(m, obj, 1, 14u, 1, 1);    return 0; /* ROM 0x1799B2 */
    case 8u: anim8_tick(m, obj);                     return 0;
    case 9u: anim9_tick(m, obj);                     return 0;
    /* ROM 0x1798D4 (index 10 tick): the same 6-byte directional record the
     * index-10 init 0x179614 picks, re-picked from the +0x20 angle byte. */
    case 10u: anim_dir_tick(m, obj, 1, 6u, 0, 0);    return 0;
    case 14u: anim14_tick(m, obj);                   return 0; /* ROM 0x17997A */
    default: return -1;
    }
}

int Obj_AnimInit(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    switch (idx) {
    case 0u:  return Obj_179550_Tick(m, obj);
    case 1u:  return Obj_1795F6_Tick(m, obj);
    case 2u:  return Obj_17960A_Tick(m, obj);
    case 5u:  return Obj_1795A4_Tick(m, obj);
    case 8u:  return Obj_17955E_Tick(m, obj);
    case 9u:  return Obj_179600_Tick(m, obj);
    case 10u: return Obj_179614_Tick(m, obj);
    case 3u:  return Obj_179572_Tick(m, obj);
    case 11u:
        /* 0x179588 loads the timer and returns; the 0x179D9E body's tail then
         * steps the list again (see walk_after_init) */
        Obj_179588_Tick(m, obj);
        walk_after_init(m, obj);
        return 0;
    case 14u:
        wr16(m, obj + 0x3Eu, 1u);
        anim14_tick(m, obj);
        return 0;
    default:  return -1;
    }
}

/* ROM 0x179B06: choose between the frame lists at 0x179B3A and 0x179B8C.
 *
 *   0x179B06  lea 0x179B3A(pc),A1
 *   0x179B0C  moveq #1,D0 ; jsr 0x1794A4 (Obj_RandRange) ; tst.w D0
 *   0x179B16  beq -> 0x179B20 (keep 0x179B3A) ; else A1 = 0x179B8C
 *   0x179B22  move.w (A1)+,D0w ; (0x40,A0) = D0w ; (0x3A,A0) = A1
 *   0x179B32  movea.l (0x179A5C,D0w*8),A1 ; jsr (A1)       -- the INIT half
 */
int Obj_AnimList179B06(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x00179B3Au;
    if (Obj_RandRange(m, 1u) != 0u)
        a1 = 0x00179B8Cu;

    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1);
    return Obj_AnimInit(m, obj, d0);
}
