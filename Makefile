CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror
SRC = src/kernel/rng.c src/io/input.c src/common/f3mem.c src/player/player_move.c src/collision/collision.c src/audio/sound_cmd.c src/objects/obj_pool.c src/objects/obj_common.c src/objects/objscript_CF1E4.c src/objects/objscript_CF4B8.c src/stage/stage_timeline.c src/video/sprite.c src/enemy/enemy_bullet.c src/player/player_shot.c src/enemy/obj_D9116.c src/enemy/obj_CABF4.c src/enemy/obj_EAD28.c src/enemy/obj_DF844.c src/enemy/obj_DF7EA.c src/enemy/obj_DC0F8.c src/enemy/obj_DC328.c src/enemy/obj_13E4EE.c src/enemy/obj_117C28.c src/enemy/obj_117B44.c src/enemy/obj_102F52.c src/enemy/obj_F30B4.c src/enemy/obj_E0266.c src/enemy/obj_E029E.c src/enemy/obj_DEC9E.c src/enemy/obj_DC586.c src/enemy/obj_DC456.c src/enemy/obj_D9092.c src/enemy/obj_CC0F4.c src/enemy/obj_D901A.c src/enemy/obj_D540A.c src/enemy/obj_CBF6C.c src/enemy/obj_E02C4.c src/enemy/obj_DC542.c src/enemy/obj_C9D7E.c src/enemy/obj_C9978.c src/enemy/obj_C6866.c src/enemy/obj_DC3F6.c src/enemy/obj_F302E.c src/enemy/obj_117BDE.c src/enemy/obj_E0278.c src/enemy/obj_EACDE.c src/enemy/obj_DECF8.c src/enemy/obj_E04CC.c src/enemy/obj_F30F6.c src/enemy/obj_DC8BE.c src/enemy/obj_DCDF2.c src/enemy/obj_CBF8A.c src/enemy/obj_119714.c src/enemy/obj_CAB36.c src/enemy/obj_13E5B2.c src/enemy/obj_D5464.c src/enemy/obj_117D06.c src/enemy/obj_119CBA.c src/enemy/obj_CBF5E.c src/enemy/obj_119990.c src/enemy/obj_1027B4.c src/enemy/obj_102860.c src/enemy/obj_102928.c src/enemy/obj_C9F2E.c src/enemy/obj_10252E.c src/enemy/obj_102A9A.c

.PHONY: test clean
test: build/test_kernel build/test_player build/test_collision build/test_sound build/test_objpool build/test_stage build/test_sprite build/test_objects
	./build/test_kernel
	./build/test_player
	./build/test_collision
	./build/test_sound
	./build/test_objpool
	./build/test_stage
	./build/test_sprite
	./build/test_objects

build/test_kernel: tests/test_kernel.c $(SRC) src/kernel/rng.h src/io/input.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_kernel.c src/kernel/rng.c src/io/input.c

build/test_player: tests/test_player.c $(SRC) src/common/f3mem.h src/player/player_move.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_player.c $(SRC)

build/test_collision: tests/test_collision.c $(SRC) src/collision/collision.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_collision.c $(SRC)

build/test_sound: tests/test_sound.c $(SRC) src/audio/sound_cmd.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_sound.c $(SRC)

build/test_objpool: tests/test_objpool.c $(SRC) src/objects/obj_pool.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_objpool.c $(SRC)

build/test_stage: tests/test_stage.c $(SRC) src/stage/stage_timeline.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_stage.c $(SRC)

build/test_sprite: tests/test_sprite.c tests/capture.h $(SRC) src/video/sprite.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_sprite.c $(SRC)

build/test_objects: tests/test_objects.c tests/capture.h $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_objects.c $(SRC)

clean:
	rm -rf build

.PHONY: coverage
coverage:
	python3 scripts/coverage.py
