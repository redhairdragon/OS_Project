#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>	
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include "ext2_fs.h"
#include <time.h>
#include <string.h>

int fs_des;
//var set in getsuperblock
unsigned int block_size;
unsigned short inode_size;
unsigned int blocks_count;
unsigned int inodes_count;
unsigned int num_inodes_per_group;
unsigned int num_blocks_per_group;
unsigned int num_group;

//var set in getgroup
struct ext2_group_desc* des_table;

//declarations
char file_type(unsigned short i_mode);
void get_superblock();
void get_group();
void get_free_block_entries
	(unsigned int bg_block_bitmap,unsigned int num_blocks_in_group,unsigned int group_index);
void get_free_inode_entries
	(unsigned int bg_inode_bitmap,unsigned int num_inodes_in_group,unsigned int group_index);
void get_inodes_in_group
	(unsigned int bg_inode_table,unsigned int num_inodes_in_group,int group_index);
void get_directory_entries(struct ext2_inode* inode_stuct);
void get_indirect_directory_entries(int level,unsigned int indirect_block_num);
void handle_directory_data_block(unsigned int block_number);
int get_indirect_blocks(unsigned int block_num,unsigned int inode_num ,int counted_size);
int get_sec_indirect_blocks(unsigned int block_num,unsigned int inode_num ,int counted_size);
int get_thr_indirect_blocks(unsigned int block_num,unsigned int inode_num,int counted_size);

