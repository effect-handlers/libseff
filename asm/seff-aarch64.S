# Copyright (c) 2023 Huawei Technologies Co., Ltd.
# Copyright (c) 2024 Brian Campbell, University of Edinburgh
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
# This file is for the Arm's AArch64 instruction set architecture
#

#include "dwarf.S"
#include "seff_types.S"

#ifdef STACK_POLICY_SEGMENTED
#error Segmented stacks not currently supported on AArch64
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

.section .tbss

_seff_current_coroutine:
    .zero 8
_seff_paused_coroutine_stack:
    .zero 8
_seff_system_stack:
    .zero 8

.macro swap_registers
    # Saved registers
    # TODO: ldp/stp?
    ldr x9, [x0, seff_coroutine_t__resume_point + seff_cont_t__r19]
    ldr x10, [x0, seff_coroutine_t__resume_point + seff_cont_t__r20]
    ldr x11, [x0, seff_coroutine_t__resume_point + seff_cont_t__r21]
    ldr x12, [x0, seff_coroutine_t__resume_point + seff_cont_t__r22]
    ldr x13, [x0, seff_coroutine_t__resume_point + seff_cont_t__r23]
    ldr x14, [x0, seff_coroutine_t__resume_point + seff_cont_t__r24]
    ldr x15, [x0, seff_coroutine_t__resume_point + seff_cont_t__r25]
    ldr x16, [x0, seff_coroutine_t__resume_point + seff_cont_t__r26]
    ldr x17, [x0, seff_coroutine_t__resume_point + seff_cont_t__r27]
    ldr x8, [x0, seff_coroutine_t__resume_point + seff_cont_t__r28]

    str x19, [x0, seff_coroutine_t__resume_point + seff_cont_t__r19]
    str x20, [x0, seff_coroutine_t__resume_point + seff_cont_t__r20]
    str x21, [x0, seff_coroutine_t__resume_point + seff_cont_t__r21]
    str x22, [x0, seff_coroutine_t__resume_point + seff_cont_t__r22]
    str x23, [x0, seff_coroutine_t__resume_point + seff_cont_t__r23]
    str x24, [x0, seff_coroutine_t__resume_point + seff_cont_t__r24]
    str x25, [x0, seff_coroutine_t__resume_point + seff_cont_t__r25]
    str x26, [x0, seff_coroutine_t__resume_point + seff_cont_t__r26]
    str x27, [x0, seff_coroutine_t__resume_point + seff_cont_t__r27]
    str x28, [x0, seff_coroutine_t__resume_point + seff_cont_t__r28]

    mov x19, x9
    mov x20, x10
    mov x21, x11
    mov x22, x12
    mov x23, x13
    mov x24, x14
    mov x25, x15
    mov x26, x16
    mov x27, x17
    mov x28, x8
.endm

.macro swap_stack
    ldr x10, [x0, seff_coroutine_t__resume_point + seff_cont_t__rbp]
    ldr x11, [x0, seff_coroutine_t__resume_point + seff_cont_t__rsp]

    str x29, [x0, seff_coroutine_t__resume_point + seff_cont_t__rbp]
    mov x29, sp
    str x29, [x0, seff_coroutine_t__resume_point + seff_cont_t__rsp]

    mov x29, x10
    mov sp, x11
.endm

.text

# void* seff_yield(seff_coroutine_t* self, void* arg)
seff_yield:
    .cfi_startproc
    # self in x0
    # arg1 in x1
    # arg2 in x2

    swap_registers

    # We get to pummel x3-x15 so we use those for fast swaps

    mrs x9, TPIDR_EL0

    add x9, x9, :tprel_hi12:_seff_current_coroutine
    add x9, x9, :tprel_lo12_nc:_seff_current_coroutine
    # Keep x9 for later
    ldr x4, [x9]

    ldr x6, [x0, seff_coroutine_t__parent_coroutine]

    # _seff_current_coroutine
    str x6, [x9]

    str x4, [x0, seff_coroutine_t__resume_point + seff_cont_t__current_coroutine]

    # Save return pointer
    mov x3, x30
    ldr x30, [x0, seff_coroutine_t__resume_point + seff_cont_t__ip]
    str x3, [x0, seff_coroutine_t__resume_point + seff_cont_t__ip]

    swap_stack

    mov x0, x1
    mov x1, x2
    ret
    .cfi_endproc
.size seff_yield, . - seff_yield

