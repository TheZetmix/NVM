/*
  to add new instructions, you must first add their processing
  to the assembler, then implement them in the VM
  and implement them in the interpreter

  the header consists of 8 bytes: 4 bytes of the NVMB signature (NvmBinary),
  2 bytes of bytecode location in memory and 2 bytes of entry point address

  the processor boots at address 0x2000, so the start of the
  executable's instructions must be there

  instruction style (in bytes): opcode sign data sign2 data2 ...

  signatures:
  F0 - register
  F1 - number
  F2 - pointer

  argument data:
  register - one byte (register serial number)
  number   - 4 bytes (unsigned 32 bit integer big-endian)
  pointer  - 0 byte (only signature)

  register serial numbers:
  d0 - d7: 0x00 - 0x07 (dword)
  w0 - w7: 0x08 - 0x0F (word)
  b0 - b7: 0x10 - 0x17 (byte)

  all registers are independent of each other

  pointers are a signature before an operand that tells the interpreter to take a
  value from memory at the address that is the value from the operand
  that is, if the interpreter encounters mov d0 *10, the number 10 will be read here as
  a memory address

  the same with registers, for example:
  mov d1 addr
  mov d0 *d1
  will work too
  in this case, the interpreter will take the value from memory located at address d0

  for example, this is what the exit system call would look like in bytecode:
  (implemented in nvmasm):
  01  F0  00 F1  00 00 00 00
  |   |   |  |   |
  mov reg d7 num 0

  22     F1  00 00 00 FF
  |      |   |
  vmcall num 0xFF

  also, this is what the code for working with memory through pointers looks like:
  01  F0  00 F2  F1  00 00 00 00 (suppose the variable address is 0)
  |   |   |  |   |   |
  mov reg d0 ptr num 0
 */

#include "../cpu/cpu.h"
#include "string.h"

enum {
    INSTR_NOP     = 0x00, // 00
    INSTR_MOV     = 0x01, // 01
    INSTR_ADD     = 0x02, // 02
    INSTR_SUB     = 0x03, // 03
    INSTR_MUL     = 0x04, // 04
    INSTR_DIV     = 0x05, // 05
    INSTR_INC     = 0x06, // 06
    INSTR_DEC     = 0x07, // 07
    INSTR_AND     = 0x08, // 08
    INSTR_OR      = 0x09, // 09
    INSTR_XOR     = 0x0A, // 0A
    INSTR_NOT     = 0x0B, // 0B
    INSTR_STR     = 0x0C, // 0C
    INSTR_LDR     = 0x0D, // 0D
    INSTR_STRB    = 0x0E, // 0E
    INSTR_LDRB    = 0x0F, // 0F
    INSTR_STRS    = 0x10, // 10
    INSTR_LDRS    = 0x11, // 11
    INSTR_LDAPC   = 0x12, // 12
    INSTR_LDAB    = 0x13, // 13
    INSTR_PUSH    = 0x14, // 14
    INSTR_POP     = 0x15, // 15
    INSTR_JMP     = 0x16, // 16
    INSTR_JE      = 0x17, // 17
    INSTR_JNE     = 0x18, // 18
    INSTR_JL      = 0x19, // 19
    INSTR_JG      = 0x1A, // 1A
    INSTR_JLE     = 0x1B, // 1B
    INSTR_JGE     = 0x1C, // 1C
    INSTR_CMP     = 0x1D, // 1D
    INSTR_CALL    = 0x1E, // 1E
    INSTR_RET     = 0x1F, // 1F
    INSTR_PUSHF   = 0x20, // 20
    INSTR_POPF    = 0x21, // 21
    INSTR_VMCALL  = 0x22  // 22
};

typedef enum {
    OP_REG,
    OP_NUM
} OperandType;

typedef struct {
    OperandType type;
    uint32_t data;
    uint8_t is_ptr;
} Operand;

typedef struct {
    char sign[4];
    uint16_t location;
    uint16_t entry_point_addr;
} Header;

