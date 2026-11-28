#include "fn_auto.h"
/* ROM 0x130D18: steer and draw the falling enemy, or advance to its landing, explosion, or cleanup state. */
#include "fn_boss_camera.h"
#include "round8_projectile_regs.h"

int Fn_130D18(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];

    if (rd8(m, a5 - 0x144Bu) & 0x10u)
        goto spawn_debris;                              /* 0x130D1E -> 0x130E6C */

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        push_long(m, r, 0x130E62u);
        r->a[1] = Obj_0CBDDE(m, obj);                   /* 0x130E5C */
        r->a[7] += 4u;
        r->d[0] = rd16(m, obj + 0x4Cu);
        uint32_t score = r->a[1] + 0x48u;
        /* The 68EC020 bus discards writes into program ROM. */
        if ((score & 0xFFFFFFu) >= F3_ROM_SIZE)
            wr32(m, score, rd32(m, score) + r->d[0]);   /* 0x130E68 */
        goto spawn_debris;
    }

    uint16_t timer = rd16(m, obj + 0x22u);
    if ((int16_t)timer < 0) {
        r->a[1] = a5 + 0x1D2Cu;
        uint16_t count = rd16(m, a5 + 0x1D34u);
        for (;;) {                                      /* 0x130D3A */
            int16_t previous = (int16_t)count;
            count--;
            r->d[1] = set_word(r->d[1], count);
            if (previous <= 0)
                goto falling;
            r->a[2] = rd32(m, r->a[1]);
            r->a[1] += 4u;
            uint16_t dy = (uint16_t)(rd16(m, r->a[2] + 0x18u) - rd16(m, obj + 0x18u));
            r->d[2] = set_word(r->d[2], dy);
            if ((int16_t)dy < 100)
                break;
        }
        timer = 0x80u;
        wr16(m, obj + 0x22u, timer);                    /* 0x130D50 */
    }
    timer -= 8u;
    wr16(m, obj + 0x22u, timer);                        /* 0x130D56 */
    if ((int16_t)timer < 0) {
        call_hooked_sound(m, r, 0x29u, 0x130DFAu, 0x130E00u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x18u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x18u));
        wr16(m, obj + 0x2Cu, 8u);
        wr16(m, obj + 0x30u, 8u);
        wr16(m, obj + 0x2Eu, 0x47u);
        wr16(m, obj + 0x32u, 0x47u);
        wr16(m, obj + 0x1Cu, 0u);
        wr16(m, obj + 6u, 0x64u);
        wr32(m, obj, 0x130E3Au);                        /* 0x130E32 */
        goto done;
    }

falling:;
    uint16_t heading = rd16(m, obj + 0x20u);
    uint8_t rotated = (uint8_t)((heading >> 8) + 0x40u);
    r->d[1] = (rotated & 0x80u) ? 0xFFFFFF03u : 0u;    /* 0x130D68 BFEXTS / ANDI.B */
    wr8(m, obj + 0x1Cu, (uint8_t)r->d[1]);
    r->d[0] = (rotated >> 4) & 7u;                      /* 0x130D74 BFEXTU */
    wr32(m, obj + 0x10u, rd32(m, 0x130EB4u + 4u * r->d[0]));

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if ((rd8(m, a5 + 0x1D58u) & 2u) && x >= -10 && x <= 284) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        r->a[1] = rd32(m, a5 + 0x1D50u);               /* 0x130DA0 -> 0x0CBD12 */
        r->d[0] = rd16(m, obj + 0x4Cu);
        uint32_t score = r->a[1] + 0x48u;
        if ((score & 0xFFFFFFu) >= F3_ROM_SIZE)
            wr32(m, score, rd32(m, score) + r->d[0]);
        explode_projectile(m, r);                      /* 0x0CBD20 -> 0x0CBB2C */
        goto done;
    }

    int16_t turn = (int16_t)(uint16_t)(heading - 0x4000u) >> 5;
    heading = (uint16_t)(heading - turn);               /* 0x130DA6..0x130DB0 */
    wr16(m, obj + 0x20u, heading);
    uint16_t speed = rd16(m, obj + 6u);
    wr16(m, obj + 0x42u, speed >> 8);
    wr16(m, obj + 6u, (uint16_t)(speed + 0x40u));
    r->d[0] = heading >> 8;
    boss_move(m, r, 0x130DD0u);
    push_long(m, r, 0x130DD6u);                         /* 0x0CBDA4 */
    r->a[1] = rd32(m, a5 + 0x8FEu);
    wr32(m, r->a[1], obj);
    r->a[1] += 4u;
    wr32(m, a5 + 0x8FEu, r->a[1]);
    register_projectile(m, r);                         /* 0x0CBDAE */
    r->a[7] += 4u;
    push_long(m, r, 0x130DDCu);
    Obj_11C32C(m, obj);
    r->a[7] += 4u;
    draw_projectile(m, r, 0x130DE2u);
    if ((int16_t)rd16(m, obj + 0x18u) < 0xE8) {
        wr32(m, obj, 0x130D18u);                       /* 0x130D10 */
        goto done;
    }
    goto cleanup;

spawn_debris:
    call_spawn(m, r, 0x130E86u, 0x13E4EEu, 5u, 2u, 2u);
    call_hooked_sound(m, r, 0x34u, 0x130E94u, 0x130E9Au);
cleanup:
    wr32(m, obj, 0x130EA6u);                           /* 0x130E9E */
done:
    r->a[7] += 4u;
    return 0;
}
