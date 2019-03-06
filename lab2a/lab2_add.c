#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h> 
#include <pthread.h>

#define NANO_PER_SEC 1000000000
long long counter=0;
int thread_num=1;
int iterations=1;
int opt_yield=0;
int sync_m=0;
int sync_s=0;
int sync_c=0;
long long diff;

volatile int spin_lock;
volatile int CAS_lock;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

void get_opts(int argc, char* const argv[]){
	struct option long_options[] =
    {{"threads",optional_argument,NULL, 't'}, 
     {"iterations",optional_argument,NULL, 'i'},
     {"yield",no_argument,NULL, 'y'},
     {"sync",required_argument,NULL, 's'},  
     {0, 0, 0, 0}};
    int option_index = 0;
    int opt;
    while((opt =getopt_long(argc,argv,"",long_options,&option_index))!= -1)
    {
    	switch(opt)
    	{
    		case 't':
    			if(optarg)
    				thread_num=atoi(optarg);
    			else
    				thread_num=1;
    			break;
    		case 'i':
    			if(optarg)
    				iterations=atoi(optarg);
    			else
    				iterations=1;
    			break;
    		case 'y':
    			opt_yield=1;
    			break;
    		case 's':
    			if(strcmp(optarg,"m")==0)
    				sync_m=1;
    			else if(strcmp(optarg,"c")==0)
    				sync_c=1;
    			else if(strcmp(optarg,"s")==0)
    				sync_s=1;
    			else
    			{
   					fprintf(stderr,"unknown sync type\n");
   					exit(1);
    			}
    			break;
    		case '?':
    			fprintf(stderr,"wrong args\n");
				exit(1);
			default:
				fprintf(stderr,"should not print\n");
				exit(1);
    	}
    }
}
void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield)
        sched_yield();
	*pointer = sum;
}
void* thread_counter(){
	for(int i=0;i<iterations;i++)
		add(&counter,1);
	for(int i=0;i<iterations;i++)
		add(&counter,-1);
	return NULL;
}
void* mutex_counter(){
	for(int i=0;i<iterations;i++)
	{
		pthread_mutex_lock(&mutex);
		add(&counter,1);
		pthread_mutex_unlock(&mutex);
	}
	for(int i=0;i<iterations;i++)
	{
		pthread_mutex_lock(&mutex);
		add(&counter,-1);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}	

void* spin_counter(){
  	for(int i=0;i<iterations;i++)
  	{
  		while (__sync_lock_test_and_set(&spin_lock, 1));
		add(&counter,1);
		__sync_lock_release(&spin_lock);
  	}
  	
  	for(int i=0;i<iterations;i++)
  	{
  		while (__sync_lock_test_and_set(&spin_lock, 1));
		add(&counter,-1);
		__sync_lock_release(&spin_lock);
  	}
	return NULL;
}

void* CAS_counter(){
	long long old;

	for(int i=0;i<iterations;i++)
  	{
  		while(__sync_val_compare_and_swap (&CAS_lock,0,1));
		add(&counter,1);
		CAS_lock=0;
  	}
	for(int i=0;i<iterations;i++)
  	{
  		while(__sync_val_compare_and_swap (&CAS_lock,0,1));
		add(&counter,-1);
		CAS_lock=0;
  	}
	return NULL;
}

void output(void){
	long long operation_num=iterations*2*thread_num;
	long long avg_time=diff/operation_num;

	printf("add");
	if(opt_yield)
		printf("-yield");
	if(sync_m)
		printf("-m,");
	else if(sync_c)
		printf("-c,");
	else if(sync_s)
		printf("-s,");
	else
		printf("-none,");
	printf("%d,", thread_num);
	printf("%d,", iterations);
	printf("%lld,", operation_num);
	printf("%lld,", diff);
	printf("%lld,", avg_time);
	printf("%lld\n", counter);
}

int main(int argc, char* const argv[])
{
	get_opts(argc,argv);
	pthread_t* threads;
	
	threads=malloc(sizeof(pthread_t)*thread_num);
	if(!threads){perror("malloc failed");exit(1);}

	struct timespec before,after;
	
	if(clock_gettime(CLOCK_REALTIME,&before)==-1){perror("Gettime failed");exit(1);}
	
	for(int i=0;i<thread_num;i++){
		if(sync_m){
			if(pthread_create(threads+i,NULL,mutex_counter,NULL))
			{perror("pthread create failed");exit(1);}
		}
		else if(sync_s){
			if(pthread_create(threads+i,NULL,spin_counter,NULL))
			{perror("pthread create failed");exit(1);}
		}
		else if(sync_c){
			if(pthread_create(threads+i,NULL,CAS_counter,NULL))
			{perror("pthread create failed");exit(1);}
		}
		else{
			if(pthread_create(threads+i,NULL,thread_counter,NULL))
			{perror("pthread create failed");exit(1);}
		}
	}
	for(int i=0;i<thread_num;i++){
		if(pthread_join(threads[i],NULL))
			{perror("join failed");exit(1);}
	}
	if(clock_gettime(CLOCK_REALTIME,&after)==-1){perror("Gettime failed");exit(1);}
	diff=after.tv_sec*NANO_PER_SEC+after.tv_nsec
			-before.tv_sec*NANO_PER_SEC-before.tv_nsec;
	output();

	return 0;
}
