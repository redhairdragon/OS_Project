// NAME: SHEN TENG
// EMAIL: REDHAIRDRAGON@UCLA.EDU
// ID: 104758168
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mcrypt.h>

#define BUFFER_SIZE 256
#define READ_END 0
#define WRITE_END 1

static int status;
static int port_num=-1;
static int sockfd,newfd;

static pid_t cpid;
static int ParentToChild[2];
static int ChildToParent[2];
static int encrypt_opt;
static char* key;
static int keylen;
MCRYPT td,td2;

void set_opt(int argc, char** const  argv);
void pipe_handler(int signum);
void copyIOonce(int In,int Out,int pipeout);
void close_prog();
void get_key(char* filename);
void encrypt_setup();
void encrypt_close();


int main(int argc, char**  const argv)
{
	set_opt(argc,argv);
	sockfd=socket(AF_INET,SOCK_STREAM,0);

	if(sockfd<0)
	{
		perror("\r\nsocket failed");
		exit(1);
	}
	struct sockaddr_in serv_addr,cli_addr;
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_port=htons(port_num);
	if(bind(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr))==-1)
	{
		perror("\r\n bind failed");
		exit(1);
	}
	listen(sockfd,5);
	socklen_t clilen=sizeof(cli_addr);
	newfd=accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
	if(newfd<0)
	{
		perror("\r\n accept failed");
		exit(1);	
	}
	
	signal(SIGPIPE, pipe_handler);
	if (pipe(ParentToChild) <0) 
	{
    	perror("pipe error");
    	exit(1);
	}

	if (pipe(ChildToParent) <0) 
	{
    	perror("pipe error");
    	exit(1);
	}

	cpid=fork();
	if(cpid==-1)
	{
		perror("fork failed");
		exit(1);
	}
	if(cpid==0)//child
	{
		if(
		dup2(ParentToChild[READ_END],STDIN_FILENO)==-1||
		dup2(ChildToParent[WRITE_END],STDOUT_FILENO)==-1||
		dup2(ChildToParent[WRITE_END],STDERR_FILENO)==-1)
		{
			perror("dup2 failed");
			exit(1);
		}

		if(
			close(ChildToParent[WRITE_END])==-1||
			close(ParentToChild[WRITE_END])==-1||
			close(ChildToParent[READ_END])==-1||
			close(ParentToChild[READ_END])==-1)
		{
			perror("close1 failed");
			exit(1);
		}


		if(execlp("/bin/bash","bin/bash",NULL)==-1)
		{
			perror("execlp error");
			exit(1);
		}	
	}
	else
	{
		atexit(close_prog);
		if(
		dup2(newfd,STDIN_FILENO)==-1||
		dup2(newfd,STDOUT_FILENO)==-1)
		{
			perror("dup2 failed");
			exit(1);
		}
		struct pollfd pollfds[2];
		pollfds[0].fd=STDIN_FILENO;
		pollfds[1].fd=ChildToParent[READ_END];
		pollfds[0].events=POLLIN;
		pollfds[1].events=POLLIN;

		close(ParentToChild[READ_END]);
		close(ChildToParent[WRITE_END]);
		while(1)
		{
			int pe=poll(pollfds,2,0);
			if(pe==-1)
			{
				perror("poll error");
				exit(1);
			}

			for (int i =0;i<2;i++)
			{
				if((pollfds[i].revents&POLLERR)||(pollfds[i].revents&POLLHUP))
					exit(0);
				if(pollfds[i].revents&POLLIN&&i==1)
					copyIOonce(ChildToParent[READ_END],STDOUT_FILENO,0);
				if(pollfds[i].revents&POLLIN&&i==0)
					copyIOonce(STDIN_FILENO,STDOUT_FILENO,ParentToChild[WRITE_END]);
			}
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
	keylen = buf.st_size;
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

void copyIOonce(int In,int Out,int pipeout)	
{
	char buffer[BUFFER_SIZE];
	int nchars=0;

	if((nchars=read(In,buffer,BUFFER_SIZE))>0)
	{
		for(int i=0;i<nchars;i++)
		{
			int w_err;
			char curr_char=buffer[i];

			if(encrypt_opt&&In==STDIN_FILENO)
			{
				mdecrypt_generic(td2,&curr_char,1);
			}

			if(curr_char==0x04)
			{
				if(In==STDIN_FILENO)
				{
					if (close(ParentToChild[WRITE_END])==-1)
					{
						perror("close Pipe failed");
						exit(1);
					}
					break;
				}
				if(In==ChildToParent[READ_END])
				{
					close(ChildToParent[READ_END]);
					exit(0);
				}
			}

			if(curr_char==0x03)
			{
				if(cpid!=0)
					kill(cpid,SIGINT);
				exit(0);
			}

			if (curr_char =='\r'|| curr_char=='\n')
			{
				if(pipeout>0)
				{
					w_err=write(pipeout,"\n",1);
					if(w_err==-1)
					{
						perror("error on write2");
						exit(1);
					}
				}
				char temp[2]="\r\n";
				if(encrypt_opt&&Out==STDOUT_FILENO)
				{
					mcrypt_generic(td,temp,2);
				}
				w_err=write(Out,temp,2);
				if(w_err==-1)
				{
					perror("error on write1");
					exit(1);
				}

			}
			else
			{
				if(pipeout>0)
				{
					w_err=write(pipeout,&curr_char,1);
					if(w_err==-1)
					{
						perror("error on write4");
						exit(1);
					}
				}
				if(encrypt_opt&&Out==STDOUT_FILENO)
				{
					mcrypt_generic(td,&curr_char,1);
				}
				w_err=write(Out,&curr_char,1);
				if(w_err==-1)
				{
					perror("error on write3");
					exit(1);
				}
			}
		}
	}

	if(nchars==-1)
	{
		perror("error on read1");	
		exit(1);
	}
}

void close_prog()
{
	if(waitpid(cpid,&status,0)==-1)
		perror("waitpid");

	fprintf(stderr,"SHELL EXIT SIGNAL=%d ",WTERMSIG(status));
	fprintf(stderr, "STATUS=%d\n",WEXITSTATUS(status));
	// if(close(newfd)==-1||close(sockfd)==-1||close(1)==-1||close(0)==-1)
	// 	perror("error on shutdown");

	shutdown(newfd,SHUT_WR);
	shutdown(sockfd,SHUT_WR);
	
	if(encrypt_opt)
		encrypt_close();
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