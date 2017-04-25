FILES = README lab1b-client.c lab1b-server.c Makefile my.key

.SILENT:

default:
	gcc -lmcrypt -g -O2 -o lab1b-server lab1b-server.c
	gcc -lmcrypt -g -O2 -o lab1b-client lab1b-client.c

lab1b-server:
	gcc -lmcrypt -g -O2 -o lab1b-server lab1b-server.c

lab1b-client:
	gcc -lmcrypt -g -O2 -o lab1b-client lab1b-client.c

clean:
	rm -f lab1b-server lab1b-client *.tar.gz *.txt

dist:
	tar -czf lab1b-000000000.tar.gz $(FILES)
