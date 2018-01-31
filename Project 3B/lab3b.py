#!/usr/bin/python

"""
NAME: Yuxing Chen / Yuhong Wang
UID:  
EMAIL: 
"""

from __future__ import print_function
import sys
import csv

flag = 0

class Superblock:
    def __init__(self, tbn=0, tin=0, bs=0, ins=0, bpg=0, ipg=0, fni=0, sin=0):
        self.total_block_num = tbn
        self.total_inode_num = tin
        self.block_size = bs
        self.inode_size = ins
        self.block_per_group = bpg
        self.inode_per_group = ipg
        self.first_nr_inode = fni
        self.start_inode_num = sin
    
    def start_of_datablock(self):
        return self.start_inode_num + (128 * self.total_inode_num - 1) / self.block_size + 1

class Block:
    def __init__(self, blocknumber=0, level=0,offset=0, inode=0):
        self.block_number = blocknumber
        self.block_level = level
        self.reference_inode = inode
        self.block_offset = offset

class Inode:
    def __init__(self, inodenumber=0, mode=0, linkcount=0):
        self.inode_number = inodenumber
        self.inode_mode = mode
        self.link_count = linkcount
        self.reference_list = set()
        self.pointers = []
        self.links = 0

class Dirent:
    def __init__(self, parentInodeNum, InodeNum, name):
        self.parent_inode_num = parentInodeNum
        self.inode_num = InodeNum
        self.entry_name = name


superBlock = Superblock()
freeInodeList = set()
freeBlockList = set()
dirEntryList = set()
parentInode = dict()
blockDict = dict()
inodeDict = dict()


def parse_input_file():
    if len(sys.argv) != 2:
        print('Too many arguments!\nUsage: lab3b Filename', file=sys.stderr)
        sys.exit(1)

    try:
        inputFile = open(sys.argv[1], 'rb')
    except:
        print('File {} does not exist'.format(sys.argv[1]), file=sys.stderr)
        sys.exit(1)

    csvreader = csv.reader(inputFile, delimiter=',')
    for row in csvreader:    
        if row[0] == 'GROUP':
            superBlock.start_inode_num = int(row[8])
            
        if row[0] == 'IFREE':
            freeInodeList.add(int(row[1]))
        if row[0] == 'BFREE':
            freeBlockList.add(int(row[1]))
            
        if row[0] == 'SUPERBLOCK':
            superBlock.total_block_num = int(row[1])
            superBlock.total_inode_num = int(row[2])
            superBlock.block_size = int(row[3])
            superBlock.inode_size = int(row[4])
            superBlock.block_per_group = int(row[5])
            superBlock.inode_per_group = int(row[6])
            superBlock.first_nr_inode = int(row[7])

        if row[0] == 'INODE':
            inode_num = int(row[1])
            inode = Inode(inode_num,int(row[3]),int(row[6]))
            for i in range(15):
                block_num = int(row[12+i])
                inode.pointers.append(block_num)
                level = 0
                offset = i
                if i > 11:
                    level = i - 11
                    offset = (level == 1) * 12 + (level == 2) * 268 + (level == 3) * 65804

                # add this block_num to blocks if block_num > 0
                if block_num > 0:
                    block = Block(block_num, level, offset, inode_num)  # create new block
                    if block_num not in blockDict:
                        blockDict[block_num] = set()
                    blockDict[block_num].add(block)
            inodeDict[inode.inode_number] = inode

        if row[0] == 'INDIRECT':
            inode_num = int(row[1])
            level = int(row[2]) - 1
            offset = int(row[3])
            block_num = int(row[5])

            block = Block(block_num, level, offset, inode_num)
            if block_num not in blockDict:
                blockDict[block_num] = set()
            blockDict[block_num].add(block)
                
        if row[0] == 'DIRENT':
            parent_inode_num = int(row[1])
            inode_num = int(row[3])
            entry_name = row[6]
            dirent = Dirent(parent_inode_num, inode_num, entry_name)
            dirEntryList.add(dirent)
            if entry_name != '\'..\'' and entry_name != '\'.\'':
                parentInode[inode_num] = parent_inode_num


def is_free_inode(inode_num):
    if 2 < inode_num < superBlock.first_nr_inode:
        return False
    if inode_num > superBlock.total_inode_num:
        return False
    return inode_num in freeInodeList


def is_data_block(block_num):
    #   Here we assume there won't be multiple groups
    if block_num < superBlock.start_of_datablock() or block_num > superBlock.total_block_num:
        return False
    return True


def is_free_block(block_num):
    if not is_data_block(block_num):
        return False
    else:
        return block_num in freeBlockList


