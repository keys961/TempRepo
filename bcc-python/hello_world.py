#!/usr/bin/python

from bcc import BPF

# 1. text: BPF program in C
# 2. kprobe__sys_clone(): Using kprobe with 'kprobe__' prefix, the instrument is sys_clone()
# 3. void *ctx: ctx has arguments
# 4. bpf_trace_printk(): Kernel facility of printf()
# 5. return 0: Return normally
# 6. .trace_print(): Read from the kernel debug trace pipe and print on stdout
BPF(text='int kprobe____x64_sys_clone(void *ctx) { bpf_trace_printk("Hello, World!\\n"); return 0; }').trace_print()