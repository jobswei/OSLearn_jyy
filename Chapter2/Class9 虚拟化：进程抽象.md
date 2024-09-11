### Class9 虚拟化：进程抽象

###### （单线程）程序的状态机模型

- 绝大部分指令只能执行(*M*,*R*) 上的确定性计算
- 少数指令可能造成不确定性 (例如 rdrand)
- 唯一一个和 “外界” 交互的指令：syscall

程序的状态就是（M，R），如果我们想要看一个正在执行的程序的内存

~~~shell
ps | grep firefox
pmap [进程号]
cat /proc/[进程号]
~~~

###### 运行多个程序 = 运行多个状态机

这就是 “虚拟化”

- 程序仿佛自己独占 CPU 执行 (进程在执行时确实独占)
- 但还有它看不见的 “其他状态”

所有状态机都保存在共享内存里

- 操作系统代码借助中断、虚拟存储等机制完成切换
  - “操作系统是一个中断处理程序”
    - 被动的中断：硬件 (时钟、I/O设备、NMI, ...)
    - 主动的中断：系统调用

我想让多少程序一起执行，我就把多少程序的状态都放在内存里，我可以有特殊一条指令，可以把cpu上其他的部分都屏蔽掉，使得当前cpu只能看到一个状态机的状态，看不到其他部分，假装只有一个状态机在执行。然后执行中断之后会再切换到操作系统代码执行。

程序就是一个状态机，每一个状态机在每一时刻有一个确定的状态。操作系统就是状态机的管理者，它里面有很多状态机，每一次可以选一个状态机放在cpu上执行。

###### 经典操作系统中最重要的三类系统调用

进程 (状态机) 管理

- fork, execve, exit - 状态机的创建/改变/删除

------

存储 (地址空间) 管理

- mmap - 对进程虚拟地址空间的一部分进行映射
- brk - 虚拟地址空间管理

------

文件 (数据对象) 管理

- open, close - 文件访问管理
- read, write - 数据管理
- mkdir, link, unlink - 目录管理

##### fork()

执行fork()，复制状态机，新创建的进程返回 0，执行 fork 的进程返回进程号。返回的两个进程除了返回的进程号完全一样，pc指针也一样

![fork1](D:\About_coding\OSLearn_jyy\Chapter2\pict\fork1.jpg)

###### fork bomb

![2](D:\About_coding\OSLearn_jyy\Chapter2\pict\2.jpg)

~~~shell
:(){:|:&};:   # 刚才的一行版本

:() {         # 格式化一下
  : | : &
}; :

fork() {      # bash: 允许冒号作为标识符……
  fork | fork &
}; 
fork
~~~

~~~c
int main(){
    pid_t pid1 = fork();
    pid_t pid2 = fork();
    pid_t pid3 = fork();
    printf("Hello World from (%d, %d, %d)\n", pid1, pid2, pid3);
}
~~~

会打印出8行，每一个fork分叉一次。

系统每次只会选择一个状态机执行，所以第一次fork出来的两个状态机，他们接下来的fork不是同时发生的，不是同步的

~~~c
#define n 2

int main() {
  for (int i = 0; i < n; i++) {
    fork();
    printf("Hello\n");
  }
}
~~~

在终端运行，看到6个hello

~~~shell
./a.out | wc -l
./a.out | less
~~~

会打印出8

##### why？

~~~c
#include<stdio.h>

int main(){
    printf("hello");
    *((int *)(NULL))=0;
}
~~~

理论上来说应该先输出hello，再报错。但是由于libc会做缓存，它看到换行符才会输出一次（为了省系统调用），所以在遇到bug强行终止时，就输出不出来了。

把printf("hello")改为printf("hello\n")，就可以在终端看到输出。但是如果用管道，他会认为你在进行大量的输出，它看到换行符也不会输出了。

