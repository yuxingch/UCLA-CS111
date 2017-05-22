FILES=Makefile README lab4b.c

default: build

build:
	@gcc -std=c11 -lmraa -lm -pthread -o lab4b lab4b.c

check: build
	@printf "PERIOD=2\nSCALE=C\nSTOP\nSTART\nOFF\n" > command.in
	@echo >> command.in
	@echo "Case 1 ... ./lab4b"
	@./lab4b < command.in &>/dev/null
	@if [ $$? -ne 0 ]; \
	then \
		echo "Error when there is no argument passed in" >> errormsg.txt; \
	fi
	@echo "Case 2 ... ./lab4b --period=5"
	@./lab4b --period=5 < command.in &>/dev/null
	@if [ $$? -ne 0 ];  \
	then \
		echo "Error when period is specified" >> errormsg.txt;\
	fi
	@echo "Case 3 ... ./lab4b --period=5 --scale=F"
	@./lab4b --period=5 --scale=F < command.in &>/dev/null
	@if [ $$? -ne 0 ]; \
	then \
		echo "Error when period and scale are specified" >> errormsg.txt; \
	fi
	@echo "Case 4 ... ./lab4b --period=5 --scale=F --log=LOGFILE"
	@./lab4b --period=5 --scale=F --log=LOGFILE < command.in &>/dev/null
	@if [ $$? -ne 0 ]; \
	then \
		echo "Error when period and scale are specified" >> errormsg.txt; \
	fi
	@if [ -e errormsg.txt ]; \
	then \
		echo -e "The following tests failed:\n"; \
		cat errormsg.txt; \
		rm -f errormsg.txt; \
	else \
		echo -e "All tests passed!"; \
	fi
	@rm -f command.in LOGFILE

make dist:
	@tar -czf lab4b-304316037.tar.gz $(FILES)

clean:
	@rm -f lab4b *.tar.gz 
