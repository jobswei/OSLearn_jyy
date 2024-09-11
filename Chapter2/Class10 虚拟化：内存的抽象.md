### Class10 虚拟化：内存的抽象

如果你想把进程的M中的一个格子的内容改一下，那在进程内部就可以实现。但是如果想把一段不合法的M地址变成合法的，就要调用syscall。

pmap：查看进程的地址空间。也可以用gdb的info registers

/proc/self/maps  在进程中可以用self，不用它的pid，就可以打开自己的进程文件

pmap 是基于 procfs 中的 maps 实现的

gdb是以程序自己的角度看程序的运行，strace是以操作系统的角度。

~~~shell
$ cat /proc/[pid]/maps
00400000-004b6000                 r-xp  /tmp/a.out (代码)
006b6000-006bc000                 rw-p  /tmp/a.out (数据)
006bc000-006bd000                 rw-p             (bss)
0131f000-01342000                 rw-p  [heap]
7fff993c9000-7fff993ea000         rw-p  [stack]
7fff993f4000-7fff993f7000         r--p  [vvar]     <- WTF?
7fff993f7000-7fff993f9000         r-xp  [vdso]     <- WTF?
ffffffffff600000-ffffffffff601000 r-xp  [vsyscall] <- WTF? 这地址?
~~~

一些频繁的系统调用如果不进入内核执行，可以节省时间，比如获取时间time()

![vsc](D:\About_coding\OSLearn_jyy\Chapter2\pict\vsc.jpg)

* vvar: 程序创建是内核映射过去的一小段数据，只读

* vdso：程序创建时内核映射过去的一小段代码

##### 为进程创建地址空间

进程创建时，操作系统只会向地址空间里加载很少的一部分，a.out，动态链接还有ld.iso，剩下的部分？

- 静态链接：代码、数据、堆栈、堆区
- 动态链接：代码、数据、堆栈、堆区、INTERP (ld.so)

是加载器干的。用哪一个系统调用干的？mmap

将操作系统中任何的对象中的一段搬到内存里

~~~c
// 映射
void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset);
int munmap(void *addr, size_t length);

// 修改映射权限
int mprotect(void *addr, size_t length, int prot);
~~~

addr：地址，NULL为默认分配。length：加载长度。prot：访问权限。fd：文件标识符。offset：偏移量。

如果fd=-1，不加载文件，就可以实现快速分配内存

为什么可以把128G的磁盘映射到物理内存只有4G的内存空间？



##### 实现虚拟存储：分页和存储保护

![虚存](D:\About_coding\OSLearn_jyy\Chapter2\pict\虚存.jpg)

###### 空间局部性

###### ![虚存2](D:\About_coding\OSLearn_jyy\Chapter2\pict\虚存2.jpg)

### Class 11 可执行文件

~~~python
#!/usr/bin/env python3
print('Hello')
~~~

~~~sh
#!/bin/bash -x
echo Hello
~~~



以#!开头，linux会默认认为这是一个可执行文件

操作系统会execve path中的程序，将后面的脚本作为参数传入。

###### 真正的进程 (ELF) 初始化

RTFM

- System V ABI (x86-64)
  - Section 3.4: Process Initialization
    - 之前用 gdb 调试过 “初始状态”
      - 看到了寄存器的初始值
    - 手册完整规定了 execve 后的进程状态
      - libc 会使用它
      - 根据 ABI，你可以开发自己的 libc!
