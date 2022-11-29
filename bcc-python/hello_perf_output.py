from bcc import BPF

# BPF_PERF_OUTPUT(events): create an output channel named 'events'
# bpf_get_current_pid_tgid(): return the process ID (lower 32 bits) and thread group ID (upper 32 bits)
# bpf_get_current_comm(): Populates the first argument address, which is the current process name
# events.perf_submit(): Submit the event for user space to read via a perf ring buffer.
prog = """
#include <linux/sched.h>

// define output data structure in C
struct data_t {
    u32 pid;
    u64 ts;
    char comm[TASK_COMM_LEN];
};
BPF_PERF_OUTPUT(events);

int hello(struct pt_regs *ctx) {
    struct data_t data = {};

    data.pid = bpf_get_current_pid_tgid();
    data.ts = bpf_ktime_get_ns();
    bpf_get_current_comm(&data.comm, sizeof(data.comm));

    events.perf_submit(ctx, &data, sizeof(data));

    return 0;
}
"""

# load BPF program
b = BPF(text=prog)
b.attach_kprobe(event=b.get_syscall_fnname("clone"), fn_name="hello")

# header
print("%-18s %-16s %-6s %s" % ("TIME(s)", "COMM", "PID", "MESSAGE"))

# process event
start = 0
def print_event(cpu, data, size):
    global start
    event = b["events"].event(data)
    if start == 0:
            start = event.ts
    time_s = (float(event.ts - start)) / 1000000000
    print("%-18.9f %-16s %-6d %s" % (time_s, event.comm, event.pid,
        "Hello, perf_output!"))

# loop with callback to print_event
# Associate the Python print_event function with the events stream.
# b['events']: Fetch the channel names 'events' defined in BPF C program.
b["events"].open_perf_buffer(print_event)
while 1:
    b.perf_buffer_poll()