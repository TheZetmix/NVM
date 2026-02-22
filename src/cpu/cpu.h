/*
  this is a file describing the instructions and architecture of the processor.
  
  registers:
  d0 - d7: 32 bit registers
  w0 - w7: 16 bit registers
  b0 - b7: 8 bit registers
  
  flag registers:
  zf, cf, sf, of: 8 bit
  
  special registers:
  sp, pc: 32 bit
  
  all registers are independent of each other and can be changed
  
  memory is divided into 2 parts, main memory (where data is written and the
  executable file is loaded) and stack (exactly the same memory, but only
  certain instructions interact with it)
  
  the memory size can be changed via the MEMORY_MAX constant
  
  each byte has its value and permissions, meaning in the actual memory
  of your machine one memory cell takes up 2 bytes, so be careful when
  changing the amount of VM memory
 */

#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#define MEMORY_MAX 65536
#define CPU_LOAD_ADDR    0x2000 // the processor loads at this address

typedef enum {
    REG_d0,
    REG_d1,
    REG_d2,
    REG_d3,
    REG_d4,
    REG_d5,
    REG_d6,
    REG_d7,

    REG_w0,
    REG_w1,
    REG_w2,
    REG_w3,
    REG_w4,
    REG_w5,
    REG_w6,
    REG_w7,

    REG_b0,
    REG_b1,
    REG_b2,
    REG_b3,
    REG_b4,
    REG_b5,
    REG_b6,
    REG_b7,

    REG_sp,
    REG_pc,
    
    REG_zf,
    REG_sf,
    REG_cf,
    REG_of
} IREG;

typedef struct {
    uint32_t d0, d1, d2, d3, d4, d5, d6, d7;
    uint16_t w0, w1, w2, w3, w4, w5, w6, w7;
    uint8_t  b0, b1, b2, b3, b4, b5, b6, b7;
    
    uint32_t sp, pc;
    uint8_t  zf, sf, cf, of;
} Registers;

typedef struct {
    uint8_t value;
    uint8_t permission;
} MemoryByte;

typedef struct {
    MemoryByte data[MEMORY_MAX];
    MemoryByte stack[MEMORY_MAX];
} Memory;

typedef struct {
    Registers reg;
    Memory mem;
} VM;

void SystemDump(const VM *vm);

VM* init_vm() {
    static VM res = {0};
    res.reg.pc = CPU_LOAD_ADDR;
    res.reg.sp = 0;
    return &res;
}

void error(VM* vm, char* msg) {
    printf("%s 0x%02x\n", msg, vm->reg.pc);
    exit(1);
}

void set_register(VM* vm, int register_offset, uint32_t value) {
    switch (register_offset) {
        case REG_zf: vm->reg.zf = (uint8_t)value; break;
        case REG_cf: vm->reg.cf = (uint8_t)value; break;
        case REG_of: vm->reg.of = (uint8_t)value; break;
        case REG_sf: vm->reg.sf = (uint8_t)value; break;
        
        case REG_sp: vm->reg.sp = (uint32_t)value; break;
        case REG_pc: vm->reg.pc = (uint32_t)value; break;
        
        case REG_d0: vm->reg.d0 = (uint32_t)value; break;
        case REG_d1: vm->reg.d1 = (uint32_t)value; break;
        case REG_d2: vm->reg.d2 = (uint32_t)value; break;
        case REG_d3: vm->reg.d3 = (uint32_t)value; break;
        case REG_d4: vm->reg.d4 = (uint32_t)value; break;
        case REG_d5: vm->reg.d5 = (uint32_t)value; break;
        case REG_d6: vm->reg.d6 = (uint32_t)value; break;
        case REG_d7: vm->reg.d7 = (uint32_t)value; break;
        
        case REG_w0: vm->reg.w0 = (uint16_t)value; break;
        case REG_w1: vm->reg.w1 = (uint16_t)value; break;
        case REG_w2: vm->reg.w2 = (uint16_t)value; break;
        case REG_w3: vm->reg.w3 = (uint16_t)value; break;
        case REG_w4: vm->reg.w4 = (uint16_t)value; break;
        case REG_w5: vm->reg.w5 = (uint16_t)value; break;
        case REG_w6: vm->reg.w6 = (uint16_t)value; break;
        case REG_w7: vm->reg.w7 = (uint16_t)value; break;
        
        case REG_b0: vm->reg.b0 = (uint8_t)value; break;
        case REG_b1: vm->reg.b1 = (uint8_t)value; break;
        case REG_b2: vm->reg.b2 = (uint8_t)value; break;
        case REG_b3: vm->reg.b3 = (uint8_t)value; break;
        case REG_b4: vm->reg.b4 = (uint8_t)value; break;
        case REG_b5: vm->reg.b5 = (uint8_t)value; break;
        case REG_b6: vm->reg.b6 = (uint8_t)value; break;
        case REG_b7: vm->reg.b7 = (uint8_t)value; break;
    }
}