int main(int argc, char const *argv[])
{
	if(argc!=2){fprintf(stderr, "Invalid args\n");exit(1);}
	fs_des=open(argv[1],O_RDONLY);
	if(fs_des==-1){perror("Fail to open Img file\n");exit(1);}
	get_superblock();
	get_group();

	close(fs_des);
	return 0;
}
char file_type(unsigned short i_mode){
	switch(i_mode&0xF000){
		case 0x8000:
			return 'f';
		case 0x4000:
			return 'd';
		case 0xA000:
			return 's';
		default:
			return '?';
	}
}
void get_superblock(){
	struct ext2_super_block sb;
	int n=pread(fs_des,&sb,sizeof(struct ext2_super_block),1024);
	if(n!=sizeof(struct ext2_super_block)){printf("corrupted file\n");exit(1);}
	
	//set vars for other uses
	block_size=EXT2_MIN_BLOCK_SIZE << sb.s_log_block_size;
	inode_size=sb.s_inode_size;
	blocks_count=sb.s_blocks_count;
	inodes_count=sb.s_inodes_count;
	num_inodes_per_group=sb.s_inodes_per_group;
	num_blocks_per_group=sb.s_blocks_per_group;
	num_group=ceil((double)blocks_count/num_blocks_per_group);

	//print out summary
	printf("SUPERBLOCK,");	
	printf("%u,",blocks_count);
	printf("%u,",inodes_count);
	printf("%u,",block_size);
	printf("%hu,",inode_size);
	printf("%u,",num_blocks_per_group);
	printf("%u,",num_inodes_per_group);
	printf("%u\n",sb.s_first_ino);
}
void get_group(){	
	//get block group des table
	des_table=malloc(num_group*sizeof(struct ext2_group_desc));
	if(des_table==NULL){perror("malloc failed");exit(2);}
	int n=pread(fs_des,des_table,sizeof(struct ext2_group_desc)*num_group,2048);
	if(n!=sizeof(struct ext2_group_desc)*num_group){printf("corrupted file\n");exit(1);}

	for(int i=0;i<num_group;i++){
		//calculate number of blocks in current block group
		unsigned int num_blocks_in_group=num_blocks_per_group;
		unsigned int num_inodes_in_group=num_inodes_per_group;
		if(i==num_group-1){
			num_blocks_in_group=blocks_count-(num_group-1)*num_blocks_per_group;
			num_inodes_in_group=inodes_count-(num_group-1)*num_inodes_per_group;
		}
		//print out summary
		printf("GROUP,");
		printf("%d,",i);
		printf("%d,",num_blocks_in_group);
		printf("%d,",num_inodes_in_group);
		printf("%hu,",des_table[i].bg_free_blocks_count);
		printf("%hu,",des_table[i].bg_free_inodes_count);
		printf("%u,",des_table[i].bg_block_bitmap);
		printf("%u,",des_table[i].bg_inode_bitmap);
		printf("%u\n",des_table[i].bg_inode_table);
		get_free_block_entries(des_table[i].bg_block_bitmap,num_blocks_in_group,i);
		get_free_inode_entries(des_table[i].bg_inode_bitmap,num_inodes_in_group,i);
		get_inodes_in_group(des_table[i].bg_inode_table,num_inodes_in_group,i);
	}
}
void get_free_block_entries(unsigned int bg_block_bitmap,unsigned int num_blocks_in_group,unsigned int group_index){
	unsigned int offset=group_index*num_blocks_per_group+1;
	char* bitmap=malloc(num_blocks_in_group/8);
	if(bitmap==NULL){perror("malloc failed");exit(2);}
	int n=pread(fs_des,bitmap,num_blocks_in_group/8,bg_block_bitmap*block_size);
	if(n!=num_blocks_in_group/8){printf("corrupted file\n");exit(1);}

	for(int i=0;i<num_blocks_in_group;i++){
		int bit=1&bitmap[i/8]>>(i%8);
		if(bit==0){
			printf("BFREE,");
			printf("%d\n",offset+i);
		}
	}
	free(bitmap);
}
void get_free_inode_entries(unsigned int bg_inode_bitmap,unsigned int num_inodes_in_group,unsigned int group_index){
	unsigned int offset=group_index*num_inodes_in_group+1;
	char* bitmap=malloc(num_inodes_in_group/8);
	if(bitmap==NULL){perror("malloc failed");exit(2);}
	
	int n=pread(fs_des,bitmap,num_inodes_in_group/8,bg_inode_bitmap*block_size);
	if(n!=num_inodes_in_group/8){printf("corrupted file\n");exit(1);}

	for(int i=0;i<num_inodes_in_group;i++){
		int bit=1&bitmap[i/8]>>(i%8);
		if(bit==0){
			printf("IFREE,");
			printf("%d\n",offset+i);
		}
	}
	free(bitmap);
}
void get_inodes_in_group(unsigned int bg_inode_table,unsigned int num_inodes_in_group,int group_index){
	unsigned int offset=group_index*num_inodes_in_group+1;
	struct ext2_inode* inode_table=malloc(sizeof(struct ext2_inode)*num_inodes_in_group);
	if(inode_table==NULL){perror("malloc failed");exit(2);}
	int n=pread(fs_des,inode_table,sizeof(struct ext2_inode)*num_inodes_in_group,block_size*bg_inode_table);
	if(n!=sizeof(struct ext2_inode)*num_inodes_in_group)
		{printf("corrupted file\n");exit(1);}

	for(int i=0;i<num_inodes_in_group;i++){
		if(inode_table[i].i_mode!=0&&inode_table[i].i_links_count!=0){
			long change_time=inode_table[i].i_ctime;
			long mod_time=inode_table[i].i_mtime;
			long last_access_time=inode_table[i].i_atime;

			printf("INODE,");
			printf("%d,",i+offset);
			printf("%c,",file_type(inode_table[i].i_mode));
			printf("%o,",0777&(inode_table[i].i_mode));
			printf("%hu,",inode_table[i].i_uid);
			printf("%hu,",inode_table[i].i_gid);
			printf("%hu,",inode_table[i].i_links_count);
			char mbstr[100];
			strftime(mbstr, sizeof(mbstr), "%m/%d/%y %H:%M:%S",gmtime(&change_time));
			printf("%.24s,",mbstr);
			strftime(mbstr, sizeof(mbstr), "%m/%d/%y %H:%M:%S",gmtime(&mod_time));
			printf("%.24s,",mbstr);
			strftime(mbstr, sizeof(mbstr), "%m/%d/%y %H:%M:%S",gmtime(&last_access_time));
			printf("%.24s,",mbstr);
			printf("%d,",inode_table[i].i_size);
			printf("%d,",inode_table[i].i_blocks);

			


			if(file_type(inode_table[i].i_mode)!='s'){
				for(int j=0;j<EXT2_NDIR_BLOCKS;j++)
					printf("%u,",inode_table[i].i_block[j]);
				printf("%u,",inode_table[i].i_block[12]);
				printf("%u,",inode_table[i].i_block[13]);
				printf("%u\n",inode_table[i].i_block[14]);

				//if directory,then read blocks and get 
				get_directory_entries(&inode_table[i]);
				int counted_size=12;
				if(inode_table[i].i_block[12]!=0)
					counted_size=get_indirect_blocks(inode_table[i].i_block[12],i+offset,counted_size);
				if(inode_table[i].i_block[13]!=0)
					counted_size=get_sec_indirect_blocks(inode_table[i].i_block[13],i+offset,counted_size);
				if(inode_table[i].i_block[14]!=0)
					counted_size=get_thr_indirect_blocks(inode_table[i].i_block[14],i+offset,counted_size);
			}
			else{
				printf("%u\n",inode_table[i].i_block[0]);
			}
			
		}
	}
	free(inode_table);
}
void handle_directory_data_block(unsigned int block_number){
	if(block_number==0)
		return;

	struct ext2_dir_entry * start=malloc(block_size);
	if(start==NULL){perror("malloc failed");exit(2);}

	int n=pread(fs_des,start,block_size,block_size*block_number);
	if(n!=block_size){printf("corrupted file\n");exit(1);}

	int pos=0;
	struct ext2_dir_entry * curr=start;
	unsigned int parent_inode_num=0;

	while(pos<block_size){
		if(curr->inode!=0){
			if(strcmp(".",curr->name)==0)
				parent_inode_num=curr->inode;
			printf("DIRENT,");
			printf("%u,",parent_inode_num);
			printf("%d,",pos);
			printf("%u,",curr->inode);
			printf("%hu,",curr->rec_len);
			printf("%hhu,",curr->name_len);
			printf("'%.*s'\n",curr->name_len,curr->name);
		}
		pos+=curr->rec_len;
		curr=(struct ext2_dir_entry *)((char*)start+pos);
	}
	free(start);
}
void get_directory_entries(struct ext2_inode* inode_stuct){
	if(file_type(inode_stuct->i_mode)=='d'){
		for(int j=0;j<(inode_stuct->i_blocks/(block_size/512.0));j++)
			handle_directory_data_block(inode_stuct->i_block[j]);
		if(inode_stuct->i_block[12]!=0)
			get_indirect_directory_entries(1,inode_stuct->i_block[12]);
		if(inode_stuct->i_block[13]!=0)
			get_indirect_directory_entries(2,inode_stuct->i_block[13]);
		if(inode_stuct->i_block[14]!=0)
			get_indirect_directory_entries(3,inode_stuct->i_block[14]);
	}
}
void get_indirect_directory_entries(int level,unsigned int indirect_block_num){
	if(level==1)
		handle_directory_data_block(indirect_block_num);
	else if(level>1){
		unsigned int* blocks=malloc(block_size);
		int n=pread(fs_des,blocks,block_size,indirect_block_num*block_size);
		if(n!=block_size){printf("corrupted file\n");exit(1);}

		for(int i=0;i<block_size/sizeof(unsigned int);i++){
			if(blocks[i]!=0)
				get_indirect_directory_entries(level-1,blocks[i]);
		}
		free(blocks);
	}
}



