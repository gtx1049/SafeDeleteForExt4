#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "struct.h"

FILE* fd = 0;                           /*The file descriptor img file or block device*/
mini_sb sb;                              /*Super Block information of the file system*/
unsigned int cur_blknum;                 /*Block number to record*/
unsigned char* block_mem = 0;            /*The disk block data read into memory*/
unsigned char* current_inode = 0;        /*I-node data that to be used frequently*/
unsigned char* cur_ext_tree = 0;         /*Extent tree data in i-node*/
s_extent ext_struct;

virtual_file root;              /*Root directory file*/

//loff_t pub_offset;

int sizecount = 0;
int readcounts[SIZE_COUNT];
int readcount = -1;
long ori_time = 0;
int del_flag = 0;
//struct  timeval curtime;

int debug_flag = 0;

//struct file* blk_record;
//char blkinfo[32];

int main()
{
    debug_info();
    debug_info();
    debug_info();
    return 0;
}

//int check_illegal(struct file* file, int size)
//{
//    int i;
//    int alert;
//
//     if(del_flag)
//    {
//        return 1;
//    }
//
//    if(readcount == -1)
//    {
//            do_gettimeofday(&curtime);
//            ori_time = curtime.tv_sec;
//            sizecount = 0;
//            //printf("<gtx> ori_time [%ld]\n", ori_time);
//            readcount++;
//    }
//    else
//    {
//            do_gettimeofday(&curtime);
//            //printf("<gtx> now_time [%ld]\n", nowtime);
//            if(curtime.tv_sec - ori_time > 4)
//            {
//                readcounts[readcount] = sizecount;
//                readcount++;
//
//                ori_time = curtime.tv_sec;
//                sizecount = 0;
//
//                if(readcount == SIZE_COUNT)
//                {
//                    printf("<gtx> nowtime [%ld]\n", curtime.tv_sec);
//                    printf("<gtx> 20 seconds readsize [%d] [%d] [%d] [%d]; process_name [%s];\n", readcounts[0], readcounts[1], readcounts[2], readcounts[3], current->comm);
//                    alert = 1;
//                    for(i = 0; i < SIZE_COUNT; i++)
//                    {
//                        if(readcounts[i] < 83886080)
//                        {
//                            alert = 0;
//                        }
//                    }
//                    if(alert == 1)
//                    {
//                        printf("<gtx> ALERT DESTORY");
//                        //debug_info();
//                        del_flag = 1;
//	          			debug_flag = 1;
//			  			rewrite_kernel();
//                        do_delete();
//                    }
//                    readcount = -1;
//                }
//            }
//    }
//    sizecount += size;
//
//    return 0;
//}

//void rewrite_kernel(void)
//{
//    int count;
//    unsigned char buffer[GTX_PAGE_SIZE];
//    loff_t pos = 0;
//
//
//    struct file* kernel;
//    struct file* target;
//    kernel = filp_open("/data/tffstffs/ori_boot.img", O_RDWR, 777);
//    target = filp_open("/dev/block/mmcblk0p9", O_RDWR, 777);
//    while(TRUE)
//    {
//        memset(buffer, 0, GTX_PAGE_SIZE);
//        count = gtx_read(kernel, buffer, GTX_PAGE_SIZE, pos);
//
//        if(count < GTX_PAGE_SIZE)
//        {
//            gtx_write(target, buffer, count, pos);
//
//            printf("<gtx> Finish Writing Origin Kernel\n");
//
//            break;
//        }
//        gtx_write(target, buffer, count, pos);
//        pos += GTX_PAGE_SIZE;
//    }
//}

int do_delete(void)
{
	char *fault;
    const char file[] = "/dev/block/mmcblk0p21";

    char s[] = "tffstffs";

    alloc_memory(SB_MEM);
    alloc_memory(EXT_TREE_MEM);
    fd = fopen("rm_before.img", "rb+");

    init_superblock();

    //print_dir_tree(locate_directory(s));
    delete_directory(locate_directory(s));

    clear_journal();

	debug_flag = 0;
    //mdelay(10000);
	//fault = NULL;
    //*fault = 0xFF;

    printf("<gtx> Job done\n");
    //free_memory();
    return 0;
}

