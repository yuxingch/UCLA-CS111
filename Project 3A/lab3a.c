//
//  main.c
//  lab3a
//
//  Created by Yuxing Chen on 5/25/17.
//  Copyright © 2017 Yuxing Chen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "ext2_fs.h"

const int SUCCESSFUL    =   0;
const int BAD_ARGUMENT  =   1;
const int OTHER_ERRORS  =   2;

int fs_fd   =   -1; //  file descriptor of the file system file

typedef struct ext2_super_block  super_block;
typedef struct ext2_group_desc group_desc;

typedef struct {
    __u32 blocks_count;
    __u32 inodes_count;
    __u32 block_size;
    __u16 inode_size;
    __u32 blocks_per_group;
    __u32 inodes_per_group;
    __u32 first_nr_inode;
    // ?
    __u32 group_count;
} superblock_parameters;
superblock_parameters superBlockSummary;

/** superblock summary
 *  1. SUPERBLOCK
 *  2. total number of blocks (decimal)
 *  3. total number of i-nodes (decimal)
 *  4. block size (in bytes, decimal)
 *  5. i-node size (in bytes, decimal)
 *  6. blocks per group (decimal)
 *  7. i-nodes per group (decimal)
 *  8. first non-reserved i-node (decimal)
 */

void superblock_summary() {
    /*  allocate memory for the superblock  */
    super_block* superBlock = malloc(sizeof(super_block));
    if (superBlock == NULL) {
        fprintf(stderr, "ERROR in Superblock: Fail to allocate memory for storing superblock info.\n");
        exit(BAD_ARGUMENT);
    }
    /*  read superblock info from file system descriptor    */
    /*  http://man7.org/linux/man-pages/man2/pread.2.html   */
    if ((pread(fs_fd, superBlock, sizeof(super_block), EXT2_MIN_BLOCK_SIZE)) == -1) {   // the block offset should be 1024
        fprintf(stderr, "ERROR in Superblock: Fail to read superblock info.\n");
        free(superBlock);
        exit(BAD_ARGUMENT);
    }

    //  Q: Need to check the magic? Magic number must be correct
    if (superBlock->s_magic != EXT2_SUPER_MAGIC) {
        fprintf(stderr, "ERROR in Superblock: Invalid magic signature.\n");
        free(superBlock);
        exit(BAD_ARGUMENT);
    }

    /*  total number of blocks and i-nodes (decimal)    */
    superBlockSummary.blocks_count = superBlock->s_blocks_count;
    superBlockSummary.inodes_count = superBlock->s_inodes_count;

    /*  block and i-node sizes  */
    /* bsize = EXT2_MIN_BLOCK_SIZE << s_log_block_size*/
    superBlockSummary.block_size = EXT2_MIN_BLOCK_SIZE << superBlock->s_log_block_size;

    //  16bit value indicating the size of the inode structure.
    //  In revision 1 and later, this value must be a perfect power of 2 and must be smaller or equal to the block size (1<<s_log_block_size).
    //  http://www.nongnu.org/ext2-doc/ext2.html#S-INODE-SIZE
    superBlockSummary.inode_size = superBlock->s_inode_size;

    /*  blocks and inodes per group    */
    superBlockSummary.blocks_per_group = superBlock->s_blocks_per_group;
    superBlockSummary.inodes_per_group = superBlock->s_inodes_per_group;

    /*  first non-reserved inode    */
    superBlockSummary.first_nr_inode = superBlock->s_first_ino;

    /*  prepare to output to csv    */
    FILE* super_csv = fopen("summary.csv", "a");
    if (super_csv == NULL) {
        fprintf(stderr, "ERROR in Superblock: Fail to create summary.csv\n");
        free(superBlock);
        exit(BAD_ARGUMENT);
    }
    //  SUPERBLOCK,100,24,1024,128,8192,24,11
    fprintf(super_csv, "%s,%u,%u,%u,%u,%u,%u,%u\n", "SUPERBLOCK", superBlockSummary.blocks_count, superBlockSummary.inodes_count,
            superBlockSummary.block_size, superBlockSummary.inode_size, superBlockSummary.blocks_per_group,
            superBlockSummary.inodes_per_group, superBlockSummary.first_nr_inode);
    fclose(super_csv);
    free(superBlock);
}

