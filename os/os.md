<!--
 * @Author: 魏子喻
 * @Date: 2023-06-13 21:45:56
 * @LastEditors: 魏子喻
 * @LastEditTime: 2023-08-10 10:58:19
-->
### 第一节代码

#### 认真学习gdb手册
```c
#include <stdio.h>

int main(){
    return 1;
}
```
使用
```
gcc -c a.c && ld a.o
```
ld 是链接器（Linker）的命令。链接器的作用是将多个目标文件（如上一步生成的 a.o）合并成一个可执行文件。

```
objdump -d a.out  反汇编
```

运行会出错 segmentation default
使用gdb查看

```
layout asm

start

info registers 查看寄存器
x/i $rip
```

1. x 是 GDB 的一个打印命令，用于打印内存中的数据。
2. /i 是 x 命令的一个选项，表示打印指令（instruction）。
3. $rip 是 GDB 的一个寄存器表示当前指令的地址。在 x86 架构中,rip 是指令指针寄存器，存储着下一条将要执行的指令的地址。
因此，x/i $rip 的含义是在当前指令的地址上打印（显示）指令的内容。

```
info inferiors 查看进程号
!cat /proc/****/maps  打印内存映射

x 00400000 打印内存
```

```
p $rsp 查看rsp指针
x $rsp 打印rsp指向的内容
```

```
echo $?  shell 中查看上一个命令的返回值
```


man 2 syscall  查看syscall的用法
syscalls
man 2 write  看具体操作


telnet towel.blinkenlights.nl
dialog --msgbox 'Hello OS World!' 8 32

readelf

很重要的工具
strace

xedit 图形界面程序


### 第二节
cat /proc/cpuinfo    看cpu情况