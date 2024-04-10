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

#include "../asm/dwarf.S"
#include "seff_types.S"

# Clang/GCC -fsplit-stack expects the address of the top of the current segment to be
# located in %fs:0x70 (for 64 bits) or %fs:0x40 (for 32 bits). When we're not living
# inside a coroutine, this is zeroed so the runtime thinks there's always enough space
# This means the main system stack is not resizable, we just stack overflow as usual
#define SEFF_STACK_TOP %fs:0x70

.global __morestack_non_split
.global __morestack
.global seff_mem_thread_init
.global syscall_segment_size
.type __morestack_non_split,%function
.type __morestack,%function
.type seff_mem_thread_init,%function


__morestack_non_split:
    # r10 := r10 + SYSCALL_SEGMENT_SIZE
    # RSP - r10 < SEFF_STACK_TOP?
    .cfi_startproc
    .cfi_def_cfa %rsp,16 # this skips the previous stack frame, which is just the prologue of the function
    movq syscall_segment_size(%rip), %rax
    addq %rax, %r10
    subq %r10, %rsp
    cmpq SEFF_STACK_TOP, %rsp

    jb morestack_non_split_reserve_stack

    # Restore rsp after the comparison decrements
    addq %r10, %rsp
    # This breaks call/return prediction, as described above.
    # Increment the return address & return
    incq (%rsp)
    ret

morestack_non_split_reserve_stack:
    # Restore rsp after the comparison decrements
    addq %r10, %rsp

    # Fallthrough into __morestack
    .cfi_endproc
.size __morestack_non_split, . - __morestack_non_split

__morestack:
    .cfi_startproc
    .cfi_signal_frame  # needed to make gdb happy
    .cfi_def_cfa %rsp, 16  # skip the function prologue stack frame, for the better

    # Save the stack pointer of the segment we're about to leave
    # Henceforth, we cannot clobber %rbx
    pushq %rbx
    .cfi_adjust_cfa_offset 8
	.cfi_offset %rbx, -24

    movq %rsp, %rbx
    .cfi_def_cfa_register %rbx  # CFA is now on $rbx
    .cfi_offset rip, -8
    # .cfi_val_offset  rsp, 1  # this directive should work, but it doesnt
    .cfi_escape DW_CFA_val_offset, DW_REG_rsp, 1


    # Switch to the system stack
    movq %fs:_seff_system_stack@tpoff, %rsp

    # New frame
    .cfi_remember_state
    pushq %rbp

    pushq %rbp

    movq %rsp, %rbp

    # Save registers -- TODO: reduce!
    pushq %r10 # Frame size -- NO BACKOFF
    pushq %rax
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %r8
    pushq %r9

    # we'd like to change the CFA to %rbp, but I don't think that's a good idea
    .cfi_escape DW_CFA_expression, DW_REG_rbp, 2, DW_OP_breg(DW_REG_rbp), 0x00
    .cfi_escape DW_CFA_expression, DW_REG_r10, 2, DW_OP_breg(DW_REG_rbp), 0x08
    .cfi_escape DW_CFA_expression, DW_REG_rax, 2, DW_OP_breg(DW_REG_rbp), 0x10
    .cfi_escape DW_CFA_expression, DW_REG_rdi, 2, DW_OP_breg(DW_REG_rbp), 0x18
    .cfi_escape DW_CFA_expression, DW_REG_rsi, 2, DW_OP_breg(DW_REG_rbp), 0x20
    .cfi_escape DW_CFA_expression, DW_REG_rdx, 2, DW_OP_breg(DW_REG_rbp), 0x28
    .cfi_escape DW_CFA_expression, DW_REG_rcx, 2, DW_OP_breg(DW_REG_rbp), 0x30
    .cfi_escape DW_CFA_expression, DW_REG_r8, 2, DW_OP_breg(DW_REG_rbp),  0x38
    .cfi_escape DW_CFA_expression, DW_REG_r9, 2, DW_OP_breg(DW_REG_rbp),  0x40

    leaq -8(%rbp), %rdi # Address of frame size
    leaq 24(%rbx), %rsi # Address of parameters
    movq %r11, %rdx     # Number of parameters

    call seff_mem_allocate_frame

    # New frame size goes to %r10
    movq -8(%rbp), %r10

    # Switch to new stack, update STACK_TOP
    movq %rax, %rsp
    subq %r10, %rax
    movq %rax, SEFF_STACK_TOP

    # Restore registers -- TODO: reduce!
    movq -16(%rbp),%rax
    movq -24(%rbp),%rdi
    movq -32(%rbp),%rsi
    movq -40(%rbp),%rdx
    movq -48(%rbp),%rcx
    movq -56(%rbp),%r8
    movq -64(%rbp),%r9
    movq (%rbp), %rbp
    .cfi_restore_state

    # This is the return address of the call to __morestack
    movq 8(%rbx), %r10
    incq %r10
    call *%r10

    # Original function returns here. %rbx points to the old %rsp.
    pushq %rax
    pushq %rdx

    call seff_mem_release_frame
    movq %rax, SEFF_STACK_TOP

    popq %rdx
    popq %rax

    movq %rbx,%rsp
    popq %rbx
    ret
    .cfi_endproc
.size __morestack, . - __morestack

seff_mem_thread_init:
    endbr64
    xorq %rax, %rax
    movq %rax, %fs:_seff_current_coroutine@tpoff
    movq %rax, SEFF_STACK_TOP
    ret
.size seff_mem_thread_init, . - seff_mem_thread_init

# Run these when the program starts
# Priority 65535 will run after all others
# TODO: .ctors is old, replace with .init_array
.section .ctors.65535, "aw", @progbits
.align 8
    .quad seff_mem_thread_init

.section .note.GNU-stack, "", @progbits
.section .note.GNU-split-stack, "", @progbits
.section .note.GNU-no-split-stack, "", @progbits
