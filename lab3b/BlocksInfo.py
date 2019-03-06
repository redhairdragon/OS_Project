from SuperBlock import *
from Inode import *
from Blocks import *
import math 

class GroupDescriptor():
	def __init__(self, entry):
		self.inode_table_block=int(entry[8])
		self.num_inodes=int(entry[3])

class BlocksInfo():
	def __init__(self, report_list):
		self.inodes=[]		
		self.indirects=[]
		self.superblock=SuperBlock(report_list)
		self.free_blocks_bitmap=[False]*self.superblock.number_blocks

		for entry in report_list:
			if entry[0]=='GROUP':
				self.group_descriptor=GroupDescriptor(entry)
			if entry[0]=='BFREE':
				self.free_blocks_bitmap[int(entry[1])]=True
			if entry[0]=='INODE':
				self.inodes.append(Inode(entry))
			if entry[0]=='INDIRECT':
				self.indirects.append(IndirectBlock(entry))
	
	def is_free(self,block_num):
		if not self.in_range(block_num):
			return False
		return self.free_blocks_bitmap[block_num]

	def in_range(self,block_num):
		if block_num>=0 and block_num<self.superblock.number_blocks:
			return True
		return False

	def is_reserved(self,block_num):
		first_non_reserved_block= self.group_descriptor.inode_table_block	\
					+math.ceil(self.superblock.inode_size*self.superblock.number_inodes	\
						/self.superblock.block_size)
		if block_num<first_non_reserved_block and block_num>0:
			return True
		return False

	def check_block_consistency(self):
		#check valid and  reserved
		#inode entries
		for inode in self.inodes:
			for i in range(len(inode.direct_data)):
				string=""
				if not self.in_range(inode.direct_data[i]):
					string="INVALID "
				elif self.is_reserved(inode.direct_data[i]):
					string="RESERVED "

				if len(string)!=0:
					offset=i
					if i==14: 
						string+="TRIPPLE INDIRECT "
						offset=65804
					if i==13: 
						string+="DOUBLE INDIRECT "
						offset=268
					if i==12: 
						string+="INDIRECT "

					string+="BLOCK "+str(inode.direct_data[i])+" IN INODE "+str(inode.inode_num)+" AT OFFSET "+str(offset)
					print(string)

		#indirects entries 
		for i in range(len(self.indirects)):
			string=""
			if not self.in_range(self.indirects[i].refer_block):
				string="INVALID "
			elif self.is_reserved(self.indirects[i].refer_block):
				string="RESERVED "
			if len(string)!=0:
				if self.indirects[i].level == 3:
					string+= "TRIPLE "
				elif self.indirects[i].level == 2:
					string+= "DOUBLE "
				string+="INDIRECT BLOCK "
				string+=str(self.indirects[i].refer_block)
				string+=" IN INODE "+str(self.indirects[i].inode_num)+" AT OFFSET "+str(self.indirects[i].offset)
				print(string)


		allocated_block=[]
		#check allocation on freelist
		for inode in self.inodes:
			for i in range(len(inode.direct_data)):
				if self.is_free(inode.direct_data[i]):
					print("ALLOCATED BLOCK "+str(inode.direct_data[i])  +" ON FREELIST")
				else: 
					allocated_block.append(inode.direct_data[i])

		for indir in self.indirects:
			if  self.is_free(indir.refer_block):
				print("ALLOCATED BLOCK "+str(indir.refer_block)  +" ON FREELIST")
			else: 
				allocated_block.append(indir.refer_block)


		#check if a block on free list but not assigned to any inode
		first_data_block=self.group_descriptor.inode_table_block	\
					+math.ceil(self.superblock.inode_size*self.superblock.number_inodes	\
					/self.superblock.block_size)

		for i in range(first_data_block,self.superblock.number_blocks):
			if not self.is_free(i) and i not in allocated_block:
				print("UNREFERENCED BLOCK "+str(i))

		#check duplicates
		allocated_map=dict()
		for inode in self.inodes:
			for i in range(len(inode.direct_data)):
				string="DUPLICATE "
				offset=i
				if i==14: 
					string+="TRIPPLE INDIRECT "
					offset=65804
				if i==13: 
					string+="DOUBLE INDIRECT "
					offset=268
				if i==12: 
					string+="INDIRECT "
				if inode.direct_data[i] not in allocated_map:
					allocated_map[inode.direct_data[i]]=[]
				string+="BLOCK "+str(inode.direct_data[i])+" IN INODE "+str(inode.inode_num)+" AT OFFSET "+str(offset)
				allocated_map[inode.direct_data[i]].append(string)
		
		for indir in self.indirects:
			if indir.refer_block not in allocated_map:
				allocated_map[indir.refer_block]=[]
			string="DUPLICATE "
			if indir.level == 3:
				string+= "TRIPLE "
			elif indir.level == 2:
				string+= "DOUBLE "
			string+="INDIRECT BLOCK "
			string+=str(indir.refer_block)
			string+=" IN INODE "+str(indir.inode_num)+" AT OFFSET "+str(indir.offset)
			allocated_map[indir.refer_block].append(string)

		for key in allocated_map.keys():
			if len(allocated_map[key])>1 and key != 0:
				for string in allocated_map[key]:
					print(string)

