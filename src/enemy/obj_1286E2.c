#include "obj_1286E2.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0013F238 + 0x0039D8: queue a sound command (+0x53 in remap mode). */
static void play_sound(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp[F3_DPRAM_SIZE] = {0};
        f3mem_t tm = *m;
        tm.dpram = tmp;
        Sound_PlayCmd(&tm, cmd);
    }
}

/* ROM 0x0013E4D0: build the packed shot-direction word from the sprite def at
 * (0x10,A0). Two 16-bit entries, each shifted left 3, inserted at bit 8 and
 * bit 16 of D0; the caller pushes D0 as the spawn argument. */
static uint32_t sub_13E4D0(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d1 = rd16(m, a1); a1 += 2u;
    d1 = (uint16_t)(d1 << 3);
    d0 = (d0 & 0x00FFFFFFu) | ((uint32_t)(uint8_t)d1 << 24);
    d1 = rd16(m, a1);
    d1 = (uint16_t)(d1 << 3);
    d0 = (d0 & 0xFF00FFFFu) | ((uint32_t)(uint8_t)d1 << 16);
    return d0;
}

/* ROM 0x000CD3D6: take record D0w of the 0x0CD444 chart (the chart entry is an
 * optional sound word followed by the frame list), then switch to 0x0CD412. */
static void sub_0CD3D6(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint32_t a1 = rd32(m, 0x000CD444u + (d0 & 0xFFu) * 4u);
    uint16_t snd = rd16(m, a1);
    a1 += 2u;
    if (snd != 0u)
        play_sound(m, (int16_t)snd);
    wr32(m, obj + 0x0Cu, a1);              /* 0x0CD3F6 move.l A1,(0xC,A0) */
    wr16(m, obj + 0x3Au, 0u);              /* clr.w (0x3A,A0) */
    wr16(m, obj + 0x3Eu, 0u);              /* clr.w (0x3E,A0) */
    wr16(m, obj + 0x1Eu, 0u);              /* clr.w (0x1E,A0) */
    wr16(m, obj + 0x1Cu, 0u);              /* clr.w (0x1C,A0) */
    wr32(m, obj + 0x00u, 0x000CD412u);     /* 0x0CD40A */
}

/* ROM 0x1286E2: a burst emitter. While X is inside the screen and the "shot"
 * flag is clear it steers, draws and hazard-lists itself; once Y drops under
 * 0xE8 it fires the burst -- sound 0x15, one 0x13E4EE shot plus 31 0x1287C4
 * fragments -- and switches to the 0x1287A4 tick.
 *
 * Raw ROM (work/maincpu.bin, offset == address):
 *   082d 0004 ebb5  btst.b #4,(-0x144B,A5)
 *   66 00 00C8       bne.w 0x1287B2
 *   082d 0001 1d58  btst.b #1,(0x1D58,A5) ; 67 16 beq ; x in [-0x0A,0x11C] -> jmp 0x0CBB2C
 *   082d 0002 1d58  btst.b #2,(0x1D58,A5) ; 66 00 00A0 bne 0x1287B2
 *   3028 0020       move.w (0x20,A0),D0w
 *   4eb9 000CBE12   jsr 0x0CBE12
 *   302d 0878 0240 0001  move.w (0x878,A5),D0w ; andi.w #1,D0w
 *   b068 0022       cmp.w (0x22,A0),D0w ; 66 08 bne
 *   4eb9 00001282   jsr 0x1282
 *   4eb9 000CBDAE   jsr 0x0CBDAE        (hazard list)
 *   0c68 00E8 0018  cmpi.w #0xE8,(0x18,A0) ; 6d 98 blt 0x1286DA
 *   ...             sound 0x15; 7005 moveq #5 ; jsr 0x13E4D0
 *                   Obj_Spawn(0x13E4EE, group 1); D1=0x1E; loop Obj_Spawn(0x1287C4, 2, 5) dbf
 *                   20bc 001287A4  fn = 0x1287A4
 * 0x1286DA: 20bc 001286E2  fn = 0x1286E2
 * 0x1287B2: D0=1 ; addq.w #8,(0x14,A0) ; addq.w #8,(0x18,A0) ; jmp 0x0CD3D6
 */
int Obj_1286E2_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
        goto tail_1287b2;                              /* 0x1286E8 bne -> 0x1287B2 */

    /* 0x1286EC: btst #1,(0x1D58,A5) ; beq 0x12870A. Falls through to 0x12870A
     * unless the object is on screen, in which case it hands over to the
     * bomb orbit 0x0CBB2C. */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x32u, 0x00u);                /* 0x128704 jmp 0x0CBB2C */
            wr8(m, obj + 0x34u, 0xFFu);
            wr32(m, obj + 0x00u, 0x000CBB2Cu);
            return 0;
        }
    }

    /* 0x12870A: btst #2,(0x1D58,A5) ; bne.w 0x1287B2 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x04u) != 0u)
        goto tail_1287b2;

    /* 0x128714: move.w (0x20,A0),D0w ; jsr 0x0CBE12 */
    /* `move.w (0x20,A0),D0w` loads the HIGH word of the 16.16 heading
     * (0x00520000 -> D0w = 0x0052), which 0x0CBE12 masks to a table index. */
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) & 0xFFu));

    /* 0x12871E: move.w (0x878,A5),D0w ; andi.w #1 ; cmp.w (0x22,A0) ; bne */
    if ((rd16(m, F3_A5 + 0x0878u) & 1u) == rd16(m, obj + 0x22u))
        Obj_DrawSprite(m, obj);                        /* 0x12872E jsr 0x1282 */

    Coll_AddHazard(m, obj);                            /* 0x128734 jsr 0x0CBDAE */

    /* 0x12873A: cmpi.w #0xE8,(0x18,A0) ; blt 0x1286DA */
    if ((int16_t)rd16(m, obj + 0x18u) < 0x00E8)
        goto rearm_1286da;

    play_sound(m, 0x15);                               /* 0x128742 */
    /* 0x128756: moveq #5,D0 ; jsr 0x13E4D0 ; move.l D0,-(SP) -- the packed
     * word becomes the child argument. */
    uint32_t arg = sub_13E4D0(m, obj, 5u);
    Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, arg);

    /* 0x128778: 31 fragments 0x1287C4 (group 2, five groups), dbf D1 */
    {
        uint16_t d1 = 0x1Eu;
        do {
            Obj_Spawn(m, obj, 0x001287C4u, 5u, 2u, 0u);
        } while ((uint16_t)(d1--) != 0u);
    }

    wr32(m, obj + 0x00u, 0x001287A4u);                 /* 0x12879E */
    return 0;

rearm_1286da:
    wr32(m, obj + 0x00u, 0x001286E2u);                 /* 0x1286DA */
    return 0;

tail_1287b2:
    /* 0x1287B2: move.w #1,D0 ; addq.w #8,(0x14,A0) ; addq.w #8,(0x18,A0) ;
     * jmp 0x0CD3D6 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 8u));
    sub_0CD3D6(m, obj, 1u);
    return 0;
}
