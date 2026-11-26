/* ROM 0x1292A8: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

/* ROM 0x0039D8: Sound command dispatcher */
void Sound_PlayCmd(const f3mem_t *m, int16_t cmd);

/* ROM 0x13F238: Hooked sound player */
void Sound_PlayHooked(const f3mem_t *m, uint16_t cmdw);


/* Helper to execute the block at 0x129724 */
static void exec_129724(const f3mem_t *m, f3regs_t *r) {
    r->a[7] -= 2u;
    wr16(m, r->a[7], 0x0013u);
    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x0012972Eu);
    Sound_PlayHooked(m, 0x0013u);
    r->a[7] += 4u;
    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x00129734u);
        /* 0x39D8 already sent by Sound_PlayHooked (13F238 + 39D8) */
    r->a[7] += 4u;
    r->a[7] += 2u;
    
    uint16_t val = rd16(m, F3_A5 + 0x30dau);
    if ((int16_t)val < 0x1F4) {
        /* Jump to 0x129D46 */
        r->a[7] -= 2u;
        wr16(m, r->a[7], 0x0013u);
        r->a[7] -= 4u;
        wr32(m, r->a[7], 0x00129D50u);
        Sound_PlayHooked(m, 0x0013u);
        r->a[7] += 4u;
        r->a[7] -= 4u;
        wr32(m, r->a[7], 0x00129D56u);
        /* 0x39D8 already sent by Sound_PlayHooked (13F238 + 39D8) */
        r->a[7] += 4u;
        r->a[7] += 2u;
        wr32(m, F3_A5 + 0x30eeu, 0u);
        wr8(m, r->a[0] + 0x54u, 0x06u);
        wr16(m, r->a[0] + 0x50u, 0x00FAu);
        wr16(m, r->a[0] + 0x52u, 0x0078u);
        wr16(m, r->a[0] + 6u, 0x003Cu);
        wr8(m, r->a[0] + 0x54u, 0x02u);
        wr16(m, r->a[0] + 0x50u, 0x00DCu);
        wr16(m, r->a[0] + 0x52u, 0x0078u);
        wr8(m, F3_A5 + 0x30f2u, 0x01u);
        wr16(m, r->a[0] + 6u, 0x0050u);
        uint16_t v = rd16(m, F3_A5 + 0x30eeu);
        v |= 0x0800u;
        wr16(m, F3_A5 + 0x30eeu, v);
        wr16(m, r->a[0] + 6u, 0x0001u);
        v &= (uint16_t)~0x0800u;
        wr16(m, F3_A5 + 0x30eeu, v);
        wr8(m, r->a[0] + 0x54u, 0u);
        wr16(m, r->a[0] + 6u, 0x00B4u);
        wr32(m, r->a[0], 0x00129E5Eu);
    } else {
        wr8(m, r->a[0] + 0x54u, 0x03u);
        wr16(m, r->a[0] + 0x50u, 0x00D2u);
        wr16(m, r->a[0] + 0x52u, 0x0078u);
        wr8(m, F3_A5 + 0x30f2u, 0x03u);
        wr16(m, r->a[0] + 6u, 0x0028u);
        wr32(m, r->a[0], 0x001297B2u);
    }
}


