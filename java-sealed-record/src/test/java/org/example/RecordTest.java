package org.example;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

import java.io.Closeable;
import java.io.IOException;

public class RecordTest {
    @Test
    public void testRecord() {
        Record r = new Record(1, "2");
        Record rr = new Record(1, "2");
        Record rrr = new Record();
        System.out.println(r.b());
        System.out.println(r);
        System.out.println(r.hashCode());
        System.out.println(rrr.fuck());
        Assertions.assertEquals(r, rr);
    }
}

record Record(int a, String b) implements Closeable {
    Record () {
        this(1, "123");
    }

    String fuck() {
        return a + "b" + b;
    }

    @Override
    public void close() throws IOException {
    }
}