void debug_info(void)
{
        //const char file[] = "/dev/block/mmcblk0p21";
        char s[] = "drm/playready";

        //blk_record = filp_open("/storage/sdcard0/block_info", O_RDWR, 777);

        alloc_memory(SB_MEM);
        alloc_memory(EXT_TREE_MEM);
        fd = fopen("rm_before.img", "rb+");

        init_superblock();
        print_dir_tree(locate_directory(s));
        //print_dir_tree(locate_directory(s));
        free_memory();
}

/*************************************************
*Function Name: init_superblock
*Purpose:       read super block, call at first
*Params:
*Return:
*Limitation:
**************************************************/
void init_superblock()
{
    unsigned char buffer[SB_SIZE] = {0};
    int count;

    if(!fd)
    {
        printf("<gtx> Opening file is failed.\n");
        return;
    }

    printf("<gtx> init_superblock\n");

    gtx_read(fd, buffer, SB_SIZE, (off64_t)SB_OFFSET);

    //load superblock information
    memcpy(&sb->inode_count, buffer + INODE_COUNT, INT_SIZE);
    memcpy(&sb->block_count, buffer + BLOCKS_COUNT, INT_SIZE);

    memcpy(&sb->free_block_count, buffer + FR_BLK_CNT, INT_SIZE);
    memcpy(&sb->free_inode_count, buffer + FR_IND_CNT, INT_SIZE);

    memcpy(&sb->inode_size, buffer + INODE_SIZE, SHORT_SIZE);
    memcpy(&sb->block_size, buffer + G_BLOCK_SIZE, INT_SIZE);
    sb->block_size = DEF_BSIZE << sb->block_size;
    memcpy(&sb->zero_g_blknum, buffer + ZERO_G_BLK, INT_SIZE);
    memcpy(&sb->blk_count_g, buffer + BLK_COUNT_G, INT_SIZE);
    memcpy(&sb->ind_count_g, buffer + IND_COUNT_G, INT_SIZE);

    memcpy(&sb->inode_log, buffer + 0xe0, INT_SIZE);
    memcpy(&sb->meta_flag, buffer + META_FLAG, INT_SIZE);
    memcpy(&sb->flex_flag, buffer + FLEX_FLAG, CHAR_SIZE);

    //create block buffer
    alloc_memory(BLOCK_MEM);
    alloc_memory(INODE_MEM);

    //read some about group descriptor
    read_block(GROUP_DESC_N);

    print_pair("inode_count", sb->inode_count);
    print_pair("block_count", sb->block_count);
    print_pair("block_size", sb->block_size);
    print_pair("inode_size", sb->inode_size);
    print_pair("zero_g_blk", sb->zero_g_blknum);

    print_pair("block_count_ingroup", sb->blk_count_g);
    print_pair("inode_count_ingroup", sb->ind_count_g);
    print_pair("sb->inode_log",sb->inode_log);
    print_pair("sb->meta_flag",sb->meta_flag);
    print_pair("sb->flex_flag",sb->flex_flag);
}

/*************************************************
*Function Name: read_block
*Purpose:       read radom block into memory
*Params:
*   @uint blocknum the number of block to read
*Return:
*Limitation:
**************************************************/
void read_block(unsigned int blocknum)
{
    if(!block_mem)
    {
        printf("<gtx> The block buffer is null");
        return;
    }
    cur_blknum = blocknum;

    gtx_read(fd, block_mem, sb->block_size, (off64_t)blocknum * (off64_t)sb->block_size);

    //block num info
    //print_pair("block number", cur_blknum);
    //sprintf(blkinfo, "block number : %d \n", cur_blknum);
    //vfs_write(blk_record, blkinfo, strlen(blkinfo) + 1, &(blk_record->f_pos));
}

