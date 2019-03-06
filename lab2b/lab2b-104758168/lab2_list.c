// NAME: SHEN TENG
// EMAIL: REDHAIRDRAGON@UCLA.EDU
// ID: 104758168

#include "SortedList.h"
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define NANO_PER_SEC 1000000000

long long lock_time=0;
long long* lock_time_per_thread;
long long diff=0;
int thread_num=1;
int iterations=1;
int opt_yield=0;
int sync_m=0;
int sync_s=0;
int list_num=1;

SortedList_t* lists;
SortedListElement_t* elems;

int* spin_locks;
pthread_mutex_t* mutexes;

void sighandler(int signum){
	fprintf(stderr, "segfault caught\n");
	exit(2);
}
void get_opts(int argc, char* const argv[]){
	struct option long_options[] =
    {{"threads",optional_argument,NULL, 't'}, 
     {"iterations",optional_argument,NULL, 'i'},
     {"yield",required_argument,NULL, 'y'},
     {"sync",required_argument,NULL, 's'},  
     {"lists",optional_argument,NULL, 'l'},   
     {0, 0, 0, 0}};
    int option_index = 0;
    int opt;
    while((opt =getopt_long(argc,argv,"",long_options,&option_index))!= -1)
    {	
    	int i=0;
    	switch(opt)
    	{
    		case 's':
    			if(strcmp(optarg,"m")==0)
    				sync_m=1;
    			else if(strcmp(optarg,"s")==0)
    				sync_s=1;
    			else{
    				fprintf(stderr, "unknown sync type\n");
   					exit(1);
    			}
    			break;
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
    		case 'l':
    			if(optarg)
    				list_num=atoi(optarg);
    			else
    				list_num=1;
    			break;
    		case 'y':
    			while(optarg[i]!='\0'){
    				if(optarg[i]=='i')
    					opt_yield|= INSERT_YIELD;
    				else if(optarg[i]=='d')
    					opt_yield|= DELETE_YIELD;
    				else if(optarg[i]=='l')
    					opt_yield|= LOOKUP_YIELD;
    				else{
    					fprintf(stderr, "unknown yield arg\n" );
    					exit(1);
    				}
    				i++;
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
char* random_key(){
	int size=rand()%10+1;
	char* key=calloc(size+1,sizeof(char));
	if(key==NULL){perror("malloc failed");exit(1);}
	for(int i=0;i<size;i++)
		key[i]=rand()%26+'A';		
	return key;
}
void print(SortedListElement_t* list){
	SortedListElement_t* curr=list->next;
	while(curr!=list)
	{
		printf("%s\n", curr->key);
		curr=curr->next;
	}
	printf("\n");
}
void* thread_task(void* arg){
	int id=*(int*)arg;
	
	for(int i = 0;i<iterations;i++){
		int hash=strlen(elems[i*thread_num+id].key)%list_num;
		SortedList_insert(&(lists[hash]),elems+i*thread_num+id);
	}

	int len=0;
	for(int i=0;i<list_num;i++){
		len+=SortedList_length(&(lists[i]));
	}
	
	for(int i = 0;i<iterations;i++){
		int hash=strlen(elems[i*thread_num+id].key)%list_num;
		SortedListElement_t* del_elem=	
			SortedList_lookup(&(lists[hash]),(elems+i*thread_num+id)->key);
		if(del_elem==NULL){
			fprintf(stderr, "can't find elems to delete\n");
			exit(2);
		}
		if(SortedList_delete(del_elem)==1){
			fprintf(stderr, "corrupted list\n");
			exit(2);
		}
	}
	return NULL;
}
void* mutex_thread_task(void* arg){
	struct timespec before,after;
	int id=*(int*)arg;

	for(int i = 0;i<iterations;i++){
		int hash=strlen(elems[i*thread_num+id].key)%list_num;
		if(clock_gettime(CLOCK_REALTIME,&before)==-1){perror("Gettime failed");exit(1);}
		pthread_mutex_lock(&(mutexes[hash]));
		if(clock_gettime(CLOCK_REALTIME,&after)==-1){perror("Gettime failed");exit(1);}
		lock_time_per_thread[id]+=after.tv_sec*NANO_PER_SEC+after.tv_nsec
			-before.tv_sec*NANO_PER_SEC-before.tv_nsec;
		
		SortedList_insert(&(lists[hash]),elems+i*thread_num+id);
		pthread_mutex_unlock(&(mutexes[hash]));
	}

	int len=0;
	for(int i=0;i<list_num;i++){
		if(clock_gettime(CLOCK_REALTIME,&before)==-1){perror("Gettime failed");exit(1);}
		pthread_mutex_lock(&(mutexes[i]));
		if(clock_gettime(CLOCK_REALTIME,&after)==-1){perror("Gettime failed");exit(1);}
		lock_time_per_thread[id]+=after.tv_sec*NANO_PER_SEC+after.tv_nsec
			-before.tv_sec*NANO_PER_SEC-before.tv_nsec;

		len+=SortedList_length(&(lists[i]));
		pthread_mutex_unlock(&(mutexes[i]));
	}



	for(int i = 0;i<iterations;i++){
		int hash=strlen(elems[i*thread_num+id].key)%list_num;

		if(clock_gettime(CLOCK_REALTIME,&before)==-1){perror("Gettime failed");exit(1);}
		pthread_mutex_lock(&(mutexes[hash]));
		if(clock_gettime(CLOCK_REALTIME,&after)==-1){perror("Gettime failed");exit(1);}
		lock_time_per_thread[id]+=after.tv_sec*NANO_PER_SEC+after.tv_nsec
			-before.tv_sec*NANO_PER_SEC-before.tv_nsec;

		SortedListElement_t* del_elem=	
			SortedList_lookup(&(lists[hash]),(elems+i*thread_num+id)->key);
		if(del_elem==NULL){
			fprintf(stderr, "can't find elems to delete\n");
			exit(2);
		}
		if(SortedList_delete(del_elem)==1){
			fprintf(stderr, "corrupted list\n");
			exit(2);
		}
		pthread_mutex_unlock(&(mutexes[hash]));
	}
	return NULL;
}
void* spin_thread_task(void* arg){
	int id=*(int*)arg;
	struct timespec before,after;

	for(int i = 0;i<iterations;i++){
		int hash=strlen(elems[i*thread_num+id].key)%list_num;

		if(clock_gettime(CLOCK_REALTIME,&before)==-1){perror("Gettime failed");exit(1);}
		while (__sync_lock_test_and_set(&(spin_locks[hash]), 1));
		if(clock_gettime(CLOCK_REALTIME,&after)==-1){perror("Gettime failed");exit(1);}
		lock_time_per_thread[id]+=after.tv_sec*NANO_PER_SEC+after.tv_nsec
			-before.tv_sec*NANO_PER_SEC-before.tv_nsec;

		SortedList_insert(&(lists[hash]),elems+i*thread_num+id);
		__sync_lock_release(&(spin_locks[hash]));
	}

	int len=0;
	for(int i=0;i<list_num;i++){
		if(clock_gettime(CLOCK_REALTIME,&before)==-1){perror("Gettime failed");exit(1);}
		while (__sync_lock_test_and_set(&(spin_locks[i]), 1));
		if(clock_gettime(CLOCK_REALTIME,&after)==-1){perror("Gettime failed");exit(1);}
		lock_time_per_thread[id]+=after.tv_sec*NANO_PER_SEC+after.tv_nsec
			-before.tv_sec*NANO_PER_SEC-before.tv_nsec;

		len+=SortedList_length(&(lists[i]));
		__sync_lock_release(&(spin_locks[i]));
	}

	for(int i = 0;i<iterations;i++){
		int hash=strlen(elems[i*thread_num+id].key)%list_num;

		if(clock_gettime(CLOCK_REALTIME,&before)==-1){perror("Gettime failed");exit(1);}
		while (__sync_lock_test_and_set(&(spin_locks[hash]), 1));
		if(clock_gettime(CLOCK_REALTIME,&after)==-1){perror("Gettime failed");exit(1);}
		lock_time_per_thread[id]+=after.tv_sec*NANO_PER_SEC+after.tv_nsec
			-before.tv_sec*NANO_PER_SEC-before.tv_nsec;

		SortedListElement_t* del_elem=	
			SortedList_lookup(&(lists[hash]),(elems+i*thread_num+id)->key);
		if(del_elem==NULL){
			fprintf(stderr, "can't find elems to delete\n");
			exit(2);
		}
		if(SortedList_delete(del_elem)==1){
			fprintf(stderr, "corrupted list\n");
			exit(2);
		};
		__sync_lock_release(&(spin_locks[hash]));
	}
	return NULL;
}
void output(){

	int op_num=iterations*thread_num*3;
	long ave=diff/op_num;
	printf("list-");
	if(opt_yield==0)
		printf("none");
	if(opt_yield&INSERT_YIELD)
		printf("i");
	if(opt_yield&DELETE_YIELD)
		printf("d");
	if(opt_yield&LOOKUP_YIELD)
		printf("l");
	printf("-");
	if(sync_s)
		printf("s");
	else if(sync_m)
		printf("m");
	else
		printf("none");

	printf(",%d",thread_num);
	printf(",%d",iterations);
	printf(",%d",list_num);
	printf(",%d",op_num);
	printf(",%lld",diff);
	printf(",%ld",ave);
	printf(",%lld\n",lock_time/(iterations*thread_num*2));
}

int main(int argc, char* const argv[])
{
	srand(time(NULL));
	get_opts(argc,argv);
	signal(SIGSEGV, sighandler);

	spin_locks=malloc(list_num*sizeof(int));
	lists=malloc(list_num*sizeof(SortedList_t));
	mutexes=malloc(list_num*sizeof(pthread_mutex_t));
	if(spin_locks==NULL||lists==NULL||mutexes==NULL)
		{perror("malloc failed");exit(1);}

	for(int i=0;i<list_num;i++){
		lists[i].next=&(lists[i]);
		lists[i].prev=&(lists[i]);
		lists[i].key=NULL;
		pthread_mutex_init(&(mutexes[i]), NULL);
		spin_locks[i]=0;
	}
	
	lock_time_per_thread=calloc(thread_num,sizeof(long long));
	if(lock_time_per_thread==NULL){perror("calloc failed");exit(1);}

	elems=calloc(iterations*thread_num,sizeof(SortedListElement_t));
	if(elems==NULL){perror("calloc failed");exit(1);}
	for(int i=0;i<thread_num*iterations;i++){
		elems[i].key=random_key();
	}
	

	struct timespec before,after;
	pthread_t* threads;
	threads=malloc(sizeof(pthread_t)*thread_num);
	if(!threads){perror("malloc failed");exit(1);}

	int* threads_id=malloc(sizeof(int)*thread_num);
	if(threads_id==NULL){perror("malloc failed");exit(1);}

	if(clock_gettime(CLOCK_REALTIME,&before)==-1){perror("Gettime failed");exit(1);}
	for(int i=0;i<thread_num;i++){
		threads_id[i]=i;
		if(sync_m){
			if(pthread_create(threads+i,NULL,mutex_thread_task,(void*)(threads_id+i)))
			{perror("pthread create failed");exit(1);}

		}
		else if(sync_s){
			if(pthread_create(threads+i,NULL,spin_thread_task,(void*)(threads_id+i)))
			{perror("pthread create failed");exit(1);}
		}
		else{
			if(pthread_create(threads+i,NULL,thread_task,(void*)(threads_id+i)))
			{perror("pthread create failed");exit(1);}
		}
	}
	for(int i=0;i<thread_num;i++){
		if(pthread_join(threads[i],NULL))
			{perror("join failed");exit(1);}
		lock_time+=lock_time_per_thread[i];
	}

	if(clock_gettime(CLOCK_REALTIME,&after)==-1){perror("Gettime failed");exit(1);}
	diff=after.tv_sec*NANO_PER_SEC+after.tv_nsec
			-before.tv_sec*NANO_PER_SEC-before.tv_nsec;

	for(int i=0;i<list_num;i++){
		if(SortedList_length(&(lists[i]))!=0){
		fprintf(stderr, "none empty list\n");
		exit(2);
		}
	}
	output();
	return 0;
}