/** group summary
 *  1. GROUP
 *  2. group number (decimal, starting from 0)
 *  3. total number of blocks in this group (decimal) == blocks_per_group
 *  4. total number of i-nodes in this group (decimal)
 *  5. number of free blocks (decimal)
 *  6. number of free i-nodes (decimal)
 *  7. block number of free block bitmap for this group (decimal)
 *  8. block number of free i-node bitmap for this group (decimal)
 *  9. block number of first block of i-nodes in this group (decimal)
 */

typedef struct {
    __u32 blocks_in_group;
    __u32 inodes_in_group;
    __u16 free_blocks_count;
    __u16 free_inodes_count;
    __u32 free_block_bitmap;
    __u32 free_inode_bitmap;
    __u32 free_first_block_of_inodes;
} group_info;
group_info* groupSummary = NULL;

void group_summary() {
    /*  get the group number    */
    __u32 group_count = 0;   //  But, in the images we give you, there will only be one group.
    //  from piazza post: (adding 1 at the beginning because of the round-up)
    group_count = 1 + (superBlockSummary.blocks_count - 1) / superBlockSummary.blocks_per_group;
    superBlockSummary.group_count = group_count;    //  store the info

    groupSummary = malloc(sizeof(group_info) * group_count);
    if (groupSummary == NULL) {
        fprintf(stderr, "ERROR in Group: Fail to allocate memory for storing group descriptor info.\n");
        exit(BAD_ARGUMENT);
    }

    group_desc groupDesc[group_count];
    /*  read all group info from file system descriptor    */

    if ((pread(fs_fd, groupDesc, sizeof(group_desc) * group_count, EXT2_MIN_BLOCK_SIZE + EXT2_MIN_BLOCK_SIZE)) == -1) {   // offset should be 1024*2
        fprintf(stderr, "ERROR in Group: Fail to read group descriptor info.\n");
        exit(BAD_ARGUMENT);
    }
  /*  repeatedly, get the info for each group */
    for (int i = 0; i < group_count; i++) {
    /*  total number of blocks and inodes in each group */
        if ((i + 1) == group_count) { //  check if it is the last group
            groupSummary[i].blocks_in_group = superBlockSummary.blocks_count - (i * superBlockSummary.blocks_per_group);
            groupSummary[i].inodes_in_group = superBlockSummary.inodes_count - (i * superBlockSummary.inodes_per_group);
        } else {
            groupSummary[i].blocks_in_group = superBlockSummary.blocks_per_group;
            groupSummary[i].inodes_in_group = superBlockSummary.inodes_per_group;
        }
        /*  number of free blocks/inodes   */
        groupSummary[i].free_blocks_count = groupDesc[i].bg_free_blocks_count;
        groupSummary[i].free_inodes_count = groupDesc[i].bg_free_inodes_count;

        /*  block number of free block bitmap for this group    */
        groupSummary[i].free_block_bitmap = groupDesc[i].bg_block_bitmap;
        groupSummary[i].free_inode_bitmap = groupDesc[i].bg_inode_bitmap;
        groupSummary[i].free_first_block_of_inodes = groupDesc[i].bg_inode_table;

        FILE* group_csv = fopen("summary.csv", "a");
        if (group_csv == NULL) {
            fprintf(stderr, "ERROR in Group: Fail to open summary.csv\n");
            exit(BAD_ARGUMENT);
        }
        //  SUPERBLOCK,100,24,1024,128,8192,24,11
        fprintf(group_csv, "%s,%u,%u,%u,%u,%u,%u,%u,%u\n", "GROUP", (__u32)i, groupSummary[i].blocks_in_group,
                groupSummary[i].inodes_in_group, groupSummary[i].free_blocks_count, groupSummary[i].free_inodes_count,
                groupSummary[i].free_block_bitmap, groupSummary[i].free_inode_bitmap, groupSummary[i].free_first_block_of_inodes);
        fclose(group_csv);
    }
}