- Gitlab [repo](https://gitlab.com/x86-psABIs/x86-64-ABI)

![ABI](D:\About_coding\OSLearn_jyy\Chapter2\pict\ABI.jpg)



###### 自己加载程序（ELF）

程序头表：指明了需要把程序的哪些部分加载到哪些内存里

然后根据上面的把参数、环境变量放到内存里

再把指针移过去，程序就可以运行了

既然完全可以 “自己加载” 可执行文件。我们也可以在一个操作系统里实现另一个操作系统的 API 啊



状态机视角

- 程序 = 状态机
- 操作系统 = 状态机的虚拟化

##### xv6进程的加载和执行

### Class12 处理器调度

将机制和策略分开

理解常见的处理器的调度策略

- 轮转调度 (round-robin)
- 优先级/反馈调度
- 公平调度

![调度1](D:\About_coding\OSLearn_jyy\Chapter2\pict\调度1.jpg)

![调度2](D:\About_coding\OSLearn_jyy\Chapter2\pict\调度2.jpg)

处理器定时中断的机制不变，引入优先级的策略

###### Unix niceness

越nice，越被不nice的进程抢占

nice指数 -20--19

~~~shell
taskset -c 0 nice -n 19 ./a.out &
taskset -c 0 nice -n 9 ./a.out &
~~~

* -c：绑定到哪个cpu上
* -n：nice指数
* &：放到后台执行

##### 真实的处理器调度（1）

为了让用户体验好，交互进程（如vim）的优先级会高一些

对于某些专用服务器，计算进程会优先一些。

设置若干个 Round-Robin队列，每个队列对应一个优先级

但是像vim这样的，它并不是一直在活动，输入的时候才活动。不活动的时候她有syscall，会主动放弃cpu。

如果进程用完了一整个时、时间片，那么很可能是一个计算进程。如果没用完就放弃cpu，就很可能是交互进程。根据这样的准则，让系统自动识别进程，自动进行优先级调度。

但是对于现在的计算机，各个进程不是独立的，他们有互相交流，计算进程也有频繁的交流，所以这样的准则就并不准确了。



###### 又退回到了完全公平调度（CFS：Complete Faire Scheduling）

记录在每个周期内进程实际占用cpu的时间，如果cpu有空闲，优先唤醒占用cpu总时间最少的。

如果有3个运行了1h的进程，来了一个新的进程，或者sleep(3600)以后刚刚被唤醒的进程，那么该进程会获得所有进程中最小的运行时间作为它的运行时间，而不是0.否则接下来的一个小时都会运行这个新的程序。

fork的子进程会比父进程多运行一小段时间，这样在后面的execve中不会与父进程争抢共享的内存空间。

![调度3](D:\About_coding\OSLearn_jyy\Chapter2\pict\调度3.jpg)

###### vruntime： 

virtual runtime 不再是真实的运行时间，在真实的基础上，加上时间增加比例，实现人工调控

###### 时间太长会溢出

只记录相对时长

##### 真实的处理器调度 (2)

###### 优先级反转

very nice guy 在持有锁的时候让出了处理器……

- bad guy 顺便也无法运行了 (nice guy 抢在了它前面 👎)

~~~c
void bad_guy() { // 高优先级
  mutex_lock(&lk);
  ...
  mutex_unlock(&lk);
}

void nice_guy() { // 中优先级
  while (1) ;
}

void very_nice_guy() { // 最低优先级
  mutex_lock(&lk);
  ...
  mutex_unlock(&lk);
}
~~~

会造成卡死，以及重启

- 优先级继承 (Priority Inheritance)/优先级提升 (Priority Ceiling)
  - 持有 mutex 的线程/进程会继承 block 在该 mutex 上的最高优先级
  - 不总是能 work (例如条件变量唤醒)

##### 真实的处理器调度 (3)

多处理器调度

还没完：我们的 CPU 里有多个共享内存的处理器啊！

- 不能简单地每个处理器上执行 CFS
  - 出现 “一人出力，他人围观”
- 也不能简单地一个 CFS 维护队列
  - 在处理器之间迁移会导致 L1 cache/TLB 全都白给
    - 迁移？可能过一会儿还得移回来
    - 不迁移？造成处理器的浪费

多用户，多任务，用户之间的公平

cgroups：为每个用户设置一个天花板

namespaces：实现用户之间的隔离

cgroups+namespace $\rightarrow$ docker



多个cpu，如何设置各自的频率，实现性能与功耗的平衡

内存共享的假象
