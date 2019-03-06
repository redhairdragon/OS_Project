// NAME: SHEN TENG
// EMAIL: REDHAIRDRAGON@UCLA.EDU
// ID: 104758168
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>

#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>

#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <mcrypt.h>

#define BUFFER_SIZE 256

static struct termios curr_term,new_term;
static int port_num=-1;
static int log_fd=-1;
static int sockfd;
static int encrypt_opt;
static char* key;
static int keylen;
static int broken_socket;
MCRYPT td,td2;

void set_opt(int argc, char** const  argv);
void get_key(char* filename);
void get_attr();
void set_attr();
void res_attr();
void copyIOonce(int In,int Out);
void encrypt_setup();
void encrypt_close();
void pipe_handler(int signum);

int main(int argc, char**  const argv)
{

	set_opt(argc,argv);
	get_attr();
	set_attr();
	atexit(res_attr);
	signal(SIGPIPE, pipe_handler);
	
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd==-1)
	{
		perror("socket error");
		exit(1);
	}
	struct sockaddr_in serv_addr;
	struct hostent* serv_host;	
	serv_host=gethostbyname("localhost");
	if(serv_host==NULL)
	{
		perror("gethostbyname error");
		exit(1);
	}
	memcpy(&serv_addr.sin_addr,serv_host->h_addr_list[0],serv_host->h_length);
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_port=htons(port_num);
	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)))
	{
		perror("connect error");
		exit(1);
	}
	
	struct pollfd pollfds[2];
	pollfds[0].fd=STDIN_FILENO;
	pollfds[1].fd=sockfd;
	pollfds[0].events=POLLIN;
	pollfds[1].events=POLLIN;
	
	while(1)
	{
		int pe=poll(pollfds,2,0);
		if(pe==-1)
		{
			perror("\r\npoll error");
			exit(1);
		}

		for (int i =0;i<2;i++)
		{	
			if((pollfds[i].revents&POLLERR)||(pollfds[i].revents&POLLHUP))
				exit(0);	
			if(pollfds[i].revents&POLLIN&&i==1)
				copyIOonce(sockfd,STDOUT_FILENO);
			if(pollfds[i].revents&POLLIN&&i==0)
				copyIOonce(STDIN_FILENO,sockfd);
			if(broken_socket==1)
				exit(0);
		}
	}

	return 0;
}
void pipe_handler(int signum)
{
  	exit(0);
}

void set_opt(int argc, char** const  argv)
{
	static struct option long_options[] =
    {{"port", required_argument,NULL, 'p'},
     {"log",required_argument,NULL,'l'},
     {"encrypt",required_argument,NULL,'e'},
     {0, 0, 0, 0}};

    int option_index = 0;
    int opt;
    while((opt =getopt_long(argc,argv,"",long_options,&option_index))!= -1)
    {
    	switch(opt)
    	{
    		case 0:
    			break;
    		case 'p':
    			port_num=atoi(optarg);
    			break;
    		case 'l':
    			log_fd=creat(optarg,00666);
    			if(log_fd==-1)
    			{
    				perror("creat log file failed");
    				exit(1);
    			}
    			break;
    		case 'e':
    			encrypt_opt=1;
    			get_key(optarg);
    			encrypt_setup();
    			break;
    		case '?':
    			fprintf(stderr,"wrong args\n");
				exit(1);
    	}
    }
}
void get_key(char* filename)
{
	int encrypt_fd=open(filename,O_RDONLY);
	if(encrypt_fd==-1)
	{
		perror("open encryption file failed");
		exit(1);
	}
	struct stat buf;
	if(fstat(encrypt_fd, &buf))
	{
		perror("fstat failed");
		exit(1);
	}
	keylen= buf.st_size;
	key= (char*) malloc(keylen*sizeof(char));	
	if(key==NULL)
	{
		perror("malloc failed");
		exit(1);
	}
	if(read(encrypt_fd,key,keylen)==-1)
	{
		perror("malloc failed");
		exit(1);
	}
	if(close(encrypt_fd)==-1)
	{
		perror("close encryption file failed");
		exit(1);
	}
}

