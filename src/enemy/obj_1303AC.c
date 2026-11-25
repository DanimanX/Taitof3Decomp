#include "obj_1303AC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x13F238 + 0x3B66: forced sound command via Sound_PreCmdHook. */
static void play_sound_forced(const f3mem_t *m, int16_t cmd)
{
    cmd = Sound_PreCmdHook(m, cmd);
    if (m->dpram) {
        Sound_PlayCmdForced(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmdForced(&tmp_m, cmd);
    }
}

/* ROM 0x1305E4: position = parent + signed byte offsets packed in +0x0C (x:8, y:8) */
static void sub_1305E4(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, obj + 0x08u);
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, p + 0x14u) + (uint16_t)(int16_t)(int8_t)(d0 >> 24)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, p + 0x18u) + (uint16_t)(int16_t)(int8_t)(d0 >> 16)));
}

/* ROM 0x1306BA: draw the shadow/base sprite below the parent, preserving x/y/rot */
static void sub_1306BA(const f3mem_t *m, uint32_t obj)
{
    uint16_t x = rd16(m, obj + 0x14u), y = rd16(m, obj + 0x18u), r = rd16(m, obj + 0x1Eu);
    uint32_t p = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, rd16(m, p + 0x14u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, p + 0x18u) + 0x38u));
    wr16(m, obj + 0x1Eu, 0u);
    wr8(m, obj + 0x1Cu, 0u);
    wr8(m, obj + 0x1Du, rd8(m, p + 0x1Du));
    wr32(m, obj + 0x10u, 0x00037FCCu);
    Obj_DrawSprite(m, obj);
    wr16(m, obj + 0x1Eu, r);
    wr16(m, obj + 0x18u, y);
    wr16(m, obj + 0x14u, x);
}

/* ROM 0x130706: random-flip debris sprites stepping left by 0x80 at y + d0w */
static void sub_130706(const f3mem_t *m, uint32_t obj, uint16_t d2)
{
    uint16_t x = rd16(m, obj + 0x14u), y = rd16(m, obj + 0x18u), r = rd16(m, obj + 0x1Eu);
    if (!(rd8(m, F3_A5 + 0x879u) & 0x01u)) {
        wr8(m, obj + 0x1Du, 0u);
        sub_1305E4(m, obj);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d2));
        uint32_t w = Rng_NextMem(m) & 0xFFFFu;                  /* 0x001D8A; andi.l #0xffff */
        uint32_t q = w / 3u, rem = w % 3u;                      /* divu.w #3 */
        wr8(m, obj + 0x1Cu, (uint8_t)(q & 0x02u));
        wr32(m, obj + 0x10u, rd32(m, 0x00130788u + rem * 4u));  /* table of 3 sprite defs */
        for (;;) {                                              /* 0x13074E */
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x80u));
            if ((int16_t)rd16(m, obj + 0x14u) <= -0x80)
                break;
            Obj_DrawSprite(m, obj);
        }
    }
    wr16(m, obj + 0x1Eu, r);
    wr16(m, obj + 0x18u, y);
    wr16(m, obj + 0x14u, x);
}

/* ROM 0x1303AC */
int Obj_1303AC_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x10u) {                      /* 0x130592 */
        play_sound_forced(m, 0x22);
        Obj_Free(m, obj);
        return 0;
    }
    wr16(m, obj + 0x14u, 0x00A0u);
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0xA && x <= 0x11C) {                          /* 0x1305B4 */
            wr16(m, obj + 0x06u, 0x10u);
            wr32(m, obj, 0x001305C2u);
            return 0;
        }
    }
    if ((int16_t)rd16(m, obj + 0x06u) < 0x14)
        sub_130706(m, obj, (uint16_t)-0x10);
    /* 0x1303EC */
    uint32_t p = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, rd16(m, p + 0x14u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, p + 0x18u) + 0x40u));
    Obj_Animate(m, obj, 0x00130774u);                           /* 0x0CBD52 */
    Obj_DrawSprite(m, obj);                                     /* 0x001282 */
    sub_1305E4(m, obj);
    sub_1306BA(m, obj);
    Obj_Spawn(m, obj, 0x00130794u, 0u, 1u, 0u);                 /* 0x01057C */
    uint16_t old = rd16(m, obj + 0x06u);
    wr16(m, obj + 0x06u, (uint16_t)(old - 1u));
    if ((int32_t)(int16_t)old - 1 > 0) {                        /* subq.w; bgt */
        wr32(m, obj, 0x001303ACu);
        return 0;
    }
    play_sound_forced(m, 0x23);                                 /* 0x13F238 + 0x3B66 */
    Sound_PlayHooked(m, 0x22u);                                 /* 0x13F238 + 0x39D8 */
    wr16(m, F3_A5 + 0x32A6u, (uint16_t)-0xC);
    wr8(m, obj + 0x1Eu, 0xA0u);
    wr16(m, obj + 0x06u, 0x04u);
    wr32(m, obj, 0x00130482u);
    return 0;
}
