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

#include "dwarf.S"
#include "seff_types.S"

#ifdef STACK_POLICY_SEGMENTED
#define SEFF_STACK_TOP %fs:0x70
#endif

.global seff_exit
.global seff_yield
.global seff_resume
.global coroutine_prelude

.type seff_exit,%function
.type seff_yield,%function
.type seff_resume,%function
.type coroutine_prelude,%function

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
    .cfi_startproc
    # self in %rdi
    # arg1 in %rsi
    # arg2 in %rdx

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
    # second argument is already in %rdx
    jmp *%r10
    .cfi_endproc
.size seff_yield, . - seff_yield

# void* seff_resume(seff_coroutine_t* k, void* arg, effect_set handled)
seff_resume:
    .cfi_startproc
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
    jnz seff_resume_after_store_system_stack
    movq %rsp, %fs:_seff_system_stack@tpoff
seff_resume_after_store_system_stack:

    # Stack
    swap_stack

    movq %rsi, %rax
    jmp *%r10
    .cfi_endproc
.size seff_resume, . - seff_resume

# void coroutine_prelude(seff_coroutine_t *k, seff_start_fun_t *fn, void *arg);
# k in r13
# arg in r14
# fn in r15
coroutine_prelude:
    .cfi_startproc
    .cfi_signal_frame           # needed or else gdb does not allow switching frames to a lower address in the backtrace

    # At this point we are already at a new stack frame, we need to recover the CFA from rdi
    .cfi_escape DW_CFA_def_cfa_expression, 2, DW_OP_breg(DW_REG_r13), seff_coroutine_t__resume_point
    # And then every other register from its offset to CFA
    .cfi_offset rip, seff_cont_t__ip
    .cfi_offset rbx, seff_cont_t__rbx
    .cfi_offset rsp, seff_cont_t__rsp
    .cfi_offset rbp, seff_cont_t__rbp
    .cfi_offset r12, seff_cont_t__r12
    .cfi_offset r13, seff_cont_t__r13
    .cfi_offset r14, seff_cont_t__r14
    .cfi_offset r15, seff_cont_t__r15

    movq %r14, %rdi
    # Since rdi is a scratch register, from now our CFA relates to some position in memory
    # Note that this instruction is executed when unwinding from below this line, with whatever $rdi
    # has at that moment

    call *%r15
    movq %r13, %rdi
    # FALLTHROUGH TO SEFF_RETURN
    # set the return value to (seff_result_t){ 0xFF...FF, %rax }
    pushq %rax
    movq $RETURN_EFFECT_ID, %rsi
    movq %rax, %rdx
    .cfi_endproc
.size coroutine_prelude, . - coroutine_prelude

# void seff_exit(seff_coroutine_t* k, void* result)
seff_exit:
    .cfi_startproc
    # k in %rdi
    # eff_id in %rsi
    # result in %rdx
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
    # The effect id is already in %rdx, no need to move it
    jmp *%r10
    .cfi_endproc
.size seff_exit, . - seff_exit

# This notes indicate that this file was compiled with split stack
.section	.note.GNU-stack,"",@progbits
.section	.note.GNU-split-stack,"",@progbits
.section	.note.GNU-no-split-stack,"",@progbits
