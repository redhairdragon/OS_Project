# NAME: SHEN TENG
# EMAIL: REDHAIRDRAGON@UCLA.EDU
# ID: 104758168

all : lab1a
lab1a : lab1a.o
	@gcc lab1a.o -o lab1a
lab1a.o : lab1a.c
	@gcc -c lab1a.c -o lab1a.o

clean :
	@rm -f lab1a  lab1a.o lab1a-104758168.tar.gz
dist :
	@tar -zcf lab1a-104758168.tar.gz Makefile lab1a.c README
