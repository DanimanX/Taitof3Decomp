#include "fn_auto.h"
/* ROM 0x0DF9D8: advance the orbiting enemy, or spawn its destruction effect. */
#include "round8_projectile_regs.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../player/player_move.h"

static uint32_t swap_words(uint32_t value)
{
    return (value << 16) | (value >> 16);
}

/* ROM 0x0D0690. Recover the register outputs from the next record before
 * letting Obj_StepScript apply its writes. */
static void step_behaviour(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t obj = r->a[0];
    wr32(m, r->a[7] - 4u, ret);
    if (rd16(m, obj + 0x4Cu) == 1u) {
        uint32_t entry = rd32(m, obj + 0x48u) + 12u;
        uint16_t duration = rd16(m, entry);
        if ((int16_t)duration < 0) {
            int16_t offset = (int16_t)rd16(m, entry + 2u);
            if (offset == 0) {
                r->a[1] = entry + 2u;
                r->d[0] = set_word(r->d[0], duration);
                Obj_StepScript(m, obj);
                return;
            }
            entry = entry + 2u - (uint32_t)(int32_t)offset;
        }
        r->a[1] = entry + 8u;
        r->d[0] = set_word(r->d[0], rd16(m, entry + 2u));
    }
    Obj_StepScript(m, obj);
}

/* ROM 0x0D0678. */
static void start_behaviour(const f3mem_t *m, f3regs_t *r, uint32_t entry, uint32_t ret)
{
    wr32(m, r->a[7] - 4u, ret);
    r->d[0] = set_word(r->d[0], rd16(m, entry + 2u));
    Obj_StartBehaviour(m, r->a[0], entry);
    r->a[1] = entry + 8u;
}

/* ROM 0x0CF1E4, restricted to this object's scripts at 0x0DFBA8 and
 * 0x0DEC44. The final opcode is 12, 14 or 19. At 0x0DFBF6 a successful
 * opcode 24 loops back to 14 and zeroes the upper word of D0. */
static void start_motion(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t entry = rd32(m, obj + 0x44u);
    uint32_t sp = r->a[7];
    push_long(m, r, 0x0DFA26u);
    push_long(m, r, 0x0CF1F4u);
    if (entry == 0x0DFBF6u && rd8(m, obj + 0x1Eu) >= rd8(m, entry + 2u))
        r->d[0] = 0u;
    (void)ObjScript_CF1E4(m, obj);
    r->a[1] = rd32(m, obj + 0x44u) - 4u;
    uint8_t mode = rd8(m, obj + 0x20u);
    if (mode == 12u) {
        r->d[0] = rd32(m, obj + 0x50u);
        r->d[1] = rd32(m, obj + 0x54u);
    } else {
        r->d[0] = set_word(r->d[0], mode);
    }
    r->a[7] = sp;
}

/* ROM 0x0CF4B8. For this object's modes 12 and 14, 0x0D2D44 leaves the
 * horizontal and vertical step in D0/D1. The mode-19 entry is a bare rts. */
static void step_motion(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint8_t mode = rd8(m, obj + 0x20u);
    r->a[1] = 0x0DEAF4u;                                        /* 0x0DFA30 + 0xF0C4 */
    r->d[0] = set_word(r->d[0], mode);
    uint32_t sp = r->a[7];
    push_long(m, r, 0x0DFA38u);
    push_long(m, r, 0x0CF4C4u);
    (void)ObjScript_CF4B8(m, obj, r->a[1]);
    if (mode == 12u || mode == 14u) {
        uint32_t offset = (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x58u) * 4);
        uint16_t speed = rd16(m, obj + 0x60u);
        r->d[0] = rd32(m, 0x0D2D64u + offset) * speed;
        r->d[1] = rd32(m, 0x0D2ECCu + offset) * speed;
    }
    r->a[7] = sp;
}

/* ROM 0x0DFA60 uses a preindexed long pointer (extension 0x0521). The table
 * at 0x0DFB3A contains a null slot and handlers for states 1 through 5. */
