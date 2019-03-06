from SuperBlock import *
from Inode import *

class InodesInfo():
	def __init__(self, report_list):
		self.superblock=SuperBlock(report_list)
		self.free_inodes_bitmap=[False]*self.superblock.number_inodes
		self.inodes=[]		
		for entry in report_list:
			if entry[0]=='IFREE':
				self.free_inodes_bitmap[int(entry[1])-1]=True
			if entry[0]=='INODE':
				self.inodes.append(Inode(entry))

	def is_free(self,inode_num):
		if(self.in_range(inode_num)):
			return self.free_inodes_bitmap[inode_num-1]
		return False;

	def in_range(self,inode_num):
		if inode_num>=1 and inode_num<=self.superblock.number_inodes:
			return True
		else:
			return False

	def is_unallocated(self,inode_num):
		allocated_nodes=[]
		for inode in self.inodes:
			if inode.file_type!='0':
				allocated_nodes.append(inode.inode_num)
		return inode_num not in allocated_nodes

	def check_inodes_allocation(self):
		allocated_nodes=[]
		for inode in self.inodes:
			if self.is_free(inode.inode_num)==True:
					print("ALLOCATED INODE "+str(inode.inode_num)+" ON FREELIST")
			else:
				if inode.file_type=='0':
					print("UNALLOCATED INODE "+str(inode.inode_num)+" NOT ON FREELIST")
				else: allocated_nodes.append(inode.inode_num)


		for i in range(self.superblock.first_non_reserved_inode,len(self.free_inodes_bitmap)):
			bit=self.free_inodes_bitmap[i]
			if bit == False and i+1 not in allocated_nodes:
				print("UNALLOCATED INODE "+str(i+1)+" NOT ON FREELIST")






