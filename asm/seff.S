#
# Copyright (c) 2023 Huawei Technologies Co., Ltd.
#
# libseff is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
# 	    http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
#
# Architecture-specific code for context-switching
# This file is for the System V (Linux/OSX) 64 bit architecture
#
#include "seff_types.S"

#ifdef STACK_POLICY_SEGMENTED
#define SEFF_STACK_TOP %fs:0x70
#endif

# TODO: if we add a type to these, with `.type <sym>, @function`, the tests crash
.global seff_return
.global seff_implicit_return
.global seff_yield
.global seff_handle
.global coroutine_prelude

.global _seff_current_coroutine
.global _seff_system_stack
.global _seff_paused_coroutine_stack
#ifdef STACK_POLICY_SEGMENTED
.global _seff_paused_coroutine_stack_top
#endif

.section .tbss

_seff_current_coroutine:
    .zero 8
_seff_paused_coroutine_stack:
    .zero 8
_seff_system_stack:
    .zero 8
#ifdef STACK_POLICY_SEGMENTED
_seff_paused_coroutine_stack_top:
    .zero 8
#endif

.macro swap_registers
    # Saved registers
    # this might be faster than swaps
    movq (seff_coroutine_t__resume_point + seff_cont_t__rbx)(%rdi), %rax
    movq (seff_coroutine_t__resume_point + seff_cont_t__r12)(%rdi), %rcx
    movq (seff_coroutine_t__resume_point + seff_cont_t__r13)(%rdi), %r9
    movq (seff_coroutine_t__resume_point + seff_cont_t__r14)(%rdi), %r10
    movq (seff_coroutine_t__resume_point + seff_cont_t__r15)(%rdi), %r11

    movq %rbx, (seff_coroutine_t__resume_point + seff_cont_t__rbx)(%rdi)
    movq %r12, (seff_coroutine_t__resume_point + seff_cont_t__r12)(%rdi)
    movq %r13, (seff_coroutine_t__resume_point + seff_cont_t__r13)(%rdi)
    movq %r14, (seff_coroutine_t__resume_point + seff_cont_t__r14)(%rdi)
    movq %r15, (seff_coroutine_t__resume_point + seff_cont_t__r15)(%rdi)

    movq %rax, %rbx
    movq %rcx, %r12
    movq %r9, %r13
    movq %r10, %r14
    movq %r11, %r15
.endm

.macro swap_stack
    movq (seff_coroutine_t__resume_point + seff_cont_t__rbp)(%rdi), %rax
    movq (seff_coroutine_t__resume_point + seff_cont_t__rsp)(%rdi), %rcx

    movq %rbp, (seff_coroutine_t__resume_point + seff_cont_t__rbp)(%rdi)
    movq %rsp, (seff_coroutine_t__resume_point + seff_cont_t__rsp)(%rdi)

    movq %rax, %rbp
    movq %rcx, %rsp
.endm

.text

# void* seff_yield(seff_coroutine_t* self, void* arg)
seff_yield:
    # self in %rdi
    # arg in %rsi

    swap_registers

    # We get to pummel %rax, %rcx, %r9, %r10, %r11, so we use those for fast swaps
    # Note that xchg sucks

#ifdef STACK_POLICY_SEGMENTED
    movq SEFF_STACK_TOP, %rax
#endif
    movq %fs:_seff_current_coroutine@tpoff, %rcx

#ifdef STACK_POLICY_SEGMENTED
    movq (seff_coroutine_t__resume_point + seff_cont_t__stack_top)(%rdi), %r9
#endif
    movq (seff_coroutine_t__parent_coroutine)(%rdi), %r10

#ifdef STACK_POLICY_SEGMENTED
    movq %r9, SEFF_STACK_TOP
#endif
    movq %r10, %fs:_seff_current_coroutine@tpoff

#ifdef STACK_POLICY_SEGMENTED
    movq %rax, (seff_coroutine_t__resume_point + seff_cont_t__stack_top)(%rdi)
#endif
    movq %rcx, (seff_coroutine_t__resume_point + seff_cont_t__current_coroutine)(%rdi)

    # Save return pointer
    popq %r11
    movq (seff_coroutine_t__resume_point + seff_cont_t__ip)(%rdi), %r10
    movq %r11, (seff_coroutine_t__resume_point + seff_cont_t__ip)(%rdi)

    swap_stack

    movq %rsi, %rax
    jmp *%r10