static void step_state(const f3mem_t *m, f3regs_t *r, uint8_t state)
{
    uint32_t obj = r->a[0];
    r->d[0] = set_word(r->d[0], state);
    push_long(m, r, 0x0DFA66u);
    switch (rd32(m, 0x0DFB3Au + 4u * state)) {
    case 0x0DFB52u:                                             /* 1 and 5 */
        step_behaviour(m, r, 0x0DFB58u);
        break;
    case 0x0DFB5Au:                                             /* 2 */
        start_behaviour(m, r, 0x0DEC7Au, 0x0DFB64u);
        wr8(m, obj + 6u, 3u);
        break;
    case 0x0DFB6Cu:                                             /* 3 */
        step_behaviour(m, r, 0x0DFB72u);
        if (rd16(m, obj + 0x4Cu) == 0u) {
            start_behaviour(m, r, 0x0DEC56u, 0x0DFB84u);
            wr8(m, obj + 6u, 1u);
        }
        break;
    case 0x0DFB8Cu:                                             /* 4 */
        wr32(m, r->a[7] - 4u, 0x0DFB96u);
        Obj_StartScript(m, obj, 0x0DEC44u);
        start_behaviour(m, r, 0x0DEC56u, 0x0DFBA0u);
        wr8(m, obj + 6u, 5u);
        break;
    }
    r->a[7] += 4u;
}

/* ROM 0x0CF898. Shared orbit routine applies the four descriptor updates
 * and the two summed projections. Compute its exposed scratch registers
 * using the updated record and the same 0x0D1EBC coefficient tables. */
static void orbit(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, r->a[7] - 4u, 0x0DFA6Eu);
    r->a[1] = rd32(m, obj + 0x0Cu);
    r->d[6] = set_word(r->d[6], 0x168u);
    (void)Obj_0CF898(m, obj, &r->d[0], &r->d[1]);

    uint32_t desc = r->a[1];
    uint16_t angle = rd16(m, desc + 8u);
    uint16_t radius = rd16(m, desc + 0x10u);
    uint32_t index = (uint32_t)((int32_t)(int16_t)angle * 4);
    r->d[4] = rd32(m, 0x0D1EDCu + index) * radius;
    r->d[5] = rd32(m, 0x0D2044u + index) * radius;
    uint16_t relative = (uint16_t)(angle - rd16(m, desc + 0x18u));
    if ((int16_t)relative < 0)
        relative = (uint16_t)(relative + 0x168u);
    index = (uint32_t)((int32_t)(int16_t)relative * 4);
    uint16_t projection = (uint16_t)((rd32(m, 0x0D2044u + index) * radius) >> 16);
    r->d[3] = 0x5Au;
    if (relative < 0xB4u) {
        projection = (uint16_t)(0u - projection);
        r->d[3] += 0xB4u;
    }
    r->d[2] = set_word(r->d[2], projection);
}

static void die(const f3mem_t *m, f3regs_t *r)
{
    wr8(m, r->a[0] + 6u, 0u);                                  /* 0x0DFB1A */
    Obj_StartDying(m, r->a[0]);
}

static void spawn_burst(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    call_spawn(m, r, 0x0DFAD8u, 0x0DC6D0u, 3u, 2u, 0u);
    if ((int32_t)r->d[0] >= 0) {
        uint32_t child = r->d[0];                                /* 0x0DFAE2 */
        r->a[1] = child;
        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, child + 0x58u, 0xB4u);
        wr16(m, child + 0x60u, rd16(m, obj + 0x60u));
        wr16(m, child + 0x10u, rd16(m, obj + 0x1Eu));
        uint8_t angle = rd8(m, obj + 0x1Eu);
        r->d[2] = set_word(r->d[2], angle);
        uint32_t scale = rd32(m, 0x0D4A5Au + 4u * angle);
        r->d[0] = swap_words(scale * 16u);
        r->d[1] = swap_words(scale * 8u);
        wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + r->d[0]));
        wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + r->d[1]));
    }
    die(m, r);
}

