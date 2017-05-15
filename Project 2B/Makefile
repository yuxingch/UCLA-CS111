FILES=Makefile README lab2_list.c SortedList.c SortedList.h test.sh *.png *.csv *.gp *.out
.SILENT:

all: build

build:
	gcc -std=gnu11 -pthread -o lab2_list lab2_list.c SortedList.c


tests: build
	chmod +x test.sh
	-./test.sh

graphs:
	gnuplot lab2_list.gp

clean:
	rm -f *.csv *.png lab2_list *.out *.tar.gz

profile: profile.gperf

profile.gperf: build
#	mv lab2_list lab2_list_saved
#	gcc -std=gnu11 -pthread -o -g -L /u/eng/class/classyux/lib -lprofiler lab2_list lab2_list.c SortedList.o
	mv lab2_list lab2_list_saved
	gcc -std=gnu11 -pthread -g -L /u/eng/class/classyux/lib -lprofiler -o lab2_list SortedList.c lab2_list.c
	rm -f ./raw.gperf
	LD_PRELOAD=/u/eng/class/classyux/lib/libprofiler.so.0 \
	CPUPROFILE=./raw.gperf \
	./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.gperf
	pprof --list=my_thread_list ./lab2_list ./raw.gperf >> $@
	rm -f raw.gperf lab2_list
	mv lab2_list_saved lab2_list
	mv profile.gperf profile.out

dist:
	tar -czf lab2b-304316037.tar.gz $(FILES)
