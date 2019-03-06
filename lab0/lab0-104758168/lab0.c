#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

static struct option long_options[] =
	{{"input", required_argument, 0, 'i'},
	{"output", required_argument, 0, 'o' },
	{"segfault", no_argument, 0, 's'},
	{"catch", no_argument, 0, 'c'},
	{0,0,0,0}};

void sighandler(int signum)
{
   fprintf(stderr,"Caught signal %d\n",signum);
   exit(4);
}

int main(int argc, char const *argv[])
{
	int count=0;
	int option_index = 0;
	int find_opt;
	int in_fd,out_fd;
	char * null_ptr=NULL;


	
	while ((find_opt=getopt_long(argc,(char**)argv,"",long_options,&option_index))!=-1)
	{
		switch(find_opt)
		{
			case('i'):
				in_fd=open(optarg,O_RDONLY);
				if(in_fd==-1)
				{
					fprintf(stderr, "--input Fail to open file: %s\n %s\n",optarg,strerror(errno));
					exit(2);
				}	
				close(0);
				dup(in_fd);
				break;
			case('o'):
				out_fd=creat(optarg,00700);
				close(1);
				if(out_fd==-1)
				{
					fprintf(stderr, "--output Fail to create file: %s\n %s\n",optarg,strerror(errno));
					exit(3);
				}
				dup(out_fd);
				break;
			case('c'):
				signal(SIGSEGV,sighandler);
				break;
			case('s'):
				*null_ptr='x';
				break;
			default:
				printf("Unrecognized argument found.\n"
						"lab0 --input infile --output outfile --segfault --catch\n");

				exit(1);
		}
	}



	char buff[1];
	int re=read(0,buff,1);
	while(re>0)
	{
		write(1,buff,1);
		re=read(0,buff,1);
	}
	close(in_fd);
	close(out_fd);
	exit(0);
}