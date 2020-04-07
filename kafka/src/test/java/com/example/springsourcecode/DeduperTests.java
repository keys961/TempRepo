package com.example.springsourcecode;

import com.example.springsourcecode.dedup.KafkaChronicleCacheDeduper;
import com.example.springsourcecode.dedup.KafkaOffHeapCacheDeduper;
import lombok.extern.slf4j.Slf4j;
import org.caffinitas.ohc.CacheSerializer;
import org.junit.jupiter.api.Test;

import java.nio.ByteBuffer;

@Slf4j
public class DeduperTests {

    @Test
    public void testChronicle() {
        KafkaChronicleCacheDeduper cacheDeduper = new KafkaChronicleCacheDeduper();
        for (int i = 0; i < 10; i++) {
            log.info("{}", cacheDeduper.filter(String.valueOf(i)));
        }

        log.info(":{}", cacheDeduper.getSet().entrySet().size());
    }

    @Test
    public void testOhc() {
        KafkaOffHeapCacheDeduper<String, String> deduper = new KafkaOffHeapCacheDeduper<>(
                v -> v,
                new CacheSerializer<>() {
                    @Override
                    public void serialize(String value, ByteBuffer buf) {
                        byte[] bytes = value.getBytes();
                        buf.putInt(bytes.length);
                        buf.put(bytes);
                    }

                    @Override
                    public String deserialize(ByteBuffer buf) {
                        int length = buf.getInt();
                        byte[] bytes = new byte[length];
                        buf.get(bytes);
                        return new String(bytes);
                    }

                    @Override
                    public int serializedSize(String value) {
                        return value.getBytes().length + 4;
                    }
                }, 1, 2000
        );
        for (int i = 0; i < 10; i++) {
            log.info("{}", deduper.filter(String.valueOf(i)));
        }
        log.info(":{}", deduper.getCache().get("1"));
        log.info(":{}", deduper.getCache().size());
    }

}
