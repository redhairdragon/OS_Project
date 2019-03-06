 // NAME: SHEN TENG
 // EMAIL: REDHAIRDRAGON@UCLA.EDU
 // ID: 104758168
#include "mraa.h"
#include <stdio.h>
#include <time.h>
#include <getopt.h> 
#include <string.h>
#include <stdlib.h> 
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

int period=1;
int log_op;
FILE* log_fp;
char scale='F';
int stop;
int to_record=1;

int count;
char cmd[1000];

//parse options
void get_opts(int argc, char* const argv[]){
	struct option long_options[] =
    {{"period",required_argument,NULL, 'p'},  
    {"scale",required_argument,NULL, 's'},
    {"log",required_argument,NULL, 'l'},
     {0, 0, 0, 0}};
    int option_index = 0;
    int opt;
     while((opt =getopt_long(argc,argv,"",long_options,&option_index))!= -1){
    	switch(opt){
    		case 'p':
				period=atoi(optarg);
				if(period<0){
					fprintf(stderr, "invalid period\n");
					exit(1);
				}
				break;
			case 's':
				scale=optarg[0];
				if(scale!='C'&&scale!='F'){
					fprintf(stderr, "invalid scale\n");
					exit(1);
				}
				break;
			case 'l':
				log_op=1;
				log_fp=fopen(optarg,"a");
				if(log_fp==NULL){perror("log file open failed");exit(1);}
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
//get current temperature
float get_temp(){
	mraa_aio_context tmpr=mraa_aio_init(0);
	uint16_t a=mraa_aio_read(tmpr);
	mraa_aio_close(tmpr);
	int B = 4275;               // B value of the thermistor
	int R0 = 100000;            // R0 = 100k
	float R = 1023.0/a-1.0;
    R = R0*R;
    float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
	if(scale=='C')
		return temperature;
	else
		return temperature*1.8+32;
}
//output temperature to stdout or logfile if it's enabled
void record_temp(){
	if(to_record){
		time_t curr_time_t=time(NULL);
		struct tm* curr_time=localtime(&curr_time_t);
		float curr_temp=get_temp();
		if(curr_time==NULL){perror("get time failed");exit(1);}
		fprintf(stdout,"%02d:%02d:%02d %.1f\n", curr_time->tm_hour,curr_time->tm_min,curr_time->tm_sec,curr_temp);
		if(log_op)
			fprintf(log_fp,"%02d:%02d:%02d %.1f\n", curr_time->tm_hour,curr_time->tm_min,curr_time->tm_sec,curr_temp);
		to_record=0;
	}
}
//handle the button pressing
void handle_btn(){
	mraa_gpio_context button;
	button =mraa_gpio_init(3);
	mraa_gpio_dir(button,MRAA_GPIO_IN);
	int read=mraa_gpio_read(button); //!0 button pushed 
	mraa_gpio_close(button);
	if(read!=0){
		time_t curr_time_t=time(NULL);
		struct tm* curr_time=localtime(&curr_time_t);
		fprintf(stdout,"%d:%d:%d SHUTDOWN\n", curr_time->tm_hour,curr_time->tm_min,curr_time->tm_sec);
		if(log_op)
			fprintf(log_fp,"%d:%d:%d SHUTDOWN\n", curr_time->tm_hour,curr_time->tm_min,curr_time->tm_sec);
		exit(0);
	}
}
//action when receive alarm sig
void alarm_handle(){
	alarm(period);
	if(stop==0)
		to_record=1;
}
void execute_cmd(){

	cmd[count]='\0';
	printf("%s\n",cmd);
	if(log_op)
		fprintf(log_fp, "%s\n",cmd);

	if(strcmp(cmd,"SCALE=F")==0)
		scale='F';
	else if(strcmp(cmd,"SCALE=C")==0)
		scale='C';
	else if(strcmp(cmd,"STOP")==0)
		stop=1;
	else if(strcmp(cmd,"START")==0)
		stop=0;
	else if(strcmp(cmd,"OFF")==0){
		time_t curr_time_t=time(NULL);
		struct tm* curr_time=localtime(&curr_time_t);
		if(log_op)
			fprintf(log_fp,"%02d:%02d:%02d SHUTDOWN\n", curr_time->tm_hour,curr_time->tm_min,curr_time->tm_sec);
		else
			fprintf(stdout,"%02d:%02d:%02d SHUTDOWN\n", curr_time->tm_hour,curr_time->tm_min,curr_time->tm_sec);
		exit(0);
	}
	
	cmd[6]='\0';
	if(strcmp(cmd,"PERIOD")==0){
		period=atoi(&cmd[7]);
	}
	count=0;
}
//read command
void handle_cmd(){
	char temp;
	read(0,&temp,1);
	if(temp!=EOF&&temp!='\n'){
		cmd[count]=(char)temp;
		if(count<998)
			count++;
	}
	else
		execute_cmd();
}


int main(int argc, char* const argv[]){
	get_opts(argc,argv);
	signal(SIGALRM,alarm_handle);
	alarm(period);

	struct pollfd pollfds[2];
	pollfds[0].fd=STDIN_FILENO;
	pollfds[0].events=POLLIN;
	pollfds[1].fd=-1;

	while(1){
		int i;
		for (i =0;i<2;i++){
			int pe=poll(pollfds,2,0);
			if(pollfds[i].revents&POLLIN&&i==0)
				handle_cmd();
			else{
				handle_btn();
				record_temp();
			}
		}
	}
	return 0;
}