# void* seff_resume(seff_coroutine_t* k, void* arg, effect_set handled)
seff_resume:
    .cfi_startproc
    # k in x0
    # arg in x1
    # handled in x2
    str x2, [x0, seff_coroutine_t__handled_effects]

    swap_registers

    # We get to pummel x3-x15 so we use those for fast swaps

    mrs x9, TPIDR_EL0

    add x10, x9, :tprel_hi12:_seff_current_coroutine
    add x10, x10, :tprel_lo12_nc:_seff_current_coroutine
    ldr x4, [x10]

    ldr x6, [x0, seff_coroutine_t__resume_point + seff_cont_t__current_coroutine]

    str x6, [x10] /* _seff_current_coroutine */

    str x4, [x0, seff_coroutine_t__parent_coroutine]

    # Save return pointer
    mov x3, x30
    ldr x30, [x0, seff_coroutine_t__resume_point + seff_cont_t__ip]
    str x3, [x0, seff_coroutine_t__resume_point + seff_cont_t__ip]

    # At this point, the stack is aligned on a 16-byte boundary, so we use
    # this as the system stack if possible. Note that we need to do this
    # before clobbering x4
    cbnz x4, seff_resume_after_store_system_stack
    add x10, x9, :tprel_hi12:_seff_system_stack
    add x10, x10, :tprel_lo12_nc:_seff_system_stack
    mov x11, sp
    str x11, [x10]
seff_resume_after_store_system_stack:

    # Stack
    swap_stack

    mov x0, x1
    ret
    .cfi_endproc
.size seff_resume, . - seff_resume

# void coroutine_prelude(seff_coroutine_t *k, seff_start_fun_t *fn, void *arg);
# k in x26
# arg in x27
# fn in x28
coroutine_prelude:
    .cfi_startproc
    # needed or else gdb does not allow switching frames to a lower address in the backtrace
    .cfi_signal_frame

    # At this point we are already at a new stack frame, we need to recover the CFA from x26
    .cfi_escape DW_CFA_def_cfa_expression, 2, DW_OP_breg(DW_REG_aarch64_r26), seff_coroutine_t__resume_point
    # And then every other register from its offset to CFA
    .cfi_offset x30, seff_cont_t__ip
    .cfi_offset sp, seff_cont_t__rsp
    .cfi_offset x29, seff_cont_t__rbp
    .cfi_offset x19, seff_cont_t__r19
    .cfi_offset x20, seff_cont_t__r20
    .cfi_offset x21, seff_cont_t__r21
    .cfi_offset x22, seff_cont_t__r22
    .cfi_offset x23, seff_cont_t__r23
    .cfi_offset x24, seff_cont_t__r24
    .cfi_offset x25, seff_cont_t__r25
    .cfi_offset x26, seff_cont_t__r26
    .cfi_offset x27, seff_cont_t__r27
    .cfi_offset x28, seff_cont_t__r28

    mov x0, x27
    # x26 is a callee-save register, so the debugger will continue to find the correct CFA

    blr x28
    # FALLTHROUGH TO SEFF_EXIT
    # set the return value to (seff_result_t){ 0xFF...FF, %rax }
    mov x2, x0
    mov x0, x26
    mov x1, #RETURN_EFFECT_ID
    .cfi_endproc
.size coroutine_prelude, . - coroutine_prelude

# void seff_exit(seff_coroutine_t* k, void* result)
seff_exit:
    .cfi_startproc
    # k in x0
    # eff_id in x1
    # result in x2
    mov x9, #seff_coroutine_state_t__FINISHED
    str x9, [x0, seff_coroutine_t__state]

    # TODO: it would be faster to avoid swapping, since we're never restarting this coroutine
    swap_registers

    ldr x9, [x0, seff_coroutine_t__parent_coroutine]
    mrs x10, TPIDR_EL0
    add x10, x10, :tprel_hi12:_seff_current_coroutine
    add x10, x10, :tprel_lo12_nc:_seff_current_coroutine
    str x9, [x10]

    ldr x9, [x0, seff_coroutine_t__resume_point + seff_cont_t__ip]

    ldr x29, [x0, seff_coroutine_t__resume_point + seff_cont_t__rbp]
    ldr x10, [x0, seff_coroutine_t__resume_point + seff_cont_t__rsp]
    mov sp, x10

#ifndef NDEBUG
    # Cleaning up
    str xzr, [x0, seff_coroutine_t__resume_point + seff_cont_t__ip]

    str xzr, [x0, seff_coroutine_t__resume_point + seff_cont_t__rbp]
    str xzr, [x0, seff_coroutine_t__resume_point + seff_cont_t__rsp]
#endif

    mov x0, x1
    mov x1, x2
    br x9
    .cfi_endproc
.size seff_exit, . - seff_exit