/** free block entries
 *  1. BFREE
 *  2. number of the free block (dec)
 *  MAKE SURE THE FOLLOWINGS:
 *      understand whether 1 means allocated or free
 *      have correctly understood the block number to which the first bit corresponds
 *      do not interpret more bits than there are blocks in the group
 */

void free_block_entries () {
    __u32 group_count = superBlockSummary.group_count;
    __u32 block_size = superBlockSummary.block_size;

    FILE* blockbitmap_csv = fopen("summary.csv", "a");
    if (blockbitmap_csv == NULL) {
        fprintf(stderr, "ERROR in Free Block Entries: Fail to open summary.csv\n");
        exit(BAD_ARGUMENT);
    }

    //  Scan the free block bitmap for each group
    __u32 num_block = 1;
    for (int i = 0; i < group_count; i++) {
        __u32 block_number = groupSummary[i].free_block_bitmap;
        __u32 offset = superBlockSummary.blocks_per_group * i;
        __u32 group_block_count = groupSummary[i].blocks_in_group;

        //  collect bitmap, the size of uint8_t is 1 byte
        uint8_t *curr_bitmap = malloc(block_size);
        if ((pread(fs_fd, curr_bitmap, block_size, block_number * block_size)) == -1) {
            fprintf(stderr, "ERROR in Free Block Entries: Fail to read bitmap info.\n");
            exit(BAD_ARGUMENT);
        }
        for (int j = 0; j < block_size; j++) {
            uint8_t bit = curr_bitmap[j];
            uint8_t mask = 1;   //  ?
            for (int k = 0; k < 8; k++, num_block++) {
                if (num_block > group_block_count) {
                    free(curr_bitmap);
                    return;
                }
                //  check if the block is free
                if ( (bit & mask) == 0)
                    fprintf(blockbitmap_csv, "%s,%u\n", "BFREE", num_block + offset);
                mask = mask << 1;
            }
        }
        free(curr_bitmap);
    }
}

/** free inode entries
 *  1. IFREE
 *  2. number of the free Inode (dec)
 *  MAKE SURE THE FOLLOWINGS:
 *      understand whether 1 means allocated or free
 *      have correctly understood the inode number to which the first bit corresponds
 *      do not interpret more bits than there are inodes in the group
 */

void free_inode_entries() {
    __u32 group_count = superBlockSummary.group_count;
    __u16 inode_size = superBlockSummary.inode_size;

    FILE* my_csv = fopen("new.csv", "a");
    if (my_csv == NULL) {
        fprintf(stderr, "ERROR in Free Inode Entries: Fail to open new.csv\n");
        exit(BAD_ARGUMENT);
    }

    __u32 num_inode = 1;
    for (int i = 0; i < group_count; i++) {
        __u32 block_number = groupSummary[i].free_inode_bitmap;
        __u32 offset = superBlockSummary.inodes_per_group * i;
        __u32 inode_block_count = groupSummary[i].inodes_in_group;
        __u32 block_size = superBlockSummary.block_size;
    
        uint8_t *curr_bitmap = malloc(block_size);
        if ((pread(fs_fd, curr_bitmap, block_size, block_number * superBlockSummary.block_size)) == -1) {
            fprintf(stderr, "ERROR in Free Inode Entries: Fail to read bitmap info.\n");
            exit(BAD_ARGUMENT);
        }
        for (int j = 0; j < block_size; j++) {
            uint8_t bit = curr_bitmap[j];
            uint8_t mask = 1;   //  ?
            for (int k = 0; k < 8; k++, num_inode++) {
                if (num_inode > inode_block_count) {
                    free(curr_bitmap);
                    return;
                }
                //  check if the block is free
                if ( (bit & mask) == 0)
                    fprintf(my_csv, "%s,%u\n", "IFREE", num_inode + offset);
                mask = mask << 1;
            }
        }
        free(curr_bitmap);
    }
    fclose(my_csv);
}


