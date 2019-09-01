bits 32
section .data
bits 32

times ($$-$)&7 db 0
section .text
bits 32

global rdtscCapableCpu_
global _rdtscCapableCpu
global _ticker
global ticker_

_ticker:
ticker_:
    push edx
    rdtsc
    shr eax,8
    shl edx,24
    and edx,0xff000000
    or  eax,edx
    pop edx
    ret

rdtscCapableCpu_:
_rdtscCapableCpu:
    push ebx
    push ecx
    push edx
    mov eax,1
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    cpuid
    test edx,0x10
    setne al
    and eax,1
    pop edx
    pop ecx
    pop ebx
    ret
end