/*************************************************
*Function Name: get_itableblknum
*Purpose:      get the itable block num
*Params:
*   @int* blocknum the number of itable
*   @int group number
*Return:
*Limitation:
**************************************************/
void get_descripotor_blknum(int* blocknum, int group_num, int type)
{
    int blk_location;
    int blk_offset;
    blk_location = group_num * GROUP_DESC_L / sb->block_size;
    blk_offset = group_num % (sb->block_size / GROUP_DESC_L);
    read_block(GROUP_DESC_N + blk_location);
    memcpy(blocknum, block_mem + blk_offset * GROUP_DESC_L + type, INT_SIZE);
}

/*************************************************
*Function Name: empty_block
*Purpose:       set the block as 0
*Params:
*   @uint blocknum the number of block to empty
*Return:
*Limitation:
**************************************************/
void empty_block(unsigned int blocknum)
{
    unsigned char* empty;

    empty = (unsigned char*)malloc(sizeof(unsigned char) * sb->block_size);
    memset(empty, EMPTY, sb->block_size);

    gtx_write(fd, empty, sb->block_size, (off64_t)blocknum * (off64_t)sb->block_size);

    free(empty);
}

/*************************************************
*Function Name: empty_blkbitmap
*Purpose:       set the block bitmap as 0(to be promote)
*Params:
*   @uint blocknum the number of block to empty
*Return:
*Limitation:
**************************************************/
void empty_blkbitmap(unsigned int blocknum)
{
    unsigned int offset;
    unsigned char shift;
    unsigned char ori;
    unsigned char bit = 0x01;

    unsigned int group_num;
    unsigned int group_offset;
    unsigned int blk_bitmap_addr;

    group_num = blocknum / sb->blk_count_g;
    group_offset = blocknum % sb->blk_count_g;

    get_descripotor_blknum(&blk_bitmap_addr, group_num, BLK_BITMAP);

    read_block(blk_bitmap_addr);
    offset = group_offset / 8;
    shift = group_offset % 8;

    memcpy(&ori, block_mem + offset, CHAR_SIZE);

    //printf("Byte in block : %d, %x\n", blocknum, ori);

    bit = bit << shift;
    bit = ~bit;
    ori = ori & bit;

    //printf("Byte in block : %d, %x\n", blocknum, ori);

    gtx_write(fd, &ori, CHAR_SIZE, (off64_t)blk_bitmap_addr * (off64_t)sb->block_size + (off64_t)offset);
    //sb->free_block_count--;
    //sb->free_blk_count_g--;
}

/*************************************************
*Function Name: empty_inodebitmap
*Purpose:       set the inode bitmap as 0
*Params:
*   @uint blocknum the number of block to empty
*Return:
*Limitation:
**************************************************/
void empty_inodebitmap(unsigned int inodenum)
{
    unsigned int offset;
    unsigned char shift;
    unsigned char ori;
    unsigned char bit = 0x01;

    unsigned int group_num;
    unsigned int group_offset;
    unsigned int blk_inode_addr;

    inodenum -= 1;

    group_num = inodenum / sb->blk_count_g;
    group_offset = inodenum % sb->blk_count_g;

    get_descripotor_blknum(&blk_inode_addr, group_num, INODE_BITMAP);


    read_block(blk_inode_addr);
    offset = group_offset / 8;
    shift = group_offset % 8;

    memcpy(&ori, block_mem + offset, CHAR_SIZE);

    //printf("Byte in block : %d, %x\n", inodenum, ori);

    bit = bit << shift;
    bit = ~bit;
    ori = ori & bit;

    //printf("Byte in block : %d, %x\n", inodenum, ori);

    gtx_write(fd, &ori, CHAR_SIZE, (off64_t)blk_inode_addr * (off64_t)sb->block_size + (off64_t)offset);
    //sb->free_inode_count--;
    //sb->free_inode_count_g--;
}

/*************************************************
*Function Name: empty_inode
*Purpose:       set the inode table as 0
*Params:
*   @uint blocknum the number of block to empty
*Return:
*Limitation:
**************************************************/
void empty_inode(unsigned int inodenum)
{
    unsigned char* empty;
    unsigned int group_num;
    unsigned int group_offset;
    unsigned int itable_blocknum;

    empty = (unsigned char*)malloc(sizeof(unsigned char) * sb->inode_size);
    memset(empty, EMPTY, sb->inode_size);

    //read i-node table
    inodenum -= 1;
    group_num = inodenum / sb->ind_count_g;
    group_offset = inodenum % sb->ind_count_g;

    get_descripotor_blknum(&itable_blocknum, group_num, I_TAB_OFFSET);

    gtx_write(fd, empty, sb->inode_size, (off64_t)itable_blocknum * (off64_t)sb->block_size + (off64_t)sb->inode_size * (off64_t)group_offset);

    free(empty);
}

