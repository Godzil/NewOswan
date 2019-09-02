all:
	make -C seal-hack
	make -C oswan

clean:
	make -C seal-hack clean
	make -C oswan clean
