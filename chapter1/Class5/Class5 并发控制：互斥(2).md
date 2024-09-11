### Class5 并发控制：互斥(2)

自旋锁：会导致时间的浪费。所有线程都需要同一把锁，那么只有一个线程可以执行，其他CPU上的线程只能围观，如果线程交替执行，那么当持有锁的线程不被执行时，会出现“全核围观”，CPU利用率为0.

##### 改进自旋锁的性能

 在获得锁失败之后，调用syscall，让该线程暂时不参与调度

```c
void yield_lock(spinlock_t *lk) {
  while (xchg(&lk->locked, 1)) {
    syscall(SYS_yield); // yield() on AbstractMachine
  }
}
void yield_unlock(spinlock_t *lk) {
  xchg(&lk->locked, 0);
}
```

还有一个更彻底的方法，lock和unlock都有操作系统来管理。

![1](D:\About_coding\OSLearn_jyy\Class5\pict\1.jpg)

操作系统内核中的互斥锁

~~~c
void mutex_lock(mutexlock_t *lk) {
  if (lk->locked != 0) {
    append(lk->wait_list, current); // 睡眠
    current->status = BLOCKED;
    yield(); // 操作系统控制了中断，有调度权
  } else {
    lk->locked = 1;
  }
}
void mutex_unlock(mutexlock_t *lk) {
  if (!is_empty(lk->wait_list)) {
    pop_front(lk->wait_list)->status = RUNNABLE; // 唤醒
  } else {
    lk->locked = 0;
  }
}
~~~

##### futex

自旋锁：线程直接共享locked，更快的fast path，更慢的slow path：等待浪费cpu。

睡眠锁：操作系统参与调度，通过操作系统访问locked，更快的slow path，更慢的fast path：每次访问，即使没有冲突也要经过操作系统。

编译运行 sum-mutex.c ，观察系统调用

```shell
strace -f ./a.out
strace -f ./z.out |&grep '] futex'
strace -f ./z.out |&grep '] futex' | wc -l
```

gdb调试

- `set scheduler-locking on`
- `info threads`; `thread X`

有的线程进入了syscall，有的直接通过自旋获得了锁

![futex](D:\About_coding\OSLearn_jyy\Class5\pict\futex.jpg)

~~~c
void futex_lock() {
  while (1) {
    int r = xchg(&locked, 1);
    if (r == 0) break;
    syscall(SYS_futex, &locked, FUTEX_WAIT, 1);
  }
}

void futex_unlock() {
  xchg(&locked, 0);
  syscall(SYS_futex, &locked, FUTEX_WAKE, 1);
}
~~~

- unlock 的时候还是有一次 syscall
- 验证正确性 futex.yaml