void inode_summary(){
    __u32 group_count = superBlockSummary.group_count;
    __u16 inode_size = superBlockSummary.inode_size;
    __u32 block_size = superBlockSummary.block_size;
    
    FILE* my_csv = fopen("inodes.csv", "a");
    if (my_csv == NULL) {
        fprintf(stderr, "ERROR in Free Inode Entries: Fail to open inodes.csv\n");
        exit(BAD_ARGUMENT);
    }


    for (int i = 0; i < group_count; i++) {    
        //get block id of the first block of inode table;
        __u32 inode_table_block_id = groupSummary[i].free_first_block_of_inodes;
        __u32 offset = inode_table_block_id * block_size;
        __u32 num_inodes_in_group = groupSummary[i].inodes_in_group;
    
        //read the inode bitmap of the current group.
        uint8_t *curr_bitmap = malloc(block_size);
        if ((pread(fs_fd, curr_bitmap, block_size, groupSummary[i].free_inode_bitmap * block_size)) == -1) {
            fprintf(stderr, "ERROR in Free Inode Entries: Fail to read bitmap info.\n");
            exit(BAD_ARGUMENT);
        }

	//how should we calculate the inode number;
        __u32 inode_num = 1;
        for(int j = 0; j < block_size; j++){
            uint8_t curr_byte = curr_bitmap[j];
            uint8_t mask = 1;

            for(int k = 0; k < 8; k++, inode_num++){
                if( inode_num > num_inodes_in_group ) {
                    free(curr_bitmap);
                    fclose(my_csv);
                    return;
                }
                
                if(curr_byte & mask){
                    struct ext2_inode curr_inode;
                    if ((pread(fs_fd, &curr_inode, inode_size, offset + inode_size * ((inode_num-1)% superBlockSummary.inodes_per_group))) == -1) {
                        fprintf(stderr, "ERROR in Free Inode Entries: Fail to read bitmap info.\n");
                        exit(BAD_ARGUMENT);
                    }
                    
                    
                    uint16_t i_mode = curr_inode.i_mode & 0x0FFF;
                    uint16_t i_uid = curr_inode.i_uid;
                    uint32_t i_size = curr_inode.i_size;
                    time_t i_atime = curr_inode.i_atime;
                    time_t i_ctime = curr_inode.i_ctime;
                    time_t i_mtime = curr_inode.i_mtime;
                    uint16_t i_gid = curr_inode.i_gid;
                    uint16_t i_link_counts = curr_inode.i_links_count;
                    uint32_t i_blocks = curr_inode.i_blocks;
                    
                    uint32_t i_block[15];
                    for(int c = 0; c < 15; c++){
                        i_block[c] = curr_inode.i_block[c];
                    }
                    
                    char type;
                    if (curr_inode.i_mode & 0x8000)
                        type = 'f';
                    else if (curr_inode.i_mode & 0x4000)
                        type = 'd';
                    else if (curr_inode.i_mode & 0xA000)
                        type = 's';
                    else
                        type = '?';
                    
                    struct tm *gmt_a, *gmt_c, *gmt_m;
                    char atime[20], ctime[20], mtime[20];
                    gmt_a = gmtime(&i_atime);
                    strftime(atime, sizeof(atime), "%m/%d/%y %H:%M:%S", gmt_a);
                    gmt_c = gmtime(&i_ctime);
                    strftime(ctime, sizeof(ctime), "%m/%d/%y %H:%M:%S", gmt_c);
                    gmt_m = gmtime(&i_mtime);
                    strftime(mtime, sizeof(mtime), "%m/%d/%y %H:%M:%S", gmt_m);

		    if (i_link_counts > 0 && i_mode != 0)
		      fprintf(my_csv, "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
			      inode_num, type, i_mode, i_uid, i_gid, i_link_counts, ctime, mtime, atime, i_size, i_blocks,
			      i_block[0], i_block[1], i_block[2], i_block[3], i_block[4], i_block[5], i_block[6], i_block[7],
			      i_block[8], i_block[9], i_block[10], i_block[11], i_block[12], i_block[13], i_block[14]);
                    
                }//if
                mask <<= 1;
            }//end k loop
        }//end j loop
        free(curr_bitmap);
    }//end i loop
    fclose(my_csv);
}


