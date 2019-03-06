class SuperBlock():
	def __init__(self, report_list):
		for entry in report_list:
			if entry[0]=='SUPERBLOCK':
				self.number_blocks=int(entry[1])
				self.number_inodes=int(entry[2])
				self.first_non_reserved_inode=int(entry[7])
				self.inode_size=int(entry[4])
				self.block_size=int(entry[3])
				break