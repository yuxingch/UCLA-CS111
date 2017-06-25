/**
 *	inconsistency
 *	1kb 
 * 	file size? three pointers? you just read the first one
 *	in our file image contains only one group
 *	stop at 4.2


 *	The number of groups

 *	The number of blocks, should be the same except the last one (and check the total number)

 *	inode table is always the same size

 *	#/8 how many bytes in this bitmap
 *	not all bits in the bit map are valid. read x, this x corresponds to the number of inode, blocks in that group.
 
 * Free Block
 *	unique id for each one, starts from 0

 * inode
 *	0 is reserved
 *	unique id for each one, but does not start from 0


 * Inode Summary
 *	if it has 0 int count and 0 mode, skip
 *	extract the first four bits to get the file type
 *	file size: # of data blocks + blocks contain indirect pointers (previously)
 			now: # of data blocks
 * direct: 12; indirect: 1024/4=256
 * double indirect blocks, max number of blocks we can access is 256^2
 *		maximum file size for a single file on this disk: 12+256+256^2+256^3
 		disk usage: +1+1+256+1+256+256^2

 * 4096 inodes, inode id: 9000, ceiling of 9000/4096 = 3, 9000 - 4096*2

 * Directory entries
 *	if the file type is a directory, you have to read the directory pointers
 *	he trusts the file size
 *	you first calculate the file size (how many data blocks you have to read)



 * Indirect Block References
 *	
 	+ ---> block, block offset 0
 	+ ---> block, block offset 0 + block size
 	+
 	+
 	...
 	+ (12 data blocks), block offset = 11*blocksize
	+ indirect: 12*blocksize
	+ double: 268
	+ triple: 256^2 + 256 + 12

	before we examine the double ..., we need to examine the pointer offsets pointed by the indirect... 
	(assume 1kib block size) => 256 pointers
	12*blocksize
	-> 12*blocksize
	(256+11)*1kb


	doubly indirect block
	offset: 256+12



	255*256^2+256^2+256+12

 */