__u32 calculate_block_num(int *level, __u32 *block, __u32 global_block_num) {
    __u32 my_block_size = superBlockSummary.block_size;
    __u32 local_block_num[4] = {0,0,0,0};
    const __u32 offsets[4] = {12,
			      my_block_size+12,
			      my_block_size*my_block_size + my_block_size + 12,
			      my_block_size*my_block_size*my_block_size + my_block_size*my_block_size + my_block_size + 12};

    
    //calculate the level;
    if(global_block_num > offsets[2]){
        *level = 3;
    }
    else if(global_block_num > offsets[1]){
        *level = 2;
    }
    else if(global_block_num > offsets[0]){
        *level = 1;
    }

    if(*level == 0){
        local_block_num[0] = global_block_num;
        return block[local_block_num[0]-1];
    }
    if(*level == 1){
        local_block_num[1] = global_block_num - offsets[0];
        local_block_num[0] = 13;
        __u32 *lvl1_ind_block;
        lvl1_ind_block = malloc(my_block_size);
        if((pread(fs_fd, lvl1_ind_block, my_block_size, block[local_block_num[0]-1]*my_block_size)) == -1){
            fprintf(stderr, "ERROR in calculate_block_num: Failed to read first indirected block.\n");
            exit(BAD_ARGUMENT);
        }
        return lvl1_ind_block[local_block_num[1] - 1];
    }
    else if(*level == 2){
        local_block_num[2] = (global_block_num - offsets[1] - 1) % my_block_size + 1;
        local_block_num[1] = (global_block_num - offsets[1] - 1) / my_block_size + 1;
        local_block_num[0] = 14;
        
        __u32 *lvl1_ind_block;
        __u32 *lvl2_ind_block;
        lvl1_ind_block = malloc(my_block_size);
        lvl2_ind_block = malloc(my_block_size);
        
        if((pread(fs_fd, lvl1_ind_block, my_block_size, block[local_block_num[0] - 1]*my_block_size)) == -1){
            fprintf(stderr, "ERROR\n");
            exit(BAD_ARGUMENT);
        }
        
        if((pread(fs_fd, lvl2_ind_block, my_block_size, lvl1_ind_block[local_block_num[1] - 1]*my_block_size)) == -1){
            fprintf(stderr, "ERROR\n");
            exit(BAD_ARGUMENT);
        }
        
        return lvl2_ind_block[local_block_num[2] - 1];
    }
    else if(*level == 3){
        local_block_num[3] = (global_block_num - offsets[2] - 1) % my_block_size + 1;
        local_block_num[2] = ((global_block_num - offsets[2] - 1) % (my_block_size*my_block_size)) / my_block_size + 1;
        local_block_num[1] = (global_block_num - offsets[2] - 1) / (my_block_size*my_block_size) + 1;
        local_block_num[0] = 15;
        
        __u32 *lvl1_ind_block;
        __u32 *lvl2_ind_block;
        __u32 *lvl3_ind_block;
        lvl1_ind_block = malloc(my_block_size);
        lvl2_ind_block = malloc(my_block_size);
        lvl3_ind_block = malloc(my_block_size);
        
        if((pread(fs_fd, lvl1_ind_block, my_block_size, block[local_block_num[0] - 1]*my_block_size)) == -1){
            fprintf(stderr, "ERROR\n");
            exit(BAD_ARGUMENT);
        }
        
        if((pread(fs_fd, lvl2_ind_block, my_block_size, block[local_block_num[1] - 1]*my_block_size)) == -1){
            fprintf(stderr, "ERROR\n");
            exit(BAD_ARGUMENT);
        }
        
        if((pread(fs_fd, lvl3_ind_block, my_block_size, block[local_block_num[2] - 1]*my_block_size)) == -1){
            fprintf(stderr, "ERROR\n");
            exit(BAD_ARGUMENT);
        }
        
        return lvl3_ind_block[local_block_num[3]-1];
    }
}