int read_file(unsigned char* bytes, char* name) {
    FILE* fd = fopen(name, "rb");
    if (fd == NULL) return 0;

    int c;
    int file_size = 0;
    while ((c = fgetc(fd)) != EOF) {
        bytes[file_size] = c;
        file_size++;
    }

    return file_size;
}

unsigned char fetch(VM *vm) {
    return vm->mem.data[vm->reg.pc++].value;
}

uint32_t parse_int(VM *vm) {
    return (fetch(vm) << 24) | (fetch(vm) << 16) | (fetch(vm) << 8) | fetch(vm);
}

Operand parse_operand(VM *vm) {
    unsigned char sign = fetch(vm);
    Operand res = {0};

    switch (sign) {
    case 0xF0: {
        res.type = OP_REG;
        res.data = fetch(vm);
        break;
    }
    case 0xF1: {
        res.type = OP_NUM;
        res.data = parse_int(vm);
        break;
    }
    case 0xF2: {
        res.is_ptr = 1;
        Operand ptr_addr = parse_operand(vm);
        res.type = ptr_addr.type;
        res.data = ptr_addr.data;
        break;
    }
    }
    return res;
}

uint32_t get_data(VM *vm, Operand op) {
    switch (op.type) {
    case OP_REG: {
        if (op.is_ptr) return read_32(vm->mem.data, get_register(vm, (IREG)op.data));
        else           return get_register(vm, (IREG)op.data);
    }
    case OP_NUM: {
        if (op.is_ptr) return read_32(vm->mem.data, op.data);
        else           return op.data;
    }
    default: error(vm, "Unexpected signature");
    }
    return 0;
}

Header parse_header(unsigned char *bytes) {
    Header res = {0};
    res.sign[0] = bytes[0];
    res.sign[1] = bytes[1];
    res.sign[2] = bytes[2];
    res.sign[3] = bytes[3];
    res.location = (uint16_t)(bytes[4] << 8 | bytes[5]);
    res.entry_point_addr = (uint16_t)(bytes[6] << 8 | bytes[7]);
    return res;
}

