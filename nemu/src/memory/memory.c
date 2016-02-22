#include "common.h"
#include "memory/l1_cache.h"
#include "memory/segmentation.h"
#include "memory/page.h"

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

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
#ifdef IA32_PAGE
    Log("cr0 = 0x%x", read_cr0());
    if (cpu.cr0.PG)
        addr = page_translate(addr);
#endif
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
#ifdef IA32_PAGE
    if (cpu.cr0.PG)
        addr = page_translate(addr);
#endif
	hwaddr_write(addr, len, data);
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
#ifdef IA32_SEG
    if (cpu.cr0.PE)
        addr = seg_translate(addr, sreg);
#endif
	return lnaddr_read(addr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
#ifdef IA32_SEG
    if (cpu.cr0.PE)
        addr = seg_translate(addr, sreg);
#endif
	lnaddr_write(addr, len, data);
}