void directory_entries(){
    __u32 group_count = superBlockSummary.group_count;
    __u16 inode_size = superBlockSummary.inode_size;
    __u32 block_size = superBlockSummary.block_size;

    FILE* my_csv = fopen("dir_entry.csv", "a");
    if (my_csv == NULL) {
        fprintf(stderr, "ERROR in Free Inode Entries: Fail to open inodes.csv\n");
        exit(BAD_ARGUMENT);
    }

    __u32 inode_num = 1;
    for (int i = 0; i < group_count; i++) {

        //get block id of the first block of inode table;
        __u32 inode_table_block_id = groupSummary[i].free_first_block_of_inodes;
        __u32 offset = inode_table_block_id * block_size;
        __u32 num_inodes_in_group = groupSummary[i].inodes_in_group;

        //read the inode bitmap of the current group.
        uint8_t *curr_bitmap = malloc(block_size);
        if ((pread(fs_fd, curr_bitmap, block_size, groupSummary[i].free_inode_bitmap * block_size)) == -1) {
            fprintf(stderr, "ERROR in Free Inode Entries: Fail to read bitmap info.\n");
            exit(BAD_ARGUMENT);
        }

        for(int j = 0; j < block_size/8; j++){
            uint8_t curr_byte = curr_bitmap[j];
            uint8_t mask = 1;
            for(int k = 0; k < 8; k++, inode_num++){
                if( inode_num > num_inodes_in_group ) {
                    free(curr_bitmap);
                    fclose(my_csv);
                    return;
                }

                if(curr_byte & mask){

                    struct ext2_inode curr_inode;
                    if ((pread(fs_fd, &curr_inode, inode_size, offset + inode_size * ((inode_num-1)% superBlockSummary.inodes_per_group))) == -1) {
                        fprintf(stderr, "ERROR in Directory Entries: Fail to read bitmap info.\n");
                        exit(BAD_ARGUMENT);
                    }
	  
                    uint16_t i_mode = curr_inode.i_mode;
                    uint32_t i_block[15];
      

                    for(int c = 0; c < 15; c++){
                        i_block[c] = curr_inode.i_block[c];
                    }
	  
                    if( i_mode & 0x4000 ){
                        __u32 global_block_num = 1;
                        int level = 0;
                        __u32 block_offset = calculate_block_num(&level, i_block, global_block_num) * block_size;
                        __u32 local_offset = 0;
			__u32 global_offset = 0;
                        __u32 parent_inode = inode_num;
                        struct ext2_dir_entry curr_dir_entry;

                        while(1){

                            if(local_offset >= block_size){
			      global_block_num++;
                                block_offset = calculate_block_num(&level, i_block, global_block_num) * block_size;
                                local_offset -= block_size;
                            }
	      
                            //read dir entry
                            if ((pread(fs_fd, &curr_dir_entry, sizeof(struct ext2_dir_entry), block_offset+local_offset)) == -1){
                                fprintf(stderr, "ERROR in dir_entry: Fail to read dir_entry.\n");
                                exit(BAD_ARGUMENT);
                            }

                            __u32 referenced_inode = curr_dir_entry.inode;
                            __u16 rec_len = curr_dir_entry.rec_len;
                            __u8 name_len = curr_dir_entry.name_len;
			    char *name;
			    name = malloc(sizeof(char)*(name_len+1));
			    for(int i = 0; i < name_len; i++){
			      name[i] = curr_dir_entry.name[i];
			    }
			    name[name_len] = 0;
                            if(referenced_inode == 0){
                                break;
			    }
                            fprintf(my_csv, "DIRENT,%u,%u,%u,%u,%u,'%s'\n", parent_inode, global_offset,
                                    referenced_inode, rec_len, name_len, name);

                            local_offset += rec_len;
			    global_offset += rec_len;
                        } //end while
                    } //directory
                } // valid inode
		mask = mask << 1;
            }//k
        }//j
    }//i
    fclose(my_csv);
}

void process_indirect_block(int level, __u32 block_num, __u32 offset, __u32 inode_num){

  FILE* my_csv = fopen("ind.csv", "a");
  if (my_csv == NULL) {
    fprintf(stderr, "ERROR in Free Inode Entries: Fail to open inodes.csv\n");
    exit(BAD_ARGUMENT);
  }

  __u32 *block;
  __u32 block_size = superBlockSummary.block_size;
  block = malloc(block_size);
  pread(fs_fd, block, block_size, block_num*block_size);
  if(level == 1){
    for(int i = 0; i < 256; i++){
      if(block[i] != 0){
        fprintf(my_csv, "INDIRECT,%u,%d,%u,%u,%u\n",inode_num, level, offset+i, block_num, block[i]);
      }
    }
  }

  if(level == 2 || level == 3){
    for(int i = 0; i < 256; i++){
      if(block[i] != 0){
        fprintf(my_csv, "INDIRECT,%u,%d,%u,%u,%u\n",inode_num, level, offset+i, block_num, block[i]);
        process_indirect_block(level-1, block[i], offset+i, inode_num);
      }
    }
  }
  fclose(my_csv);
}