int Fn_0DF9D8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    int level_clear = (rd8(m, r->a[5] - 0x144Bu) & 0x20u) != 0;

    if ((rd8(m, r->a[5] + 0x1D58u) & 2u) && x >= -10 && x <= 0x11C) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        r->d[0] = 0x12Cu;                                      /* 0x0DFB24 */
        wr32(m, r->a[7] - 4u, 0x0DFB30u);
        r->a[1] = rd32(m, r->a[5] + 0x1D50u);
        Obj_RelayTimer(m, r->d[0]);
        wr8(m, obj + 6u, 0u);
        if (level_clear) {
            Obj_StartDying(m, obj);                              /* 0x0CEF14 */
        } else {
            int16_t current_x = (int16_t)rd16(m, obj + 0x14u);    /* 0x0CEF18 */
            if ((rd8(m, r->a[5] + 0x1D58u) & 2u) && current_x >= -10 && current_x <= 0x11C) {
                wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));        /* 0x0CEF30 */
                explode_projectile(m, r);
            } else {
                wr32(m, obj, 0x0CEF44u);                         /* 0x0CEF3C */
            }
        }
    } else if ((int32_t)rd32(m, obj + 0x24u) <= 0 || level_clear) {
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
            r->d[0] = 0x12Cu;                                  /* 0x0DFAB4 */
            wr32(m, r->a[7] - 4u, 0x0DFAC0u);
            uint8_t credit = rd8(m, obj + 0x35u);
            if (credit == 1u || credit == 2u) {
                r->a[1] = rd32(m, obj + 0x36u);
                if (credit == 1u)
                    r->a[1] = rd32(m, r->a[1] + 8u);
            }
            Obj_0D0E04(m, obj, r->d[0]);
        }
        spawn_burst(m, r);
    } else {
        uint8_t state = rd8(m, obj + 6u);
        uint8_t flags = rd8(m, obj + 7u);
        uint8_t mode = rd8(m, obj + 0x20u);
        if (state < 1u || state > 5u)
            return -1;
        if (flags & 2u) {
            uint32_t entry = rd32(m, obj + 0x44u);
            if (!((entry >= 0x0DFBA8u && entry <= 0x0DFC14u && (entry - 0x0DFBA8u) % 6u == 0u) ||
                  (entry >= 0x0DEC44u && entry <= 0x0DEC50u && (entry - 0x0DEC44u) % 6u == 0u)))
                return -1;
        } else if (mode != 0u && mode != 12u && mode != 14u && mode != 19u) {
            return -1;
        }

        r->d[0] = rd32(m, r->a[5] + 0x4228u);                  /* 0x0DFA0E */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - r->d[0]);
        wr8(m, obj + 7u, (uint8_t)(flags & ~2u));
        if (flags & 2u)
            start_motion(m, r);
        if (rd8(m, obj + 0x20u))
            step_motion(m, r);

        wr32(m, r->a[7] - 4u, 0x0DFA3Eu);
        Obj_ArmOnScreen(m, obj);
        wr32(m, r->a[7] - 4u, 0x0DFA44u);
        Obj_FlagOffscreen(m, obj);
        if (rd8(m, obj + 7u) & 8u) {
            die(m, r);
        } else {
            wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
            wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
            step_state(m, r, state);
            orbit(m, r);

            int negative = (int32_t)r->d[1] < 0;                /* 0x0DFA70 */
            uint32_t offset = swap_words(r->d[1]);
            if (negative)
                offset = set_word(offset, (uint16_t)(0u - offset));
            r->d[1] = offset;                                  /* 0x0D4A1C EXG */
            uint32_t scale = rd32(m, 0x0D4A5Au + 4u * rd8(m, obj + 0x1Eu));
            r->d[0] = swap_words(scale * (uint16_t)offset);
            uint16_t y = rd16(m, obj + 0x18u);
            wr16(m, obj + 0x18u, (uint16_t)(negative ? y - r->d[0] : y + r->d[0]));
            wr32(m, r->a[7] - 4u, 0x0DFAA4u);
            Obj_BlinkColour(m, obj);
            if (rd8(m, obj + 0x1Eu) == 0u) {
                wr32(m, r->a[7] - 4u, 0x0DFAB2u);
                Obj_0D0DC2(m, obj);
                r->a[1] = rd32(m, r->a[5] + 0x0906u);
            }
        }
    }
    r->a[7] += 4u;
    return 0;
}
