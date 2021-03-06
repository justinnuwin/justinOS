#include "gdt.h"

#include <stdint.h>
#include "string.h"

extern "C" const struct GdtDescriptor bs_gdt asm("gdt64.descriptor");
extern "C" void *ist1_top;
extern "C" void *ist2_top;
extern "C" void *ist3_top;
extern "C" void *ist4_top;
extern "C" void *ist5_top;
extern "C" void *ist6_top;
extern "C" void *ist7_top;

/*
 * Segment limit and base ignored in long mode (paging is enforced)
 */
struct CodeSegmentDescriptor {
    uint16_t limit_0_15;            // 0 - 15   (8 byte GDT entry)
    uint16_t base_0_15;             // 16 - 32
    uint8_t  base_16_23;            // 32 - 39
    uint8_t  access : 1;            // 40
    uint8_t  readable : 1;          // 41
    uint8_t  conforming : 1;        // 42
    uint8_t  executable : 1;        // 43
    uint8_t  type : 1;              // 44
    uint8_t  protection_level : 2;  // 45 - 46
    uint8_t  present : 1;           // 47
    uint8_t  limit_16_19 : 4;       // 48 - 51
    uint8_t  available : 1;         // 52
    uint8_t  long_mode : 1;         // 53
    uint8_t  default_op_size : 1;   // 54
    uint8_t  granularity : 1;       // 55
    uint8_t  base_24_31;            // 56 - 63
} __attribute__ ((packed));

struct TaskStateSegmentDescriptor {
    uint16_t limit_0_15;            // 0 - 15   (8 byte GDT entry)
    uint16_t base_0_15;             // 16 - 32
    uint8_t  base_16_23;            // 32 - 39
    uint8_t  access : 1;            // 40
    uint8_t  readable : 1;          // 41
    uint8_t  conforming : 1;        // 42
    uint8_t  executable : 1;        // 43
    uint8_t  type : 1;              // 44
    uint8_t  protection_level : 2;  // 45 - 46
    uint8_t  present : 1;           // 47
    uint8_t  limit_16_19 : 4;       // 48 - 51
    uint8_t  available : 1;         // 52
    uint8_t  long_mode : 1;         // 53
    uint8_t  size : 1;              // 54
    uint8_t  granularity : 1;       // 55
    uint8_t  base_24_31;            // 56 - 63
    uint32_t base_32_63;            // 64 - 95  (10 byte extension)
    uint8_t  reserved1;             // 96 - 103
    uint8_t  zero1 : 5;             // 104 - 108
    uint8_t  reserved2 : 3;         // 109 - 111
    uint32_t reserved3;             // 112 - 133
} __attribute__ ((packed));

struct Tss {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    void **ist1;
    void **ist2;
    void **ist3;
    void **ist4;
    void **ist5;
    void **ist6;
    void **ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t io_map_base_addr;
} __attribute__ ((packed));

struct GdtDescriptor {
    uint16_t size;
    void *base;
} __attribute__ ((packed));

struct Gdt {
    uint64_t zero;
    struct CodeSegmentDescriptor csd;
    struct TaskStateSegmentDescriptor tss;
} __attribute__ ((packed));

static struct GdtDescriptor GDTR;

void lgdt(struct Gdt *gdt) {
    GDTR = {sizeof(*gdt) - 1, gdt};

    asm volatile ("lgdt %0" : : "m"(GDTR));
}

void ltr(struct Gdt *gdt) {
    uint16_t tss_offset = (uint16_t)((uint64_t)&(gdt->tss) - (uint64_t)gdt);

    asm volatile ("ltr %0" : : "m"(tss_offset));
}

void copy_bootstrap_gdt(struct Gdt *gdt) {
    memcpy(gdt, bs_gdt.base, bs_gdt.size + 1);
}

struct Tss *setup_tss() {
    static struct Tss tss;
    memset(&tss, 0, sizeof(tss));
    tss.ist1 = &ist1_top;
    tss.ist2 = &ist2_top;
    tss.ist3 = &ist3_top;
    tss.ist4 = &ist4_top;
    tss.ist5 = &ist5_top;
    tss.ist6 = &ist6_top;
    tss.ist7 = &ist7_top;
    tss.io_map_base_addr = sizeof(tss);
    return &tss;
}

void setup_gdt_tss() {
    static struct Gdt gdt;
    copy_bootstrap_gdt(&gdt);
    struct Tss *tss = setup_tss();
    uint16_t tss_limit = sizeof(*tss);
    gdt.tss.limit_0_15 = tss_limit & 0xffffffff;
    gdt.tss.limit_16_19 = (uint8_t)((tss_limit >> 16) & 0xffff);
    gdt.tss.base_0_15 =   (uint16_t) ((uint64_t)tss & 0xffffffff);
    gdt.tss.base_16_23 =  (uint8_t)  ((((uint64_t)tss) >> 16) & 0xffffffff);
    gdt.tss.base_24_31 =  (uint8_t)  ((((uint64_t)tss) >> 24) & 0xffffffff);
    gdt.tss.base_32_63 =  (uint32_t) ((((uint64_t)tss) >> 32) & 0xffffffff);
    gdt.tss.access = 1;
    gdt.tss.executable = 1;
    gdt.tss.present = 1;
    gdt.tss.size = 1;
    lgdt(&gdt);
    ltr(&gdt);
}
