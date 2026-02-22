const char *GREEN = "\033[32m";
const char *RED = "\033[31m";
const char *RESET = "\033[0m";

void dump_regs(const VM *vm) {
        printf("Double-Word Registers (32-bit):\n");
    printf("%sd0%s: 0x%08X  %sd1%s: 0x%08X  %sd2%s: 0x%08X  %sd3%s: 0x%08X\n",
           vm->reg.d0 ? GREEN : RED, RESET, vm->reg.d0,
           vm->reg.d1 ? GREEN : RED, RESET, vm->reg.d1,
           vm->reg.d2 ? GREEN : RED, RESET, vm->reg.d2,
           vm->reg.d3 ? GREEN : RED, RESET, vm->reg.d3);
    printf("%sd4%s: 0x%08X  %sd5%s: 0x%08X  %sd6%s: 0x%08X  %sd7%s: 0x%08X\n",
           vm->reg.d4 ? GREEN : RED, RESET, vm->reg.d4,
           vm->reg.d5 ? GREEN : RED, RESET, vm->reg.d5,
           vm->reg.d6 ? GREEN : RED, RESET, vm->reg.d6,
           vm->reg.d7 ? GREEN : RED, RESET, vm->reg.d7);
    
    printf("\nWord Registers (16-bit):\n");
    printf("%sw0%s: 0x%04X      %sw1%s: 0x%04X      %sw2%s: 0x%04X      %sw3%s: 0x%04X\n",
           vm->reg.w0 ? GREEN : RED, RESET, vm->reg.w0,
           vm->reg.w1 ? GREEN : RED, RESET, vm->reg.w1,
           vm->reg.w2 ? GREEN : RED, RESET, vm->reg.w2,
           vm->reg.w3 ? GREEN : RED, RESET, vm->reg.w3);
    printf("%sw4%s: 0x%04X      %sw5%s: 0x%04X      %sw6%s: 0x%04X      %sw7%s: 0x%04X\n",
           vm->reg.w4 ? GREEN : RED, RESET, vm->reg.w4,
           vm->reg.w5 ? GREEN : RED, RESET, vm->reg.w5,
           vm->reg.w6 ? GREEN : RED, RESET, vm->reg.w6,
           vm->reg.w7 ? GREEN : RED, RESET, vm->reg.w7);
    
    printf("\nByte Registers (8-bit):\n");
    printf("%sb0%s: 0x%02X        %sb1%s: 0x%02X        %sb2%s: 0x%02X        %sb3%s: 0x%02X\n",
           vm->reg.b0 ? GREEN : RED, RESET, vm->reg.b0,
           vm->reg.b1 ? GREEN : RED, RESET, vm->reg.b1,
           vm->reg.b2 ? GREEN : RED, RESET, vm->reg.b2,
           vm->reg.b3 ? GREEN : RED, RESET, vm->reg.b3);
    printf("%sb4%s: 0x%02X        %sb5%s: 0x%02X        %sb6%s: 0x%02X        %sb7%s: 0x%02X\n",
           vm->reg.b4 ? GREEN : RED, RESET, vm->reg.b4,
           vm->reg.b5 ? GREEN : RED, RESET, vm->reg.b5,
           vm->reg.b6 ? GREEN : RED, RESET, vm->reg.b6,
           vm->reg.b7 ? GREEN : RED, RESET, vm->reg.b7);
    
    printf("\nSpecial Registers:\n");
    printf("SP: 0x%08X  PC: 0x%08X\n", vm->reg.sp, vm->reg.pc);
    
    printf("\nFlags:\n");
    printf("ZF: %s%d%s  SF: %s%d%s  CF: %s%d%s  OF: %s%d%s\n",
           vm->reg.zf ? GREEN : RED, vm->reg.zf, RESET,
           vm->reg.sf ? GREEN : RED, vm->reg.sf, RESET,
           vm->reg.cf ? GREEN : RED, vm->reg.cf, RESET,
           vm->reg.of ? GREEN : RED, vm->reg.of, RESET);
}

