
FILES = Makefile README lab2_add.c lab2_list.c SortedList.c SortedList.h test.sh *.png *.csv *.gp

.SILENT:

all: build

build:
	gcc -std=gnu11 -pthread -o lab2_add lab2_add.c
	gcc -std=gnu11 -pthread -o  lab2_list lab2_list.c SortedList.c

tests: build
	chmod +x test.sh
	-./test.sh

graphs:
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp

dist:
	tar -czf lab2a-000000000.tar.gz $(FILES)

clean:
	rm -f lab2_add lab2_list *.o *.png *.csv *.tar.gz
