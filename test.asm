signature NVMB
loc 0x2000
entry main

main:
    mov d0 msg
    vmcall 0x12
    
    mov d0 0
    vmcall 0xFF

~msg: bytes "Hello, World!" 0x0A 0x00