/* ROM 0x1292A8: Object initialization / state machine entry */
int Fn_1292A8(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1292A8: Decrement counter at A0+6 */
    uint16_t cnt = (uint16_t)(rd16(m, r->a[0] + 6u) - 1u);
    wr16(m, r->a[0] + 6u, cnt);
    
    /* ROM 0x1292AC: If counter > 0, jump to re-arm (0x1292A0) */
    if (cnt != 0xFFFFu) {
        wr32(m, r->a[0], 0x001292A8u);
        return 0;
    }

    /* ROM 0x1292AE: Set timer to 10 at A0+6 */
    wr16(m, r->a[0] + 6u, 0x000Au);
    
    /* ROM 0x1292B4: Branch if equal (taken if Z=1, i.e., counter was 1) */
    if (cnt == 0) {
        /* ROM 0x1292C4: move.w #-0x8000,(0x20,A0) */
        wr16(m, r->a[0] + 0x20u, 0x8000u);
        
        /* ROM 0x1292CA: move.w #0xc8,(0x50,A0) */
        wr16(m, r->a[0] + 0x50u, 0x00C8u);
        
        /* ROM 0x1292D0: move.w #0x64,(0x52,A0) */
        wr16(m, r->a[0] + 0x52u, 0x0064u);
        
        /* ROM 0x1292D6: clr.w (0x42,A0) */
        wr16(m, r->a[0] + 0x42u, 0u);
        
        /* ROM 0x1292DA: clr.w (0x56,A0) */
        wr16(m, r->a[0] + 0x56u, 0u);
        
        /* ROM 0x1292DE: move.b #0x3,(0x54,A0) */
        wr8(m, r->a[0] + 0x54u, 0x03u);
        
        /* ROM 0x1292E4: move.w #0x104,(0x6,A0) */
        wr16(m, r->a[0] + 6u, 0x0104u);
        
        /* ROM 0x1292EA: beq.b 0x12930C (Taken because Z=1) */
        
        /* ROM 0x12930C: sf (0x30f3,A5) */
        wr8(m, F3_A5 + 0x30f3u, 0u);
        
        /* ROM 0x129310: move.w #0x13,-(SP) */
        r->a[7] -= 2u;
        wr16(m, r->a[7], 0x0013u);
        
        /* ROM 0x129314: jsr 0x13F238 */
        r->a[7] -= 4u;
        wr32(m, r->a[7], 0x0012931Au);
        Sound_PlayHooked(m, 0x0013u);
        r->a[7] += 4u;
        
        /* ROM 0x12931A: jsr 0x39D8 */
        r->a[7] -= 4u;
        wr32(m, r->a[7], 0x00129320u);
        /* 0x39D8 already sent by Sound_PlayHooked (13F238 + 39D8) */
        r->a[7] += 4u;
        
        /* ROM 0x129320: lea (0x2,SP),SP */
        r->a[7] += 2u;
        
        /* ROM 0x129324: sf (0x54,A0) */
        wr8(m, r->a[0] + 0x54u, 0u);
        
        /* ROM 0x129328: move.w #0x3c,(0x6,A0) */
        wr16(m, r->a[0] + 6u, 0x003Cu);
        
        /* ROM 0x12932E: beq.b 0x129350 (Taken because Z=1) */
        
        /* ROM 0x129350: move.b #0x2,(0x54,A0) */
        wr8(m, r->a[0] + 0x54u, 0x02u);
        
        /* ROM 0x129356: move.b #0x0,(0x30f2,A5) */
        wr8(m, F3_A5 + 0x30f2u, 0u);
        
        /* ROM 0x12935C: bfset (0x30ee,A5){0x2:0x1} */
        {
            uint16_t v = rd16(m, F3_A5 + 0x30eeu);
            v |= 0x0004u;
            wr16(m, F3_A5 + 0x30eeu, v);
        }
        
        /* ROM 0x129362: bfset (0x30ee,A5){0x0:0x1} */
        {
            uint16_t v = rd16(m, F3_A5 + 0x30eeu);
            v |= 0x0001u;
            wr16(m, F3_A5 + 0x30eeu, v);
        }
        
        /* ROM 0x129368: bfset (0x30ee,A5){0x3:0x1} */
        {
            uint16_t v = rd16(m, F3_A5 + 0x30eeu);
            v |= 0x0008u;
            wr16(m, F3_A5 + 0x30eeu, v);
        }
        
        /* ROM 0x12936E: move.w #0xc8,(0x6,A0) */
        wr16(m, r->a[0] + 6u, 0x00C8u);
        
        /* ROM 0x129374: beq.b 0x129396 (Taken because Z=1) */
        
        /* ROM 0x129396: bfclr (0x30ee,A5){0x2:0x1} */
        {
            uint16_t v = rd16(m, F3_A5 + 0x30eeu);
            v &= (uint16_t)~0x0004u;
            wr16(m, F3_A5 + 0x30eeu, v);
        }
        
        /* ROM 0x12939C: bfclr (0x30ee,A5){0x0:0x1} */
        {
            uint16_t v = rd16(m, F3_A5 + 0x30eeu);
            v &= (uint16_t)~0x0001u;
            wr16(m, F3_A5 + 0x30eeu, v);
        }
        
        /* ROM 0x1293A2: bfclr (0x30ee,A5){0x3:0x1} */
        {
            uint16_t v = rd16(m, F3_A5 + 0x30eeu);
            v &= (uint16_t)~0x0008u;
            wr16(m, F3_A5 + 0x30eeu, v);
        }
        
        /* ROM 0x1293A8: cmpi.w #0x578,(0x30da,A5) */
        uint16_t val = rd16(m, F3_A5 + 0x30dau);
        
        /* ROM 0x1293AE: blt.w 0x1293B6 */
        if ((int16_t)val < 0x578) {
            /* ROM 0x1293B6: move.w #0x13,-(SP) */
            r->a[7] -= 2u;
            wr16(m, r->a[7], 0x0013u);
            
            /* ROM 0x1293BA: jsr 0x13F238 */
            r->a[7] -= 4u;
            wr32(m, r->a[7], 0x001293C0u);
            Sound_PlayHooked(m, 0x0013u);
            r->a[7] += 4u;
            
            /* ROM 0x1293C0: jsr 0x39D8 */
            r->a[7] -= 4u;
            wr32(m, r->a[7], 0x001293C6u);
        /* 0x39D8 already sent by Sound_PlayHooked (13F238 + 39D8) */
            r->a[7] += 4u;
            
            /* ROM 0x1293C6: lea (0x2,SP),SP */
            r->a[7] += 2u;
            
            /* ROM 0x1293CA: cmpi.w #0x3e8,(0x30da,A5) */
            val = rd16(m, F3_A5 + 0x30dau);
            
            /* ROM 0x1293D0: blt.w 0x129724 */
            if ((int16_t)val < 0x3E8) {
                exec_129724(m, r);
            } else {
                /* ROM 0x12941C: move.b #0x3,(0x54,A0) */
                wr8(m, r->a[0] + 0x54u, 0x03u);
                
                /* ROM 0x129422: move.w #0xd2,(0x50,A0) */
                wr16(m, r->a[0] + 0x50u, 0x00D2u);
                
                /* ROM 0x129428: move.w #0x78,(0x52,A0) */
                wr16(m, r->a[0] + 0x52u, 0x0078u);
                
                /* ROM 0x12942E: move.b #0x3,(0x30f2,A5) */
                wr8(m, F3_A5 + 0x30f2u, 0x03u);
                
                /* ROM 0x129434: move.w #0x28,(0x6,A0) */
                wr16(m, r->a[0] + 6u, 0x0028u);
                
                /* ROM 0x12943A: beq.b 0x12945C (Taken) */
                
                /* ROM 0x12945C: sf (0x54,A0) */
                wr8(m, r->a[0] + 0x54u, 0u);
                
                /* ROM 0x129460: move.w #0x1e,(0x6,A0) */
                wr16(m, r->a[0] + 6u, 0x001Eu);
                
                /* ROM 0x129466: beq.b 0x129488 (Taken) */
                
                /* ROM 0x129488: bfset (0x30ee,A5){0x4:0x1} */
                {
                    uint16_t v = rd16(m, F3_A5 + 0x30eeu);
                    v |= 0x0010u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                }
                
                /* ROM 0x12948E: move.w #0x14,(0x6,A0) */
                wr16(m, r->a[0] + 6u, 0x0014u);
                
                /* ROM 0x129494: beq.b 0x1294B6 (Taken) */
                
                /* ROM 0x1294B6: bfset (0x30ee,A5){0x5:0x1} */
                {
                    uint16_t v = rd16(m, F3_A5 + 0x30eeu);
                    v |= 0x0020u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                }
                
                /* ROM 0x1294BC: move.w #0x50,(0x6,A0) */
                wr16(m, r->a[0] + 6u, 0x0050u);
                
                /* ROM 0x1294C2: beq.b 0x1294E4 (Taken) */
                
                /* ROM 0x1294E4: bfset (0x30ee,A5){0x0:0x1} */
                {
                    uint16_t v = rd16(m, F3_A5 + 0x30eeu);
                    v |= 0x0001u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                }
                
                /* ROM 0x1294EA: bfset (0x30ee,A5){0x3:0x1} */
                {
                    uint16_t v = rd16(m, F3_A5 + 0x30eeu);
                    v |= 0x0008u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                }
                
                /* ROM 0x1294F0: move.w #0x12c,(0x6,A0) */
                wr16(m, r->a[0] + 6u, 0x012Cu);
                
                /* ROM 0x1294F6: beq.b 0x129518 (Taken) */
                
                /* ROM 0x129518: bfclr (0x30ee,A5){0x5:0x1} */
                {
                    uint16_t v = rd16(m, F3_A5 + 0x30eeu);
                    v &= (uint16_t)~0x0020u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                }
                
                /* ROM 0x12951E: move.w #0x3c,(0x6,A0) */
                wr16(m, r->a[0] + 6u, 0x003Cu);
                
                /* ROM 0x129524: beq.b 0x129546 (Taken) */
                
                /* ROM 0x129546: bfclr (0x30ee,A5){0x4:0x1} */
                {
                    uint16_t v = rd16(m, F3_A5 + 0x30eeu);
                    v &= (uint16_t)~0x0010u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                }
                
                /* ROM 0x12954C: bfclr (0x30ee,A5){0x0:0x1} */
                {
                    uint16_t v = rd16(m, F3_A5 + 0x30eeu);
                    v &= (uint16_t)~0x0001u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                }
                
                /* ROM 0x129552: bfclr (0x30ee,A5){0x3:0x1} */
                {
                    uint16_t v = rd16(m, F3_A5 + 0x30eeu);
                    v &= (uint16_t)~0x0008u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                }
                
                /* ROM 0x129558: cmpi.w #0x3e8,(0x30da,A5) */
                val = rd16(m, F3_A5 + 0x30dau);
                
                /* ROM 0x12955E: blt.w 0x129724 */
                if ((int16_t)val < 0x3E8) {
                    exec_129724(m, r);
                } else {
                    /* ROM 0x129562: move.b #0x3,(0x54,A0) */
                    wr8(m, r->a[0] + 0x54u, 0x03u);
                    
                    /* ROM 0x129568: move.w #0xc8,(0x50,A0) */
                    wr16(m, r->a[0] + 0x50u, 0x00C8u);
                    
                    /* ROM 0x12956E: move.w #0x78,(0x52,A0) */
                    wr16(m, r->a[0] + 0x52u, 0x0078u);
                    
                    /* ROM 0x129574: move.b #0x7,(0x30f2,A5) */
                    wr8(m, F3_A5 + 0x30f2u, 0x07u);
                    
                    /* ROM 0x12957A: move.w #0x3c,(0x6,A0) */
                    wr16(m, r->a[0] + 6u, 0x003Cu);
                    
                    /* ROM 0x129580: beq.b 0x1295A2 (Taken) */
                    
                    /* ROM 0x129582: move.l #0x12958A,(A0) */
                    wr32(m, r->a[0], 0x0012958Au);
                    
                    /* ROM 0x129588: rts */
                    return 0;
                }
            }
        } else {
            /* ROM 0x1293B2: bra.w 0x129324 */
            /* Loop back to 0x129324 */
            while (1) {
                wr8(m, r->a[0] + 0x54u, 0u);
                wr16(m, r->a[0] + 6u, 0x003Cu);
                val = rd16(m, F3_A5 + 0x30dau);
                if ((int16_t)val < 0x578) {
                    break;
                }
            }
            /* Continue from 0x129350 */
            wr8(m, r->a[0] + 0x54u, 0x02u);
            wr8(m, F3_A5 + 0x30f2u, 0u);
            uint16_t v = rd16(m, F3_A5 + 0x30eeu);
            v |= 0x0004u;
            wr16(m, F3_A5 + 0x30eeu, v);
            v |= 0x0001u;
            wr16(m, F3_A5 + 0x30eeu, v);
            v |= 0x0008u;
            wr16(m, F3_A5 + 0x30eeu, v);
            wr16(m, r->a[0] + 6u, 0x00C8u);
            v = rd16(m, F3_A5 + 0x30eeu);
            v &= (uint16_t)~0x0004u;
            wr16(m, F3_A5 + 0x30eeu, v);
            v &= (uint16_t)~0x0001u;
            wr16(m, F3_A5 + 0x30eeu, v);
            v &= (uint16_t)~0x0008u;
            wr16(m, F3_A5 + 0x30eeu, v);
            val = rd16(m, F3_A5 + 0x30dau);
            if ((int16_t)val < 0x578) {
                /* Jump to 0x1293B6 */
                r->a[7] -= 2u;
                wr16(m, r->a[7], 0x0013u);
                r->a[7] -= 4u;
                wr32(m, r->a[7], 0x001293C0u);
                Sound_PlayHooked(m, 0x0013u);
                r->a[7] += 4u;
                r->a[7] -= 4u;
                wr32(m, r->a[7], 0x001293C6u);
        /* 0x39D8 already sent by Sound_PlayHooked (13F238 + 39D8) */
                r->a[7] += 4u;
                r->a[7] += 2u;
                val = rd16(m, F3_A5 + 0x30dau);
                if ((int16_t)val < 0x3E8) {
                    exec_129724(m, r);
                } else {
                    /* ROM 0x12941C: move.b #0x3,(0x54,A0) */
                    wr8(m, r->a[0] + 0x54u, 0x03u);
                    wr16(m, r->a[0] + 0x50u, 0x00D2u);
                    wr16(m, r->a[0] + 0x52u, 0x0078u);
                    wr8(m, F3_A5 + 0x30f2u, 0x03u);
                    wr16(m, r->a[0] + 6u, 0x0028u);
                    wr8(m, r->a[0] + 0x54u, 0u);
                    wr16(m, r->a[0] + 6u, 0x001Eu);
                    v = rd16(m, F3_A5 + 0x30eeu);
                    v |= 0x0010u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                    wr16(m, r->a[0] + 6u, 0x0014u);
                    v |= 0x0020u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                    wr16(m, r->a[0] + 6u, 0x0050u);
                    v |= 0x0001u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                    v |= 0x0008u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                    wr16(m, r->a[0] + 6u, 0x012Cu);
                    v &= (uint16_t)~0x0020u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                    wr16(m, r->a[0] + 6u, 0x003Cu);
                    v &= (uint16_t)~0x0010u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                    v &= (uint16_t)~0x0001u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                    v &= (uint16_t)~0x0008u;
                    wr16(m, F3_A5 + 0x30eeu, v);
                    val = rd16(m, F3_A5 + 0x30dau);
                    if ((int16_t)val < 0x3E8) {
                        exec_129724(m, r);
                    } else {
                        /* ROM 0x129562: move.b #0x3,(0x54,A0) */
                        wr8(m, r->a[0] + 0x54u, 0x03u);
                        wr16(m, r->a[0] + 0x50u, 0x00C8u);
                        wr16(m, r->a[0] + 0x52u, 0x0078u);
                        wr8(m, F3_A5 + 0x30f2u, 0x07u);
                        wr16(m, r->a[0] + 6u, 0x003Cu);
                        wr32(m, r->a[0], 0x0012958Au);
                    }
                }
            }
        }
    } else {
        /* ROM 0x1292B6: Counter was not 1 (was <= 0) */
        /* ROM 0x1292B6: move.l #0x1292BE,(A0) */
        wr32(m, r->a[0], 0x001292BEu);
        
        /* ROM 0x1292BC: rts */
        return 0;
    }
    
    /* If we reached here, we returned from a helper or loop */
    return 0;
}
