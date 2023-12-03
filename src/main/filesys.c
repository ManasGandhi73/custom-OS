#include "filesys.h"

//filesys fs_info;
file_descriptor_table_t fdt_os;
int32_t fd_os = 0;  // start at 0
dentry_t dummy_dentry;
 //directory read dentry index in BootBlock

/* 
 * file_sys_init
 *   DESCRIPTION: Fills  fs_info
 *   INPUTS: mbi -- pointer to multiboot_info_t struct
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: writes entire fs_info structure
 */
void file_sys_init(multiboot_info_t* mbi){
    module_t* mod = (module_t*)mbi->mods_addr;
    boot_block_t * bb = (boot_block_t*) mod->mod_start; //start of filesys
    //populate fs_info params
    fs_info.boot_block_base = bb; //bootblock ptr
    fs_info.inode_base = (inode_t *)(bb+1); //start of inodes
    fs_info.data_block_base = (data_block_t*) (fs_info.inode_base+ (bb->inode_count)); //start of data blocks

    return;
}

/*----------helper functions specified by docs----------*/
//called in sys_open() system call

/* 
 * read_dentry_by_name
 *   DESCRIPTION: Populates dentry with existing dentry data for a valid fname
 *   INPUTS: fname -- filename
 *           dentry -- pointer to dentry_t struct
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for non-existant fname, 0 on success
 *   SIDE EFFECTS: writes into dentry structure
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    boot_block_t * bb = fs_info.boot_block_base;
    int32_t num_dentries = bb->dir_count; //get number of files in sys 
    // check file name size
    if(strlen((int8_t *)fname) > FILENAME_SIZE) {
        return -1;
    }
    int32_t i = 0;  // set to 0 to start increment
    while(i < num_dentries){ //iterate over direntries in BootBlock indexed by i
        //check fname equals a file_name param at direntries[i]
        if(strncmp(((bb->direntries)[i]).file_name, (int8_t*)fname, FILENAME_SIZE) == 0){   //if the strings are the same
            read_dentry_by_index(i, dentry); //populate dentry
            return 0; // worked
        }
        
        i++; //go to next direntry in bootblock
    }
    return -1;  // means invalid didnt work

}

/* 
 * read_dentry_by_index
 *   DESCRIPTION: Populates dentry with existing dentry data for a valid index
 *   INPUTS: index -- direntry index in boot_block
 *           dentry -- pointer to dentry_t struct
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for invalid index, 0 on success
 *   SIDE EFFECTS: writes into dentry structure
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    boot_block_t * bb = fs_info.boot_block_base;
    int32_t num_dentries = bb->dir_count;

    //index bounds check
    if(index < 0 || index >= num_dentries)  //check bounds
        return -1;  // invalid
    
    dentry_t existing = ((bb->direntries)[index]); //get existing dentry

    //-----populate dentry params based on existing dentry-----
    int32_t j;
    for(j = 0; j < FILENAME_SIZE; j++){ // parse through filename
        dentry->file_name[j] = existing.file_name[j]; //fill in dentry file_name field
        if(j < 24) //24 reserved bytes
            dentry->reserved[j] = existing.reserved[j]; //fill in dentry reserved field
    }
        //puts(dentry->file_name);
    dentry->file_type = existing.file_type; //fill in dentry file_type field
        //printf("file type: %d", dentry->file_type);
    dentry->inode_num = existing.inode_num; //fill in dentry inode_num field
        //printf("inode num: %d", dentry->inode_num);
    return 0;   // worked

}

/* 
 * read_data
 *   DESCRIPTION: Populates buf with file data
 *   INPUTS: inode -- inode num of file to read from
 *           offset -- starting position of data to read from
 *           buf -- buffer to populate
 *           length -- number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for inode num, number of bytes read on success
 *   SIDE EFFECTS: writes into buf
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    //inode bounds and invalid length check cannot be greater than 62
    if(inode < 0 || inode > 63 || length <= 0) //only 63 inodes
        return -1;  //invalid
    
    //pointer to inode based on inode num param
    inode_t * cur_inode = fs_info.inode_base + inode;

    //length bounds check
    if(cur_inode->length < length)
        length = cur_inode->length;
    //offset bounds check  
    if(cur_inode->length < offset)
        return 0;   // just return out, dont need to do anything more
        //printf("length: %d", length);

    int32_t data_block_idx = offset/FOURKB;         //starting data_block num
    int32_t data_block_idx_offset = offset%FOURKB;  //starting offset

    int32_t bytes_read = 0; //havent read any bytes yet
    while(bytes_read < length){
        //check end of data_block, move to next and reset offset
        if(data_block_idx_offset == FOURKB){
            data_block_idx++;
            data_block_idx_offset = 0;  //offset should be 0
        }
        //get ptr to data_block (base of data blocks + data_block_num from inode data_block_num array indexed by data_block_idx)
        data_block_t * cur_block= fs_info.data_block_base + (cur_inode->data_block_num)[data_block_idx];
        //copy data in data_block to buf
        *buf = (cur_block->byte)[data_block_idx_offset];

        //increments
        buf++;
        bytes_read++;
        data_block_idx_offset++;
    }
    return bytes_read;
}

/*----------basic functions----------*/

