all: libA.so main.elf

libA.so: a.c
	$(CC) -o $@ $^ -shared -lm

main.elf: main.c libA.so
	$(CC) -o $@ $^ -L./ -lA

clean:
	rm -f libA.so main.elf