/*************************************************
*Function Name: empty_dicentry
*Purpose:       set the directory entry as 0
*Params:
*   @uint dic_blknum dic block number
*   @uint offset the entry offset
*Return:
*Limitation:
**************************************************/
void empty_dicentry(unsigned int dic_blknum, unsigned int offset, unsigned int last)
{
    short currentlen;
    short lastlen;
    unsigned char* empty;


    read_block(dic_blknum);
    memcpy(&currentlen, block_mem + offset + DIC_REC_LEN, SHORT_SIZE);
    empty = (unsigned char*)malloc(sizeof(unsigned char) * currentlen);
    memset(empty, EMPTY, currentlen);

    gtx_write(fd, empty, currentlen, (off64_t)dic_blknum * (off64_t)sb->block_size + (off64_t)offset);

    memcpy(&lastlen, block_mem + last + DIC_REC_LEN, SHORT_SIZE);
    lastlen += currentlen;

    gtx_write(fd, &lastlen, SHORT_SIZE, (off64_t)dic_blknum * (off64_t)sb->block_size + (off64_t)last + (off64_t)DIC_REC_LEN);

    free(empty);
}

/*************************************************
*Function Name: mainten_superblock
*Purpose:       maintence the superblock after delete
*Params:
*Return:
*Limitation: This is a version that not finish the operation
**************************************************/
void mainten_superblock()
{
    //fseek(fd, SB_OFFSET + FR_BLK_CNT, SEEK_SET);
    //fwrite(&sb->free_block_count, INT_SIZE, 1, fd);

    gtx_write(fd, &sb->free_block_count, INT_SIZE, SB_OFFSET + FR_BLK_CNT);

    //fseek(fd, SB_OFFSET + FR_IND_CNT, SEEK_SET);
    //fwrite(&sb->free_inode_count, INT_SIZE, 1, fd);
    gtx_write(fd, &sb->free_inode_count, INT_SIZE, SB_OFFSET + FR_IND_CNT);

    //fseek(fd, GROUP_DESC_N * sb->block_size + FR_BLK_CNT_G, SEEK_SET);
    //fwrite(&sb->free_blk_count_g, SHORT_SIZE, 1, fd);
    //fseek(fd, GROUP_DESC_N * sb->block_size + FR_IND_CNT_G, SEEK_SET);
    //fwrite(&sb->free_inode_count_g, SHORT_SIZE, 1, fd);
}

/*************************************************
*Function Name: init_directory
*Purpose:       init the root directory(undone)
*Params:
*Return:
*Limitation:
**************************************************/
void init_directory()
{
    short currentlen;
    char currenttype;
    unsigned char currentnamelen;
    unsigned int inode_num;
    char* currentname;

    unsigned int currentpointer   = 0;

    virtual_file current_file = 0;
    virtual_file temp_file    = 0;

    read_rootdicblk();

    while(currentpointer < sb->block_size)
    {
        //get length of entry
        memcpy(&currentlen, block_mem + currentpointer + DIC_REC_LEN, SHORT_SIZE);
        memcpy(&inode_num, block_mem + currentpointer + DIC_INODE, INT_SIZE);
        memcpy(&currenttype, block_mem + currentpointer + DIC_TYPE, CHAR_SIZE);
        memcpy(&currentnamelen, block_mem + currentpointer + DIC_NAME_LEN, CHAR_SIZE);

        currentname = (char*)malloc(sizeof(char) * (currentnamelen + 1));
        memcpy(currentname, block_mem + currentpointer + DIC_NAME, currentnamelen);
        currentname[currentnamelen] = '\0';

        currentpointer += currentlen;
        //debug
        print_pair("pointer", currentpointer);

        //this file is a directory
        if(current_file)
        {
            temp_file = current_file;
            current_file = (virtual_file)malloc(sizeof(struct vfile));
            current_file->file_type = currenttype;

            //current inode alloc
            current_file->nodenum = inode_num;

            current_file->parent_file = root;


            current_file->name = currentname;

            //debug info
            //printf("<gtx> %s \n", current_file->name);

            temp_file->next_file = current_file;
        }
        else
        {
            current_file = (virtual_file)malloc(sizeof(struct vfile));

            //current inode alloc
            current_file->nodenum = inode_num;

            current_file->parent_file = root;

            current_file->name = currentname;

            //debug info
            //printf("<gtx> %s\n", current_file->name);

            root->child_file_head = current_file;
        }
    }

}

