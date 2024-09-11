/*
 * @Author: Wei Ziyu
 * @Date: 2023-09-06 11:26:29
 * @LastEditors: Wei Ziyu
 * @LastEditTime: 2023-09-06 11:27:33
 */
#include<pthread.h>
#include<stdlib.h>
#include<assert.h>

void create(void *fn);
void join(void (*fn)()); // 等待所有线程执行结束后，调用fn
static inline void *entry_all(void *arg);

struct thread
{
    /* data */
    int id;
    pthread_t thread;
    void (*entry)(int);  // 线程的入口函数
    struct thread *next; //链表
};

struct thread *threads=NULL;
void (*join_fn)();


__attribute__((destructor)) static void join_all(){
  for(struct thread *next; threads; threads=next){
    pthread_join(threads->thread,NULL); // 在哪个线程中调用这个，哪个线程就会阻塞。而不是阻塞里面的线程
    next=threads->next;
    free(threads);
  }
  join_fn ? join_fn() : (void)0;
}
void create(void *fn){
    struct thread *cur=(struct thread *)malloc(sizeof(struct thread));
    assert(cur);
    cur->id     = threads?threads->id +1 : 1;
    cur->next = threads;
    cur->entry = (void (*)(int))fn; // 把fn强制转化为一个接收int参数返void的函数
    // (*)表示指针指向的是函数，而不是变量
    threads = cur;
    // int WINPTHREAD_API pthread_create(pthread_t *th, const pthread_attr_t *attr, void *(* func)(void *), void *arg);
    //  void *(* func) 返回空指针的函数的指针
    // cur 就作为entry_all 的参数
    pthread_create(&cur->thread,NULL,entry_all,cur);
}

static inline void *entry_all(void *arg) {
  struct thread *thread = (struct thread *)arg;
  thread->entry(thread->id);
  return NULL;
}

static inline void join(void (*fn)()){
  join_fn = fn;
}