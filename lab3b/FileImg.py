import csv
from BlocksInfo import *
from InodesInfo import *
from DirectoryInfo import *

class FileImgChecker():
	def __init__(self,filename):
		report_list=[]
		with open(filename, newline='\n') as f:
			reader = csv.reader(f)
			for row in reader:
				report_list.append(row)

		self.blocks=BlocksInfo(report_list)
		self.inodes=InodesInfo(report_list)	
		self.directories=DirectoryInfo(report_list)

	def check(self):
		self.inodes.check_inodes_allocation();
		self.blocks.check_block_consistency()
		self.directories.check_directory_consistency();
		



