// NAME: SHEN TENG
// EMAIL: REDHAIRDRAGON@UCLA.EDU
// ID: 104758168


#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h> 
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256
#define READ_END 0
#define WRITE_END 1

static int eof_flag;
static int status;
static int shell_opt;
static int poll_error_flag;
static struct termios curr_term,new_term;
static pid_t cpid;
static int ParentToChild[2];
static int ChildToParent[2];


void pipe_handler(int signum);
void get_attr();
void set_attr();
void res_attr();
void set_opt(int argc, char** const  argv);
void copyIOonce(int In,int Out,int pipeout);

int main(int argc, char**  const argv)
{
	
	set_opt(argc,argv);
	get_attr();
	set_attr();
	
	if(shell_opt)
	{	
		signal(SIGPIPE, pipe_handler);
		if (pipe(ParentToChild) <0) 
		{
        	perror("pipe error");
        	res_attr();
        	exit(1);
    	}

		if (pipe(ChildToParent) <0) 
		{
        	perror("pipe error");
        	res_attr();
        	exit(1);
    	}

    	cpid=fork();
		if(cpid==-1)
		{
			perror("fork error");
			res_attr();
        	exit(1);
		}
	

		if(!cpid)//child process
		{	
			
			if(
			dup2(ParentToChild[READ_END],STDIN_FILENO)==-1||
			dup2(ChildToParent[WRITE_END],STDOUT_FILENO)==-1||
			dup2(ChildToParent[WRITE_END],STDERR_FILENO)==-1)
			{
				res_attr();
				perror("dup2");
				exit(1);
			}
			if(
			close(ParentToChild[WRITE_END])==-1||
			close(ChildToParent[READ_END])==-1||
			close(ParentToChild[READ_END])==-1||
			close(ChildToParent[WRITE_END])==-1)
			{
				res_attr();
				perror("close");
				exit(1);
			}

			if(execlp("/bin/bash","bin/bash",NULL)==-1)
			{
				res_attr();
				perror("execlp error");
				exit(1);
			}			
		}
		else //parent
		{	
			struct pollfd pollfds[2];
			pollfds[0].fd=STDIN_FILENO;
			pollfds[1].fd=-1;
			pollfds[0].events=POLLIN;

			close(ParentToChild[READ_END]);
			close(ChildToParent[WRITE_END]);

			while(1)
			{
				int pe=poll(pollfds,2,0);
				if(pe==-1)
				{
					res_attr();
					perror("poll error");
					exit(1);
				}

				for (int i =0;i<2;i++)
				{
					if((pollfds[i].revents&POLLERR)||(pollfds[i].revents&POLLHUP))
					{
						if(waitpid(cpid,&status,0)==-1)
						{
							perror("waitpid error");
							exit(1);
						}
						res_attr();
						fprintf(stderr,"SHELL EXIT SIGNAL=%d ",WTERMSIG(status));
						fprintf(stderr, "STATUS=%d\r\n", WEXITSTATUS(status));
						exit(0);	
					}
					if(pollfds[i].revents&POLLIN&&i==1)
						copyIOonce(ChildToParent[READ_END],STDOUT_FILENO,0);
					if(pollfds[i].revents&POLLIN&&i==0)
						copyIOonce(STDIN_FILENO,STDOUT_FILENO,ParentToChild[WRITE_END]);

				}
			}
		}
		
	}
	else	
	{
		while(1)
			copyIOonce(0,1,0);
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
	if(tcsetattr(0,TCSANOW,&curr_term)==-1)
	{
		perror("resattr failed");
		exit(1);
	}
}
void get_attr()
{
	if(tcgetattr(0,&curr_term)==-1)
	{
		perror("getattr failed");
		exit(1);
	}
}
void set_opt(int argc, char** const  argv)
{
	shell_opt=0;
	static struct option long_options[] =
    {{"shell", no_argument,&shell_opt, 1}, 
     {0, 0, 0, 0}};
    int option_index = 0;
    int opt;
    while((opt =getopt_long(argc,argv,"",long_options,&option_index))!= -1)
    {
    	switch(opt)
    	{
    		case 0:
    			break;
    		case '?':
    			fprintf(stderr,"wrong args\n");
				exit(1);
    	}
    }
}

void pipe_handler(int signum)
{
  	res_attr();
  	fprintf(stderr,"SHELL EXIT SIGNAL=%d ",WTERMSIG(status));
	fprintf(stderr, "STATUS=%d\r\n", WEXITSTATUS(status));
  	exit(0);
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
			// printf("%c\r\n",curr_char );
			if(curr_char==0x04)
			{

				if(shell_opt==0)
				{
					res_attr();
					exit(0);
				}
				else
				{
					if(In==STDIN_FILENO)
					{
						eof_flag=1;
						if (close(ParentToChild[WRITE_END])==-1)
						{
							res_attr();
							perror("close Pipe failed");
							exit(1);
						}
					}
					if(In==ChildToParent[READ_END])
					{
						
						close(ChildToParent[READ_END]);
						res_attr();
						fprintf(stderr,"SHELL EXIT SIGNAL=%d ",WTERMSIG(status));
						fprintf(stderr, "STATUS=%d\n",WEXITSTATUS(status));
						exit(0);
					}

				}
			}
			if(curr_char==0x03)
			{
				if(cpid!=0)
					kill(cpid,SIGINT);
				res_attr();
				if(shell_opt)
				{
					fprintf(stderr,"SHELL EXIT SIGNAL=%d ",WTERMSIG(status));
					fprintf(stderr, "STATUS=%d\n",WEXITSTATUS(status));
				}
				exit(0);
			}
			
			if (curr_char =='\r'|| curr_char=='\n')
			{
				w_err=write(Out,"\r\n",2);
				if(w_err==-1)
				{
					res_attr();
					perror("error on write1");
					exit(1);
				}
				if(pipeout>0&&!eof_flag)
				{
					w_err=write(pipeout,"\n",1);
					if(w_err==-1)
					{
						res_attr();
						perror("error on write2");
						exit(1);
					}
				}					
			}
			else
			{
				w_err=write(Out,&curr_char,1);
				if(w_err==-1)
				{
					res_attr();
					perror("error on write3");
					exit(1);
				}
				if(pipeout>0&&!eof_flag)
				{
					w_err=write(pipeout,&curr_char,1);
					if(w_err==-1)
					{
						res_attr();
						perror("error on write4");
						exit(1);
					}
				}
			}
		}
	}

	if(nchars==-1)
	{
		res_attr();
		perror("error on read1");	
		exit(1);
	}
}