uint32_t get_register(VM *vm, int register_offset) {
    switch (register_offset) {
        case REG_zf: return (uint32_t)vm->reg.zf;
        case REG_cf: return (uint32_t)vm->reg.cf;
        case REG_of: return (uint32_t)vm->reg.of;
        case REG_sf: return (uint32_t)vm->reg.sf;
        
        case REG_sp: return (uint32_t)vm->reg.sp;
        case REG_pc: return (uint32_t)vm->reg.pc;
        
        case REG_d0: return (uint32_t)vm->reg.d0;
        case REG_d1: return (uint32_t)vm->reg.d1;
        case REG_d2: return (uint32_t)vm->reg.d2;
        case REG_d3: return (uint32_t)vm->reg.d3;
        case REG_d4: return (uint32_t)vm->reg.d4;
        case REG_d5: return (uint32_t)vm->reg.d5;
        case REG_d6: return (uint32_t)vm->reg.d6;
        case REG_d7: return (uint32_t)vm->reg.d7;
        
        case REG_w0: return (uint32_t)vm->reg.w0;
        case REG_w1: return (uint32_t)vm->reg.w1;
        case REG_w2: return (uint32_t)vm->reg.w2;
        case REG_w3: return (uint32_t)vm->reg.w3;
        case REG_w4: return (uint32_t)vm->reg.w4;
        case REG_w5: return (uint32_t)vm->reg.w5;
        case REG_w6: return (uint32_t)vm->reg.w6;
        case REG_w7: return (uint32_t)vm->reg.w7;
        
        case REG_b0: return (uint32_t)vm->reg.b0;
        case REG_b1: return (uint32_t)vm->reg.b1;
        case REG_b2: return (uint32_t)vm->reg.b2;
        case REG_b3: return (uint32_t)vm->reg.b3;
        case REG_b4: return (uint32_t)vm->reg.b4;
        case REG_b5: return (uint32_t)vm->reg.b5;
        case REG_b6: return (uint32_t)vm->reg.b6;
        case REG_b7: return (uint32_t)vm->reg.b7;
        
        default: return 0;
    }
}

void write_32(MemoryByte* data, uint16_t address, uint32_t value) {
    data[address].value   = (value >> 24) & 0xFF;
    data[address+1].value = (value >> 16) & 0xFF;
    data[address+2].value = (value >> 8)  & 0xFF;
    data[address+3].value =  value        & 0xFF;
}

uint32_t read_32(const MemoryByte* data, uint16_t address) {
    return ((uint32_t)data[address].value   << 24) |
           ((uint32_t)data[address+1].value << 16) |
           ((uint32_t)data[address+2].value << 8)  |
           ((uint32_t)data[address+3].value);
}

void spush(VM *vm, uint32_t value) {
    vm->reg.sp += 4;
    write_32(vm->mem.stack, vm->reg.sp, value);
}

uint32_t spop(VM *vm) {
    if (vm->reg.sp == 0) {
        error(vm, "Stack underflow");
    }
    uint32_t r = read_32(vm->mem.stack, vm->reg.sp);
    write_32(vm->mem.stack, vm->reg.sp, 0); // clear memory in stack
    vm->reg.sp -= 4;
    return r;
}

/* instruction implementation starts here */

void NOP(VM *vm) {(void)vm;}

