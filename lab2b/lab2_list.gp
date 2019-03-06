#! /usr/local/cs/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2_list-1.png ... cost per operation vs threads and iterations
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


set xtics

set title "List: Perfomance of synchronization mechanisms"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "the total number of operations per second"
set logscale y
set output 'lab2b_1.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/spin-lock' with linespoints lc rgb 'green'


set title "List: Wait time for mutex sync"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "average wait time per lock(ns)"
set logscale y
set output 'lab2b_2.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'the wait-for-lock time' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'the average time per operation' with linespoints lc rgb 'green'


set title "List: Threads and Iterations that run without failure"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Iterations per thread"
set logscale y 10
set output 'lab2b_3.png'
# grep out only successful (sum=0) yield runs
plot \
     "< grep list-id-none lab2b_list.csv " using ($2):($3) \
	title 'unprotected w/yields=id' with points lc rgb 'red', \
	"< grep list-id-s lab2b_list.csv " using ($2):($3) \
	title 'spin w/yields=id' with points lc rgb 'green', \
     "< grep list-id-m lab2b_list.csv " using ($2):($3) \
	title 'mutex w/yields=id' with points lc rgb 'orange'

set title "List: Perfomance of Improved synchronization mechanisms(sync=m)"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "the total number of operations per second"
set logscale y
set output 'lab2b_4.png'
set key left top
plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list=1' with linespoints lc rgb 'blue', \
    "< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list=4' with linespoints lc rgb 'green', \
	"< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list=8' with linespoints lc rgb 'red', \
	"< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list=16' with linespoints lc rgb 'orange'


set title "List: Perfomance of Improved synchronization mechanisms(sync=s)"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "the total number of operations per second"
set logscale y
set output 'lab2b_5.png'
set key left top
plot \
    "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list=1' with linespoints lc rgb 'blue', \
    "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list=4' with linespoints lc rgb 'green', \
	"< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list=8' with linespoints lc rgb 'red', \
	"< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list=16' with linespoints lc rgb 'orange'
     