.size seff_yield, . - seff_yield

# void* seff_handle(seff_coroutine_t* k, void* arg, effect_set handled)
seff_handle:
    # k in %rdi
    # arg in %rsi
    # handled in %rdx
    movq %rdx, (seff_coroutine_t__handled_effects)(%rdi)

    swap_registers

    # We get to pummel %rax, %rcx, %r9, %r10, %r11, so we use those for fast swaps
    # Note that xchg sucks

#ifdef STACK_POLICY_SEGMENTED
    movq SEFF_STACK_TOP, %rax
#endif
    movq %fs:_seff_current_coroutine@tpoff, %rcx

#ifdef STACK_POLICY_SEGMENTED
    movq (seff_coroutine_t__resume_point + seff_cont_t__stack_top)(%rdi), %r9
#endif
    movq (seff_coroutine_t__resume_point + seff_cont_t__current_coroutine)(%rdi), %r10

#ifdef STACK_POLICY_SEGMENTED
    movq %r9, SEFF_STACK_TOP
#endif
    movq %r10, %fs:_seff_current_coroutine@tpoff

#ifdef STACK_POLICY_SEGMENTED
    movq %rax, (seff_coroutine_t__resume_point + seff_cont_t__stack_top)(%rdi)
#endif
    movq %rcx, (seff_coroutine_t__parent_coroutine)(%rdi)

    # Save return pointer
    popq %r11
    movq (seff_coroutine_t__resume_point + seff_cont_t__ip)(%rdi), %r10
    movq %r11, (seff_coroutine_t__resume_point + seff_cont_t__ip)(%rdi)

    # At this point, the stack is aligned on a 16-byte boundary, so we use
    # this as the system stack if possible. Note that we need to do this
    # before clobbering RCX
    test %rcx, %rcx
    jnz seff_handle_after_store_system_stack
    movq %rsp, %fs:_seff_system_stack@tpoff
seff_handle_after_store_system_stack:

    # Stack
    swap_stack

    movq %rsi, %rax
    jmp *%r10
.size seff_handle, . - seff_handle

/* DWARF unwind info instructions: <http://dwarfstd.org/doc/DWARF5.pdf>
   Register mapping: <https://raw.githubusercontent.com/wiki/hjl-tools/x86-psABI/x86-64-psABI-1.0.pdf> (page 61)
*/
#define DW_def_cfa_expression     0x0F
#define DW_CFA_val_offset         0x14
#define DW_expression             0x10
#define DW_val_expression         0x16
#define DW_OP_deref               0x06        /* dereference the top of the expression stack */
#define DW_OP_breg(r)             (0x70+r)    /* push `register + ofs` on the expression stack */
#define DW_OP_plus_uconst         0x23
#define DW_OP_lit(n)              (0x30+n)
#define DW_OP_minus               0x1C
#define DW_REG_rip                16
#define DW_REG_rax                0
#define DW_REG_rdx                1
#define DW_REG_rcx                2
#define DW_REG_rbx                3
#define DW_REG_rdi                5
#define DW_REG_rbp                6
#define DW_REG_rsp                7
#define DW_REG_r15                15

coroutine_prelude:
    .cfi_startproc
    .cfi_signal_frame           # needed or else gdb does not allow switching frames to a lower address in the backtrace

    popq %rdi
    # At this point we are already at a new stack frame, we need to recover the CFA from rdi somehow
    .cfi_escape DW_def_cfa_expression, 2, DW_OP_breg(DW_REG_rdi), seff_coroutine_t__resume_point /* jmpbuf_t* cfa = (0(%rsp)) */
    .cfi_offset rip, seff_cont_t__ip

    .cfi_offset rbx, seff_cont_t__rbx
    .cfi_offset rsp, seff_cont_t__rsp
    .cfi_offset rbp, seff_cont_t__rbp
    .cfi_offset r12, seff_cont_t__r12
    .cfi_offset r13, seff_cont_t__r13
    .cfi_offset r14, seff_cont_t__r14
    .cfi_offset r15, seff_cont_t__r15

    mov %rdi, %r15
    .cfi_register %rdi, %r15
    .cfi_escape DW_def_cfa_expression, 2, DW_OP_breg(DW_REG_r15), seff_coroutine_t__resume_point /* jmpbuf_t* cfa = (0(%rsp)) */
    popq %rsi
    popq %rdx
    call *%rdx
    # FALLTHROUGH TO SEFF_RETURN
    movq %rax, %rsi
    mov %r15, %rdi
    .cfi_endproc
