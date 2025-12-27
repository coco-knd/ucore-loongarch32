#include <stdio.h>
#include <monitor.h>
#include <kmalloc.h>
#include <assert.h>


// Initialize monitor.
void     
monitor_init (monitor_t * mtp, size_t num_cv) {
    int i;
    assert(num_cv>0);
    mtp->next_count = 0;
    mtp->cv = NULL;
    sem_init(&(mtp->mutex), 1); //unlocked
    sem_init(&(mtp->next), 0);
    mtp->cv =(condvar_t *) kmalloc(sizeof(condvar_t)*num_cv);
    assert(mtp->cv!=NULL);
    for(i=0; i<num_cv; i++){
        mtp->cv[i].count=0;
        sem_init(&(mtp->cv[i].sem),0);
        mtp->cv[i].owner=mtp;
    }
}

// Unlock one of threads waiting on the condition variable. 
void 
cond_signal (condvar_t *cvp) {
#ifdef LAB7_EX1
   //LAB7 EXERCISE1: YOUR CODE
   // 如果有进程在等待这个条件变量 (cvp->count > 0)
   if(cvp->count > 0) {
      // 1. 修改管程状态：当前进程（唤醒者）将进入 next 队列睡眠，所以 next_count 加 1
      cvp->owner->next_count ++;
      
      // 2. 唤醒等待在条件变量上的进程
      up(&(cvp->sem));
      
      // 3. 唤醒者自己睡眠在 next 信号量上，等待被唤醒
      down(&(cvp->owner->next));
      
      // 4. 唤醒者醒来后，next_count 减 1
      cvp->owner->next_count --;
   }
#endif
     if(cvp->count>0) {
        cvp->owner->next_count ++;
        up(&(cvp->sem));
        down(&(cvp->owner->next));
        cvp->owner->next_count --;
      }
   kprintf("cond_signal end: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}

// Suspend calling thread on a condition variable waiting for condition Atomically unlocks 
// mutex and suspends calling thread on conditional variable after waking up locks mutex. Notice: mp is mutex semaphore for monitor's procedures
void
cond_wait (condvar_t *cvp) {
#ifdef LAB7_EX1
    //LAB7 EXERCISE1: YOUR CODE
   // 1. 增加等待该条件变量的进程计数
    cvp->count++;
    
    // 2. 释放管程锁。
    // 如果有高优先级的进程（即之前发出 signal 而睡眠的进程）在 next 队列中，则唤醒它。
    // 否则，释放互斥锁 mutex，允许新进程进入管程。
    if(cvp->owner->next_count > 0)
       up(&(cvp->owner->next));
    else
       up(&(cvp->owner->mutex));
    
    // 3. 自身睡眠在条件变量的信号量上
    down(&(cvp->sem));
    
    // 4. 醒来后，等待计数减 1
    cvp->count --;
#endif
      cvp->count++;
      if(cvp->owner->next_count > 0)
         up(&(cvp->owner->next));
      else
         up(&(cvp->owner->mutex));
      down(&(cvp->sem));
      cvp->count --;
    kprintf("cond_wait end:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}
