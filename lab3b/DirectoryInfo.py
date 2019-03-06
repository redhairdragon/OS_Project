from Inode import *
from InodesInfo import *

class DirectoryEntry():
	def __init__(self, entry):
		self.inode_num=int(entry[3])
		self.parent_inode_num=int(entry[1])
		self.name=entry[6]

class DirectoryInfo():
	def __init__(self, report_list):
		self.inodes=[]
		self.directories=[]
		self.inodes_info=InodesInfo(report_list)

		for entry in report_list:
			if entry[0]=='INODE':
				self.inodes.append(Inode(entry))
			if entry[0]=='DIRENT':
				self.directories.append(DirectoryEntry(entry))

	def check_directory_consistency(self):
		for inode in self.inodes:
			ref_count=0
			for direc in self.directories:			
				if direc.inode_num==inode.inode_num:
					ref_count+=1
			if ref_count!=inode.link_num:
				print("INODE "+str(inode.inode_num)+" HAS "+str(ref_count)+" LINKS BUT LINKCOUNT IS "+str(inode.link_num))

		for direc in self.directories:
			if not self.inodes_info.in_range(direc.inode_num):
				print("DIRECTORY INODE "+str(direc.parent_inode_num)+" NAME "+direc.name+" INVALID INODE "+str(direc.inode_num))
			elif self.inodes_info.is_unallocated(direc.inode_num):
				print("DIRECTORY INODE "+str(direc.parent_inode_num)+" NAME "+direc.name+" UNALLOCATED INODE "+str(direc.inode_num))
			

							

		folders=[]
		for direc in self.directories:
			if direc.name =="'.'":
				if direc.parent_inode_num!=direc.inode_num:
					print("DIRECTORY INODE "+str(direc.parent_inode_num)+" NAME "+direc.name+" LINK TO INODE "+str(direc.inode_num)+" SHOULD BE "+str(correct))
			if direc.name =="'..'":
				correct=2
				for direc2 in self.directories:
					if direc2.inode_num == direc.parent_inode_num and direc2.name!="'.'"and direc2.name!="'..'":
						correct=direc2.parent_inode_num
				if correct!=direc.inode_num:
					print("DIRECTORY INODE "+str(direc.parent_inode_num)+" NAME "+direc.name+" LINK TO INODE "+str(direc.inode_num)+" SHOULD BE "+str(correct))