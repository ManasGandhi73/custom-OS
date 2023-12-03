#include "x86_desc.h"

#define  PDBITMASK          0xFFC00000
#define  PTBITMASK          0x003FF000
#define _4MBOFFSET          22
#define _4KBOFFSET          12
#define NUM_PAGE_ENTRIES    1024
#define VID_MEM_INDEX       0xB8
#define _4MB                0x400000
#define _4KB                4096
/* paging_init
 * 
 * initializes paging, setting pde and pte
 * Inputs: None
 * Outputs: None
 * Side Effects: Allows for virtual memory to be moved to physical memory
 */
void paging_init(){

    // info on initialization at https://courses.engr.illinois.edu/ece391/sp2023/secure/references/descriptors.pdf
    // Need two PDE entries, 1 4MB w/ PT, 1 4MB continuous

    /*
    *    Page-Table Base Address: Upper 20 bits of the physical address of a 4K-aligned page table. 
    *       In MP3, kernel physical and virtual addresses are the same, 
    *       so the page table base address should be just a label or variable name from their code. 
    *    Avail: Not used in MP3
    *    G: Set the bit for kernel page(s) only, since the kernel is mapped into every process’s address space, 
    *       so kernel pages not flushed when changing to new program or address space (when cr3 changes).
    *    PS: If 0, this is a 4K page directory entry. If 1, this is a 4M page directory entry.
    *    A: Not used in MP3
    *    PCD: The bit should be 1 for program code and data pages (kernel pages, program pages).
    *    PWT: Always want writeback, so this bit should always be 0.
    *    U/S: This bit should be set to 1 for all user-level pages and memory ranges, and 0 for kernel pages.
    *    R/W: For MP3, mark all pages read/write
    *    P: All valid PDEs need to have their present bit set to 1.
    * 
    *    PAT: Not used in MP3
    *    D: Not used in MP3
    *    Reserved: These bits must be set to 0, or they will get an invalid PDE exception
    */

   // Initialize 0-4 MB table
   // shift 12 bits right b/c of address offset in diagram
    pde[0].KB4_address = ((uint32_t) pte >> _4KBOFFSET);
    pde[0].KB4_available = 0;
    // ignored for 4kb PDE
    pde[0].KB4_global = 0;
    pde[0].KB4_page_size = 0;
    pde[0].KB4_accessed = 0;
    pde[0].KB4_cache_dis = 0;
    pde[0].KB4_Wthrough = 0;
    pde[0].KB4_US = 0; //changed this to 1 because we should be able to access it in user space? 
    pde[0].KB4_RW = 1; 
    pde[0].KB4_present = 1;
    pde[0].KB4_dirty = 0;

    // Initialize 4-8 MB PG (KERNEL)
    // starts at 4 MB(since pdt takes up first 0x3FFFFF), so = 1.
    // since 1 in this context = 0100 0000 0000 0000 0000 0000
    //                         = 0x400000 = 4MB offset
    pde[1].MB4_address = _4MB >> _4MBOFFSET;

    pde[1].MB4_available = 0;
    pde[1].MB4_global = 1;
    pde[1].MB4_page_size = 1;
    pde[1].MB4_accessed = 0;
    pde[1].MB4_cache_dis = 1;
    pde[1].MB4_Wthrough = 0;
    pde[1].MB4_US = 0;
    pde[1].MB4_RW = 1; 
    pde[1].MB4_present = 1;
    pde[1].MB4_PAT = 0;
    pde[1].MB4_dirty = 0;
    pde[1].MB4_res = 0;
    

    // for the rest of the pde entries, not present (1024 entries)
    int i;
    for(i = 2; i < NUM_PAGE_ENTRIES; i++) {
        pde[i].KB4_address = 0;
        pde[i].KB4_available = 0;
        pde[i].KB4_global = 0;
        pde[i].KB4_page_size = 0;
        pde[i].KB4_accessed = 0;
        pde[i].KB4_cache_dis = 0;
        pde[i].KB4_Wthrough = 0;
        pde[i].KB4_US = 0;
        pde[i].KB4_RW = 1; 
        // set rest to not present
        pde[i].KB4_present = 0;
        pde[i].KB4_dirty = 0;
    }


    // intialize pte
    // Video memory is at B8000, offset = actual address
    // because pdt is first 0-4MB.
    // when VID_MEM_INDEX since right shifted by 12
    for(i = 0; i < NUM_PAGE_ENTRIES; i++) {
        pte[i].PT_address = 0;
        pte[i].PT_available = 0;
        pte[i].PT_global = 0;
        pte[i].PT_accessed = 0;
        pte[i].PT_cache_dis = 0;
        pte[i].PT_Wthrough = 0;
        pte[i].PT_US = 0;
        pte[i].PT_RW = 1; 
        // set all to not present
        pte[i].PT_present = 0;
        pte[i].PT_PAT = 0;
        pte[i].PT_dirty = 0;
    }
    // Will crash kernel if this address isn't set to VID_MEM_INDEX
    pte[VID_MEM_INDEX].PT_address = VID_MEM_INDEX;
    pte[VID_MEM_INDEX].PT_available = 0;
    pte[VID_MEM_INDEX].PT_global = 0;
    pte[VID_MEM_INDEX].PT_accessed = 0;
    pte[VID_MEM_INDEX].PT_cache_dis = 0;
    pte[VID_MEM_INDEX].PT_Wthrough = 0;
    pte[VID_MEM_INDEX].PT_US = 0; 
    pte[VID_MEM_INDEX].PT_RW = 1;
    // change present to 1 
    pte[VID_MEM_INDEX].PT_present = 1;
    pte[VID_MEM_INDEX].PT_PAT = 0;
    pte[VID_MEM_INDEX].PT_dirty = 0;

    // 128 +  8MB, 4kb for fish
    for(i = 0; i < NUM_PAGE_ENTRIES; i++) {
        pte_video[i].PT_address = 0;
        pte_video[i].PT_available = 0;
        pte_video[i].PT_global = 0;
        pte_video[i].PT_accessed = 0;
        pte_video[i].PT_cache_dis = 0;
        pte_video[i].PT_Wthrough = 0;
        pte_video[i].PT_US = 0;
        pte_video[i].PT_RW = 1; 
        // _videoset all to not present
        pte_video[i].PT_present = 0;
        pte_video[i].PT_PAT = 0;
        pte_video[i].PT_dirty = 0;
    }

    // Call assembly function in x86.S to set cr3 to 
    // start of pde, enable 4MB pages (cr4), and enable paging (cr0)
    load_and_enable_PD(pde);


}

