### Class3

互斥算法，实现一段任意代码的原子性

```c
void do_sum() {
  for (int i = 0; i < n; i++) {
    lock(); // 保证顺序、原子性、可见性
    // critical section; 临界区
    sum++;
    unlock();
  }
}
```

#### Peterson算法

----

假设你和你舍友的行动速度都太快了，必须有一个机制保证你们无法同时进入 WC

- 一个人瞬间只能完成
  1. 举起/放下自己手里的旗
  2. 向厕所门上贴一个标签 (覆盖之前贴的任何内容)
  3. 查看旗或标签的状态
- 想上厕所的人
  1. 举起自己的旗
  2. 贴上有对方名字的标签 (谦让对方)
  3. 当对方没有举旗或标签写着自己名字时，进入 WC
  4. 出 WC 时放下旗子

```c
int turn, x = 0, y = 0;

void thread1() {
  x = 1; turn = T2;
  while (y && turn == T2) ;
  // critical section
  x = 0;
}

void thread2() {
  y = 1; turn = T1;
  while (x && turn == T1) ;
  // critical section
  y = 0;
}
```

##### 代码课