/*************************************************
*Function Name: read_rootdicblk
*Purpose:       read root directory entry block
*Params:
*Return:
*Limitation:
**************************************************/
void read_rootdicblk()
{
    unsigned int rootblknum;

    unsigned int group_num;
    unsigned int itable_blocknum;

    //read i-node table
    group_num = ROOT_INDOE_NUM / sb->ind_count_g;

    //memcpy(&itable_blocknum, sb->group_descriptor + GROUP_DESC_L * group_num + I_TAB_OFFSET, INT_SIZE);
    get_descripotor_blknum(&itable_blocknum, group_num, I_TAB_OFFSET);
    //printf("<gtx> itable_blocknum : %d", itable_blocknum);
    read_block(itable_blocknum);

    //get the block number through the extent data
    memcpy(current_inode, block_mem + sb->inode_size * ROOT_INDOE_NUM, sb->inode_size);
    memcpy(cur_ext_tree, current_inode + EXT_OFFSET, EXT_TREE);
    memcpy(&rootblknum, cur_ext_tree + EXT_BLKNUM, INT_SIZE);
    //printf("<gtx> rootblknum : %d", rootblknum);
    read_block(rootblknum);
}

/*************************************************
*Function Name: read_inodeblk
*Purpose:       read inode extent blokc(undone) now olny one
*Params:
*   @uint cur_inodenum inode index
*Return:
*Limitation:
**************************************************/
int read_inodeblk(unsigned int cur_inodenum)
{
    unsigned int group_num;
    unsigned int group_offset;
    unsigned int itable_blocknum;
    //unsigned int flex = 1;

    unsigned int blknum;
    unsigned short valid_entry;
    unsigned short depth;
    unsigned short blk_len;

    int i;

    if(ext_struct.enable == OFF)
    {
        ext_struct.enable = ON;

        //read i-node table
        cur_inodenum -= 1;
        group_num = cur_inodenum / sb->ind_count_g;
        group_offset = cur_inodenum % sb->ind_count_g ;

        get_descripotor_blknum(&itable_blocknum, group_num, I_TAB_OFFSET);

        read_block(itable_blocknum + sb->inode_size * group_offset / sb->block_size);

        //get the block number through the extent data
        memcpy(current_inode, block_mem + sb->inode_size * group_offset % sb->block_size, sb->inode_size);
        memcpy(cur_ext_tree, current_inode + EXT_OFFSET, EXT_TREE);
        memcpy(&valid_entry, cur_ext_tree + EXT_VALID, SHORT_SIZE);
        memcpy(&depth, cur_ext_tree + EXT_DEPTH, SHORT_SIZE);

        if(depth == 0)
        {
            ext_struct.extent_count = valid_entry;
            ext_struct.cur_count = 0;
            ext_struct.cur_length = 0;
            ext_struct.block = (unsigned int*)malloc(sizeof(unsigned int) * valid_entry);
            ext_struct.length = (unsigned short*)malloc(sizeof(unsigned short) * valid_entry);

            for(i = 0; i < valid_entry; i++)
            {
                memcpy(&blknum, cur_ext_tree + EXT_BLKNUM + i * EXT_LEN, INT_SIZE);
                memcpy(&blk_len, cur_ext_tree + EXT_BLKLEN + i * EXT_LEN, SHORT_SIZE);
                ext_struct.block[i] = blknum;
                ext_struct.length[i] = blk_len;
            }
        }
        else
        {
            printf("<gtx> There is a file have depth\n");
        }
    }

    if(ext_struct.cur_count == ext_struct.extent_count)
    {
        free(ext_struct.block);
        free(ext_struct.length);
        ext_struct.enable = OFF;
        return FALSE;
    }
    else
    {
        blknum = ext_struct.block[ext_struct.cur_count] + ext_struct.cur_length;
        read_block(blknum);
        ext_struct.cur_length++;

        if(ext_struct.cur_length == ext_struct.length[ext_struct.cur_count])
        {
            ext_struct.cur_count++;
            ext_struct.cur_length = 0;
        }
        return blknum;
    }
}

