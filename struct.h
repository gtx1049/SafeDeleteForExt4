#ifndef STRUCT_H_INCLUDED
#define STRUCT_H_INCLUDED

#define INT_SIZE       4
#define SHORT_SIZE     2
#define CHAR_SIZE      1

#define SB_MEM         0
#define BLOCK_MEM      1
#define INODE_MEM      2
#define EXT_TREE_MEM   3
#define ROOT_DIC       4
#define ROOT_INODE     2
#define JOURNAL_IND    8

#define GROUP_DESC_L  32
#define EXT_TREE      60
#define SB_SIZE     1024
#define SB_OFFSET   1024
#define DEF_BSIZE   1024

#define INODE_COUNT  0x0
#define BLOCKS_COUNT 0x4
#define G_BLOCK_SIZE  0x18
#define FR_BLK_CNT   0xC
#define FR_IND_CNT  0x10
#define FR_BLK_CNT_G 0xC
#define FR_IND_CNT_G 0xE
#define FR_D_CNT_G  0x10
#define ZERO_G_BLK  0x14
#define BLK_COUNT_G 0x20
#define IND_COUNT_G 0x28
#define INODE_LOG   0xE0
#define META_FLAG  0x104
#define FLEX_FLAG  0x174
#define INODE_SIZE  0x58
#define EXT_LEN      0xC
#define EXT_OFFSET  0x28
#define EXT_VALID    0x2
#define EXT_DEPTH    0X6
#define EXT_BLKLEN  0x10
#define EXT_BLKNUM  0x14
#define I_TAB_OFFSET 0x8
#define BLK_BITMAP   0x0
#define INODE_BITMAP 0x4

#define DIC_INODE    0x0
#define DIC_REC_LEN  0x4
#define DIC_NAME_LEN 0x6
#define DIC_TYPE     0x7
#define DIC_NAME     0x8
#define JUMP_ROOT   0x18

#define GROUP_DESC_N   1
#define ROOT_INDOE_NUM 1

#define DELIM        "/"
#define NOT_FIND      -1
#define ON             1
#define OFF            0
#define TRUE           1
#define FALSE          0

#define DIC          0x2
#define REGFILE      0x1
#define EMPTY        0x0

#define GTX_PAGE_SIZE 1024

typedef struct superblock*       mini_sb;
typedef struct vfile*       virtual_file;
typedef struct extent_tree      s_extent;

struct superblock
{
    unsigned int inode_count;
    unsigned int block_count;
    unsigned int free_block_count;
    unsigned int free_inode_count;
    unsigned int block_size;
    unsigned short inode_size;
    unsigned int zero_g_blknum;
    unsigned int blk_count_g;
    unsigned int ind_count_g;

    unsigned int inode_log;
    unsigned int meta_flag;
    unsigned char flex_flag;
};

struct vfile
{
    virtual_file     parent_file;
    virtual_file child_file_head;
    virtual_file       next_file;
    unsigned int       file_type;
    char* name;

    //directory entry info
    unsigned int dir_blocknum;
    unsigned int dir_offset;
    unsigned int dir_lastentry;

    //inode data
    unsigned int nodenum;
};

struct extent_tree
{
    unsigned int enable;
    unsigned int extent_count;
    unsigned int cur_count;
    unsigned int cur_length;

    unsigned int* block;
    unsigned short* length;
};

#define SIZE_COUNT 4

extern int readcounts[SIZE_COUNT];
extern int readcount;
extern long ori_time;
extern int del_flag;
extern int size_count;

extern int debug_flag;

int do_delete(void);
//int check_illegal(struct file*, int);
//void rewrite_kernel(void);
void debug_info(void);

void print_pair(const char*, unsigned int);
void alloc_memory(int);
void free_memory(void);
void print_dir_tree(virtual_file);
void free_dir_tree(virtual_file);
void print_headlist(virtual_file);

void init_superblock(void);
void read_block(unsigned int);
void empty_block(unsigned int);
void empty_blkbitmap(unsigned int);
void empty_inodebitmap(unsigned int);
void empty_inode(unsigned int);
void empty_dicentry(unsigned int, unsigned int, unsigned int);
void mainten_superblock(void);
void init_directory(void);

int read_inodeblk(unsigned int);
void init_extent(void);
void read_rootdicblk(void);
virtual_file locate_file(char*);
virtual_file locate_directory(char*);
void build_tree(virtual_file);
void delete_file(virtual_file);
void delete_directory(virtual_file);
void release(virtual_file);
unsigned int search_file_inblk(char*, virtual_file);
void clear_journal(void);

int gtx_write(FILE*, void*, size_t, off64_t);
int gtx_read(FILE*, void*, size_t, off64_t);

#endif // STRUCT_H_INCLUDED
