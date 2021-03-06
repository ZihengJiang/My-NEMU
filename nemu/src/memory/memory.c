#include "common.h"
#include "memory/l1_cache.h"
#include "cpu/reg.h"
#include "x86-inc/mmu.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
    // Log("hwaddr_read: addr = 0x%x", addr);
    return L1_cache_read(addr, len);
	// uint32_t res = dram_read(addr, len) & (~0u >> ((4 - len) << 3));
    // return res;
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
    // Log("addr = 0x%x, len = %d, data = 0x%x", addr, len, data);
    L1_cache_write(addr, len, data);
	// dram_write(addr, len, data);
}

hwaddr_t page_translate(lnaddr_t addr) {
    LinearAddress lnaddr;
    lnaddr.val = addr;
    Log("lnaddr = 0x%x", addr);
    // Log("lnaddr.offset = 0x%x", lnaddr.offset);
    // Log("lnaddr.page   = 0x%x", lnaddr.page);
    // Log("lnaddr.dir    = 0x%x", lnaddr.dir);
    // Log("CR3 = 0x%x", cpu.cr3.val);

    uint32_t pdir_entry_addr   = (cpu.cr3.page_directory_base << 12) + (lnaddr.dir << 2);
    Log("pdir_entry_addr = 0x%x", pdir_entry_addr);
    PDE pdir_entry;
    pdir_entry.val = hwaddr_read(pdir_entry_addr, 4);
    Log("pdir_entry = 0x%x", pdir_entry.val);
    Assert(pdir_entry.present, "Page directory entry's present bit is 0.");

    uint32_t ptable_entry_addr = (pdir_entry.page_frame << 12) + (lnaddr.page << 2);
    Log("ptable_entry_addr = 0x%x", ptable_entry_addr);
    PTE ptable_entry;
    ptable_entry.val = hwaddr_read(ptable_entry_addr, 4);
    Log("ptable_entry = 0x%x", ptable_entry.val);
    Assert(ptable_entry.present, "Page table entry's present bit is 0.");

    addr = (ptable_entry.page_frame << 12) + lnaddr.offset;
    Log("hwaddr = 0x%x", addr);
    return addr;
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
#ifdef PAGING
    if (cpu.cr0.protect_enable && cpu.cr0.paging)
        addr = page_translate(addr);
#endif
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
#ifdef PAGING
    if (cpu.cr0.protect_enable && cpu.cr0.paging)
        addr = page_translate(addr);
#endif
	hwaddr_write(addr, len, data);
}

lnaddr_t seg_translate(swaddr_t swaddr, uint8_t sreg) {
    // Log("swaddr = 0x%x", swaddr);
    Assert(cpu.cr0.protect_enable, "CR0 protect enable bit is not set!");
    // Log("segment");
    uint32_t segdesc_addr = cpu.gdtr.base + SREG(sreg).index;
    // Log("addr = 0x%x", segdesc_addr);

    SegDesc *segdesc = (SegDesc *)malloc(sizeof(SegDesc));
    segdesc->val[0] = lnaddr_read(segdesc_addr, 4);
    segdesc->val[1] = lnaddr_read(segdesc_addr + 4, 4);
    // int i;
    // for(i = 0; i < 2; ++i)
    //     Log("segdesc: 0x%08x", segdesc->val[i]);

    uint32_t lnaddr = swaddr + (segdesc->base_31_24 << 24) +
        (segdesc->base_23_16 << 16) + segdesc->base_15_0;
    // Log("lnaddr = 0x%x", lnaddr);
    // Assert(swaddr == lnaddr, "swaddr == lnaddr");
    return lnaddr;
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
#ifdef SEGMENTATION
    if (cpu.cr0.protect_enable)
        addr = seg_translate(addr, sreg);
#endif
	return lnaddr_read(addr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
#ifdef SEGMENTATION
    if (cpu.cr0.protect_enable)
        addr = seg_translate(addr, sreg);
#endif
	lnaddr_write(addr, len, data);
}

