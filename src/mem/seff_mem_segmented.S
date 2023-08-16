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

#include "../asm/seff_types.S"

.text

# Clang/GCC -fsplit-stack expects the address of the top of the current segment to be
# located in %fs:0x70 (for 64 bits) or %fs:0x40 (for 32 bits). When we're not living
# inside a coroutine, this is zeroed so the runtime thinks there's always enough space
# This means the main system stack is not resizable, we just stack overflow as usual
#define SEFF_STACK_TOP %fs:0x70

.global __morestack_non_split
.global __morestack
.global seff_mem_thread_init

.global syscall_segment_size

__morestack_non_split:
    # r10 := r10 + SYSCALL_SEGMENT_SIZE
    # RSP - r10 < SEFF_STACK_TOP?
    movq (syscall_segment_size), %rax
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
.size __morestack_non_split, . - __morestack_non_split

__morestack:
    # Save the stack pointer of the segment we're about to leave
    # Henceforth, we cannot clobber %rbx
    pushq %rbx
    movq %rsp, %rbx

    # Switch to the system stack
    movq %fs:_seff_system_stack@tpoff, %rsp

    # New frame
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
