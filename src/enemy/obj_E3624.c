#include "obj_E3624.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"
#include "obj_E8B72.h"                                    /* Sound_PlayHooked */

/* ROM 0x0E3624: drone that follows the object at +0x08. While that object is
 * alive (its +0x06 is zero) it is told to start dying; every tick the two
 * scroll deltas A5+0x4224/A5+0x4228 are subtracted from +0x14/+0x18, then the
 * +0x06 state dispatches through the long table at 0x0E365E:
 *   0 (0x0E366E) script step; at the end of the script (0x4C == 0, record
 *     0x0E377C) the direction animation at 0x1A0146 is installed and the
 *     state goes to 1 with SFX 0x29;
 *   1 (0x0E36B4) draw, script step, wait +0x23 ticks, then spawn one 0x0E3C48
 *     child per +0x21 rounds (child inherits +0x21 into +0x10) and reload
 *     +0x23 = 2, or move to state 2 when the rounds run out;
 *   2 (0x0E370A) 0x1E-tick pause, then both the parent and this object go to
 *     state 3;
 *   3 (0x0E3722) count down, then the parent returns to state 1 and this
 *     object raises flag +0x07 bit 3 (it dies in 0x0CEE74).
 * Every tick ends with the flag-3 check (0x0CEE74 when set). */

/* ROM 0x0D0090 (local callee 0x0D0090 + ext word at 0x0D0092 -> table
 * 0x0D00B4): pick the record pointer the table entry D0w*2 points at, and
 * load its bank into +0x48/+0x4C/+0x1C/+0x10/+0x68 (the Obj_SetAnim body,
 * with the bank written at +0x68 instead of advanced). */
static void sub_0D0090(const f3mem_t *m, uint32_t obj, uint16_t d0w, uint32_t a1)
{
    uint16_t off = rd16(m, 0x000D00B4u + (uint32_t)d0w * 2u);
    a1 = rd32(m, (a1 + (uint32_t)off) & 0xFFFFFFFFu);
    wr32(m, obj + 0x48u, a1);
    uint16_t v = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x4Cu, v);
    v = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)v);                /* move.b D0b */
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0E366E: state 0 */
static int lab_0E366E(const f3mem_t *m, uint32_t obj)
{
    /* 0x0E366E: jsr 0x0D0690 ; 0x0E3674 tst.w (0x4C,A0) ; beq 0x0E3684 */
    Obj_StepScript(m, obj);
    if (rd16(m, obj + 0x4Cu) != 0u) {
        Obj_DrawSprite(m, obj);                     /* 0x0E367C: jsr 0x001282 */
        return 0;
    }

    /* 0x0E3684: lea (0xF6,PC),A1 (ext word 0x0E3686 -> 0x0E377C) ;
     * move.w (0x58,A0),D0w ; jsr 0x0D0090 */
    sub_0D0090(m, obj, rd16(m, obj + 0x58u), 0x000E377Cu);
    Obj_DrawSprite(m, obj);                         /* 0x0E3692: jsr 0x001282 */
    wr8(m, obj + 0x06u, 0x01u);                     /* 0x0E3698 */
    /* 0x0E369E: move.w #0x29,-(SP) ; jsr 0x13F238 ; jsr 0x0039D8 */
    Sound_PlayHooked(m, 0x0029u);
    return 0;
}

/* ROM 0x0E36B4: state 1 */
static int lab_0E36B4(const f3mem_t *m, uint32_t obj)
{
    Obj_StepScript(m, obj);                         /* 0x0E36B4: jsr 0x0D0690 */
    Obj_DrawSprite(m, obj);                         /* 0x0E36BA: jsr 0x001282 */
    /* 0x0E36C0: subq.b #1,(0x23,A0) ; bne 0x0E3700 */
    uint8_t n = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, n);
    if (n != 0u)
        return 0;

    /* 0x0E36C8: pea #0 ; move.w #2,-(SP) ; move.w #9,-(SP) ;
     * move.l #0x0E3C48,-(SP) ; jsr 0x1057C */
    uint32_t child = Obj_Spawn(m, obj, 0x000E3C48u, 9u, 2u, 0u);
    if ((int32_t)child < 0)                         /* 0x0E36E4: tst.l D0 ; bmi */
        return -1;

    /* 0x0E36EA: move.b (0x21,A0),(0x10,A1) ; subq.b #1,(0x21,A0) ;
     * beq 0x0E3702 ; move.b #2,(0x23,A0) */
    wr8(m, child + 0x10u, rd8(m, obj + 0x21u));
    uint8_t rounds = (uint8_t)(rd8(m, obj + 0x21u) - 1u);
    wr8(m, obj + 0x21u, rounds);
    if (rounds == 0u) {
        wr8(m, obj + 0x06u, 0x02u);                 /* 0x0E3702 */
        return 0;
    }
    wr8(m, obj + 0x23u, 0x02u);                     /* 0x0E36FA */
    return 0;
}

/* ROM 0x0E370A: state 2 */
static int lab_0E370A(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x23u, 0x1Eu);
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr8(m, a1 + 0x06u, 0x03u);
    wr8(m, obj + 0x06u, 0x03u);
    return 0;
}

/* ROM 0x0E3722: state 3 */
static int lab_0E3722(const f3mem_t *m, uint32_t obj)
{
    /* 0x0E3722: subq.b #1,(0x23,A0) ; bne 0x0E373A */
    uint8_t n = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, n);
    if (n == 0u) {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x06u, 0x01u);                  /* 0x0E372E */
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u)); /* bset #3 */
    }
    return 0;
}

int Obj_E3624_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0E3624: movea.l (0x8,A0),A1 ; tst.b (0x6,A1) ; beq 0x0CEE74 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0E3632: sub.l (0x4224,A5),(0x14,A0) ; sub.l (0x4228,A5),(0x18,A0) */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));

    /* 0x0E3642: clr.w D0w ; move.b (0x6,A0),D0b ; jsr ([0x14,PC],D0w*4)
     * jump table 0x0E365E (ext word 0x0E364A -> 0x0E365E):
     * [0]=0x0E366E [1]=0x0E36B4 [2]=0x0E370A [3]=0x0E3722 */
    int rc;
    switch (rd8(m, obj + 0x06u)) {
    case 0: rc = lab_0E366E(m, obj); break;
    case 1: rc = lab_0E36B4(m, obj); break;
    case 2: rc = lab_0E370A(m, obj); break;
    case 3: rc = lab_0E3722(m, obj); break;
    default: return -1;                             /* not a table entry */
    }
    if (rc != 0)
        return rc;

    /* 0x0E3650: btst.b #3,(0x7,A0) ; bne 0x0CEE74 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u)
        Obj_StartDying(m, obj);
    return 0;
}
