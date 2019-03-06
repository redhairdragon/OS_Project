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
#include <ctype.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

int period=1;
int log_op;
FILE* log_fp;
char scale='F';
int stop=0;
int to_record=1;
int port_num=-1;
int id=-1;
char host_name[256];
static int sockfd;

char cmd[256];
char buff[256];
SSL * SSLClient;
SSL_CTX *newContext;

//parse options
void get_opts(int argc, char* const argv[]){
	//get port number
	int port_count=0;
	int i=1;
	for(i=1;i<argc;i++){
		int all_digit=1;
		int j=0;
		for(j=0;j<strlen(argv[i]);j++){
			if(!isdigit(argv[i][j])){
				all_digit=0;
				break;
			}
		}
		if(all_digit){
			port_num=atoi(argv[i]);
			port_count++;
		}
	}
	if(port_count!=1||port_num<0){fprintf(stderr, "Invalid Port Number\n");exit(1);}

	//Parse opts
	struct option long_options[] =
    {{"period",required_argument,NULL, 'p'},  
    {"scale",required_argument,NULL, 's'},
    {"log",required_argument,NULL, 'l'},
    {"host",required_argument,NULL, 'h'},
    {"id",required_argument,NULL, 'i'},
     {0, 0, 0, 0}};
    int option_index = 0;
    int opt;
     while((opt =getopt_long(argc,argv,"",long_options,&option_index))!= -1){
    	switch(opt){
    		case 'p':
				period=atoi(optarg);
				if(period<0){fprintf(stderr, "invalid period\n");exit(1);}
				break;
			case 's':
				scale=optarg[0];
				if(scale!='C'&&scale!='F'){fprintf(stderr, "invalid scale\n");exit(1);}
				break;
			case 'l':
				log_op=1;
				log_fp=fopen(optarg,"a");
				if(log_fp==NULL){perror("log file open failed");exit(1);}
				break;
			case 'i':
				id=atoi(optarg);
				if(id<=0||(id>999999999&&id<100000000)){fprintf(stderr, "Invalid ID\n");exit(1);}
				break;
			case 'h':
				strcpy(host_name,optarg);
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
		int n=sprintf(buff,"%02d:%02d:%02d %.1f\n", curr_time->tm_hour,curr_time->tm_min,curr_time->tm_sec,curr_temp);
		if(SSL_write(SSLClient,buff,n)<0){perror("write to socket failed");exit(2);};
		if(log_op)
			fprintf(log_fp,"%s",buff);
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
		int n=sprintf(buff,"%d:%d:%d SHUTDOWN\n", curr_time->tm_hour,curr_time->tm_min,curr_time->tm_sec);
		if(SSL_write(SSLClient,buff,n)<0){perror("write to socket failed");exit(2);};
		if(log_op)
			fprintf(log_fp,"%s",buff);
		exit(0);
	}
}
//action when receive alarm sig
void alarm_handle(){
	if(stop==0)
		to_record=1;
	else
		to_record=0;
	alarm(period);
}
void execute_cmd(){
	int count=0;
	while(cmd[count]!='\n')
		count++;
	cmd[count]='\0';

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
		int n=sprintf(buff,"%02d:%02d:%02d SHUTDOWN\n", curr_time->tm_hour,curr_time->tm_min,curr_time->tm_sec);
		if(SSL_write(SSLClient,buff,n)<0){perror("write to socket failed");exit(2);};
		if(log_op)
			fprintf(log_fp,"%s",buff);
		exit(0);
	}
	cmd[6]='\0';
	if(strcmp(cmd,"PERIOD")==0){
		period=atoi(&cmd[7]);
		alarm(period);
	}
	cmd[0]='\0';
}
//read command
void handle_cmd(){
	SSL_read(SSLClient,cmd,256);
	execute_cmd();
}
void set_TLS_connection(){
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd==-1)
	{
		perror("socket error");
		exit(1);
	}
	struct sockaddr_in serv_addr;
	struct hostent* serv_host;	
	serv_host=gethostbyname(host_name);
	if(serv_host==NULL)
	{
		perror("gethostbyname error");
		exit(1);
	}
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	memcpy(&serv_addr.sin_addr,serv_host->h_addr_list[0],serv_host->h_length);
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port_num);
	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)))
	{
		perror("connect error");
		exit(1);
	}

	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	newContext=SSL_CTX_new(TLSv1_client_method());
	SSLClient = SSL_new(newContext);
	
	if (SSL_set_fd(SSLClient,sockfd)== 0) {
		perror("ERROR with setting file descriptor as SSL");
	}
	// initiate the TLS/SSL handshake with an TLS/SSL server
	if (SSL_connect(SSLClient) != 1) {
		perror("ERROR with TLS/SSL handshake");
	}


	int n=sprintf(buff,"ID=%d\n",id);
	if(SSL_write(SSLClient,buff,n)<0){perror("write to socket failed");exit(2);};
	if(log_op)
		fprintf(log_fp, "%s",buff);
}
void close_socket(){
	fsync(sockfd);
	close(sockfd);
    SSL_free(SSLClient);
	SSL_CTX_free(newContext);

}

int main(int argc, char* const argv[]){
	get_opts(argc,argv);
	set_TLS_connection();
	atexit(close_socket);
	signal(SIGALRM,alarm_handle);
	alarm(period);

	struct pollfd pollfds[1];
	pollfds[0].fd=sockfd;
	pollfds[0].events=POLLIN;

	while(1){
		int pe=poll(pollfds,1,0);
		if(pollfds[0].revents&POLLIN)
			handle_cmd();
		else{
			handle_btn();
			record_temp();
		}
	}
	return 0;
}