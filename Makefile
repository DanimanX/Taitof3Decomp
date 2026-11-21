CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror
SRC = src/kernel/rng.c src/io/input.c src/common/f3mem.c src/player/player_move.c

.PHONY: test clean
test: build/test_kernel build/test_player
	./build/test_kernel
	./build/test_player

build/test_kernel: tests/test_kernel.c $(SRC) src/kernel/rng.h src/io/input.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_kernel.c src/kernel/rng.c src/io/input.c

build/test_player: tests/test_player.c $(SRC) src/common/f3mem.h src/player/player_move.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_player.c $(SRC)

clean:
	rm -rf build
