#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2_list-1.png ... the total number of operations per second for each synchronization method
#	lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
#	lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#	lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#

# general plot parameters
set terminal png
set datafile separator ","

#  the total number of operations per second for each synchronization method
set title "lab2b_1: Cost per Operation vs Threads for mutex and spin-lock"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Cost per Operation (Throughput)"
set logscale y 10
set output 'lab2b_1.png'

# grep out results of mutex and spin-lock
plot \
     "< grep -E 'list-none-m,[0-9]+,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list with mutex method' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-s,[0-9]+,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list with spin-lock method' with linespoints lc rgb 'blue'


set title "lab2b_2: Time vs Number of Threads for mutex"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Time"
set logscale y 10
set output 'lab2b_2.png'
set key left top

plot \
     "< grep -E 'list-none-m,[0-9],1000,1,|list-none-m,16,1000,1,|list-none-m,24,1000,1,' lab2b_list.csv" using ($2):($7) \
     	title 'completion time' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-m,[0-9],1000,1,|list-none-m,16,1000,1,|list-none-m,24,1000,1,' lab2b_list.csv" using ($2):($8) \
     	title 'wait-for-lock time' with linespoints lc rgb 'blue'


set title "lab2b_3: Iterations that run without failure"
set logscale x 2
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
set key left top

plot \
    "< grep -E 'list-id-none,[0-9]+,[0-9]+,4,' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "red" title "unprotected", \
    "< grep -E 'list-id-m,[0-9]+,[0-9]+,4,' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "green" title "mutex", \
    "< grep -E 'list-id-s,[0-9]+,[0-9]+,4,' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "blue" title "spin-Lock"


set title "lab2b_4: Throughput of Mutex-Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_4.png'

# grep out only successful (sum=0) yield runs
plot \
     "< grep -E 'list-none-m,[0-9],1000,1,|list-none-m,12,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-m,[0-9],1000,4,|list-none-m,12,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 lists' with linespoints lc rgb 'green', \
     "< grep -E 'list-none-m,[0-9],1000,8,|list-none-m,12,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< grep -E 'list-none-m,[0-9],1000,16,|list-none-m,12,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 lists' with linespoints lc rgb 'orange'

set title "lab2b_5: Throughput of Spin-Lock-Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_5.png'

# grep out only successful (sum=0) yield runs
#plot \
#     "< grep -E 'list-none-s,[0-9]+,1000,1,' lab2_list.csv" using ($2):(1000000000/($7)) \
#	title '1 list' with linespoints lc rgb 'red', \
#     "< grep -E 'list-none-s,[0-9]+,1000,4,' lab2_list.csv" using ($2):(1000000000/($7)) \
#	title '4 lists' with linespoints lc rgb 'green', \
#     "< grep -E 'list-none-s,[0-9]+,1000,8,' lab2_list.csv" using ($2):(1000000000/($7)) \
#	title '8 lists' with linespoints lc rgb 'blue', \
#     "< grep -E 'list-none-s,[0-9]+,1000,16,' lab2_list.csv" using ($2):(1000000000/($7)) \
#	title '16 lists' with linespoints lc rgb 'orange'

plot \
     "< grep -E 'list-none-s,[0-9],1000,1,|list-none-s,12,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-s,[0-9],1000,4,|list-none-s,12,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '4 lists' with linespoints lc rgb 'green', \
     "< grep -E 'list-none-s,[0-9],1000,8,|list-none-s,12,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '8 lists' with linespoints lc rgb 'blue', \
     "< grep -E 'list-none-s,[0-9],1000,16,|list-none-s,12,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '16 lists' with linespoints lc rgb 'orange'
