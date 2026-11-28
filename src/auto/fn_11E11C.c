#include "fn_auto.h"
#include "fn_call_frames.h"
#include "batch_obj_regs.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"

/* ROM 0x11E11C: homing enemy tick for the 0x13E4EE family (A0 = object).
 *
 * Dead when +0x24 (32-bit HP) is <= 0, when A5-0x144B bit 4 (pause) is set or
 * when +0x35 == 2: sound 0x32, spawn two 0x13E4EE (group 5) with the packed
 * argument from 0x13E4D0 and hand the tick over to the 0x11E252 free thunk.
 * With A5+0x1D58 bit 1 set and +0x14 in [-0xA..0x11C] it jumps straight into
 * the shared explosion tick 0x0CBB2C instead.
 *
 * Otherwise: with +0x22 == 0 a +0x06 countdown >= 0x32 is spent instead of the
 * life timer (and copied to +0x43); the first tick latches +0x22 bit 0, sets
 * +0x06 = 1, ages the +0x42 step through 0x0CBEB0 (difficulty-scaled 0xF) and
 * aims at the target A5+0x1D28 with 0x0CC3C6 into +0x20. The object is then
 * queued for drawing (0x0CBDA4), moved along +0x20 (0x0CBE12), coloured, its
 * 0x11E260 frame list stepped, the sprite drawn and -- while +0x14 > -0x20 and
 * +0x18 within [-0x20..0xE8] -- the tick re-armed to 0x11E210, else replaced
 * by Obj_Free (0x11E252). */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CBEB0: D0w scaled by the difficulty word A5+0x87E. */
static uint16_t sub_0CBEB0(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)((uint16_t)(rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u);
    int32_t p = (int32_t)(int16_t)d1w * (int32_t)(int16_t)d0w;
    uint32_t v = (uint32_t)p << 4;
    return (uint16_t)(v >> 16);
}

/* ROM 0x0CBD52 with the register effects of the listing (D0 = the +0x3A
 * frame offset, D1 = the reloaded +0x3E timer). */
static void anim_0CBD52(const f3mem_t *m, f3regs_t *r, uint32_t obj, uint32_t tbl)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);              /* 0xCBD52 */
        r->d[0] = setw(r->d[0], off);
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur < 0) {                                    /* 0xCBD5A */
            wr16(m, obj + 0x3Au, 0u);
            continue;
        }
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u); /* 0xCBD64 */
        wr16(m, obj + 0x3Eu, cnt);
        r->d[1] = setw(r->d[1], cnt);
        if ((int16_t)cnt <= dur) {                        /* 0xCBD70 */
            wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);                         /* 0xCBD74 */
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));
    }
}

/* ROM 0x0CBE12 (Obj_MoveByAngle) with D1/D2/A3 tracked. */
static void move_by_angle_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    r->a[3] = 0x00027E40u;                                          /* 0x0CBE12 */
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);                  /* 0x0CBE18 */
    uint32_t index = (uint32_t)((int32_t)(int16_t)r->d[0] * 2);     /* d0.w*2 */
    r->d[1] = (uint32_t)(speed * (int16_t)rd16(m, r->a[3] + 0x80u + index)); /* 0x0CBE1E */
    r->d[2] = (uint32_t)(speed * (int16_t)rd16(m, r->a[3] + index));         /* 0x0CBE28 */
    Obj_MoveByAngle(m, obj, (uint16_t)r->d[0]);
}

/* ROM 0x11E252: free the object (the tick the death path installs). */
static void sub_11E252(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[0]);
    push_long(m, r, 0x0011E25Au);
    Obj_Free(m, r->a[0]);
    r->a[7] += 4u;
}

