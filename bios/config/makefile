all: config.com

config.com: config.asm
	nasm -o $@ -f bin $<

clean:
	rm -f config.com

