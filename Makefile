CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror
SRC = src/kernel/rng.c src/io/input.c src/common/f3mem.c src/player/player_move.c src/collision/collision.c src/audio/sound_cmd.c src/objects/obj_pool.c src/stage/stage_timeline.c src/video/sprite.c src/enemy/enemy_bullet.c src/player/player_shot.c

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
