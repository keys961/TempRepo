from __future__ import print_function
from bcc import BPF
from time import sleep

# load BPF program
# PT_REGS_PARM1(ctx): Fetch the first argument attached function
# here is const char* in strlen()
# 
# bpf_probe_read_user(): read first arg on ctx into key.c, with sizeof(key.c)
b = BPF(text="""
#include <uapi/linux/ptrace.h>

struct key_t {
    char c[180];
};
BPF_HASH(counts, struct key_t);

int count(struct pt_regs *ctx) {
    if (!PT_REGS_PARM1(ctx))
        return 0;

    struct key_t key = {};
    u64 zero = 0, *val;

    bpf_probe_read_user(&key.c, sizeof(key.c), (void *)PT_REGS_PARM1(ctx));
    // could also use `counts.increment(key)`
    val = counts.lookup_or_try_init(&key, &zero);
    if (val) {
      (*val)++;
    }
    return 0;
};
""")

# Attach user level function using uprobe
# Library is 'c'
# Function is strlen
# When executing strlen, count will be executed
b.attach_uprobe(name="c", sym="strlen", fn_name="count")

# header
print("Tracing strlen()... Hit Ctrl-C to end.")

# sleep until Ctrl-C
try:
    sleep(99999999)
except KeyboardInterrupt:
    pass

# print output
print("%10s %s" % ("COUNT", "STRING"))
counts = b.get_table("counts")
for k, v in sorted(counts.items(), key=lambda counts: counts[1].value):
    print("%10d \"%s\"" % (v.value, k.c.encode('string-escape')))