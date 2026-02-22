import re
from enum import Enum, auto
import sys
import argparse

HEADER_SIZE = 8 # NVMB signature + 2 bytes for memory address + 2 bytes for entry point address
MEMORY_LOCATION_ADDRESS = 0
ENTRY_POINT_ADDRESS = 0
BINARY_SIGNATURE = ""
MAIN_FILEPATH = ""

class error:
    def throw(line, msg="", note="", help="", _exit=True):
        if msg:  print(f"[{line}]\033[31m error\033[0m: {msg}")
        if note: print(f"{" "*(len(str(line))+3)}\033[35mnote\033[0m:  {note}")
        if _exit: exit(1)

INSTR = {
    "nop"     : 0x00,
    "mov"     : 0x01,
    "add"     : 0x02,
    "sub"     : 0x03,
    "mul"     : 0x04,
    "div"     : 0x05,
    "inc"     : 0x06,
    "dec"     : 0x07,
    "and"     : 0x08,
    "or"      : 0x09,
    "xor"     : 0x0A,
    "not"     : 0x0B,
    "str"     : 0x0C,
    "ldr"     : 0x0D,
    "strb"    : 0x0E,
    "ldrb"    : 0x0F,
    "strs"    : 0x10,
    "ldrs"    : 0x11,
    "ldapc"   : 0x12,
    "ldab"    : 0x13,
    "push"    : 0x14,
    "pop"     : 0x15,
    "jmp"     : 0x16,
    "je"      : 0x17,
    "jne"     : 0x18,
    "jl"      : 0x19,
    "jg"      : 0x1A,
    "jle"     : 0x1B,
    "jge"     : 0x1C,
    "cmp"     : 0x1D,
    "call"    : 0x1E,
    "ret"     : 0x1F,
    "pushf"   : 0x20,
    "popf"    : 0x21,
    "vmcall"  : 0x22,
}

DIRECTIVES = [
    "entry",
    "loc",
    "signature",
    "reserve",
    "bytes",
    "words",
    "dwords"
]

REGS = {
    "d0": 0,
    "d1": 1,
    "d2": 2,
    "d3": 3,
    "d4": 4,
    "d5": 5,
    "d6": 6,
    "d7": 7,
    
    "w0": 8,
    "w1": 9,
    "w2": 10,
    "w3": 11,
    "w4": 12,
    "w5": 13,
    "w6": 14,
    "w7": 15,
    
    "b0": 16,
    "b1": 17,
    "b2": 18,
    "b3": 19,
    "b4": 20,
    "b5": 21,
    "b6": 22,
    "b7": 23,
    
    "sp": 24,
}

def split(text, separators):
    pattern = '|'.join(r'\n' if sep == '\n' else re.escape(sep) for sep in separators)
    split_pattern = f'({pattern})(?=(?:[^"\']*["\'][^"\']*["\'])*[^"\']*$)'
    
    return [item for item in re.split(split_pattern, text) if item]

class TokType(Enum):
    INSTR         = auto()
    REG           = auto()
    NUMBER        = auto()
    ID            = auto()
    STRING        = auto()
    COLON         = auto()
    TILDE         = auto()
    DIRECTIVE     = auto()
    HASH          = auto()
    AT            = auto()
    PTR           = auto()
    NEWLINE       = auto()
    LBODY         = auto()
    RBODY         = auto()
    EQ            = auto()
    DOLLAR        = auto()
    EOF           = auto()

class Token:
    def __init__(self, type, literal, line):
        self.type = type
        self.literal = literal
        self.line = line
    
