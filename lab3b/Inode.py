class Inode():
	def __init__(self,entry):
		self.entry=entry
		self.inode_num=int(entry[1])
		self.file_type=entry[2]
		self.link_num=int(entry[6])
		self.num_blocks=int(entry[11])
		self.direct_data=[0]*15
		for i in range(15):
			self.direct_data[i]=int(entry[12+i])
