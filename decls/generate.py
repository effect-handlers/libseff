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
# A tiny library for generating and maintaining a set of coherent datatype declarations
# in assembly and C
#

import typing

# A named type without a known size
class UnsizedTy:
    declare: typing.Callable[[str], str]
    def __init__(self, declare: typing.Callable[[str], str]):
        self.declare = declare

def unsized_named_ty(name):
    return UnsizedTy(lambda s: f'{name} {s}')

#  A sized type
class Ty(UnsizedTy):
    size: int
    def __init__(self, declare: typing.Callable[[str], str], size: int):
        self.size = size
        super().__init__(declare)

def named_ty(name, size):
    return Ty(lambda s: f'{name} {s}', size)

class Architecture:
    word_type: "Ty"
    size_t: "Ty"
    def __init__(self, bits: int):
        assert(int(bits/8) == bits/8)
        self.word_type = named_ty(f'uint{bits}_t', int(bits/8))
        self.half_word_type = named_ty(f'uint{bits//2}_t', int(bits/16))
        self.size_t = named_ty('size_t', int(bits/8))

arch = None

c_preamble = [
    '#include <stddef.h>',
    '#include <stdint.h>'
]
asm_preamble = []
contents = []

def c_file(name: str, path):
    filename = f'{path}/{name}.h'
    file = open(filename, "w")
    guard = 'SEFF_' + name.upper() + '_H'
    file.write(f'#ifndef {guard}\n#define {guard}\n\n')
    for line in c_preamble:
        file.write(line)
        file.write('\n')
    file.write('\n')
    for section in contents:
        for line in section.c_str():
            file.write(line)
            file.write('\n')
        file.write('\n')
    file.write('\n#endif\n')
    file.close()

def asm_file(name, path):
    filename = f'{path}/{name}.S'
    file = open(filename, "w")
    for line in asm_preamble:
        file.write(line)
        file.write('\n')
    file.write('\n')
    for section in contents:
        for line in section.asm_str():
            file.write(line)
            file.write('\n')
        file.write('\n')
    file.close()

class Defn:
    name: str
    value: int
    hex: bool
    def __init__(self, name: str, value: typing.Optional[int]=None, hex=False):
        contents.append(self)
        self.name = name
        self.value = value
        self.hex = hex
    def __value_str(self):
        if self.hex:
            # WARNING: this will not work for 32bit architectures
            return hex(0xffffffffffffffff & self.value)
        elif self.value:
            return str(self.value)
        else:
            return ""

    def c_str(self):
        return [f"#define {self.name} {self.__value_str()}"]
    def asm_str(self):
        return self.c_str()

class Struct:
    name: str
    fields: typing.Tuple["Field"]
    size: int
    ty: Ty
    def __init__(self, name: str, *fields: "Field"):
        contents.append(self)
        self.name = name
        self.fields = fields
        self.size = 0
        offset = 0
        for field in self.fields:
            field.parent = self
            field.offset = offset
            offset += field.ty.size
        self.size = offset
        self.ty = named_ty(f'{self.name}', self.size)

    def c_str(self):
        lines = ["typedef struct _" + self.name + " {"]
        for field in self.fields:
            lines.append(field.c_str())
        lines.append("} " + self.name + ";")
        return lines

    def asm_str(self):
        return [field.asm_str() for field in self.fields]

class Field:
    name: str
    ty: Ty
    parent: typing.Optional[Struct]
    offset: typing.Optional[int]
    def __init__(self, name: str, ty: Ty):
        self.name = name
        self.ty = ty
        self.parent = None
        self.offset = None

    def c_str(self):
        return "    " + self.ty.declare(self.name) + ";"

    def asm_str(self):
        assert(self.parent)
        return "" + self.parent.name + "__" + self.name + " = " + str(self.offset)

class Typedef:
    name: str
    target: Ty
    ty: Ty
    size: int
    attrs: typing.List[str]
    def __init__(self, name: str, target: Ty, attrs: typing.List[str] = []):
        contents.append(self)
        self.name = name
        self.target = target
        self.ty = named_ty(self.name, target.size)
        self.attrs = attrs
    def c_str(self):
        if (len(self.attrs)):
           return [f'typedef {self.target.declare(self.name)} __attribute__(({",".join(self.attrs)}));']
        else:
            return [f'typedef {self.target.declare(self.name)};']
    def asm_str(self):
        return []

class Enum:
    name: str
    cases: typing.List[typing.Tuple[str, int]]
    ty: Ty
    underlying: Ty
    def __init__(self, name: str, *cases: typing.Union[str,typing.Tuple[str, int]], underlying=None):
        underlying = underlying or arch.word_type
        contents.append(self)
        self.name = name
        self.cases = []
        self.ty = named_ty(self.name, underlying.size)
        counter = 0
        for c in cases:
            if isinstance(c, str):
                case_name = c
            else:
                case_name = c[0]
                counter = c[1]
            self.cases.append((case_name, counter))
            counter += 1
        self.underlying = underlying
    def c_str(self):
        lines = [f'typedef {self.underlying.declare(self.name)};']
        for (c_name, c_idx) in self.cases:
            lines.append(f'#define {c_name} (({self.name}){c_idx})')
        return lines
    def asm_str(self):
        return [
            f'{self.name}__{c_name} = {c_idx}'
            for (c_name, c_idx) in self.cases
        ]

def parameter(name, default):
    value = default
    return Defn(name, value)

byte = named_ty('uint8_t', 1)
void = named_ty('void', 0)

def ptr(ty: UnsizedTy):
    return Ty(lambda s: ty.declare('*'+s), arch.word_type.size)

def arr(ty: Ty, size: int):
    return Ty(lambda s: ty.declare(s+f'[{size}]'), size * ty.size)

def func(args: typing.List[Ty], ret: Ty):
    args_str = '('
    for (i, arg_ty) in enumerate(args):
        args_str += arg_ty.declare('')
        if i < len(args)-1:
            args_str += ', '
    args_str += ')'
    return Ty(lambda s: ret.declare(s + args_str), 0)

def atomic(ty: Ty):
    return Ty(lambda s: '_Atomic ' + ty.declare(s), ty.size)

def generate_file(path_str, target_dir):
    import os
    name = os.path.basename(path_str)[:-3]

    c_file(name, target_dir)
    asm_file(name, target_dir)

    global contents
    contents = []