所以在用管道的时候，第一次输出的hello没有输出到终端，而是在缓存里。后面fork时，就把第一次缓存里的hello也复制了，所以第一层fork输出的两个hello又被复制了一遍，所以总共会输出8个。



如果线程执行fork会发生什么？

##### execve()

将当前运行的状态机重置成成另一个程序的初始状态

```
int execve(const char *filename, char * const argv, char * const envp);
```

- 执行名为 `filename` 的程序
- 允许对新状态机设置参数argv(v) 和环境变量envp(e)
  - 刚好对应了 `main()` 的参数！
  - [execve-demo.c](https://jyywiki.cn/pages/OS/2021/demos/execve-demo.c)

##### _exit()

- 立即摧毁状态机

```
void _exit(int status)
```

- 销毁当前状态机，并允许有一个返回值
- 子进程终止会通知父进程 (后续课程解释)

![exit](D:\About_coding\OSLearn_jyy\Chapter2\pict\exit.jpg)

exit_group: 消灭整个进行

exit：只消灭当前线程

##### 进程、状态机和文件描述符

程序要想与环境进行交互，就必须有一个能访问操作系统中的对象的方法

UNIX: *Everything is a file*!

- 操作系统里的对象都是文件
- 进程持有 “文件描述符” 作为指向操作系统对象的指针

这些指针是由操作系统维护的，进程是看不到的

~~~shell
cat random | head -c 10 | hexdump
lsblk
cat /dev/sdc | head -c 512 | hexdump
~~~

Everything is a file → “Everything 描述符”

- 一个 small integer
  - 0 - stdin
  - 1 - stdout
  - 2 - stderr
  - 其他由 open 创建 (总是返回最小的可用编号)
- “补完” 了操作系统视角的进程状态

一般当在终端中运行程序时，0、1、2这三个指向的都是终端



- open 返回文件描述符
- read, write 改变文件描述符

~~~c
#include<fcntl.h>

int main(){
    int fd= open("/dev/random",O_RDONLY);
    printf("%d\n",fd);
}
~~~

一般来说会返回3，（最小可用编号） 

###### 例子：备份 stdout

- OI 选手最爱的

  ```
  freopen("a.txt", "w", stdout);
  ```

  - 如何找回丢失的 stdout?
  - [stdout-bak.c](https://jyywiki.cn/pages/OS/2021/demos/stdout-bak.c)

把stdout换成一个文件，最后的输出会输出到文件里.。怎样实现呢？可以先把2指向的对象close掉，然后再重新打开a.txt ，根据最小可用编号，会自动分配到2指针下。但是也会有一些问题，如并发等。

怎样把标准输出的指针找回来？

~~~c
#include<stdio.h>
#include<unistd.h>

int main(){
    int fd=dup(STDOUT_FILENO); // int fd = dup(1);
    
    freopen("output.txt","w",stdout); // 改掉了
    printf("Hello\n");
    fclose(stdout);
    
    freopen("/dev/null","w",stdout); // 在libc里为stdout定义一个文件
    dup2(fd,STDOUT_FILENO); // dup2(fd,1);
    close(fd);
    printf("Woed\n");
}
~~~



看看进程打开了哪些文件

- `ls -l /proc/[pid]/fd`

/dev/pts/1  pts/2 是终端



查看一个进程持有的所有操作系统对象

```
lsof -p pid 
```



##### 状态机上的魔法

深度优先分叉搜索，寻找多线程程序的bug。但是回溯很难写，可以在分叉点处fork()。

fork 跳过初始化

![fork2](D:\About_coding\OSLearn_jyy\Chapter2\pict\fork2.jpg)

![fork3](D:\About_coding\OSLearn_jyy\Chapter2\pict\fork3.jpg)

![fork4](D:\About_coding\OSLearn_jyy\Chapter2\pict\fork4.jpg)

![fork5](D:\About_coding\OSLearn_jyy\Chapter2\pict\fork5.jpg)























### Class10 虚存抽象

