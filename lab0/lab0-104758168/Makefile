all : lab0
lab0 : lab0.o
	@gcc lab0.o -o lab0
lab0.o : lab0.c
	@gcc -g -c lab0.c -o lab0.o

clean :
	@rm -f lab0  lab0.o lab0-104758168.tar.gz
check :
	@mkdir temp
	@echo "abc"|./lab0>temp/out
	@echo "abc"|./lab0>temp/in
	@diff temp/out temp/in
	@./lab0 --input=temp/in --output=temp/out
	@diff temp/out temp/in
	@rm -r temp
	@./lab0 --catch --segfault &> /dev/null ||[ $$? -eq 4 ]
	@./lab0 --input=nonexist &>/dev/null ||[ $$? -eq 2 ]
	@./lab0 --unrecog &>/dev/null ||[ $$? -eq 1 ]


dist :
	@tar -zcf lab0-104758168.tar.gz backtrace.png breakpoint.png Makefile lab0.c README
