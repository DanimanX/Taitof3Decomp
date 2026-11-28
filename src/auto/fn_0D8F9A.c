/* ROM 0x0D8F9A: animated projectile tick (A0 = object). Steps the
 * {duration.w, x/y.l} animation table 0x0D8FCE through +0x3A/+0x3E (0x0CBD24),
 * and while the new entry's duration is positive moves the object along +0x20
 * at speed +0x42 (0x0CBE12), draws its sprite (0x001282) and re-arms
 * 0x0D8F9A; on the negative terminator it frees the slot (0x0105EC). */
#include "fn_auto.h"
#include "batch_obj_regs.h"

/* ROM 0x0CBD24: advance the 6-byte {dur.w,pos.l} entry at table+offset
 * (+0x3A), counting +0x3E up to dur and stepping the offset by 6 past it.
 * Leaves D0 = offset and D1 = counter. */
static void anim_step(const f3mem_t *m, uint32_t obj, uint32_t tbl,
                      uint32_t *d0, uint32_t *d1)
{
    for (;;) {                                          /* 0x0CBD24 */
        uint16_t off = rd16(m, obj + 0x3Au);
        *d0 = (*d0 & 0xFFFF0000u) | off;
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur <= 0)                                   /* 0x0CBD28 */
            return;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u); /* 0x0CBD30 */
        wr16(m, obj + 0x3Eu, cnt);
        *d1 = (*d1 & 0xFFFF0000u) | cnt;
        if ((int16_t)cnt <= dur) {                      /* 0x0CBD38 */
            wr32(m, obj + 0x10u,
                 rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u)); /* 0x0CBD4A */
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);                       /* 0x0CBD40 */
        off = (uint16_t)(off + 6u);                     /* 0x0CBD44 */
        wr16(m, obj + 0x3Au, off);
    }
}

/* ROM 0x0CBE12: pos += speed * {cos,sin}[angle]; D1 = speed * cos, D2 = sin. */
static void move_by_angle(const f3mem_t *m, f3regs_t *r, uint16_t angle)
{
    uint32_t obj = r->a[0];
    uint32_t a3 = 0x00027E40u;
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)speed;
    r->d[2] = (r->d[2] & 0xFFFF0000u) | (uint16_t)speed;
    int32_t dx = (int32_t)speed * (int32_t)(int16_t)rd16(m, a3 + 0x80u + 2u * angle);
    int32_t dy = (int32_t)speed * (int32_t)(int16_t)rd16(m, a3 + 2u * angle);
    r->d[1] = (uint32_t)dx;
    r->d[2] = (uint32_t)dy;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)dx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)dy);
    r->a[3] = a3;
}

int Fn_0D8F9A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = 0x000D8FCEu;                              /* 0x0D8F9A lea */
    anim_step(m, obj, r->a[1], &r->d[0], &r->d[1]);     /* 0x0D8FA0 */
    /* 0x0D8FA6 tst.w (A1,D0.w): the entry just stepped to */
    if ((int16_t)rd16(m, r->a[1] + (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0]) <= 0) {
        batch_free_object(m, r, 0x000D8FC8u);           /* 0x0D8FC0 */
        return 0;
    }
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, obj + 0x20u); /* 0x0D8FAE */
    move_by_angle(m, r, (uint16_t)r->d[0]);             /* 0x0D8FB2 */
    batch_draw_sprite(m, r, 0x000D8FB8u);               /* 0x0D8FB8 */
    wr32(m, obj, 0x000D8F9Au);                          /* 0x0D8F92 */
    return 0;
}
