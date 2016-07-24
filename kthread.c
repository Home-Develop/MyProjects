#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>

struct task_struct *producer;
struct task_struct *consumer;
int cs;

/*Consumer*/
int kthread_consumer(void *data)
{
    struct task_struct *task = NULL;
    int *cs = data, ret = 0;

    task = current;

    while (1) {
        if (!kthread_should_stop()) {
            set_current_state(TASK_INTERRUPTIBLE);
            if (*cs > 0) {
                printk("%s cs = %d\n", task->comm, (*cs)--);
                if (producer)
                    wake_up_process(producer);
            } else {
                printk("CONSUMER WILL SLEEP\n");
                /* <-------------------- Here the wake up signal is lost from the Producer*/
                // set_current_state(TASK_INTERRUPTIBLE);
                // Must unlock before schedule
                io_schedule();
            }
            set_current_state(TASK_RUNNING);
        } else {
            /*The wake up is called from the kthread_stop*/
            printk("Stopping Consumer\n");
            break;
        }
    }

    return ret;
}

/*Producer*/
int kthread_producer(void *data)
{
    struct task_struct *task = NULL;
    int *cs = data, ret = 0;

    task = current;

    while (1) {
        if (!kthread_should_stop()) {
            set_current_state(TASK_INTERRUPTIBLE);
            if (*cs <= 9) {
                printk("%s cs = %d\n", task->comm, ++(*cs));
                if (consumer)
                    wake_up_process(consumer);
            } else {
                printk("PRODUCER WILL SLEEP\n"); 
                /* <-------------------- Here the wake up signal may lost from the consumer*/
                //set_current_state(TASK_INTERRUPTIBLE);
                // Must unlock before schedule
                io_schedule();
            }
            set_current_state(TASK_RUNNING);
        } else {
            printk("Stopping Producer\n");
            break;
        }
    }

    return ret;

}
static int kernel_init(void)
{
    producer = kthread_run(kthread_producer, (void *)&cs, "Producer");
    printk("PRODUCER INITIATED\n");
    consumer = kthread_run(kthread_consumer, (void *)&cs, "Consumer");
    printk("CONSUMER INITIATED\n");

    return 0;
}

static void kernel_exit(void)
{
    int ret;

    printk("The thread is closing\n");
    ret = kthread_stop(producer);
    ret = kthread_stop(consumer);
}

module_init(kernel_init);
module_exit(kernel_exit)
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aniruddha");