void MOV(VM *vm, IREG dest, uint32_t value) {
    set_register(vm, dest, value);
}

void ADD(VM *vm, IREG dest, uint32_t value) {
    set_register(vm, dest, get_register(vm, dest)+value);
}

void SUB(VM *vm, IREG dest, uint32_t value) {
    set_register(vm, dest, get_register(vm, dest)-value);
}

void MUL(VM *vm, IREG dest, uint32_t value) {
    set_register(vm, dest, get_register(vm, dest)*value);
}

void DIV(VM *vm, IREG dest, uint32_t value) {
    set_register(vm, dest, get_register(vm, dest)/value);
}

void INC(VM *vm, IREG dest) {
    set_register(vm, dest, get_register(vm, dest)+1);
}

void DEC(VM *vm, IREG dest) {
    set_register(vm, dest, get_register(vm, dest)-1);
}

void AND(VM *vm, IREG dest, uint32_t value) {
    set_register(vm, dest, get_register(vm, dest)&value);
}

void OR(VM *vm, IREG dest, uint32_t value) {
    set_register(vm, dest, get_register(vm, dest)|value);
}

void XOR(VM *vm, IREG dest, uint32_t value) {
    set_register(vm, dest, get_register(vm, dest)^value);
}

void NOT(VM *vm, IREG dest) {
    set_register(vm, dest, ~get_register(vm, dest));
}

void STR(VM *vm, uint16_t address, uint32_t value) {
    write_32(vm->mem.data, address, value);
}

void LDR(VM *vm, IREG dest, uint32_t address) {
    set_register(vm, dest, read_32(vm->mem.data, address));
}

void STRB(VM *vm, uint16_t address, uint8_t value) {
    vm->mem.data[address].value = value;
}

void LDRB(VM *vm, IREG dest, uint16_t address) {
    set_register(vm, dest, vm->mem.data[address].value);
}

void STRS(VM *vm, uint16_t address, uint32_t value) {
    write_32(vm->mem.stack, address, value);
}

void LDRS(VM *vm, IREG dest, uint32_t address) {
    set_register(vm, dest, read_32(vm->mem.stack, address));
}

void LDAPC(VM *vm, IREG dest, uint32_t address) {
    set_register(vm, dest, CPU_LOAD_ADDR + address);
}

void LDAB(VM *vm, IREG dest, uint16_t base, uint32_t address) {
    set_register(vm, dest, base + address);
}

void PUSH(VM *vm, uint32_t value) {
    spush(vm, value);
}

void POP(VM *vm, IREG dest) {
    set_register(vm, dest, spop(vm));
}

void JMP(VM *vm, uint32_t address) {
    vm->reg.pc = CPU_LOAD_ADDR+address;
}

void JE(VM *vm, uint32_t address) {
    if (vm->reg.zf) vm->reg.pc = CPU_LOAD_ADDR+address;
}

void JNE(VM *vm, uint32_t address) {
    if (!vm->reg.zf) vm->reg.pc = CPU_LOAD_ADDR+address;
}

void JL(VM *vm, uint32_t address) {
    if (vm->reg.sf != vm->reg.of) {
        vm->reg.pc = CPU_LOAD_ADDR + address;
    }
}

void JG(VM *vm, uint32_t address) {
    if (!vm->reg.zf && vm->reg.sf == vm->reg.of) {
        vm->reg.pc = CPU_LOAD_ADDR + address;
    }
}

void JLE(VM *vm, uint32_t address) {
    if (vm->reg.zf || vm->reg.sf != vm->reg.of) {
        vm->reg.pc = CPU_LOAD_ADDR + address;
    }
}

void JGE(VM *vm, uint32_t address) {
    if (vm->reg.sf == vm->reg.of) {
        vm->reg.pc = CPU_LOAD_ADDR + address;
    }
}

void CMP(VM *vm, uint32_t v1, uint32_t v2) {
    uint64_t result = (uint64_t)v1 - (uint64_t)v2;
    uint32_t signed_result = (int32_t)v1 - (int32_t)v2;
    
    vm->reg.zf = (result & 0xFFFFFFFF) == 0;
    vm->reg.sf = (signed_result >> 31) & 1;
    vm->reg.cf = (result >> 32) & 1;
    
    int32_t sv1 = (int32_t)v1;
    int32_t sv2 = (int32_t)v2;
    vm->reg.of = ((sv1 ^ sv2) & (sv1 ^ signed_result)) >> 31;
}

