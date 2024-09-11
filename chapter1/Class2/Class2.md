## Chapter2

#### 并发：多处理器编程

gcc ：-I. 会把当前目录加入到path里面

~~~shell
gcc -I. thread.c
~~~

~~~shell
gcc -I. ab.c
./a.out > /dev/null &  #在后台执行
top  # 会发现a.out 的cpu使用率超过了100%
# 如果跑非常多的线程，那么cpu有几个核，就是百分之几百
cat /proc/cpuinfo | less

~~~

通过创建线程后不断地递归增加char x 变量，以增加内存占用。看Segment default时的内存就是每个线程分配到的内存。 【stack_probe.c】

运行[sum.c](./sum.c) ，演示原子性丧失

###### 编译优化 顺序性丧失

~~~shell
gcc -O1 sum.c -I. # 允许store(x)在x一定被覆盖是取消这次写,但还是有循环
objdump -d a.out | less
gcc -O2 sum.c -I. # 直接变成了sum+=n

~~~

执行多次[print\_xy.c](./print\_xy.c) ，理解可见性丧失

###### 保证顺序

禁止编译器优化变量读写

~~~c
void delay() {
  for (volatile int i = 0; i < DELAY_COUNT; i++) ;
}
~~~

在局部禁止编译器调整指令顺序

~~~c
#define barrier() asm volatile ("":::"memory") // 意思是可能改变内存的值

void foo() {
  extern int x, y;
  x++;
  barrier(); // ============================
  x++;       // 阻止 x 的访问被合并
  asm volatile("mfence" ); // x 和 y 的操作不会被交换。可以放到print_xy 里看一看
  y++;       // y 的访问不能被移到 barrier 之前
}
~~~



#### 代码课

特殊的编译方式

```shell
gcc a.c -static -wl, --entry=main
```

![1](D:\About_coding\OSLearn_jyy\chapter2\pict\1.jpg)

![2](D:\About_coding\OSLearn_jyy\chapter2\pict\2.jpg)

模拟方案：OEMU 。传奇黑客、天才程序员Fabrice Bellard 的杰作

真机方案 ：JTAG (joint Test Action Group)。

##### 调试QEMU

![3](D:\About_coding\OSLearn_jyy\chapter1\Class2\pict\3.jpg)



