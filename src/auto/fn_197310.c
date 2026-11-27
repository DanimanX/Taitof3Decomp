/* ROM 0x197310: object initialiser (A0 = obj). Clears the movement/anim state
 * (0x17A7FA), sets speed words +0x16/+0x1A, latches the frame-list word at
 * 0x197C0C into +0x40 with the list body at +0x3A, runs that index's frame
 * INIT through the 0x179A5C dispatch (= 0x17955E, index 8), spawns one object
 * of routine 0x17A5A8 (group 3, two groups, arg 0x6020) into +0x0C, sets
 * +0x24 = 0x10000 and re-arms with tick 0x197370. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"

int Fn_197310(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t idx;
    uint32_t list;

    /* ROM 0x0017A7FA: reset the object's motion / animation fields */
    wr16(m, obj + 0x14u, 0u);                        /* 0x17A7FA */
    wr16(m, obj + 0x18u, 0u);                        /* 0x17A7FE */
    wr16(m, obj + 0x20u, 0u);                        /* 0x17A802 */
    wr32(m, obj + 0x50u, 0u);                        /* 0x17A806 */
    wr32(m, obj + 0x54u, 0u);                        /* 0x17A80A */
    wr32(m, obj + 0x58u, 0u);                        /* 0x17A80E */
    wr16(m, obj + 0x16u, 0u);                        /* 0x17A812 */
    wr16(m, obj + 0x1Au, 0u);                        /* 0x17A816 */
    wr16(m, obj + 0x42u, 0u);                        /* 0x17A81A */
    wr16(m, obj + 0x5Cu, 0u);                        /* 0x17A81E */
    wr16(m, obj + 0x5Eu, 0u);                        /* 0x17A822 */
    wr16(m, obj + 0x60u, 0u);                        /* 0x17A826 */
    wr32(m, obj + 0x4Cu, 0xFFFFFFFFu);               /* 0x17A82A move.l #-0x1,(0x4c,A0) */
    wr16(m, obj + 0x1Eu, 0u);                        /* 0x17A832 */
    wr16(m, obj + 0x1Cu, 0u);                        /* 0x17A836 */
    wr8(m, obj + 0x22u, 0u);                         /* 0x17A83A */
    wr8(m, obj + 0x23u, 0u);                         /* 0x17A83E */
    wr8(m, obj + 0x34u, 0u);                         /* 0x17A842 */
    wr8(m, obj + 0x35u, 0u);                         /* 0x17A846 */

    wr16(m, obj + 0x16u, 0x0020u);                   /* 0x197316 move.w #0x20,(0x16,A0) */
    wr16(m, obj + 0x1Au, 0x0008u);                   /* 0x19731C move.w #0x8,(0x1a,A0) */

    idx = rd16(m, 0x197C0Cu);                        /* 0x197328 move.w (A1)+,D0w, A1 = 0x197c0c */
    wr16(m, obj + 0x40u, idx);                       /* 0x19732A move.w D0w,(0x40,A0) */
    list = 0x197C0Cu + 2u;                            /* 0x19732E move.l A1,(0x3a,A0) */
    wr32(m, obj + 0x3Au, list);

    /* 0x197332 lea (0x179a5c).l,A1 ; 0x197338 movea.l (0x0,A1,D0w*0x8),A1 ;
     * 0x19733C jsr (A1) -- the INIT half of frame-list index 8 (0x17955E,
     * translated). It leaves A1 past the record: +2 dur, +4 spr, +2 byte. */
    (void)Obj_AnimInit(m, obj, idx);
    r->a[1] = list + 8u;                             /* A1 after 0x17955E: past dur/spr/byte */

    /* 0x19733E..0x197352: push 0x6020, #2, #3, #0x17a5a8 ; jsr 0x01057C */
    r->d[0] = Obj_Spawn(m, obj, 0x0017A5A8u, 3u, 2u, 0x00006020u);
    wr32(m, obj + 0x0Cu, r->d[0]);                   /* 0x19735C move.l D0,(0xc,A0) */

    wr32(m, obj + 0x24u, 0x00010000u);               /* 0x197360 move.l #0x10000,(0x24,A0) */
    wr32(m, obj, 0x00197370u);                       /* 0x197368 move.l #0x197370,(A0) */
    return 0;                                        /* 0x19736E rts */
}
