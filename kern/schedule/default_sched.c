#include <defs.h>
#include <list.h>
#include <proc.h>
#include <assert.h>
#include <default_sched.h>

#ifdef LAB6_EX2

// LAB6 EXERCISE2: YOUR CODE
// Write your Stride Scheduling here.
/* 定义 BIG_STRIDE，通常使用最大有符号整数，以保证溢出计算的正确性 */
#define BIG_STRIDE    0x7FFFFFFF /* should be > 1 */
    /* * stride_init: 
    * 初始化运行队列。
    * Stride 调度器使用斜堆 (skew_heap) 来管理就绪进程，
    * 所以初始化 lab6_run_pool 为 NULL。
    */
    static void
    stride_init(struct run_queue *rq) {
        /* LAB6: YOUR CODE */
        list_init(&(rq->run_list)); // 初始化运行队列链表（虽然主要用堆，但保留链表结构是个好习惯）
        rq->lab6_run_pool = NULL;   // 初始化斜堆为空
        rq->proc_num = 0;           // 进程数为0
    }

    /* * proc_stride_comp_f: 
    * 比较两个进程的 stride 值。
    * 返回值：
    * -1: a < b (a 的优先级更高/stride更小)
    * 0: a == b
    * 1: a > b
    */
    static int
    proc_stride_comp_f(void *a, void *b) {
        struct proc_struct *p = le2proc(a, lab6_run_pool);
        struct proc_struct *q = le2proc(b, lab6_run_pool);
        int32_t c = p->lab6_stride - q->lab6_stride;
        if (c > 0) return 1;
        else if (c == 0) return 0;
        else return -1;
    }

    /* * stride_enqueue: 
    * 将进程加入运行队列。
    * 1. 初始化时间片（如果是刚耗尽时间片的进程）。
    * 2. 将进程插入斜堆（根据 stride 排序）。
    * 3. 更新进程计数。
    * 4. 设置 process stride
    */
    static void
    stride_enqueue(struct run_queue *rq, struct proc_struct *proc) {
        /* LAB6: YOUR CODE */
        // 使用 skew_heap_insert 将进程加入优先队列
        rq->lab6_run_pool = skew_heap_insert(rq->lab6_run_pool, &(proc->lab6_run_pool), proc_stride_comp_f);

        // 如果时间片用完或是0，重置为最大时间片
        if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) {
            proc->time_slice = rq->max_time_slice;
        }
        
        proc->rq = rq;
        rq->proc_num++;
    }

    /* * stride_dequeue: 
    * 将进程从运行队列移除。
    * 1. 从斜堆中删除该进程。
    * 2. 更新进程计数。
    */
    static void
    stride_dequeue(struct run_queue *rq, struct proc_struct *proc) {
        /* LAB6: YOUR CODE */
        // 使用 skew_heap_remove 从优先队列移除进程
        rq->lab6_run_pool = skew_heap_remove(rq->lab6_run_pool, &(proc->lab6_run_pool), proc_stride_comp_f);
        rq->proc_num--;
    }

    /* * stride_pick_next: 
    * 选择下一个要运行的进程。
    * 1. 如果队列为空，返回 NULL。
    * 2. 取出斜堆顶部的进程（stride 最小的）。
    * 3. 更新该进程的 stride 值：stride += BIG_STRIDE / priority。
    */
    static struct proc_struct *
    stride_pick_next(struct run_queue *rq) {
        /* LAB6: YOUR CODE */
        if (rq->lab6_run_pool == NULL) {
            return NULL;
        }
        
        // 获取堆顶元素对应的进程
        struct proc_struct *p = le2proc(rq->lab6_run_pool, lab6_run_pool);
        
        // 更新 stride
        // pass = BIG_STRIDE / priority
        // 优先级默认为 1，防止除以 0
        uint32_t priority = p->lab6_priority;
        if (priority == 0) priority = 1;
        
        p->lab6_stride += BIG_STRIDE / priority;
        
        return p;
    }

    /* * stride_proc_tick: 
    * 时钟中断处理。
    */
    static void
    stride_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
        /* LAB6: YOUR CODE */
        if (proc->time_slice > 0) {
            proc->time_slice--;
        }
        if (proc->time_slice == 0) {
            proc->need_resched = 1;
        }
    }

    /* 定义调度类 */
    struct sched_class default_sched_class = {
        .name = "stride_scheduler",
        .init = stride_init,
        .enqueue = stride_enqueue,
        .dequeue = stride_dequeue,
        .pick_next = stride_pick_next,
        .proc_tick = stride_proc_tick,
    };
#else
    static void
    RR_init(struct run_queue *rq) {
        list_init(&(rq->run_list));
        rq->proc_num = 0;
    }

    static void
    RR_enqueue(struct run_queue *rq, struct proc_struct *proc) {
        assert(list_empty(&(proc->run_link)));
        list_add_before(&(rq->run_list), &(proc->run_link));
        if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) {
            proc->time_slice = rq->max_time_slice;
        }
        proc->rq = rq;
        rq->proc_num ++;
    }

    static void
    RR_dequeue(struct run_queue *rq, struct proc_struct *proc) {
        assert(!list_empty(&(proc->run_link)) && proc->rq == rq);
        list_del_init(&(proc->run_link));
        rq->proc_num --;
    }

    static struct proc_struct *
    RR_pick_next(struct run_queue *rq) {
        list_entry_t *le = list_next(&(rq->run_list));
        if (le != &(rq->run_list)) {
            return le2proc(le, run_link);
        }
        return NULL;
    }

    static void
    RR_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
        if (proc->time_slice > 0) {
            proc->time_slice --;
        }
        if (proc->time_slice == 0) {
            proc->need_resched = 1;
        }
    }

    struct sched_class default_sched_class = {
        .name = "RR_scheduler",
        .init = RR_init,
        .enqueue = RR_enqueue,
        .dequeue = RR_dequeue,
        .pick_next = RR_pick_next,
        .proc_tick = RR_proc_tick,
    };
#endif