void indirected_block_reference(){
  __u32 group_count = superBlockSummary.group_count;
  __u16 inode_size = superBlockSummary.inode_size;
  __u32 block_size = superBlockSummary.block_size;

  FILE* my_csv = fopen("ind.csv", "a");
  if (my_csv == NULL) {
    fprintf(stderr, "ERROR in Free Inode Entries: Fail to open inodes.csv\n");
    exit(BAD_ARGUMENT);
  }

  __u32 inode_num = 1;
  for (int i = 0; i < group_count; i++) {

    //get block id of the first block of inode table;
    __u32 inode_table_block_id = groupSummary[i].free_first_block_of_inodes;
    __u32 offset = inode_table_block_id * block_size;
    __u32 num_inodes_in_group = groupSummary[i].inodes_in_group;

    //read the inode bitmap of the current group.
    uint8_t *curr_bitmap = malloc(block_size);
    if ((pread(fs_fd, curr_bitmap, block_size, groupSummary[i].free_inode_bitmap * block_size)) == -1) {
      fprintf(stderr, "ERROR in Free Inode Entries: Fail to read bitmap info.\n");
      exit(BAD_ARGUMENT);
    }

    for(int j = 0; j < block_size/8; j++){
      uint8_t curr_byte = curr_bitmap[j];
      uint8_t mask = 1;
      for(int k = 0; k < 8; k++, inode_num++){
	if( inode_num > num_inodes_in_group ) {
	  free(curr_bitmap);
	  fclose(my_csv);
	  return;
	}

	if(curr_byte & mask){
	  
	  struct ext2_inode curr_inode;
	  if ((pread(fs_fd, &curr_inode, inode_size, offset + inode_size * ((inode_num-1)% superBlockSummary.inodes_per_group))) == -1) {
	    fprintf(stderr, "ERROR in Directory Entries: Fail to read bitmap info.\n");
	    exit(BAD_ARGUMENT);
	  }

          __u32 block_count = 0;
	  int level = 0;
	  __u32 global_block_num = 13;
	  __u16 i_mode = curr_inode.i_mode;
          __u32 max_block_index = curr_inode.i_blocks/(2<<block_size);

	  if(i_mode & 0x4000 || i_mode & 0x8000){
	    __u32 i_block[15];
	    for(int c = 0; c < 15; c++){
	      i_block[c] = curr_inode.i_block[c];
	    }

            for(int p = 0; p < 12; p++)
              if(i_block[p] != 0)
                block_count++;
          
            process_indirect_block(1, i_block[12], 12, inode_num);
            process_indirect_block(2, i_block[13], 12+256, inode_num);
            process_indirect_block(3, i_block[14], 12+256+65536, inode_num);
          }
	}// valid inode
	mask = mask << 1;
      }// k current byte in current inode bitmap
    }// j current inode bitmap
  }// i current group
}

//-------------------------------------------------

int main(int argc, const char * argv[]) {
  /*  check the number of operands */
  if (argc != 2) {
    fprintf(stderr, "ERROR in main: Wrong number of aruments. Correct usage: ./lab3a EXT2_test.img\n");
    exit(BAD_ARGUMENT);
  }

  const char* fileSystem = argv[1];
  //  In this project, we will provide EXT2 file system images in ordinary files
  if((fs_fd = open(fileSystem, O_RDONLY)) == -1) {
    fprintf(stderr, "ERROR in main: Fail to open the specified file system file.\n");
    exit(BAD_ARGUMENT); //  1? 2?
  }

  superblock_summary();
  group_summary();
  free_block_entries();
  free_inode_entries();
  inode_summary();
  directory_entries();
  indirected_block_reference();
  
  exit(SUCCESSFUL);
}
