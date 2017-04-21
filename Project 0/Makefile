# NAME: Yuxing Chen
# EMAIL: 
# ID: 

FILES = README lab0.c Makefile *.png

.SILENT:

default:
	gcc -g -O2 -o lab0 lab0.c

check: default
	# correctly copy input to output
	echo "This is my test file." > input.txt ; \
	./lab0 --input=input.txt --output=output.txt ;\
	if [ $$? -ne 0 ]; \
	then \
		echo "Error: wrong exit code. Should be 0 instead of $$? ." >> errormsg.txt ; \
	fi
	cmp input.txt output.txt ; \
	if [ $$? -ne 0 ]; \
	then \
		echo "Error: Input and output files are not the same." >> errormsg.txt ; \
	fi
	rm -f output.txt

	# correctly implements --input
	./lab0 --input=input.txt > result.txt ; \
	if [ $$? -ne 0 ]; \
	then \
		echo "Error: incorrect implement of --input. Wrong exit code." >> errormsg.txt ; \
	fi
	cmp input.txt result.txt ; \
	if [ $$? -ne 0 ]; \
	then \
		echo "Error: incorrect implement of --input. Fail to correctly read the input file." >> errormsg.txt ; \
	fi
	rm -f result.txt

	# correctly implements --output
	./lab0 --output=output.txt < input.txt ; \
	if [ $$? -ne 0 ]; \
	then \
		echo "Error: incorrect implement of --output. Should exit with 0 instead of $$? ." >> errormsg.txt ; \
	fi
	cmp input.txt output.txt ; \
	if [ $$? -ne 0 ]; \
	then \
		echo "Error: incorrect implement of --output. Fail to correctly write to the output file." >> errormsg.txt ; \
	fi
	rm -f output.txt

	# correct handling of un-openable/creatable output file
	echo "This file is not openable." > unopenable.txt ; \
	chmod -r unopenable.txt ; \
	./lab0 --input=unopenable.txt > output.txt &>/dev/null ; \
	if [ $$? -ne 2 ]; \
	then \
		echo "Error: incorrect handling of un-openable file. Should exit with 2 instead of $$? ." >> errormsg.txt ; \
	fi
	rm -f unopenable.txt output.txt ; \

	echo "This file is not creatable." > uncreatable.txt; \
	chmod -w uncreatable.txt ; \
	./lab0 --output=uncreatable.txt < input.txt &>/dev/null ; \
	if [ $$? -ne 3 ]; \
	then \
		echo "Error: incorrect handling of un-creatable file. Should exit with 3 instead of $$? ." >> errormsg.txt ; \
	fi
	rm -f uncreatable.txt ; \

	# correct handling of non-existant input file
	./lab0 --input=nonexistant.txt > output.txt &>/dev/null ; \
	if [ $$? -ne 2 ]; \
	then \
		echo "Error: incorrect handling of non-existant file. Should exit with 2 instead of $$? ." >> errormsg.txt ; \
	fi

	# correct handling of invalid arguments
	./lab0 --bad=input.txt --invalid=output.txt &>/dev/null ; \
	if [ $$? -ne 1 ]; \
	then \
		echo "Error: incorrect handling of invalid arguments. Should exit with 1 instead of $$? ." >> errormsg.txt ; \
	fi
	rm -f output.txt ; \

	# fault handling
	./lab0 --segfault --catch &>/dev/null ; \
	if [ $$? -ne 4 ]; \
	then \
		echo "Error: incorrect fault handling. Should exit with 4 instead of $$? ." >> errormsg.txt ; \
	fi

	rm -f input.txt ; \

	if [ -e errormsg.txt ]; \
	then \
		echo -e "The following tests failed:\n" ; \
		cat errormsg.txt ; \
		rm -f errormsg.txt ; \
	else \
		echo -e "All tests passed!" ; \
	fi

clean:
	rm -f lab0 *.txt *.tar.gz

dist: 
	tar -czf lab0-304316037.tar.gz $(FILES)