class Lexer:
    def __init__(self, src):
        self.src = src
        self.tokens = []
        self.split = [i for i in split(src, " \n:~#={}$@;*") if i not in ['', ' ', None]]
        
        new_split = []
        is_comment = False
        for i in self.split:
            if i == ';':
                is_comment = True
            if i == '\n':
                is_comment = False
            if not is_comment:
                new_split.append(i)
        
        self.split = new_split
        
        line = 0
        for idx, i in enumerate(self.split):
            if i in INSTR:
                self.tokens.append(Token(TokType.INSTR, i, line))
            elif i in REGS:
                self.tokens.append(Token(TokType.REG, i, line))
            elif i in DIRECTIVES:
                self.tokens.append(Token(TokType.DIRECTIVE, i, line))
            elif i.isdigit():
                self.tokens.append(Token(TokType.NUMBER, i, line))
            elif i[0] == "'" and i[-1] == "'":
                self.tokens.append(Token(TokType.NUMBER, str(ord(i[1:-1])), line))
            elif i[:2] == "0x":
                self.tokens.append(Token(TokType.NUMBER, str(int(i[2:], 16)), line))
            elif i[0] == '"' and i[-1] == '"':
                self.tokens.append(Token(TokType.STRING, i[1:-1], line))
            elif i == '\n':
                self.tokens.append(Token(TokType.NEWLINE, i, line))
                line += 1
            elif i == ':':
                self.tokens.append(Token(TokType.COLON, i, line))
            elif i == '~':
                self.tokens.append(Token(TokType.TILDE, i, line))
            elif i == '#':
                self.tokens.append(Token(TokType.HASH, i, line))
            elif i == '{':
                self.tokens.append(Token(TokType.LBODY, i, line))
            elif i == '}':
                self.tokens.append(Token(TokType.RBODY, i, line))
            elif i == '=':
                self.tokens.append(Token(TokType.EQ, i, line))
            elif i == '$':
                self.tokens.append(Token(TokType.DOLLAR, i, line))
            elif i == '@':
                self.tokens.append(Token(TokType.AT, i, line))
            elif i == '*':
                self.tokens.append(Token(TokType.PTR, i, line))
            else:
                self.tokens.append(Token(TokType.ID, i, line))
        
        self.tokens.append(Token(TokType.EOF, None, line))

