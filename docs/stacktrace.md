# Nice stacktraces for your coroutines
> _Take that stackless_

This guide explains a bit how to get good stacktraces when we're both writing
assembly and manipulating the stack frames. It's intended mainly for us, so we
don't forget what we did.

## Disclaimer

- **Not everywhere**: The main goal was to have good stacktraces from within
coroutines, some of the low level functions we have (ie _morestack) break the
debug information in the middle, and fix it later
- **Far from perfect**: Some things could be nicer, for instance, right now the
prologue of a function that calls _morestack is not represented in the stacktrace; nor is _morestack_non_split. Future work.
- **Far from thouroughly tested**: It's very likely the stacktrace will break in some
corner cases, sorry.

_If any of this points are problematic, please submit a PR_

## Resources
- [DWARF introduction](https://dwarfstd.org/doc/Debugging%20using%20DWARF-2012.pdf):
Nice introduction to get started with DWARF, mainly history and core ideas.
- [DWARF5 spec](https://dwarfstd.org/doc/Debugging%20using%20DWARF-2012.pdf):
The DWARF5 spec, reference manual for CFA operations.
- [X86_64 ABI spec](https://raw.githubusercontent.com/wiki/hjl-tools/x86-psABI/x86-64-psABI-1.0.pdf):
Reference for some things like register mapping.
- [asm directives](https://sourceware.org/binutils/docs/as/Pseudo-Ops.html):
Some directives for ASM code, the `.cfi_*` are particularly useful.
- [morestack](https://github.com/gcc-mirror/gcc/blob/releases/gcc-12.2.0/libgcc/config/i386/morestack.S):
libgcc implementation of `_morestack`, they have a similar situation to ours.
- [libmprompt](https://github.com/koka-lang/libmprompt/blob/main/src/mprompt/asm/longjmp_amd64.S#L127):
The `mp_stack_enter` function of `libmprompt`, similar to our `coroutine_prelude`.
Source of inspiration.

## DWARF

DWARF is a language and architecture agnostic debugging format.
That means it's a way for compilers to add information to the
binaries so a debugger (like gdb) can have extra information during
execution, as in location, variable types, or stack unwinding information.

### Call frame information

The Call Frame Information (CFI) provides the debugger with enough information
on how to virtually unwind the stack and restore all preserved registers and values.
For example, if %rdi is saved on some other preserved register, like %r15, the CFI
would tell this information to the debugger.

### Canonical Fram Address

The Canonical Frame Address (CFA) is "Typically (...) defined to be the value
of the stack pointer at the call site in the previous frame (which may be
different from its value on entry to the current frame)" (6.4 at DWARF5 spec).
> Note: As far as I understood, CFA has no actual meaning to gdb and it's not used,
> we set it, and then we use it as a base pointer for other important information

By instructing the debugger on how to compute the CFA we can then offset every other
preserved register from it. For example, on a regular C/C++ function, we could say
the CFA is defined to be `%rbp - 0x8`, and %rip to be stored at `*CFA`.
Of course, when we modify the stack pointers in non trivial ways this can be more complex.

### Example

Consider the example below, which is the (simplified version) prelude run for
every new coroutine:

```asm
coroutine_prelude:
    .cfi_startproc

    popq %rdi
    # (1)
    .cfi_escape DW_CFA_def_cfa_expression, 2, DW_OP_breg(DW_REG_rdi), seff_coroutine_t__resume_point
    .cfi_offset rip, seff_cont_t__ip
    .cfi_offset rsp, seff_cont_t__rsp
    .cfi_offset rbp, seff_cont_t__rbp
    # (...) And other registers

    popq %rsi
    popq %rdx
    push %rdi
    # (2)
    .cfi_escape DW_CFA_def_cfa_expression, 5, DW_OP_breg(DW_REG_rsp), 0, DW_OP_deref, DW_OP_plus_uconst, seff_coroutine_t__resume_point

    call *%rdx
    # coroutine_prelude continues
```

When the function starts, we're already running on a new stack whose information
can be obtained from the value at the top of the stack (that we pop onto `%rdi`).

At (1) we've already popped the needed information and we generate the CFI, first
we indicate how to calculate the CFA (this means if you're unwinding the stack
from the line after this `popq`, this is how you recover the CFA).
`DW_CFA_def_cfa_expression` is a DWARF operation to define the CFA from a
DWARF expression; `2, DW_OP_breg(DW_REG_rdi), seff_coroutine_t__resume_point` is
that expression, `2` indicates the number of arguments, `DW_OP_breg(DW_REG_rdi)`
indicates to take the next expression, add it to the value of `%rdi` registar,
deref the resulting address, and push the value on the stack (a DWARF expression
evaluator stack, not your machine stack); `seff_coroutine_t__resume_point` is an
immediate indicating the offset for the previous expression.
When the whole thing is evaluated the new value of CFA is at the top of the stack.

`.cfi_offset rip, seff_cont_t__ip` and alike indicate that the previous value of `%rip`
can be found at the given offset (`seff_cont_t__ip`) from the CFA.

On (2) we are pushing `%rdi`, since `%rdi` is a scratch register. This means,
from here on now the calculation of CFA changes (not by much);
note that the offsets for registers indicated on (1) won't change in relationship with the CFA.

> Note that there's no way to actually save %rdi, CFI doesn't execute with your code,
> it's executed backwards when unwinding. The `call *%rdx` could very well override
> `%rdi` and there's no way for gdb to recover it. However, because of calling
> conventions, if a called function overrides `%r15` we can assume it will have some
> CFI to recover it (probably from the stack).


### Useful commands or stuff
- On gdb, `info frame` has a bunch of information I've never actually used
until working on this
- `readelf -wf output/seff_asm.o` will output the CFI for an object file, it's hard
to read, but it will have error messages if you did something wrong.
- Apparently gdb doesn't like when %rsp jumps around too much when unwinding,
`.cfi_signal_frame` fixes it, and it also makes it look cooler while debugging.