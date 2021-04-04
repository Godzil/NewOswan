.PHONY: all clean

all: ws_irom.bin wsc_irom.bin wc_irom.bin
	
clean:
	@echo " RM"
	@rm -f *.bin	

ws_irom.bin: irom_stub.asm
	@echo " AS $@"
	@nasm $^ -o $@ -DWONDERSWAN=1

wsc_irom.bin: irom_stub.asm
	@echo " AS $@"
	@nasm $^ -o $@ -DWONDERSWANCOLOR=1
	
wc_irom.bin: irom_stub.asm
	@echo " AS $@"
	@nasm $^ -o $@ -DSWANCRYSTAL=1
	
