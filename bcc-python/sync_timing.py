from __future__ import print_function
from bcc import BPF

# BPF_HASH(last): a macro that create a hashmap named 'last'
prog = """
#include <uapi/linux/ptrace.h>

BPF_HASH(last); // Create a BPF map named 'last'

int do_trace(struct pt_reg* ctx) {
    u64 ts, *tsp, delta, key = 0;
    tsp = last.lookup(&key); // Get timestamp from the map

    if (tsp == NULL) {
        delta = bpf_ktime_get_ns() - *tsp;
        if (delta < 1000000000) {
            // Detect 2 sync() interval
            // Output if time interval is less than 1 second
            bpf_trace_printk("%d\\n", delta / 1000000);
        }
        last.delete(&key);
    }

    // Update stored timestamp
    ts = bpf_ktime_get_ns();
    last.update(&key, &ts);
    return 0;
}
"""

# Load the BPF program
b = BPF(text=prog)
b.attach_kprobe(event=b.get_syscall_fnname("sync"), fn_name="do_trace")

print("Tracing for quick sync's... Ctrl-C to end")

# format output
start = 0
while 1:
    (task, pid, cpu, flags, ts, ms) = b.trace_fields()
    if start == 0:
        start = ts
    ts = ts - start
    print("At time %.2f s: multiple syncs detected, last %s ms ago" % (ts, ms))