def scan_all_inodes():
    #   TO-DO: check is_free_inode instead of simply check if k is in the freeInodeList
    keys = inodeDict.keys()
    global flag
    for k in keys:
        i = inodeDict[k]
        #   unallocated I-node should have a type of zero
        #   previously: if i.inode_mode == 0
        #   if i.inode_mode <= 0 or i.link_count <= 0:
        if i.inode_mode <= 0:
            #   if k not in freeInodeList:
            if not is_free_inode(k):
                print("UNALLOCATED INODE {} NOT ON FREELIST".format(k))
                flag = 2
        elif i.link_count > 0:
            #   if k in freeInodeList:
            if is_free_inode(k):
                print("ALLOCATED INODE {} ON FREELIST".format(k))
                flag = 2
    for k in range(superBlock.first_nr_inode, 1+superBlock.total_inode_num):
        #   if k not in freeInodeList and k not in keys:
        if not is_free_inode(k) and k not in keys:
            print("UNALLOCATED INODE {} NOT ON FREELIST".format(k))
            flag = 2
    return


def scan_all_blocks():
    keys = blockDict.keys()
    global flag
    for k in keys:
        curr_block = blockDict[k]
        inum = list(curr_block)[0].reference_inode
        block_info = 'BLOCK'
        if list(blockDict[k])[0].block_level == 1:
            block_info = 'INDIRECT BLOCK'
        if list(blockDict[k])[0].block_level == 2:
            block_info = 'DOUBLE INDIRECT BLOCK'
        if list(blockDict[k])[0].block_level == 3:
            block_info = 'TRIPPLE INDIRECT BLOCK'

        #   check if it is invalid
        if k < 0 or k >= superBlock.total_block_num:
            print("INVALID {} {} IN INODE {} AT OFFSET {}".format(block_info, k, inum, list(curr_block)[0].block_offset))
            flag = 2
        #   check if it is duplicated
        elif len(curr_block) > 1:
            for block in curr_block:
                block_info = 'BLOCK'
                if block.block_level == 1:
                    block_info = 'INDIRECT BLOCK'
                if block.block_level == 2:
                    block_info = 'DOUBLE INDIRECT BLOCK'
                if block.block_level == 3:
                    block_info = 'TRIPPLE INDIRECT BLOCK'
                print("DUPLICATE {} {} IN INODE {} AT OFFSET {}".format(block_info, k, block.reference_inode, block.block_offset))
                flag = 2
        #   check if it is free
        elif is_free_block(k):
            print("ALLOCATED BLOCK {} ON FREELIST".format(k))
            flag = 2
        elif k < superBlock.start_of_datablock():
            print("RESERVED {} {} IN INODE {} AT OFFSET {}".format(block_info, k, inum, list(curr_block)[0].block_offset))
            flag = 2

    #   for all data blocks
    for bn in range(superBlock.start_of_datablock(), superBlock.total_block_num):
        #   if bn in freeBlockList or bn in keys:
        if is_free_block(bn) or bn in keys:
            continue
        else:
            print("UNREFERENCED BLOCK {}".format(bn))
            flag = 2
    return


def scan_all_dirents():
    global flag
    keys = inodeDict.keys()
    for entry in dirEntryList:
        parent_inode_num = entry.parent_inode_num
        inode_num = entry.inode_num
        name = entry.entry_name
        if entry.inode_num < 1 or entry.inode_num > superBlock.total_inode_num:
            print("DIRECTORY INODE {} NAME {} INVALID INODE {}".format(entry.parent_inode_num, entry.entry_name, entry.inode_num))
            flag = 2
            continue

        if entry.inode_num in inodeDict.keys():
            inodeDict[entry.inode_num].links += 1

        if name == '\'.\'':
            if parent_inode_num != inode_num:
                print("DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}".format(parent_inode_num, name, inode_num, parent_inode_num))
                flag = 2
        elif name == '\'..\'':
            # gradnparent_inode_num = 0
            # grandparent_inode_num = 0
            if inode_num == 2 or parent_inode_num == 2:
                grandparent_inode_num = 2
            else:
                grandparent_inode_num = parentInode[parent_inode_num]

            if grandparent_inode_num != inode_num:
                print("DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}".format(parent_inode_num, name, inode_num, grandparent_inode_num))
                flag = 2

        #   elif entry.inode_num in freeInodeList and entry.inode_num > superBlock.first_nr_inode:
        #   elif is_free_inode(entry.inode_num):
        elif is_free_inode(entry.inode_num):
            print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(entry.parent_inode_num, entry.entry_name, entry.inode_num))
            flag = 2

        elif entry.inode_num not in keys and entry.inode_num > superBlock.first_nr_inode:
            print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(entry.parent_inode_num, entry.entry_name, entry.inode_num))
            flag = 2

        #   newly added
        #   elif inode_num in keys and (inodeDict[inode_num].inode_mode <= 0 or inodeDict[inode_num].link_count <= 0):
        elif inode_num in keys and inodeDict[inode_num].inode_mode <= 0:
            print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(entry.parent_inode_num, entry.entry_name, entry.inode_num))
            flag = 2

    inodes = inodeDict.values()
    for inode in inodes:
        if inode.inode_mode > 0 and inode.links != inode.link_count:
            print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}".format(inode.inode_number, inode.links, inode.link_count))
            flag = 2


if __name__ == "__main__":

    parse_input_file()
    scan_all_dirents()
    scan_all_blocks()
    scan_all_inodes()

    sys.exit(flag)