.size coroutine_prelude, . - coroutine_prelude

# void seff_return(seff_coroutine_t* k, void* result)
seff_return:
    # k in %rdi
    # result in %rsi
    movq $seff_coroutine_state_t__FINISHED, (seff_coroutine_t__state)(%rdi)

    # TODO: it would be faster to avoid swapping, since we're never restarting this coroutine
    swap_registers

#ifdef STACK_POLICY_SEGMENTED
    movq (seff_coroutine_t__resume_point + seff_cont_t__stack_top)(%rdi), %r10
    movq %r10, SEFF_STACK_TOP
#endif
    movq (seff_coroutine_t__parent_coroutine)(%rdi), %r10
    movq %r10, %fs:_seff_current_coroutine@tpoff

    movq (seff_coroutine_t__resume_point + seff_cont_t__ip)(%rdi), %r10

    movq (seff_coroutine_t__resume_point + seff_cont_t__rbp)(%rdi), %rbp
    movq (seff_coroutine_t__resume_point + seff_cont_t__rsp)(%rdi), %rsp

#ifndef NDEBUG
    # Cleaning up
    movq $0x0, (seff_coroutine_t__resume_point + seff_cont_t__ip)(%rdi)

    movq $0x0, (seff_coroutine_t__resume_point + seff_cont_t__rbp)(%rdi)
    movq $0x0, (seff_coroutine_t__resume_point + seff_cont_t__rsp)(%rdi)
#endif

    movq %rsi, %rax
    jmp *%r10
.size seff_return, . - seff_return


#    .cfi_startproc
#    .cfi_signal_frame           /* needed or else gdb does not allow switching frames to a lower address in the backtrace */
#
#    /* save rcx on the stack so it is always available during unwinding */
#    pushq    %rcx
#    .cfi_adjust_cfa_offset 8
#    .cfi_remember_state
#
#    /* set the cfa to point to our return jmpbuf_t (instead of into the stack);
#        the previous registers can now be restored (during unwind) using .cfi_offset directives */
#    .cfi_escape DW_def_cfa_expression, 4, DW_OP_breg(DW_REG_rsp), 0, DW_OP_deref, DW_OP_deref /* jmpbuf_t* cfa = (0(%rsp)) */
#    .cfi_offset rip, 0
#    .cfi_offset rbx, 8
#    .cfi_offset rsp, 16
#    .cfi_offset rbp, 24
#    .cfi_offset r12, 32
#    .cfi_offset r13, 40
#    .cfi_offset r14, 48
#    .cfi_offset r15, 56
#
#    /* switch stack; push rip + rcx to mimic the old stack for the dwarf expression above */
#    movq    8(%rsp), %rax       /* old rip */
#    andq    $~0x0F, %rdi        /* align down to 16 bytes */
#    subq    $16, %rdi
#    movq    %rax, 8(%rdi)       /* old rip */
#    movq    %rcx, 0(%rdi)       /* saved rcx (jmpbuf_t**) */
#    movq    %rdi, %rsp          /* and switch stack */
#
#    /* and call the entry function */
#    movq    %r9, %rdi           /* pass the function argument */
#    movq    %rsp, %rsi
#    callq   *%r8                /* and call the function */
#
#    /* we should never get here (but the called function should longjmp, see `mprompt.c:mp_mprompt_stack_entry`) */
#    #ifdef __MACH__
#    callq   _abort
#    #else
#    callq   abort
#    #endif
#
#    .cfi_restore_state
#    popq    %rdi              /* load indirect jmpbuf_t* and longjmp */
#    movq    (%rdi), %rdi
#    jmp     mp_longjmp
#
#    .cfi_endproc
