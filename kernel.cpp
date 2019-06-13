#include "kernel.h"

#include "vga.h"
#include "printk.h"
#include "ps2.h"
#include "keyboard.h"
#include "interrupt.h"
#include "gdt.h"
#include "serial.h"
#include "multiboot2_tags.h"
#include "page_table.h"
#include "kmalloc.h"
#include "threading.h"

void wait_a_little() {
    int i = 100000;
    while (i--);
}

void wait_longer() {
    int i = 50000000;
    while (i--);
}

void test(void *) {
    printk("Hello");
}

void kernel_main(void *multiboot2_tag) {
    setup_gdt_tss();
    init_VGA();
    init_COM1();
    init_interrupts();
    if (init_ps2())
        init_keyboard();
    sti();
    read_multiboot2_tags(multiboot2_tag);
    MMU_pf_init();
    init_heap();
    wait_longer();
    clear_screen();

    splash_screen();
    wait_longer();
    wait_longer();
    clear_screen();

    printk("Success!\n");
    sti();
    PROC_create_kthread(test, 0x0);
    while (1) {
        PROC_run();
    }
}