void CALL(VM *vm, uint32_t address) {
    spush(vm, vm->reg.pc);
    vm->reg.pc = CPU_LOAD_ADDR + address;
}

void RET(VM *vm) {
    vm->reg.pc = spop(vm);
}

void PUSHF(VM *vm) {
    uint32_t flags = (vm->reg.zf << 3) | (vm->reg.sf << 2) | 
                     (vm->reg.cf << 1) | vm->reg.of;
    spush(vm, flags);
}

void POPF(VM *vm) {
    uint32_t flags = spop(vm);
    vm->reg.zf = (flags >> 3) & 1;
    vm->reg.sf = (flags >> 2) & 1;
    vm->reg.cf = (flags >> 1) & 1;
    vm->reg.of = flags & 1;
}

#include "./dump.c"

void VMCALL(VM *vm, unsigned int code) {
    switch (code) {
    case 0x10: // VM_PUTC
        putchar(vm->reg.d0);
        fflush(stdout);
        break;
    case 0x11: { // VM_GETC
        char buf = 0;
        struct termios old = {0};
        tcgetattr(0, &old);
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &old);
        read(0, &buf, 1);
        vm->reg.d0 = buf;
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        tcsetattr(0, TCSANOW, &old);
        break;
    }
    case 0x12: // VM_WRITE
        while (vm->mem.data[vm->reg.d0].value != 0)
            putchar(vm->mem.data[vm->reg.d0++].value);
        break;
    case 0x20: // VM_MEMCPY
        // d0 - dest, d1 - src, d2 - size
        for (uint32_t i = 0; i < vm->reg.d2; i++) {
            vm->mem.data[vm->reg.d0 + i].value = 
                vm->mem.data[vm->reg.d1 + i].value;
        }
        break;
    case 0x21: // VM_MEMSET
        // d0 - dest, d1 - value, d2 - size
        for (uint32_t i = 0; i < vm->reg.d2; i++) {
            vm->mem.data[vm->reg.d0 + i].value = vm->reg.d1;
        }
        break;
    case 0xFD: { // VM_TRAP
        printf("trap at pc=%04x\n", vm->reg.pc);
        int running = 1;
        char buf[256];
        while (running) {
            printf("trap> ");
            scanf("%s", buf);
            if (!strcmp(buf, "d")  || !strcmp(buf, "dump")) SystemDump(vm);
            if (!strcmp(buf, "r")  || !strcmp(buf, "regs")) dump_regs(vm);
            if (!strcmp(buf, "m")  || !strcmp(buf, "mem")) dump_mem(vm);
            if (!strcmp(buf, "s")  || !strcmp(buf, "stack")) dump_stack(vm);
            if (!strcmp(buf, "q")  || !strcmp(buf, "quit")) exit(0);
            if (!strcmp(buf, "ct") || !strcmp(buf, "continue")) running = 0;
            if (!strcmp(buf, "c")  || !strcmp(buf, "clear")) system("clear");
            if (!strcmp(buf, "a")  || !strcmp(buf, "around")) {
                for (int i = -5; i < 5; ++i) {
                    int addr = vm->reg.pc + i;
                    if (addr < 0 || addr >= MEMORY_MAX) {
                        printf("%s%04X: -- | -\n", 
                               (addr == 0) ? "> " : "  ", 
                               addr);
                        continue;
                    }
                    uint8_t val = vm->mem.data[addr].value;
                    printf("%s%04X: %02X | %c\n",
                           ((uint32_t)addr == vm->reg.pc) ? "> " : "  ",
                           addr,
                           val,
                           (val >= 32 && val <= 126) ? val : '.');
                }
            }
        }
        break;
    }
    case 0xFE: // VM_DUMP
        SystemDump(vm);
        break;
    case 0xFF: // VM_EXIT
        exit(vm->reg.d0);
        break;
    }
}