class Preprocessor:
    def __init__(self, lexer):
        self.lexer = lexer
        self.tokens = lexer.tokens.copy()
        self.macros = {}
        self.const_macros = {}
        
        self.pos = 0
        self.current = self.tokens[self.pos]
        
        # first pass: includes
        # one interesting thing here is that when we insert included file tokens,
        # we don't go to the end of it in the main tokens, this way we can handle
        # nested included files
        while self.current.type != TokType.EOF:
            if self.current.type == TokType.HASH and \
               self.peek().type == TokType.ID:
                if self.peek().literal == "include":
                    self.delete(TokType.HASH)
                    self.delete(TokType.ID)
                    filename = self.current.literal
                    self.delete(TokType.STRING)
                    self.delete(TokType.NEWLINE)
                    filepath = "/".join(MAIN_FILEPATH.split('/')[:-1])+"/"+filename
                    try:
                        src = open(filepath, "r").read()
                    except FileNotFoundError:
                        error.throw("preprocessor", f"file not found: {filepath}")
                    except PermissionError:
                        error.throw("preprocessor", f"permission denied: {filepath}")
                    except Exception as e:
                        error.throw("preprocessor", f"failed to read file: {e}")
                    i_a = Lexer(src)
                    self.tokens = self.tokens[:self.pos] + i_a.tokens[:-1] + self.tokens[self.pos:]
                    # we do [:-1] because we don't need the included file's EOF token
            self.next()
        
        # reset the token iterator for the second pass
        self.reset()
        
        # second pass: collecting macros/const macros
        while self.current.type != TokType.EOF:
            if self.current.type == TokType.HASH and \
               self.peek().type == TokType.ID:
                if self.peek().literal == "const":
                    self.delete(TokType.HASH)
                    self.delete(TokType.ID)
                    name = self.current.literal
                    self.delete(TokType.ID)
                    self.delete(TokType.EQ)
                    value = self.current.literal
                    type = self.current.type
                    self.delete()
                    self.const_macros[name] = {"value": value, "toktype": type}
                if self.peek().literal == "macro":
                    self.delete(TokType.HASH)
                    self.delete(TokType.ID)
                    name = self.current.literal
                    self.delete(TokType.ID)
                    args = []
                    while self.current.type not in [TokType.LBODY]:
                        args.append(self.current.literal)
                        self.delete()
                    self.delete(TokType.LBODY)
                    
                    body = []
                    while self.current.type not in [TokType.RBODY]:
                        body.append(self.current.literal)
                        self.delete()
                    self.delete(TokType.RBODY)
                    self.macros[name] = {"args": args, "body": body}
            self.next()
        
        # third pass: expanding macros
        self.reset()
        while self.current.type != TokType.EOF:
            if self.current.literal in self.macros:
                macro = self.current.literal
                self.delete(TokType.ID)
                passed_args = []
                while self.current.type not in [TokType.NEWLINE, TokType.EOF]:
                    passed_args.append(self.current.literal)
                    self.delete()
                
                if len(passed_args) != len(self.macros[macro]["args"]):
                    error.throw(self.current.line+1, "the number of arguments passed does not match the number of macro arguments");
                
                to_insert = []
                
                # replace all uses of arguments in the body of the macro with the passed values
                for i in self.macros[macro]["body"]:
                    if i in self.macros[macro]["args"]:
                        arg_idx = self.macros[macro]["args"].index(i)
                        to_insert.append(passed_args[arg_idx])
                    else:
                        to_insert.append(i)
                m_a = Lexer(" ".join(to_insert))
                self.tokens = self.tokens[:self.pos] + m_a.tokens[:-1] + self.tokens[self.pos:]
                
            self.next()
        
        # fourth pass: expand constant macros
        self.reset()
        while self.current.type != TokType.EOF:
            if self.current.literal in self.const_macros:
                name = self.current.literal
                self.tokens[self.pos] = Token(self.const_macros[name]["toktype"], self.const_macros[name]["value"])
            self.next()
    
    def reset(self):
        self.pos = 0
        self.current = self.tokens[self.pos] # also update the current token
    
    def delete(self, type=None):
        if type != None and self.current.type != type:
            error.throw(self.current.line, f"expected {type}, got {self.current.type}")
            
        del self.tokens[self.pos]
        self.current = self.tokens[self.pos]
    
    def next(self):
        self.pos += 1
        self.current = self.tokens[self.pos]
    
    def expect(self, type):
        if self.current.type != type:
            error.throw(self.current.line, f"expected {type}, got {self.current.type}")
        self.next()
    
    def peek(self, n=1):
        return self.tokens[self.pos+n if self.pos+n < len(self.tokens) else self.pos]

class Parser:
    def __init__(self, lexer):
        self.lexer = lexer
        self.tokens = lexer.tokens
        
        self.ir = []
        
        self.pos = 0
        self.current = self.tokens[self.pos]
        
        self.parent_label = None
        
        while self.current.type != TokType.EOF:
            if self.current.type == TokType.ID and self.peek().type == TokType.COLON:
                name = self.current.literal
                self.expect(TokType.ID)
                self.expect(TokType.COLON)
                if name[0] == ".":
                    self.ir.append(self.get_ir_node("LocLabelDef", name=name[1:], parent=self.parent_label))
                else:
                    self.parent_label = name
                    self.ir.append(self.get_ir_node("LabelDef", name=name))
            elif self.current.type == TokType.TILDE and \
                 self.peek().type  == TokType.ID:
                self.expect(TokType.TILDE)
                name = self.current.literal
                self.expect(TokType.ID)
                self.expect(TokType.COLON)
                # self.parent_label = name
                # TODO: make local labels support for relative ones
                self.ir.append(self.get_ir_node("RelLabelDef", name=name))
            elif self.current.type == TokType.ID and \
                 self.peek().type == TokType.AT:
                name = self.current.literal
                self.expect(TokType.ID)
                self.expect(TokType.AT)
                addr = self.current.literal
                self.expect(TokType.NUMBER)
                self.expect(TokType.COLON)
                self.ir.append(self.get_ir_node("AddrLabelDef", name=name, addr=int(addr)))
            elif self.current.type == TokType.INSTR:
                instr = self.current.literal
                self.expect(TokType.INSTR)
                args = []
                while self.current.type not in [TokType.NEWLINE, TokType.EOF]:
                    args.append(self.current.literal)
                    self.next()
                self.ir.append(self.get_ir_node("Instr", instr=instr, args=args))
            elif self.current.type == TokType.DIRECTIVE:
                dir = self.current.literal
                self.expect(TokType.DIRECTIVE)
                args = []
                while self.current.type not in [TokType.NEWLINE, TokType.EOF]:
                    args.append(self.current.literal)
                    self.next()
                self.ir.append(self.get_ir_node("Directive", dir=dir, args=args))
            else: self.next()
    
    def get_ir_node(self, kind, **kwargs):
        return (kind, kwargs)
    
    def next(self):
        self.pos += 1
        self.current = self.tokens[self.pos]
    
    def expect(self, type):
        if self.current.type != type:
            error.throw(error.get_tok_line(self.tokens, self.pos), f"expected {type}, got {self.current.type}")
        self.next()
    
    def peek(self, n=1):
        return self.tokens[self.pos+n if self.pos+n < len(self.tokens) else self.pos]