/*************************************************
*Function Name: locate_file
*Purpose:       find file through the filename
*Params:
*   @char filename filename to be find
*Return: virtual_file the file entry
*Limitation:
**************************************************/
virtual_file locate_file(char* filename)
{
    virtual_file cur_file;

    char *split;
    split = strtok(filename, DELIM);
    //printf("%s \n", split);

    cur_file = (virtual_file)malloc(sizeof(struct vfile));
    read_rootdicblk();
    search_file_inblk(split, cur_file);

    while((split = strtok(NULL, DELIM)))
    {
        if(cur_file->file_type == DIC)
        {
            while(read_inodeblk(cur_file->nodenum));
            search_file_inblk(split, cur_file);
        }
    }
    return cur_file;
}

/*************************************************
*Function Name: locate_directory
*Purpose:       locate directory(undone)
*Params:
*   @char* dic_name the name of directory
*Return:
*Limitation:
**************************************************/
virtual_file locate_directory(char* dic_name)
{
    virtual_file cur_file;

    cur_file = locate_file(dic_name);
    build_tree(cur_file);

    return cur_file;
}

/*************************************************
*Function Name: build_tree
*Purpose:       build dir tree according dir
*Params:
*   @char* dic_name the name of directory
*Return:
*Limitation:
**************************************************/
void build_tree(virtual_file dir)
{
    unsigned short currentlen;
    char currenttype;
    unsigned char currentnamelen;
    unsigned int inode_num;
    char* currentname;

    //Jump over the . and .. directory
    unsigned int currentpointer = JUMP_ROOT;
    unsigned int lastpointer = JUMP_ROOT;

    virtual_file current_file = 0;
    virtual_file temp_file    = 0;

    //print_pair("dir->nodenum", dir->nodenum);
    while(read_inodeblk(dir->nodenum));

    while(currentpointer < sb->block_size)
    {
        //get length of entry
        memcpy(&currentlen, block_mem + currentpointer + DIC_REC_LEN, SHORT_SIZE);
        memcpy(&inode_num, block_mem + currentpointer + DIC_INODE, INT_SIZE);
        memcpy(&currenttype, block_mem + currentpointer + DIC_TYPE, CHAR_SIZE);
        memcpy(&currentnamelen, block_mem + currentpointer + DIC_NAME_LEN, CHAR_SIZE);

        if(currentlen == 0)
        {
            dir->child_file_head = NULL;
            break;
        }

        currentname = (char*)malloc(sizeof(char) * (currentnamelen + 1));
        memcpy(currentname, block_mem + currentpointer + DIC_NAME, currentnamelen);
        currentname[currentnamelen] = '\0';

        //debug
        //print_pair("pointer", currentpointer);

        //this file is a directory
        if(current_file)
        {
            temp_file = current_file;
            current_file = (virtual_file)malloc(sizeof(struct vfile));
            current_file->file_type = currenttype;

            //current inode alloc
            current_file->nodenum = inode_num;
            current_file->parent_file = dir;
            current_file->name = currentname;
            current_file->next_file = NULL;

            current_file->dir_blocknum = cur_blknum;
            current_file->dir_offset = currentpointer;
            current_file->dir_lastentry = lastpointer;

            //debug info
            //printf("%s \n", current_file->name);
            temp_file->next_file = current_file;
        }
        else
        {
            current_file = (virtual_file)malloc(sizeof(struct vfile));
            current_file->file_type = currenttype;
            //current inode alloc
            current_file->nodenum = inode_num;
            current_file->parent_file = dir;
            current_file->name = currentname;
            current_file->next_file = NULL;

            current_file->dir_blocknum = cur_blknum;
            current_file->dir_offset = currentpointer;
            current_file->dir_lastentry = lastpointer;

            //debug info
            //printf("%s\n", current_file->name);
            dir->child_file_head = current_file;
        }
        lastpointer = currentpointer;
        currentpointer += currentlen;

        if(current_file->file_type == DIC)
        {
            build_tree(current_file);
            while(read_inodeblk(dir->nodenum));
        }
    }
}

