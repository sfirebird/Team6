/* implementing circular buffer log messages */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>

#define LOG_BUFFER_SIZE 4096
#define PROC_FILENAME "custom_logger"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Custom Kernel Logger");

static char *log_buffer;
static size_t write_index = 0;
static spinlock_t log_lock;

// Function to write logs to the buffer
static void log_message(const char *level, const char *fmt, va_list args) {
    size_t len;
    unsigned long flags;
    u64 ns_since_boot; // Nanoseconds since boot
    unsigned long sec, msec; // Seconds and milliseconds

    ns_since_boot = ktime_get_ns(); // Get nanoseconds since boot
    sec = div_u64(ns_since_boot, NSEC_PER_SEC); // Convert to seconds
    msec = div_u64(ns_since_boot % NSEC_PER_SEC, 1000000); // Milliseconds


    /*ensure thread safety while writting to the shared log_buffer 
    to avoid the race condition */
    spin_lock_irqsave(&log_lock, flags);

    // Format the timestamp with log level
    len = snprintf(log_buffer + write_index, LOG_BUFFER_SIZE - write_index,
                   "[%5lu.%03lu] %s: ", sec, msec, level);
    
    if(len < LOG_BUFFER_SIZE - write_index)
    {
        /* used to format string using variable argument lists and it returns number of bytes it written into the buffer  */
        len += vsnprintf(log_buffer + write_index + len, LOG_BUFFER_SIZE - write_index - len, fmt, args);
    }

    if (len >= LOG_BUFFER_SIZE - write_index) {
        len = LOG_BUFFER_SIZE - write_index;
    }
    /* just maintaining the circular buffer below when it reaches the last byte again
     it pointes to the 0th index of log_buffer */

    write_index = (write_index + len) % LOG_BUFFER_SIZE;

    if (write_index + 1 < LOG_BUFFER_SIZE) {
        log_buffer[write_index++] = '\n';
        log_buffer[write_index] = '\0';
    }
/*what is the actually difference between spin_lock and spin_lock_irqsave*/
    spin_unlock_irqrestore(&log_lock, flags);
}

// Log level wrappers
void log_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_message("DEBUG", fmt,args);
    va_end(args);
}

void log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_message("INFO", fmt, args);
    va_end(args);
}

void log_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_message("WARN", fmt,args);
    va_end(args);
}

void log_error(const char *fmt, ...) {
   va_list args;
    va_start(args, fmt);
    log_message("ERROR", fmt, args);
    va_end(args);
}

// Export the log level wrappers for use by other kernel modules
EXPORT_SYMBOL(log_debug);
EXPORT_SYMBOL(log_info);
EXPORT_SYMBOL(log_warn);
EXPORT_SYMBOL(log_error);

// seq_file callbacks
static void *seq_start(struct seq_file *m, loff_t *pos) {
    if (*pos == 0) {
        return log_buffer; // Start reading from the buffer
    }
    return NULL; // End of sequence
}

static void *seq_next(struct seq_file *m, void *v, loff_t *pos) {
    (*pos)++;
    return NULL; // No more data to iterate
}

static void seq_stop(struct seq_file *m, void *v) {
    // No special cleanup needed
}

static int seq_show(struct seq_file *m, void *v) {
    unsigned long flags;

    spin_lock_irqsave(&log_lock, flags);
    seq_printf(m, "%s", (char *)v);
    spin_unlock_irqrestore(&log_lock, flags);

    return 0;
}
/*i am not able to undestand about this seq_operations */
static const struct seq_operations seq_ops = {
    .start = seq_start,
    .next  = seq_next,
    .stop  = seq_stop,
    .show  = seq_show,
};

// File operations using seq_read
static int proc_open(struct inode *inode, struct file *file) {
    return seq_open(file, &seq_ops);
}   

static const struct proc_ops proc_fops = {
    .proc_open    = proc_open,
    .proc_read    = seq_read, // Use seq_read to read data
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};

// Module initialization
static int __init logger_init(void) {

    /* allocatting memory of size 4096 and returning base adress to the log_buffer */
    log_buffer = kmalloc(LOG_BUFFER_SIZE, GFP_KERNEL);
    if (!log_buffer) {
        printk(KERN_ERR "Custom Logger:Failed to allocate memory\n");
        return -ENOMEM; 
    }

    memset(log_buffer, 0, LOG_BUFFER_SIZE);
    spin_lock_init(&log_lock);

    if (!proc_create(PROC_FILENAME, 0, NULL, &proc_fops)) {
        kfree(log_buffer);
        printk(KERN_ERR "Custom Logger: Failed to create proc entry\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "Custom Logger: Module loaded\n");
    log_info("Logger initialized successfully!");
    return 0;
}

// Module cleanup
static void __exit logger_exit(void) {
    remove_proc_entry(PROC_FILENAME, NULL);
    kfree(log_buffer);
    printk(KERN_INFO "Custom Logger: Module unloaded\n");
}

module_init(logger_init);
module_exit(logger_exit);