/* 
 * file_open
 *   DESCRIPTION: Opens a file
 *   INPUTS: 
 *           filename -- directory name
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for invalid params, 0 on success
 *   SIDE EFFECTS: populates dummy_dentry global var
 */
int32_t file_open(const uint8_t * filename){
    //NULL check
    if(!filename || strlen((int8_t*)filename) > MAX_FILENAME_LENGTH)   //if filename is null
        return -1;  //didnt work

    read_dentry_by_name(filename, &dummy_dentry);
    return 0;   //worked 
}

/* 
 * file_close
 *   DESCRIPTION: Closes a file
 *   INPUTS: 
 *           fd -- file descriptor table index
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for invalid params, 0 on success
 *   SIDE EFFECTS: 
 */
int32_t file_close(int32_t fd){
    //fd bounds check
    if(fd < START_FDT_FILES || fd > MAX_FD)    // cant close stdin or stdout
        return -1;  // invlaid
    return 0;   // worked
}

/* 
 * file_write
 *   DESCRIPTION: Writes to a file
 *   INPUTS: 
 *           fd -- file descriptor table index
 *           buf -- buffer
 *           nbytes -- number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: -1 (write not enables)
 *   SIDE EFFECTS: 
 */
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes){
    //add code when WE
    return -1;  // do nothing right now
}

/* 
 * file_read
 *   DESCRIPTION: Reads from file "filename" and writes data into buf
 *   INPUTS: filename -- file name
 *           fd -- file descriptor table index
 *           buf -- buffer
 *           nbytes -- number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for invalid params, number of bytes read on success 
 *   SIDE EFFECTS: populates dummy_dentry global var
 */
int32_t file_read(int32_t fd, void * buf, int32_t nbytes){
    //NULL check
    // if(!filename)
    //     return -1;
    //fd bounds check
    if(fd < 0 || fd > MAX_FD)    
        return -1;  //invalid = return -1

    //changes that I think should be made to filesys, 
    
    //dentry_t d;
    //read_dentry_by_name(filename, &dummy_dentry);  //get inode_num in d
    // if(pcb_array[pcbIndex]->fd_array.fdt[fd].file_pos > nbytes) {
    //     return 0;
    // }
    int32_t bytes = read_data(pcb_array[pcbIndex]->fd_array.fdt[fd].inode_num, pcb_array[pcbIndex]->fd_array.fdt[fd].file_pos, buf, nbytes); //return number of bytes read, populate buf
    pcb_array[pcbIndex]->fd_array.fdt[fd].file_pos += bytes;
    if(bytes == 0) {
        pcb_array[pcbIndex]->fd_array.fdt[fd].file_pos = 0; // reset file position (offset) in file after full file has been read
    }
    return bytes;
}

