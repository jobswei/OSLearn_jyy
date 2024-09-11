### Class6 并发控制：同步

##### 生产者-消费者问题

##### 万能的同步方法

Conditional Variables (条件变量CV)。等待，某一件事情发生

~~~c
// 需要等待条件满足时
mutex_lock(&mutex);
while (!cond) {
  wait(&cv, &mutex);
}
assert(cond); // 互斥锁必须保证退出循环时 cond 依然成立
mutex_unlock(&mutex);

// 其他线程条件可能被满足时
broadcast(&cv);
~~~

练习 fish.c

##### 信号量

 

避免同步算法，用集中调度实现。



### Class7 并发编程模型

##### 线程和协程

写并发线程会造成很多问题，互斥与同步，容易写错

协程（Coroutines）

创建多个执行流，在执行流之间切换

~~~python
def all_integers_1():
    i=0
    res=[]
    while True:
        i+=1
        res.append(i)
    return res

def all_integers():
    i = 0
    while True:
        yield (i := i + 1)
for i in filter(lambda x: x % 2, all_integers()):
    print(i)
        
~~~

~~~shell
>>> all_integers
<function all_integers at 0x108bb40d0>

>>> all_integers()
<generator object all_integers at 0x108c8cf90>
>>> a=all_integers()
>>> b=all_integers()
>>> a.__next__()
~~~

调用一次all_integerrs()就是创建一个执行流，可以创建多个并行的执行流。调用next会切换到执行流运行一次

协程放弃了并行性，但便于实现同步。线程可以并行，但会有各种并发bug。

##### Go 和 Goroutine

一个cpu上有一个Go Worker，对协程进行调度。有多个协程，

- 执行 blocking API 时 (例如 sleep, read)
  - Go Worker 偷偷改成 non-blocking 的版本
    - 成功 → 立即继续执行
    - 失败 → 立即 yield 到另一个需要 CPU 的 goroutine
      - 太巧妙了！(另一种实现：把 syscall 移到另一个线程)

例子 fib.go

##### 同步和通信

go语言，提供了生产者-消费者API



JOuerry

$是一个API

~~~javascript
$("code").html("<red>Hello</red>")

$.ajax( { url: 'http://xxx.yyy.zzz/1',
  success: function(resp) {
    $.ajax( { url: 'http://xxx.yyy.zzz/2',
      success: function(resp) {
        // do something
      },
      error: function(req, status, err) { ... }
    }
  },
  error: function(req, status, err) { ... }
);

~~~

网络处理一个时间花费多少时间永远是不知道的，比如丢包重发，大延迟等，所以 JavaScript 是一定不能设计成单线程的。

$.ajax 请求网页，会立即返回，请求事件交给专用线程去做，请求有结果之后，无论成功或失败都会在event queue中放入一个事件，等到当前事件执行完之后，event queue中刚刚返回的事件再执行。

![1](D:\About_coding\OSLearn_jyy\Class6\1.jpg)





导致 callback hell 的本质：人类脑袋里想的是 “流程图”，看到的是 “回调”。

###### Promise: 描述 Workflow 的 “嵌入式语言”

Chaining

~~~javascript
loadScript("/article/promise-chaining/one.js")
  .then( script => loadScript("/article/promise-chaining/two.js") )
  .then( script => loadScript("/article/promise-chaining/three.js") )
  .then( script => {
    // scripts are loaded, we can use functions declared there
  })
  .catch(err => { ... } );
~~~

Fork-join

~~~javascript
a = new Promise( (resolve, reject) => { resolve('A') } )
b = new Promise( (resolve, reject) => { resolve('B') } )
c = new Promise( (resolve, reject) => { resolve('C') } )
Promise.all([a, b, c]).then( res => { console.log(res) } )
~~~

###### Async-Await: Even Better

async function

- 总是返回一个 `Promise` object
- `async_func()` - fork

------

await promise

- `await promise` - join

~~~javascript

A = async () => await $.ajax('/hello/a')
B = async () => await $.ajax('/hello/b')
C = async () => await $.ajax('/hello/c')
hello = async () => await Promise.all([A(), B(), C()])
hello()
  .then(window.alert)
  .catch(res => { console.log('fetch failed!') } )
~~~

### Class8 并发Bug

需求$\rightarrow$ 设计$\rightarrow$ 实现（code）

Fault(bug)$\rightarrow$ Error $\rightarrow$ Failure

一般的程序，顺序执行，二分法找bug

但是并发程序，一次运行状态机的变化有一万种可能，几乎不可复现，也有可能相同的bug，每次导致的error都不一样，二分法失效。

QENU

ctrl+A+C 进入qrmu调试页面

![2](D:\About_coding\OSLearn_jyy\Class6\2.jpg)

##### 死锁（Deadlock）

AA-Deadlock

~~~c
void os_run() {
  spin_lock(&list_lock);
  spin_lock(&xxx);
  spin_unlock(&xxx); // ---------+
}                          //    |
                           //    |
void on_interrupt() {      //    |
  spin_lock(&list_lock);   // <--+
  spin_unlock(&list_lock);
}
~~~

ABBA-Deadlock

~~~c
void obj_move(int i, int j) {
  spin_lock(&lock[i]);
  spin_lock(&lock[j]);
  arr[i] = NULL;
  arr[j] = arr[i];
  spin_unlock(&lock[j]);
  spin_unlock(&lock[i]);
} 
~~~

obj_move本身看起来没有问题

- `obj_move(1, 2)`; `obj_move(2, 1)` → 死锁

死锁产生的四个必要条件 ([Edward G. Coffman](https://en.wikipedia.org/wiki/Edward_G._Coffman,_Jr.), 1971):

- 互斥：一个资源每次只能被一个进程使用
- 请求与保持：一个进程请求资阻塞时，不释放已获得的资源
- 不剥夺：进程已获得的资源不能强行剥夺
- 循环等待：若干进程之间形成头尾相接的循环等待资源关系



![3](D:\About_coding\OSLearn_jyy\Class6\3.jpg)

![4](D:\About_coding\OSLearn_jyy\Class6\4.jpg)

[lockdep](https://jyywiki.cn/OS/OS_Lockdep), since Linux Kernel 2.6.17; also in [OpenHarmony](https://gitee.com/openharmony)!

- 为每一个 “lock class” 检查 (分配一个 key)
  - [lock-site.c](https://jyywiki.cn/pages/OS/2021/demos/lock-site.c); 我们也可以在 xv6 里实现一个！

![5](D:\About_coding\OSLearn_jyy\Class6\5.jpg)

##### 非死锁错误

- 互斥锁 (lock/unlock) - 原子性
- 条件变量 (wait/signal) - 同步

忘记上锁——原子性违反 (Atomicity Violation, AV)

忘记同步——顺序违反 (Order Violation, OV)

![6](D:\About_coding\OSLearn_jyy\Class6\6.jpg)

![7](D:\About_coding\OSLearn_jyy\Class6\7.jpg)
