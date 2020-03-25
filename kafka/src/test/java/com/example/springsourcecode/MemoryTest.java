package com.example.springsourcecode;

import io.netty.util.internal.PlatformDependent;
import lombok.extern.slf4j.Slf4j;
import org.junit.jupiter.api.Test;
import sun.misc.Unsafe;

import java.lang.reflect.Field;

public class MemoryTest {

    @Test
    public void testUnsafe() {
        long addr = PlatformDependent.allocateMemory(20);
        Unsafe unsafe = reflectGetUnsafe();
        assert unsafe != null;
        unsafe.putInt(addr, 10);
        boolean ok = unsafe.compareAndSwapInt(null, addr, 11, 20);
        int value = unsafe.getInt(addr);
        System.out.println(ok + " " + value);
        PlatformDependent.freeMemory(addr);
    }

    private static Unsafe reflectGetUnsafe() {
        try {
            Field field = Unsafe.class.getDeclaredField("theUnsafe");
            field.setAccessible(true);
            return (Unsafe) field.get(null);
        } catch (Exception e) {
            return null;
        }
    }

}