void dump_mem(const VM *vm) {
    printf("\nMemory Dump (non-zero data memory):\n");
    printf("Address  00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  0123456789ABCDEF\n");
    printf("-------  -----------------------  -----------------------  ----------------\n");
    
    for (int i = 0; i < MEMORY_MAX; i += 16) {
        // Check if this line has any non-zero values
        int has_data = 0;
        for (int j = 0; j < 16; j++) {
            if (i + j < MEMORY_MAX && vm->mem.data[i + j].value != 0) {
                has_data = 1;
                break;
            }
        }
        if (!has_data) continue;
        
        // Print address
        printf("%04X     ", i);
        
        // Print hex values
        for (int j = 0; j < 16; j++) {
            if (j == 8) printf(" ");
            if (i + j < MEMORY_MAX) {
                printf("%02X ", vm->mem.data[i + j].value);
            } else {
                printf("   ");
            }
        }
        printf(" ");
        for (int j = 0; j < 16; j++) {
            if (i + j < MEMORY_MAX) {
                unsigned char c = vm->mem.data[i + j].value;
                printf("%c", (c >= 32 && c <= 126) ? c : '.');
            }
        }
        printf("\n");
    }
    
    printf("\nStack Memory Dump (non-zero stack memory):\n");
    printf("Address  00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  0123456789ABCDEF\n");
    printf("-------  -----------------------  -----------------------  ----------------\n");
    
    for (int i = 0; i < MEMORY_MAX; i += 16) {
        // Check if this line has any non-zero values
        int has_data = 0;
        for (int j = 0; j < 16; j++) {
            if (i + j < MEMORY_MAX && vm->mem.stack[i + j].value != 0) {
                has_data = 1;
                break;
            }
        }
        if (!has_data) continue;
        
        // Print address
        printf("%04X     ", i);
        
        // Print hex values
        for (int j = 0; j < 16; j++) {
            if (j == 8) printf(" ");
            if (i + j < MEMORY_MAX) {
                printf("%02X ", vm->mem.stack[i + j].value);
            } else {
                printf("   ");
            }
        }
        printf(" ");
        for (int j = 0; j < 16; j++) {
            if (i + j < MEMORY_MAX) {
                unsigned char c = vm->mem.stack[i + j].value;
                printf("%c", (c >= 32 && c <= 126) ? c : '.');
            }
        }
        printf("\n");
    }
}

void dump_stack(const VM *vm) {
    printf("\nStack Pointer Context (SP = 0x%08X):\n", vm->reg.sp);
    
    // Calculate start and end addresses for stack display
    uint32_t sp_value = vm->reg.sp;
    uint32_t start_addr = (sp_value > 32) ? sp_value - 32 : 0;
    uint32_t end_addr = (sp_value + 32 < MEMORY_MAX) ? sp_value + 32 : MEMORY_MAX - 1;
    
    // Round to 4-byte boundaries for clarity
    start_addr = start_addr & ~0x3;
    end_addr = end_addr & ~0x3;
    
    printf("Address  Value         Int    Char\n");
    printf("-------  ----------    ----   ----\n");
    
    for (uint32_t addr = start_addr; addr <= end_addr; addr += 4) {
        if (addr + 3 < MEMORY_MAX) {
            uint32_t value = read_32(vm->mem.stack, addr);
            if (addr == sp_value) {
                printf("%s%04X ->  0x%08X  %4d    '%c'%s\n", 
                       GREEN, addr, value, value,
                       (value >= 32 && value <= 126) ? (char)value : '.', RESET);
            } else if (value != 0) {
                printf("%04X     0x%08X  %4d    '%c'\n", 
                       addr, value, value,
                       (value >= 32 && value <= 126) ? (char)value : '.');
            }
        }
    }
}

void SystemDump(const VM *vm) {
    dump_regs(vm);
    dump_mem(vm);
    dump_stack(vm);
}
