# NAME: SHEN TENG
# EMAIL: REDHAIRDRAGON@UCLA.EDU
# ID: 104758168

all : lab1b-client lab1b-server
lab1b-server :
	@gcc lab1b-server.c -o lab1b-server -lm -lmcrypt
lab1b-client :
	@gcc lab1b-client.c -o lab1b-client -lm -lmcrypt
clean :
	@ rm -f lab1b-client lab1b-server lab1b-104758168.tar.gz
dist :
	@tar -zcf  lab1b-104758168.tar.gz my.key Makefile lab1b-server.c lab1b-client.c README