argparser = argparse.ArgumentParser(sys.argv[0])
argparser.add_argument("input")
argparser.add_argument("output")
argparser.add_argument("-d", "--debug", action="store_true")
argparser.add_argument("-nh", "--no-header", action="store_true")
args = argparser.parse_args()

MAIN_FILEPATH = args.input

try:
    src = open(args.input, "r").read()
except FileNotFoundError:
    error.throw(1, f"file not found: {args.input}")
except PermissionError:
    error.throw(1, f"permission denied: {args.input}")
except Exception as e:
    error.throw(1, f"failed to read file: {e}")
a = Lexer(src)
c = Preprocessor(a)
a.tokens = c.tokens
b = Parser(a)

def get_node_size(node):
    if node[0] == "Instr":
        total_size = 1
        for i in node[1]["args"]:
            if i in REGS:
                total_size += 2
            elif i == '*':
                total_size += 1
            elif i.isdigit():
                total_size += 5
            else: # is label
                total_size += 5
        return total_size
    elif node[0] == "Directive":
        if node[1]["dir"] == "reserve":
            return int(node[1]["args"][0])
        elif node[1]["dir"] == "bytes":
            total_size = 0
            for i in node[1]["args"]:
                if i.isdigit():
                    total_size += 1
                else: # is string
                    total_size += len(i)
            return total_size
        elif node[1]["dir"] == "words":
            total_size = 0
            for i in node[1]["args"]:
                if i.isdigit():
                    total_size += 1
                else: # is string
                    total_size += len(i)
            return total_size*2
        elif node[1]["dir"] == "dwords":
            total_size = 0
            for i in node[1]["args"]:
                if i.isdigit():
                    total_size += 1
                else: # is string
                    total_size += len(i)
            return total_size*4
        else: return 0
    else: return 0

loc_def_found = False
sign_def_found = False

for i in b.ir:
    if i[0] == "Directive":
        if i[1]["dir"] == "loc":
            MEMORY_LOCATION_ADDRESS = int(i[1]["args"][0])
            loc_def_found = True
        elif i[1]["dir"] == "signature":
            BINARY_SIGNATURE = i[1]["args"][0]
            sign_def_found = True

if not loc_def_found or not sign_def_found:
    error.throw("gen", "no header definition", "try adding \"signature NVMB\" and \"loc 0x0000\" to the beginning of file")

labels = {}
addr = 0

# parse labels addresses
for i in b.ir:
    addr += get_node_size(i)
    if i[0] == "LabelDef":
        labels[i[1]["name"]] = addr + (HEADER_SIZE if not args.no_header else 0)
    elif i[0] == "RelLabelDef": 
        labels[i[1]["name"]] = addr + MEMORY_LOCATION_ADDRESS + (HEADER_SIZE if not args.no_header else 0)
    elif i[0] == "LocLabelDef":
        labels[i[1]["parent"]+"."+i[1]["name"]] = addr + (HEADER_SIZE if not args.no_header else 0)
    elif i[0] == "AddrLabelDef":
        labels[i[1]["name"]] = i[1]["addr"] + (HEADER_SIZE if not args.no_header else 0)

