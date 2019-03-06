# NAME: SHEN TENG
# EMAIL: REDHAIRDRAGON@UCLA.EDU
# ID: 104758168

build:
	gcc -g -pthread -o lab2_list lab2_list.c SortedList.c

clean:
	rm -f lab2b-104758168.tar.gz lab2_list lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png profile.out lab2b_list.csv

profile:
	LD_PRELOAD=~/shen/lib/libprofiler.so.0 CPUPROFILE=./dump.txt ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof  --text ./lab2_list ./dump.txt> profile.out
	pprof --list=spin_thread_task ./lab2_list ./dump.txt>> profile.out

dist:
	make 
	make profile
	make tests
	make graphs
	tar -zcf lab2b-104758168.tar.gz  SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png run.sh lab2_list.gp README

graphs:
	./lab2_list.gp

tests:
	 ./run.sh
	 