/* 
 * directory_open
 *   DESCRIPTION: Opens a directory
 *   INPUTS: 
 *           filename -- directory name
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for invalid params, 0 on success
 *   SIDE EFFECTS: populates dummy_dentry global var
 */
int32_t directory_open(const uint8_t * filename){
    //NULL check
    if(!filename)
        return -1;  //null didnt work
    
    read_dentry_by_name(filename, &dummy_dentry);
    return 0;   // worked
}

/* 
 * directory_close
 *   DESCRIPTION: Closes a directory
 *   INPUTS: 
 *           fd -- file descriptor table index
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for invalid params, 0 on success
 *   SIDE EFFECTS: 
 */
int32_t directory_close(int32_t fd){
    //fd bounds check 3-6
    if(fd < START_FDT_FILES || fd > MAX_FD)    //out of boudns for stdin and stdout for close
        return -1;  // return -1 for invalid
    return 0;   // return 0 for valid
}

/* 
 * directory_write
 *   DESCRIPTION: Writes to a directory
 *   INPUTS: 
 *           fd -- file descriptor table index
 *           buf -- buffer
 *           nbytes -- number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: -1 (write not enables)
 *   SIDE EFFECTS: 
 */
int32_t directory_write(int32_t fd, const void * buf, int32_t nbytes){
    //add code when WE
    return -1;  // cant write to directory yet
}

/* 
 * directory_read
 *   DESCRIPTION: Reads the next file_name under directory "." and populated buf
 *   INPUTS: dirname -- directory name
 *           fd -- file descriptor table index
 *           buf -- buffer
 *           nbytes -- number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for invalid params, number of bytes read on success
 *   SIDE EFFECTS: populates dummy_dentry global var
 */
int32_t directory_read(int32_t fd, void * buf, int32_t nbytes){
    //NULL check
    // if(!dirname)
    //     return -1;
    //fd bounds check 
    if(fd < 0 || fd > MAX_FD)    //cant for stdin
        return -1;
    //reading past end of files check
    if(dir_read_idx >= (fs_info.boot_block_base)->dir_count)
        return 0;
    //reading more bytes than file_name contains check (32)
    if(nbytes > 32) // if greater than 32
        nbytes = 32;    // set to 32
    
    //dentry_t d;
    read_dentry_by_index(dir_read_idx, &dummy_dentry);
    strncpy(buf, dummy_dentry.file_name, nbytes); //file_name into buf
    dir_read_idx++; //increment index to read next file
    if(strlen(dummy_dentry.file_name) < MAX_FILENAME_LENGTH) // cannot have more than 32 bytes
        return strlen(dummy_dentry.file_name);
    return FILENAME_SIZE;
}

/* 
 * getNextPCBIndex
 *   DESCRIPTION: gets the index for the next pcb to use
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: pcbIndex -- the next pcb's index, will return -1 if no pcbs are open 
 *   SIDE EFFECTS: None
 */
int getNextPCBIndex()
{
    // pcbIndex++; 
    // if(pcbIndex > PCBNUM-1) // if it exceeds 1
    // {
    //     pcbIndex = 0; // reset to 0
    // }
    int i; 
    for(i = 0; i < PCBNUM; i++)
    {
        if(pcb_array[i]->active == 0)
        {
            pcbIndex = i; 
            return pcbIndex; 
        }
    }
    return -1;
}

/* 
 * getNumProcess
 *   DESCRIPTION: gets the number of processes within a terminal 
 *   INPUTS: terminal number 
 *   OUTPUTS: None
 *   RETURN VALUE: returns the number of processes within this terminal 
 *   SIDE EFFECTS: None
 */
int getNumProcess(int terminal)
{
    int i; 
    int count = 0; 
    for(i = 0; i < PCBNUM; i++)
    {
        if(pcb_array[i]->terminalNum == terminal)
            count++; 
    }
    return count; 
}