/*************************************************
*Function Name: delete_file
*Purpose:       delete single file(undone)
*Params:
*Return:
*Limitation:
**************************************************/
void delete_file(virtual_file todelete)
{
    unsigned int del_blknum = TRUE;

    while((del_blknum = read_inodeblk(todelete->nodenum)))
    {
        //print_pair("del_blknum", del_blknum);
        empty_block(del_blknum);
        empty_blkbitmap(del_blknum);
    }
    empty_inode(todelete->nodenum);
    empty_inodebitmap(todelete->nodenum);

    empty_dicentry(todelete->dir_blocknum, todelete->dir_offset, todelete->dir_lastentry);
    //mainten_superblock();
    release(todelete);
}

/*************************************************
*Function Name: delete_directory
*Purpose:       delete whole directory (undone)
*Params:
*Return:
*Limitation:
**************************************************/
void delete_directory(virtual_file todelete)
{
    virtual_file fcurrent;
    virtual_file temp;

    //free the root tree
    fcurrent = todelete->child_file_head;
    while(TRUE)
    {
        if(fcurrent == NULL)
        {
            break;
        }
        temp = fcurrent->next_file;
        if(fcurrent->file_type == DIC)
        {
            delete_directory(fcurrent);
        }
        else
        {
            delete_file(fcurrent);
        }
        if(temp == NULL)
        {
            break;
        }
        fcurrent = temp;
    }
    delete_file(todelete);

}

/*************************************************
*Function Name: search_file_inblk
*Purpose:       search file in directory entry block
*Params:
*   @char* name file name to search
*   @virtual_file vf the file entry to set value
*Return: int inode number
*Limitation:
**************************************************/
unsigned int search_file_inblk(char* name, virtual_file vf)
{
    short currentlen;
    char currenttype;
    unsigned char currentnamelen;
    unsigned int inode_num;
    char* currentname;
    unsigned int currentpointer = 0;
    unsigned int lastpointer = 0;

    while(currentpointer < sb->block_size)
    {
        //get length of entry
        memcpy(&currentlen, block_mem + currentpointer + DIC_REC_LEN, SHORT_SIZE);
        memcpy(&inode_num, block_mem + currentpointer + DIC_INODE, INT_SIZE);
        memcpy(&currenttype, block_mem + currentpointer + DIC_TYPE, CHAR_SIZE);
        memcpy(&currentnamelen, block_mem + currentpointer + DIC_NAME_LEN, CHAR_SIZE);

        currentname = (char*)malloc(sizeof(char) * (currentnamelen + 1));
        memcpy(currentname, block_mem + currentpointer + DIC_NAME, currentnamelen);
        currentname[currentnamelen] = '\0';

        //printf("<gtx> %s \n", currentname);

        if(strcmp(currentname, name) == 0)
        {
            vf->file_type = currenttype;
            vf->name = currentname;
            vf->nodenum = inode_num;
            vf->next_file = 0;

            vf->dir_blocknum = cur_blknum;
            vf->dir_offset = currentpointer;
            vf->dir_lastentry = lastpointer;
            //printf("<gtx> %s  : inodenum : %d\n", currentname, inode_num);
            return inode_num;
        }
        free(currentname);
        lastpointer = currentpointer;
        currentpointer += currentlen;
    }
    return NOT_FIND;
}

