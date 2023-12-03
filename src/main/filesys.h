#include "multiboot.h"

#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "devices.h"

#define FILENAME_SIZE 32    //max chars per file name (occupies 32B of memory in each dentry)
#define PCBNUM 6 //highest is 6 proccesses at one time 
#define FOURKB        4096  //4kB
#define EIGHTKB       0x2000  /* 8KB */
#define _pcb0         0x7E0000 // location for pcb0 
#define _pcb1         0x7DF000 // location for pcb1
#define _pcb2         0x7DE000 // location for pcb2 
#define _pcb3         0x7DD000 // location for pcb3 
#define _pcb4         0x7DC000 // location for pcb4
#define _pcb5         0x7DB000 // location for pcb5 //For some reason < 0x7D8000 was casuing pcb_5 to become NULL



#define PCB0 0x7E0000 //this is where pcb should be located for process 1 
#define ESP0 0x800000   /* esp0 start */
// #define PCB1 0x7FA000 //this is where the pcb should be located for process 2 
// #define ESP1 0x7FBFFF 
#define NUM_FDT_ENTRIES     8   /* only 8 fdt entries allowed */
#define STDIN_FDT           0   /* for stdin */
#define STDOUT_FDT          1   /* for stdout */

#define FILE_TYPE           0
#define MAX_FD              7
#define START_FDT_FILES     2
#define FD_FLAG             1
#define MAX_FILENAME_LENGTH 32

/*global variable for dir_read*/
int32_t dir_read_idx;
int32_t terminal1_switch;
int32_t terminal2_switch;
int32_t terminal3_switch;

int secondTerm;
int thirdTerm; 
int wake;


/*----------structs needed----------*/ 
typedef struct dentry {
    int8_t file_name[FILENAME_SIZE];
    int32_t file_type;
    int32_t inode_num;
    int8_t reserved[24];    //24 reserved bits
} dentry_t;

typedef struct inode {
    int32_t length; //num of data blocks used by file
    int32_t data_block_num[1023];   //1023 entries
} inode_t;

typedef struct data_block {
    uint8_t byte[FOURKB];
} data_block_t;

//this is the struct for the pointer to file operations to a file descriptor array entry 
typedef struct file_operations
{
    int32_t (*read) (int32_t fd, void * buf, int32_t nbytes); 
    int32_t (*write) (int32_t fd, const void * buf, int32_t nbytes); 
    int32_t (*open) (const uint8_t * filename); 
    int32_t (*close) (int32_t fd); 
} file_operations_t; 

// entry into PCB "file descriptor array"
typedef struct file {
    file_operations_t fotp; //file operations table
    int32_t  inode_num; 
    int32_t file_pos;
    int16_t flags[2]; //file_type in the flags[0], active in flags[1]
} file_t;

//process file descriptor table
typedef struct file_descriptor_table {
    file_t fdt[NUM_FDT_ENTRIES];
} file_descriptor_table_t;

typedef struct boot_block {
    int32_t dir_count;
    int32_t inode_count;
    int32_t data_count;
    int8_t reserved[52]; //52 reserved bytes
    dentry_t direntries[63]; //max of 63 files
} boot_block_t;

typedef struct filesys {
    boot_block_t * boot_block_base;
    inode_t * inode_base;
    data_block_t * data_block_base;

} filesys;

//this is the pcb file struct 
typedef struct pcb 
{
    int32_t pid;            // gives id to process
    int32_t parentID;       // gives pid of parent process (-1 for main shell)       
    file_descriptor_table_t fd_array;   // file descriptors (0-7 files)
    int32_t has_child;      // shell is a parent process
    int32_t saved_esp;      // esp that is updated during scheduling
    int32_t saved_ebp;      // ebp that is updated during scheduling
    int32_t original_esp;   // original esp set during execute
    int32_t original_ebp;   // original ebp set during execute
    int32_t active;         // flag for active
    uint8_t * cmd_;         // (Not used anymore) User level pointer to command
    int32_t terminalNum;    // Num of terminal
    int32_t mainShell;      //  if main process in termianl 1 (not really used anymore)
    int32_t is_waiting;     // if process is waiting for keyboard input (enter makes 0)
    int8_t * name;          // kernel cmd
}   pcb_t; 

/*pcb global vars*/
pcb_t * pcb0;
pcb_t * pcb1;
pcb_t * pcb2;
pcb_t * pcb3;
pcb_t * pcb4;
pcb_t * pcb5;

pcb_t * pcb_array[6];   // max 6 prcoesses
/*global var for currenrt pcb active*/
int currentTerminal; //holds which terminal we are currently in 


filesys fs_info;    //global var for filesys pointers 

/*init (populate fs_info)*/
void file_sys_init(multiboot_info_t* mbi);

/* helper functions specified by docs*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);


/*basic functions*/
int32_t file_open(const uint8_t * filename);
int32_t file_close(int32_t fd);
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes);
//add param filename since no FDT in PCB yet
int32_t file_read(int32_t fd, void * buf, int32_t nbytes);

int32_t directory_open(const uint8_t * filename);
int32_t directory_close(int32_t fd);
int32_t directory_write(int32_t fd, const void * buf, int32_t nbytes);
//add param filename since no FDT in PCB yet
int32_t directory_read(int32_t fd, void * buf, int32_t nbytes);

/*Gets next index*/
int getNextPCBIndex(); 

/*gets number of current processes within terminal */ 
int getNumProcess(int terminal); 

/*resets pcb*/
int32_t init_pcb(pcb_t * cur_pcb);




