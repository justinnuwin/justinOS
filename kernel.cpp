#include "kernel.h"

#include "vga.h"
#include "printk.h"
#include "ps2.h"
#include "keyboard.h"
#include "interrupt.h"
#include "gdt.h"

void wait_a_little() {
    int i = 100000;
    while (i--);
}

void wait_longer() {
    int i = 50000000;
    while (i--);
}

void kernel_main(void) {
    if (init_interrupts())
        printk("Interrupts enabled!\n");
    if (init_ps2())
        init_keyboard();
    wait_longer();
    clear_screen();

    splash_screen();
    wait_longer();
    wait_longer();
    clear_screen();

    setup_gdt_tss();
    sti();
//    asm ("int $0xd");
    uintptr_t i = 1;
    i = ~i;
    *(int *)i = 10;
    printk("Success!\n");
    while (1) {
    }
}