/* Paging Helpers */

/* map
 * maps virtual address to physical address
 *
 * In the intel manual - first scans its own address, then the address you feed it
 * so you feed it its own address
 *
 * Inputs:  virt_addr -- virtual address
 *          phys_addr -- physical address
 * Outputs: None
 * Side Effects: None
 */
void map(uint32_t virt_addr, uint32_t phys_addr, int32_t pagetype){
     int pageDirIdx = (virt_addr & PDBITMASK) >> _4MBOFFSET;

    if(pagetype) {
        // set up a single 4 MB page
        // printf("%x\n", pageDirIdx); 
        // printf("%x\n", phys_addr >> 22); 



        // phys_addr >> 22 --> for the page bits in pde 
        pde[pageDirIdx].MB4_address = phys_addr >> _4MBOFFSET;
        pde[pageDirIdx].MB4_available = 0;
        pde[pageDirIdx].MB4_global = 0;
        pde[pageDirIdx].MB4_page_size = 1;
        pde[pageDirIdx].MB4_accessed = 0;
        pde[pageDirIdx].MB4_cache_dis = 1;
        pde[pageDirIdx].MB4_Wthrough = 0;
        pde[pageDirIdx].MB4_US = 1;
        pde[pageDirIdx].MB4_RW = 1; 
        pde[pageDirIdx].MB4_present = 1;
        pde[pageDirIdx].MB4_PAT = 0;
        pde[pageDirIdx].MB4_dirty = 0;
        pde[pageDirIdx].MB4_res = 0;
    }

    if(!pagetype){
        int pageTabIdx = (virt_addr & PTBITMASK) >> _4KBOFFSET;

        if(pageDirIdx != 0 && pageDirIdx != 1)
        {
            //pde maps to pte
            pde[pageDirIdx].KB4_address = ((uint32_t) pte_video >> _4KBOFFSET); // maps to user memory page table that we create (in kernel memory)
            pde[pageDirIdx].KB4_available = 0;
            pde[pageDirIdx].KB4_global = 0;
            pde[pageDirIdx].KB4_page_size = 0;
            pde[pageDirIdx].KB4_accessed = 0;
            pde[pageDirIdx].KB4_cache_dis = 1;
            pde[pageDirIdx].KB4_Wthrough = 0;
            pde[pageDirIdx].KB4_US = 1;
            pde[pageDirIdx].KB4_RW = 1; 
            // spageDirIdxt rest to not present
            pde[pageDirIdx].KB4_present = 1;
            pde[pageDirIdx].KB4_dirty = 0;
        }
        if(pageDirIdx == 0 || pageDirIdx == 1) //if its within 0-4MB
        {
            /* PAGE TABLE ENTRY */
            pte[pageTabIdx].PT_address = phys_addr >> _4KBOFFSET;
            pte[pageTabIdx].PT_available = 0;
            pte[pageTabIdx].PT_global = 0;
            pte[pageTabIdx].PT_accessed = 0;
            pte[pageTabIdx].PT_cache_dis = 0;
            pte[pageTabIdx].PT_Wthrough = 0;
            pte[pageTabIdx].PT_US = 0;
            pte[pageTabIdx].PT_RW = 1;
            // _vipte_videocpageTabIdx to 1 
            pte[pageTabIdx].PT_present = 1;
            pte[pageTabIdx].PT_PAT = 0;
            pte[pageTabIdx].PT_dirty = 0;
        }
        else //this is when vidmap is being executed 
        {
            /* PAGE TABLE ENTRY */
            pte_video[pageTabIdx].PT_address = phys_addr >> _4KBOFFSET;
            pte_video[pageTabIdx].PT_available = 0;
            pte_video[pageTabIdx].PT_global = 0;
            pte_video[pageTabIdx].PT_accessed = 0;
            pte_video[pageTabIdx].PT_cache_dis = 0;
            pte_video[pageTabIdx].PT_Wthrough = 0;
            pte_video[pageTabIdx].PT_US = 1;
            pte_video[pageTabIdx].PT_RW = 1;
            // _vipte_videocpageTabIdx to 1 
            pte_video[pageTabIdx].PT_present = 1;
            pte_video[pageTabIdx].PT_PAT = 0;
            pte_video[pageTabIdx].PT_dirty = 0;
        }
    }


    
    flush(pde);
}


/* unmap
 * unmaps virtual address from physical address
 *
 * In the intel manual - first scans its own address, then the address you feed it
 * so you feed it its own address
 *
 * Inputs:  virt_addr -- virtual address
 *          phys_addr -- physical address
 * Outputs: None
 * Side Effects: None
 */
void unmap(uint32_t virt_addr, int32_t pagetype){
    // 4 MB
    if(pagetype) {
        int pageDirIdx = (virt_addr & PDBITMASK) >> _4MBOFFSET;
        pde[pageDirIdx].MB4_address = virt_addr;
    }
    // 4 KB
    if(!pagetype) {
        int pageTabIdx = (virt_addr & PTBITMASK) >> _4KBOFFSET;
        pte_video[pageTabIdx].PT_address = virt_addr; 



    }

    flush(pde);
}
