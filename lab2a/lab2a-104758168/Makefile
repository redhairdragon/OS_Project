# NAME: SHEN TENG
# EMAIL: REDHAIRDRAGON@UCLA.EDU
# ID: 104758168

build:
	@gcc -pthread -o lab2_list lab2_list.c SortedList.c
	@gcc lab2_add.c -o lab2_add -pthread

clean:
	@ rm -f lab2_add lab2_list lab2_add.csv lab2_list.csv lab2a-104758168.tar.gz
	@ rm -f lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png
	@ rm -f lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png

dist:
	@ make 
	@ make tests
	@ make graphs
	@ tar -zcf lab2a-104758168.tar.gz lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png lab2_list.c SortedList.c SortedList.h lab2_add.c README Makefile lab2_add.csv lab2_list.csv test.sh lab2_add.gp lab2_list.gp

graphs:
	./lab2_add.gp
	./lab2_list.gp

tests:
	 ./test.sh
	 