void set_attr()
{
	new_term=curr_term;
	new_term.c_iflag = ISTRIP;	 
	new_term.c_oflag = 0;		
	new_term.c_lflag = 0;
	if(tcsetattr(0,TCSANOW,&new_term)==-1)
	{
		perror("setattr failed");
		exit(1);
	}
}
void res_attr()
{
	if(log_fd!=-1)
	{
		if(close(log_fd)==-1)
		{
			perror("\r\n close failed");
			exit(1);
		}
	}

	if(tcsetattr(0,TCSANOW,&curr_term)==-1)
	{
		perror("\r\nresattr failed");
		exit(1);
	}
	if(encrypt_opt)
		encrypt_close();
}
void get_attr()
{
	if(tcgetattr(0,&curr_term)==-1)
	{
		perror("getattr failed");
		exit(1);
	}
}
void copyIOonce(int In,int Out)	
{
	char buffer[BUFFER_SIZE];
	int nchars=0;

	if(log_fd!=-1)
	{
		if(Out==sockfd)
		{
			if(write(log_fd,"SENT ",6)==-1)
			{
				perror("\r\nwrite failed1");
				exit(1);
			}
		}
		if(In==sockfd)
		{
			if(write(log_fd,"RECEIVED ",10)==-1)
			{
				perror("\r\nwrite failed2");
				exit(1);
			}
		}
	}

	if((nchars=read(In,buffer,BUFFER_SIZE))>=0)
	{
		if(nchars==0)
			broken_socket=1;
		
		if(log_fd!=-1)
		{
			char temp[64];
			sprintf(temp, "%d", nchars);
			int size = (nchars == 0 ? 1 : (int)(log10(nchars)+1));;
			if(write(log_fd,temp,size)==-1)
			{
				perror("write failed3");
				exit(1);
			}
			if(write(log_fd," bytes: ",9)==-1)
			{
				perror("\r\nwrite failed6");
				exit(1);
			}
		}

		for(int i=0;i<nchars;i++)
		{
			int w_err;
			char curr_char=buffer[i];
			char log_char=curr_char;

			if(encrypt_opt&&In==STDIN_FILENO)
			{
				mcrypt_generic(td2,&curr_char,1);
			}
			if(encrypt_opt&&In==sockfd)
			{
				mdecrypt_generic(td,&curr_char,1);
			}

			w_err=write(Out,&curr_char,1);
			if(w_err==-1)
			{
				perror("\r\nerror on write4");
				exit(1);
			}
			if(log_fd!=-1)
			{
				if(encrypt_opt==0)
					w_err=write(log_fd,&curr_char,1);
				else
				{
					if(In==STDIN_FILENO)
						w_err=write(log_fd,&curr_char,1);
					if(In==sockfd)
						w_err=write(log_fd,&log_char,1);
				}
				if(w_err==-1)
				{
					perror("\r\nerror on write5");
					exit(1);
				}
			}
		}
	}
	if(log_fd!=-1)
	{	
		if(write(log_fd,"\n",1)==-1)
		{
			perror("\r\nerror on write5");
			exit(1);
		}
	}

	if(nchars==-1)
	{
		perror("\r\nerror on read1");	
		exit(1);
	}
}
void encrypt_setup()
{
	td =mcrypt_module_open("twofish",NULL,"cfb",NULL);
	char* IV=malloc(mcrypt_enc_get_iv_size(td));
	memset(IV,0,sizeof(char)* mcrypt_enc_get_iv_size(td));
	mcrypt_generic_init(td,(void*)key,keylen,(void *)IV);

	td2 =mcrypt_module_open("twofish",NULL,"cfb",NULL);
	char* IV2=malloc(mcrypt_enc_get_iv_size(td2));
	memset(IV2,0,sizeof(char)* mcrypt_enc_get_iv_size(td2));
	mcrypt_generic_init(td2,(void*)key,keylen,(void *)IV2);

}

void encrypt_close()
{
	mcrypt_generic_deinit(td);
	mcrypt_module_close(td);	
	mcrypt_generic_deinit(td2);
	mcrypt_module_close(td2);			
}

