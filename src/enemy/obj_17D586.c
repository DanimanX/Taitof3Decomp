#include "obj_17D586.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x17D586: the flight half of the 0x17D4F4 boss minion. Each frame it
 * zeroes the HP word while the "instakill" flag (A5-0x144C bit 4) is set,
 * runs the 0x44 behaviour script (0x1789FC: the +0x5E/+0x58 depth step and the
 * script commands) and refreshes the perspective zoom (0x17940E), leaves for
 * good (0x17C78E) while still outside the 0x184A00 box, centres the sprite on
 * the position minus its frame size, draws it, and -- if the object is still
 * visible -- refreshes the +0x4C target, re-aims at the player (0x17AF66)
 * before spawning a 0x17DDD0 child with the aim byte and a 0x17D640 hand-off
 * timer of 1. */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* cmp2.w (base)/(base+2) against D0w: bcs (out of range) unless
 * lo < D0w < hi. */
static int in_box(const f3mem_t *m, uint32_t base, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, base), hi = (int16_t)rd16(m, base + 2u);
    int16_t x = (int16_t)v;
    return x > lo && x < hi;
}

/* ROM 0x1789FC: per-frame depth step at (0x5E,A0) from the behaviour's +0xA
 * word, integrated into the 4.24 speed long at +0x58 (a negative sum wraps to
 * its low nibble), then the +0x44 script walk (0x178A2C runs the same command
 * loop as 0x178982, with 12-byte records). */
static int sub_1789FC(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    int16_t dz = (int16_t)rd16(m, a1 + 0x0Au);
    int16_t z16 = (int16_t)(rd16(m, obj + 0x5Eu) + dz);
    wr16(m, obj + 0x5Eu, (uint16_t)z16);

    /* bfexts (0x5E,A0){0:16},D0 ; muls.l #0x100,D0 ; add.l D0,(0x58,A0) */
    int32_t d0 = ((int32_t)z16 * 0x100);
    int32_t acc = (int32_t)rd32(m, obj + 0x58u) + d0;
    if (acc < 0)
        acc &= 0x0F;                                 /* andi.l #0xF */
    wr32(m, obj + 0x58u, (uint32_t)acc);

    /* 0x178A2C: the +0x44 command walk over 12-byte records */
    return Obj_Script12(m, obj);
}

/* ROM 0x17940E: perspective zoom byte +0x1E/+0x1F from the +0x58 speed long. */
static void sub_17940E(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = (rd32(m, obj + 0x58u) & 0x00FFFF00u) + 0x00010000u;
    uint32_t d1 = 0x01000000u / d0;                  /* divu.l, quotient word */
    uint8_t z = (uint8_t)(-(int8_t)(uint8_t)d1);     /* neg.b D1 */
    wr8(m, obj + 0x1Fu, z);
    wr8(m, obj + 0x1Eu, z);
}

/* ROM 0x17AF66: byte angle from the object toward the +0x4C target with the
 * (+0x18, +0x10) centre offset -- 0x006422 -- or D0 unchanged (<= 0) when the
 * target field is empty. Returns D0 as a long. */
static uint32_t sub_17AF66(const f3mem_t *m, uint32_t obj)
{
    uint32_t tgt = rd32(m, obj + 0x4Cu);
    if ((int32_t)tgt <= 0)
        return 0u;
    uint16_t d0x = (uint16_t)(rd16(m, tgt + 0x14u) + 0x0018u - rd16(m, obj + 0x50u));
    uint16_t d1y = (uint16_t)(rd16(m, tgt + 0x18u) + 0x0010u - rd16(m, obj + 0x54u));
    return (uint32_t)Obj_AtanByte(m, (int16_t)d0x, (int16_t)d1y);
}

/* ROM 0x17D586 */
int Obj_17D586_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17D586: btst.b #4,(-0x144C,A5) ; beq ; clr.l (0x24,A0) */
    if ((rd8(m, F3_A5 - 0x144Cu) & 0x10u) != 0u)
        wr32(m, obj + 0x24u, 0u);

    /* 0x17D594: jsr 0x1789FC ; 0x17D59A: jsr 0x17940E */
    if (sub_1789FC(m, obj) < 0)
        return -1;
    sub_17940E(m, obj);

    /* 0x17D5A0..0x17D5BC: cmp2.w (0x184A00)/(0x184A04); bcs.w 0x17C78E */
    if (!in_box(m, 0x00184A00u, rd16(m, obj + 0x50u)) ||
        !in_box(m, 0x00184A04u, rd16(m, obj + 0x54u))) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* 0x17D5C0: centre = the position */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x17D5CC: the 0x179A5C TICK half for the index in +0x40 */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x17D5DC: centre -= the frame size (<<3) */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1c = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0c = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0c));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1c));
    }

    /* 0x17D5F0: jsr 0x179434 ; 0x17D5F6: jsr 0x1282 */
    Obj_BossMapScreenPos(m, obj);
    Obj_DrawSprite(m, obj);

    /* 0x17D5FC: tst.b (0x1E,A0) ; bne.w 0x17D57E (park on 0x17D586) */
    if (rd8(m, obj + 0x1Eu) != 0u) {
        wr32(m, obj + 0x00u, 0x0017D586u);
        return 0;
    }

    /* 0x17D604: move.l (0x1D28,A5),(0x4C,A0) ; jsr 0x17AF66 ;
     * andi.l #0xFF,D0 -- the aim byte becomes the child's argument */
    wr32(m, obj + 0x4Cu, rd32(m, F3_A5 + 0x1D28u));
    {
        uint32_t aim = sub_17AF66(m, obj) & 0x000000FFu;

        /* 0x17D616: pea 2 ; pea 9 ; pea 0x17DDD0 ; jsr 0x1057C (Obj_Spawn) */
        Obj_Spawn(m, obj, 0x0017DDD0u, 9u, 2u, aim);
    }

    /* 0x17D630: move.w #1,(0x6,A0) ; beq 0x17D646 (never: Z is clear) ;
     * 0x17D638: move.l #0x17D640,(A0) */
    wr16(m, obj + 0x06u, 1u);
    wr32(m, obj + 0x00u, 0x0017D640u);
    return 0;
}