int get_indirect_blocks(unsigned int block_num,unsigned int inode_num,int counted_size){
	if(block_num==0)
		return counted_size+block_size/sizeof(unsigned int);

	unsigned int current_block;
	unsigned int offset=block_num*block_size;
	int n=pread(fs_des,&current_block,sizeof(unsigned int),offset);
	if(n!=sizeof(unsigned int)){printf("corrupted file\n");exit(1);}
	
	unsigned int pos=0;
	while(pos<block_size){
		if(current_block!=0){
			printf("INDIRECT,");
			printf("%u,",inode_num);
			printf("%d,",1);
			printf("%u,",counted_size);
			printf("%u,",block_num);
			printf("%u\n",current_block);
		}
		counted_size+=1;
		offset+=sizeof(unsigned int);
		pos+=sizeof(unsigned int);
		n=pread(fs_des,&current_block,sizeof(unsigned int),offset);
		if(n!=sizeof(unsigned int)){printf("corrupted file\n");exit(1);}

	}
	return counted_size;
}
int get_sec_indirect_blocks(unsigned int block_num,unsigned int inode_num,int counted_size){
	if(block_num==0)
		return counted_size;

	unsigned int* indirect_blocks=malloc(block_size);
	if(indirect_blocks==NULL){perror("malloc failed");exit(2);}
	int n=pread(fs_des,indirect_blocks,block_size,block_num*block_size);
	if(n!=block_size){printf("corrupted file\n");exit(1);}

	for(int i=0;i<block_size/sizeof(unsigned int);i++){
		if(indirect_blocks[i]!=0){
			printf("INDIRECT,");
			printf("%u,",inode_num);
			printf("%d,",2);
			printf("%u,",counted_size);
			printf("%u,",block_num);
			printf("%u\n",indirect_blocks[i]);
		}
		counted_size=get_indirect_blocks(indirect_blocks[i],inode_num,counted_size);
	}
	free(indirect_blocks);
	return counted_size;
}
int get_thr_indirect_blocks(unsigned int block_num,unsigned int inode_num,int counted_size){
	if(block_num==0)
		return counted_size;

	unsigned int* indirect_blocks=malloc(block_size);
	if(indirect_blocks==NULL){perror("malloc failed");exit(2);}
	int n=pread(fs_des,indirect_blocks,block_size,block_num*block_size);
	if(n!=block_size){printf("corrupted file\n");exit(1);}

	for(int i=0;i<block_size/sizeof(unsigned int);i++){
		if(indirect_blocks[i]!=0){
			printf("INDIRECT,");
			printf("%u,",inode_num);
			printf("%d,",3);
			printf("%u,",counted_size);
			printf("%u,",block_num);
			printf("%u\n",indirect_blocks[i]);
		}
		counted_size=get_sec_indirect_blocks(indirect_blocks[i],inode_num,counted_size);
	}
	free(indirect_blocks);
	return counted_size;
}