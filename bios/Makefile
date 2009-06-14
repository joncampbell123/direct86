all: main.rom

# to update dependencies, run nasm -M main.asm and copy output here

main.rom: main.asm config/config.com boot/int19.inc boot/noboot.inc \
	  boot/reset.inc mem/countram.inc video/text.inc \
	  keyboard/keyboard.inc cmos/cmos.inc timer/setup.inc
	nasm -o $@ -f bin $<

config/config.com:
	make -C config

clean:
	make -C config clean
	rm -f main.rom

