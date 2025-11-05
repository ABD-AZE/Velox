### Function Call

1) The first six integer arguments to a function are passed in the EDI,
ESI, EDX, ECX, R8D, and R9D registers, in that order (64-bit integers
are passed using these registers’ 64-bit names instead: RDI, RSI, RDX,
RCX, R8, and R9). Any remaining arguments are pushed onto the stack
in reverse order.

2) As we know, a function’s return value is passed in EAX (or RAX if
you’re returning a 64-bit integer). The return value must be in EAX
when the ret instruction is executed.

3) After the callee returns, the caller removes any arguments from the
stack. The callee does not clean up arguments.

4) If a register is caller-saved, the callee is allowed to overwrite it. The caller
must therefore save the register’s value to the stack before issuing the
call instruction if it will need it later. It can then pop that value off the
stack after the function returns. (If the value in a register won’t be used
after the function call, the caller doesn’t need to save it.) If a register is
callee-saved, it must have the same contents when a function returns as it
did at the start of the function. If the callee needs to use the register, it
typically pushes the register’s value onto the stack during the function
prologue, then pops it back off the stack during the function epilogue.
Registers RAX, R10, R11, and all the parameter passing registers are
caller-saved; the remaining registers are callee-saved.

5) The System V ABI requires the stack to be 16-byte aligned. In other
words, the address stored in RSP, the stack pointer, must be divisible
by 16 when we issue a call instruction. The ABI imposes this require-
ment because some instructions require 16-byte-aligned operands. It’s
easier to maintain the correct alignment of these operands if the stack
is 16-byte aligned to begin with.


6) Stack :
                  %RBP -> | rbp of caller            |     
                          | return address of caller |
                          | 7th argument             |
                          | 8th argument             |