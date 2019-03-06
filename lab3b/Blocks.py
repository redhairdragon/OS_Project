class IndirectBlock():
	def __init__(self, entry):
		self.inode_num=int(entry[1])
		self.level=int(entry[2])
		self.offset=int(entry[3])
		self.scanned_block=int(entry[4])
		self.refer_block=int(entry[5])

