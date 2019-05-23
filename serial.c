#include "serial.h"

#include "asm_functions.h"

#define COM1_PORT 0x3f8
#define COM2_PORT 0x2f8

void init_serial() {
    outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received() {
   return inb(COM1_PORT + 5) & 1;
}

char read_serial() {
   while (serial_received() == 0);

   return inb(COM1_PORT);
}

int is_transmit_empty() {
   return inb(COM1_PORT + 5) & 0x20;
}

void write_serial(char a) {
   while (is_transmit_empty() == 0);

   outb(COM1_PORT,a);
}