int Fn_11E11C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)                       /* 0x11E11C tst.l */
        goto dead;
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)                  /* 0x11E124 btst #4 */
        goto dead;
    if (rd8(m, obj + 0x35u) == 0x02u)                             /* 0x11E12E */
        goto dead;
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {                /* 0x11E138 btst #1 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {                        /* 0x11E140 / 0x11E148 */
            Obj_ExplodeTick(m, obj);                              /* 0x11E150 jmp 0x0CBB2C */
            return 0;
        }
    }

    if (rd8(m, obj + 0x22u) == 0u) {                              /* 0x11E156 */
        int16_t t = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 0x0032u);
        wr16(m, obj + 0x06u, (uint16_t)t);                        /* 0x11E15E */
        if (t >= 0) {
            wr8(m, obj + 0x43u, (uint8_t)rd16(m, obj + 0x06u));   /* 0x11E168 */
            goto draw;
        }
    }

    {
        uint8_t b = rd8(m, obj + 0x22u);                          /* 0x11E172 bset.b #0 */
        wr8(m, obj + 0x22u, (uint8_t)(b | 0x01u));
        if ((b & 0x01u) == 0u) {
            wr16(m, obj + 0x06u, 0x0001u);                        /* 0x11E17C */
            wr16(m, obj + 0x42u, 0u);                             /* 0x11E182 */
            uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);               /* 0x11E186 */
            r->d[0] = setw(r->d[0], Obj_0CC3C6(m, obj, a1));      /* 0x11E18A */
            wr16(m, obj + 0x20u, (uint16_t)r->d[0]);              /* 0x11E190 */
        }
    }

    {
        int16_t t = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x11E194 */
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t <= 0) {
            r->d[0] = setw(r->d[0], 0x000Fu);                     /* 0x11E19C */
            wr16(m, obj + 0x42u, sub_0CBEB0(m, (uint16_t)r->d[0])); /* 0x11E1A0 */
        }
    }

draw:
    {   /* 0x11E1AA: queue for drawing (0x0CBDA4 + fall-through 0x0CBDAE) */
        uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
        a1 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x0906u, a1 + 4u);
        r->a[1] = a1 + 4u;
    }
    r->d[0] = setw(r->d[0], rd16(m, obj + 0x20u));                /* 0x11E1B0 */
    move_by_angle_regs(m, r, obj);                                /* 0x11E1B4 */

    if (rd8(m, obj + 0x22u) != 0u && rd8(m, obj + 0x1Du) == 0u)   /* 0x11E1BA */
        wr8(m, obj + 0x1Du, 0x08u);                               /* 0x11E1CA */
    else
        wr8(m, obj + 0x1Du, 0u);                                  /* 0x11E1D4 sf */

    anim_0CBD52(m, r, obj, 0x0011E260u);                          /* 0x11E1DE */
    batch_draw_sprite(m, r, 0x0011E1EAu);                         /* 0x11E1E4 */

    r->d[0] = setw(r->d[0], rd16(m, obj + 0x14u));                /* 0x11E1EA */
    if ((int16_t)(uint16_t)r->d[0] <= (int16_t)rd16(m, 0x000CC112u)) /* 0x11E1EE ble */
        goto free_tick;
    r->d[0] = setw(r->d[0], rd16(m, obj + 0x18u));                /* 0x11E1F8 */
    {
        uint16_t lo = rd16(m, 0x000CC116u), hi = rd16(m, 0x000CC118u);
        if ((uint16_t)((uint16_t)r->d[0] - lo) > (uint16_t)(hi - lo)) /* 0x11E1FC cmp2 */
            goto free_tick;
    }
    wr32(m, obj + 0x00u, 0x0011E210u);                            /* 0x11E208 */
    return 0;                                                     /* 0x11E20E rts */

free_tick:
    sub_11E252(m, r);                                             /* 0x11E1F4 / 0x11E204 */
    return 0;

dead:
    Sound_PlayHooked(m, 0x0032u);                                 /* 0x11E214 */
    r->d[0] = 0u;                                                 /* 0x11E228 moveq #0 */
    r->d[0] = Obj_13E4D0(m, obj, r->d[0]);                        /* 0x11E22A */
    r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 0x0005u, 2u, r->d[0]); /* 0x11E230 */
    wr32(m, obj + 0x00u, 0x0011E252u);                            /* 0x11E24A */
    return 0;                                                     /* 0x11E250 rts */
}
