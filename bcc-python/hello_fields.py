from bcc import BPF

# The program for BPF
# The argument should be struct pt_reg*
prog = """
int hello(void *ctx) {
    bpf_trace_printk("Hello, World!\\n");
    return 0;
}
"""
# 
b = BPF(text=prog)
# Create a kprobe for syscall clone, and execute hello() function
b.attach_kprobe(event=b.get_syscall_fnname("clone"), fn_name="hello")

print("%-18s %-16s %-6s %s" % ("TIME(s)", "COMM", "PID", "MESSAGE"))

while 1:
    try:
        # Return a fixed set of fields from trace_pipe
        # Better use BPF_PERF_OUTPUT()
        (task, pid, cpu, flags, ts, msg) = b.trace_fields()
    except ValueError:
        continue
    print("%-18.9f %-16s %-6d %s" % (ts, task, pid, msg))