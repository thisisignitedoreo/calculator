CC := zig cc -target aarch64-linux-gnu
AR := ar
CFLAGS = -Wall -Wextra -Werror -Wno-initializer-overrides -Icril -Icsgl -Iproprietary/include/
CFLAGS += -DMESA_EGL_NO_X11_HEADERS
LDFLAGS = -Lcril -Lcsgl -Lproprietary/lib/ -lmali -lcril -lcsgl

all: Calculator.muxupd
.PHONY: all clean

csgl/libcsgl.a: csgl/csgl.h csgl/core.c csgl/draw.c csgl/text.c
	$(CC) $(CFLAGS) -c -o csgl/core.o csgl/core.c
	$(CC) $(CFLAGS) -c -o csgl/draw.o csgl/draw.c
	$(CC) $(CFLAGS) -c -o csgl/text.o csgl/text.c
	$(AR) rcs csgl/libcsgl.a csgl/core.o csgl/draw.o csgl/text.o

cril/libcril.a: cril/cril.h cril/core.c
	$(CC) $(CFLAGS) -c -o cril/core.o cril/core.c
	$(AR) rcs cril/libcril.a cril/core.o

calc: csgl/libcsgl.a cril/libcril.a src/main.c src/calc.c
	$(CC) $(CFLAGS) -o calc src/main.c $(LDFLAGS)

clean:
	rm -rf csgl/*.o csgl/libcsgl.a
	rm -rf cril/*.o cril/libcril.a
	rm -rf calc
	rm -rf Calculator.muxupd

Calculator.muxupd: calc
	cp calc bundle/mnt/mmc/MUOS/application/Calculator/calc
	cd bundle && 7z -tzip -r a ../Calculator.muxupd *