int main(int argc, char **argv) {
    (void)argc;

    unsigned char bytes[MEMORY_MAX];
    int file_size = read_file(bytes, argv[1]);

    if (file_size == 0) {
        printf("Could not read file\n");
        return 1;
    }

    // parse header
    Header header = parse_header(bytes);

    if (header.sign[0] != 'N' ||
        header.sign[1] != 'V' ||
        header.sign[2] != 'M' ||
        header.sign[3] != 'B') {
        printf("File signature not recognized\n");
        exit(1);
    }

    VM *vm = init_vm();

    JMP(vm, header.entry_point_addr); // go to the entry point address

    for (int i = 0; i < file_size; ++i) {
        vm->mem.data[header.location + i].value = bytes[i];
    }

    while (1) {
        unsigned char opcode = fetch(vm);
        switch (opcode) {
        case INSTR_NOP: {
            NOP(vm);
            break;
        }
        case INSTR_MOV: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            MOV(vm, (IREG)op1.data, get_data(vm, op2));
            break;
        }
        case INSTR_ADD: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            ADD(vm, (IREG)op1.data, get_data(vm, op2));
            break;
        }
        case INSTR_SUB: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            SUB(vm, (IREG)op1.data, get_data(vm, op2));
            break;
        }
        case INSTR_MUL: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            MUL(vm, (IREG)op1.data, get_data(vm, op2));
            break;
        }
        case INSTR_DIV: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            DIV(vm, (IREG)op1.data, get_data(vm, op2));
            break;
        }
        case INSTR_INC: {
            Operand op1 = parse_operand(vm);
            INC(vm, (IREG)op1.data);
            break;
        }
        case INSTR_DEC: {
            Operand op1 = parse_operand(vm);
            DEC(vm, (IREG)op1.data);
            break;
        }
        case INSTR_AND: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            AND(vm, (IREG)op1.data, get_data(vm, op2));
            break;
        }
        case INSTR_OR: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            OR(vm, (IREG)op1.data, get_data(vm, op2));
            break;
        }
        case INSTR_XOR: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            XOR(vm, (IREG)op1.data, get_data(vm, op2));
            break;
        }
        case INSTR_NOT: {
            Operand op1 = parse_operand(vm);
            NOT(vm, (IREG)op1.data);
            break;
        }
        case INSTR_STR: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            STR(vm, (uint16_t)get_data(vm, op1), get_data(vm, op2));
            break;
        }
        case INSTR_LDR: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            LDR(vm, (IREG)op1.data, get_data(vm, op2));
            break;
        }
        case INSTR_STRB: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            STRB(vm, (uint16_t)get_data(vm, op1), (uint8_t)get_data(vm, op2));
            break;
        }
        case INSTR_LDRB: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            LDRB(vm, (IREG)op1.data, (uint16_t)get_data(vm, op2));
            break;
        }
        case INSTR_STRS: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            STRS(vm, (uint16_t)get_data(vm, op1), get_data(vm, op2));
            break;
        }
        case INSTR_LDRS: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            LDRS(vm, (IREG)op1.data, get_data(vm, op2));
            break;
        }
        case INSTR_LDAPC: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            LDAPC(vm, (IREG)op1.data, get_data(vm, op2));
            break;
        }
        case INSTR_LDAB: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            Operand op3 = parse_operand(vm);
            LDAB(vm, (IREG)op1.data, (uint16_t)get_data(vm, op2), get_data(vm, op3));
            break;
        }
        case INSTR_PUSH: {
            Operand op1 = parse_operand(vm);
            PUSH(vm, get_data(vm, op1));
            break;
        }
        case INSTR_POP: {
            Operand op1 = parse_operand(vm);
            POP(vm, (IREG)op1.data);
            break;
        }
        case INSTR_JMP: {
            Operand op1 = parse_operand(vm);
            JMP(vm, get_data(vm, op1));
            break;
        }
        case INSTR_JE: {
            Operand op1 = parse_operand(vm);
            JE(vm, get_data(vm, op1));
            break;
        }
        case INSTR_JNE: {
            Operand op1 = parse_operand(vm);
            JNE(vm, get_data(vm, op1));
            break;
        }
        case INSTR_JL: {
            Operand op1 = parse_operand(vm);
            JL(vm, get_data(vm, op1));
            break;
        }
        case INSTR_JG: {
            Operand op1 = parse_operand(vm);
            JG(vm, get_data(vm, op1));
            break;
        }
        case INSTR_JLE: {
            Operand op1 = parse_operand(vm);
            JLE(vm, get_data(vm, op1));
            break;
        }
        case INSTR_JGE: {
            Operand op1 = parse_operand(vm);
            JGE(vm, get_data(vm, op1));
            break;
        }
        case INSTR_CMP: {
            Operand op1 = parse_operand(vm);
            Operand op2 = parse_operand(vm);
            CMP(vm, get_data(vm, op1), get_data(vm, op2));
            break;
        }
        case INSTR_CALL: {
            Operand op1 = parse_operand(vm);
            if (op1.type != OP_NUM) printf("can only call to address\n");
            CALL(vm, get_data(vm, op1));
            break;
        }
        case INSTR_RET: {
            RET(vm);
            break;
        }
        case INSTR_PUSHF: {
            PUSHF(vm);
            break;
        }
        case INSTR_POPF: {
            POPF(vm);
            break;
        }
        case INSTR_VMCALL: {
            Operand op1 = parse_operand(vm);
            VMCALL(vm, op1.data);
            break;
        }
        default: {
            printf("Illegal instruction 0x%02x at position %04x\n", opcode, vm->reg.pc-1);
            SystemDump(vm);
            return 1;
        }
        }
    }

    /* SystemDump(vm); */

    return 0;
}
