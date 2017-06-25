
"""
load in from csv file
process the data
	if you assume the block size is 1kb... it might fail the test script

need some data structures in your code
	class: super block
		   inode
		   block
		   direntry
		   indirect (pointers)
	keep them in your memory
		   free inode list
		   free block list

		   blockState
		   parent relationship
		   linkcount
"""

#	check legal data block
isDataBlock(blocknum):
#	superblock/block group descriptor table/block bitmap/inode bitmap/inode table
	if blocknum < 0 or bn > totalnumofblocks:
		return false
	if bn is superblock or groupdescriptor:
		return false
	if bn is a freelist(inode/block) or inodetable
		return false
	return true
#	can ignore the backup
#	for the first group, check all five things, the rest just check three

isFreeBlock(bn):
	if isDataBlock(bn) == false:
		return false
	else
		return T/F(depending on freeblockbitmap)

isFreeInode(in):
							#	get from superblock
	if (in > 2 && in < first non-reserved inode number) || in > totalnuminodes:
		return false;
	return T/F(from the free inode bitmap)


for all blocks b:
	#	mark each block with a state
	if isDataBlock(b) == false:
		b is RESERVED
	elif isFreeBlock(b):
		b is FREE
	#	root's parent is root

checkBlock(bn):
	if bn < 0 || bn > totalnumofblocks:
		b is INVALID


#	scan all the inodes
for all inode i:
	if i.filemode == 0 || i.linkcount == 0
		skip
	# (for our inodes, mode > 0 && linkcount > 0)
	# need to check all 15 pointers
	for each ptr: 
		"""
		if ptr != 0:
			checkBlock
		if ptr != RESERVED && ptr != FREE && ptr != USED:
			ptr is USED
		"""
		check if INVALID using checkBlock()
		if ptr == USED:
			ptr is DUPLICATE
		else if ptr == FREE:
			ALLOCATE BLOCK ON FREE LIST
		else if ptr == INVALID || RESERVED:
			INVALID/RESERVED Block
		else ..
			prt is USED



besides 15 pointers, we also need to check indirect block pointers


for all indirect block pointers:
	check if INVALID
	check if FREE, USED
	... copy the same code as in previous part
	if INVALID or RESERVED:
		be careful about level

		the three pointers (after the 12 pointers) is not in this list

		[see pic] we just output it as BLOCK, because its level is 0

for all DataBlocks:
			^ isDataBlock == True
	if bn != FREE/USED/DUP:
		UNREFERENCED ......
	if DUP:
		DUPLICATE ......


for all inode i:
	an inode is allocated if mode > 0 && linkcount > 0
							^ allocated
	compare i.linkcount and linkcount

for each direntry de:
	if de.inodenum > totalnuminodes:
		INVALID
	#	if UNALLOCATED
	linkcount[de.inodenum]++
	if de is not '.' '..'
		parent[de.inodenum] = de.parentinodenumber
	#	'.'
	compare parent inode number and inode number, check if they're the same
	#	'..'
	compare inode number == grandparent's inode number


"""
From previous quarter
"""
class Inode:
inode #;  // for this inode
ref_list;  //  what directory entries refers to this inode
# of links; // number of hard links for this inode, read from csv
pointers[15];  //  12 normal, 3 indirect

class Block:
block #;
ref_list; // what the inodes point to this block

class Superblock:
free_inode_list;  //  numbers corresponding to free inode numbers
free_block_list;  //  numbers corresponding to free block numbers
allocated_inodes;  //  doesn't need to be a list. inode numbers allocated
allocated_blocks;  //  block numbers allocated
indirect_ptrs;        //  hashmap like (block #, entry #, ptr), 256 at most
directory_list; //  (child directories inodes, parent directory inode)

it's up to you whether or not to include the block that contains the superblock or group descriptors or bitmap into the allocated block lists;

For bitmap: store into free_inode_list and free_block_list.
load from inode csv:
function: register_blocks_for_inodes():
	if # blocks for inode i <= 12:
		register_block(i.ptr[0 to 11]);  //  i is inode
	else: // have to go through indirect blocks
		indirect block
		double indirect block
		triple indirect block

	//  check indirect block ptr[12]
	if ptr[12] == 0 || ptr[12] > # of blocks in disk
		Invalid block error
	// go through pointers in ptrs[12]
	at most 256 pointers. Need to visit: # blocks - 12
	if (ptr[12], j) in indirect list //  j: entry number in the indirect block
		register_block ...
	else
		invalid block error

//  goal: put one block into allocated blocks in superblock
function: register_block(block #, inode #, indirect block #, entry #)  // (optional): indirect block #
	if block # == 0 or block # >= # of blocks:
		Invalid block error
	else if block # is in allocated_blocks:
		add (inode#, indirect block #, entry #) into the ref_list;
	else
		put block # into allocated blocks


//  load directory csv
for each line, you get:
	parent inode #, entry #, child inode #
	
	if parent inode # == 2 ||  (entry # > 1 && parent inode # != child inode #)
	      root dir                            ., .., always come before anything else (self, parent dir)
		directory_list.add(c i #, p i #);
	if ci # in allocated inodes:
		add( p i #, entry #) into ref_list of inode;
	else
		Unallocated inode error
	
	if entry # == 0 && pi # != ci #:  //  check '.'
		Incorrect entry error
	if entry # == 1 && ci # != parent inode # of pi#: //  parent inode # of pi# you get from directory list
		Incorrect entry error
	
	// some inodes error checking
	for each node i in allocated inodes:
		if i.inode # > 10 && len(i.ref_list) == 0:
			Missing inode error
		else if len(i.ref_list) != i.# of links:
			Incorrect link count
	
	if there is an inode i such that i belongs to free_inode_list and i also belongs to allocated inodes:
		Unallocated inode error
	for i such taht i.inode # >= 11, inode# <= # of inodes in disk
		check all inodes except first 10 in disk
		i not in free, i not in allocated
		MISSING
	
	// check blocks
	for each block b in allocated blocks:
		if len(b.ref-list) > 1:
Multiple referenced block error