# parsing the entry point address after parsing the labels addresses, so that you can specify the entry point as a label
entry_found = False
for i in b.ir:
    if i[0] == "Directive":
        if i[1]["dir"] == "entry":
            if i[1]["args"][0] not in labels:
                error.throw("gen", "entry point must be a defined label", f"try to define it as a \"{i[1]["args"][0]}:\"")
            ENTRY_POINT_ADDRESS = labels[i[1]["args"][0]]
            entry_found = True

if not entry_found:
    error.throw("gen", "entry point not found", "try adding a \"entry main\" (or your entry point label name) to beginning of the file")

output_bytes = []
if not args.no_header:
    output_bytes.extend([ord(i) for i in BINARY_SIGNATURE])
    output_bytes.extend(MEMORY_LOCATION_ADDRESS.to_bytes(2, byteorder="big"))
    output_bytes.extend(ENTRY_POINT_ADDRESS.to_bytes(2, byteorder="big"))

for i in b.ir:
    if i[0] == "Instr":
        output_bytes.append(INSTR[i[1]["instr"]])
        for j in i[1]["args"]:
            if j in REGS:
                output_bytes.append(0xF0)
                output_bytes.append(REGS[j])
            elif j.isdigit():
                output_bytes.append(0xF1)
                output_bytes.extend(int(j).to_bytes(4, byteorder="big"))
            elif j == '*':
                output_bytes.append(0xF2)
            else: # is label
                output_bytes.append(0xF1)
                if j not in labels:
                    error.throw("gen", f"id {j} not found")
                output_bytes.extend(labels[j].to_bytes(4, byteorder="big"))
    elif i[0] == "Directive":
        if i[1]["dir"] == "reserve":
            for j in range(int(i[1]["args"][0])):
                output_bytes.append(0)
        elif i[1]["dir"] == "bytes":
            for j in i[1]["args"]:
                if j.isdigit():
                    output_bytes.append(int(j))
                else: # is string
                    output_bytes.extend([ord(k) for k in j])
        elif i[1]["dir"] == "words":
            for j in i[1]["args"]:
                if j.isdigit():
                    output_bytes.extend(int(j).to_bytes(2, byteorder="big"))
                else: # is string
                    output_bytes.extend([ord(k).to_bytes(2, byteorder="big") for k in j])
        elif i[1]["dir"] == "dwords":
            for j in i[1]["args"]:
                if j.isdigit():
                    output_bytes.extend(int(j).to_bytes(4, byteorder="big"))
                else: # is string
                    error.throw(0, "not implemented")
                    output_bytes.extend([ord(k).to_bytes(4, byteorder="big") for k in j])
            
if args.debug:
    print("--------- Tokens ---------")
    for i in a.tokens:
        print(i.literal, end=' ' if i.literal != '\n' else '')
    print()
    
    print("--------- Ir ---------")
    for i in b.ir:
        print(i)
    print()
    
    print("--------- Labels ---------")
    print(labels)
    print()
    
    print("--------- Macros ---------")
    for i in c.macros.items():
        print(i)
    print()
    print("--------- ConstMacros ---------")
    for i in c.const_macros.items():
        print(i)
    print()
    
    print("--------- Generated ---------")
    print("Addr  00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F")
    print("----  -----------------------  -----------------------")
    
    for i, byte in enumerate(output_bytes):
        if i % 16 == 0:
            if i != 0:
                print()
            print(f"{i:04X}: ", end="")
            
        if i % 8 == 0 and i % 16 != 0:
            print(" ", end="")
            
        print(f"{byte:02X}", end=" ")
        
    if len(output_bytes) > 0:
        print()
    print()
    
with open(args.output, "wb") as f:
    f.write(bytes(output_bytes))

print(f"{args.input}: generated {len(output_bytes)} bytes to {args.output}")