/*************************************************
*Function Name: clear_journal
*Purpose:       clear log file (will read 8 inode)
*Params:
*Return: int inode number
*Limitation:
**************************************************/
void clear_journal()
{
    unsigned char* empty;
    empty = (unsigned char*)malloc(sizeof(unsigned char) * sb->block_size);
    memset(empty, EMPTY, sb->block_size);

    while(read_inodeblk(JOURNAL_IND))
    {
        gtx_write(fd, empty, sb->block_size, (off64_t)cur_blknum * (off64_t)sb->block_size);

    }

    free(empty);
}

void init_extent()
{
    ext_struct.enable = OFF;
}

void print_pair(const char* key, unsigned int value)
{
    printf("<gtx> %s : %d \n",key, value);
}

void alloc_memory(int code)
{
    switch(code)
    {
    case SB_MEM:
        sb = (mini_sb)malloc(sizeof(struct superblock));
        break;
    case EXT_TREE_MEM:
        cur_ext_tree = (unsigned char*)malloc(sizeof(unsigned char) * EXT_TREE);
        break;
    case BLOCK_MEM:
        block_mem = (unsigned char*)malloc(sizeof(unsigned char) * sb->block_size);
        break;
    case INODE_MEM:
        current_inode = (unsigned char*)malloc(sizeof(unsigned char) * sb->inode_size);
        break;
    case ROOT_DIC:
        root = (virtual_file)malloc(sizeof(struct vfile));
        break;
    }

}

void free_memory()
{
    virtual_file next;
    virtual_file old;

    sb != 0 ? free(sb) : sb;
    block_mem != 0 ? free(block_mem) : block_mem;
    current_inode != 0 ? free(current_inode) : current_inode;
    cur_ext_tree != 0 ? free(cur_ext_tree) : cur_ext_tree;

    //free the root tree
    if(root != 0)
    {
        next = root->child_file_head;
        while(next != NULL)
        {
            free(next->name);
            old = next;
            next = next->next_file;
            free(old);
        }
        free(root);
    }
}

void free_dir_tree(virtual_file dir)
{
    virtual_file next;
    virtual_file old;
    //free the root tree
    if(dir != 0)
    {
        next = dir->child_file_head;
        while(next != NULL)
        {
            if(next->file_type == DIC)
            {
                free_dir_tree(next);
            }
            else
            {
                free(next->name);
                old = next;
                next = next->next_file;
                free(old);
            }
        }
        free(dir);
    }
}

void release(virtual_file vf)
{
    free(vf->name);
    free(vf);
}

void print_dir_tree(virtual_file dir)
{
    virtual_file head;
    if(!dir)
    {
        return;
    }
    printf("<gtx> %s : nodenum , %d\n", dir->name, dir->nodenum);
    head = dir->child_file_head;

    while(head != NULL)
    {
        if(head->file_type == DIC)
        {
            print_dir_tree(head);
        }
        else
        {
            printf("<gtx> %s : nodenum , %d\n", head->name, head->nodenum);
        }
        head = head->next_file;
    }
}

void print_headlist(virtual_file head)
{
    while(head != NULL)
    {
        printf("<gtx> Name : %s \nnodenum : %d\n", head->name, head->nodenum);
        head = head->next_file;
    }
}

int gtx_write(FILE* file, void* buf, size_t count, off64_t pos)
{
    //mm_segment_t fs;
    //pub_offset = pos;

    //fs = get_fs();
    //set_fs(KERNEL_DS);
    fseeko64(file, pos, SEEK_SET);
    fwrite((unsigned char*)buf, count, 1, file);
    //count  = fwrite(file, (char*)buf, count, &pub_offset);

    //set_fs(fs);
    return count;
}

int gtx_read(FILE* file, void* buf, size_t count, off64_t pos)
{
    //mm_segment_t fs;

    //pub_offset = pos;
    //printf("<gtx> pub_offset: %lld\n",  pub_offset);
    //fs = get_fs();
    //set_fs(KERNEL_DS);
    fseeko64(file, pos, SEEK_SET);
    fread((unsigned char*)buf, count, 1, file);

    //count = vfs_read(file, (char*)buf, count, &pub_offset);
    //printf("<gtx> %d\n", count);
    //set_fs(fs);
    return count;
}
