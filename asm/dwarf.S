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
# This file has preprocessor definitions for many DWARF related constants

# DWARF instructions: <http://dwarfstd.org/doc/DWARF5.pdf>
# Only the ones we use (and perhaps a few more) are here

# Call frame instructions (§ 7.24)
#define DW_CFA_def_cfa_expression     0x0F  /* cfa := <expr> */
#define DW_CFA_expression             0x10  /* <reg> := *<expr> */
#define DW_CFA_val_offset             0x14
#define DW_CFA_val_expression         0x16

# Expression and location (§ 7.7)
#define DW_OP_deref               0x06        /* top := *pop() */
#define DW_OP_plus_uconst         0x23        /* top := pop() + const */
#define DW_OP_breg(r)             (0x70+r)    /* push `register + ofs` on the expression stack */

#
# The register mappings are architecture specific, this may need to jbe specialized
#
# Register mapping: <https://raw.githubusercontent.com/wiki/hjl-tools/x86-psABI/x86-64-psABI-1.0.pdf> (page 61)
#define DW_REG_rax                0
#define DW_REG_rdx                1
#define DW_REG_rcx                2
#define DW_REG_rbx                3
#define DW_REG_rsi                4
#define DW_REG_rdi                5
#define DW_REG_rbp                6
#define DW_REG_rsp                7
#define DW_REG_r8                 8
#define DW_REG_r9                 9
#define DW_REG_r10                10
#define DW_REG_r11                11
#define DW_REG_r12                12
#define DW_REG_r13                13
#define DW_REG_r14                14
#define DW_REG_r15                15
#define DW_REG_rip                16

#define DW_REG_aarch64_r26       26
