package com.example.springsourcecode.dedup;

import lombok.Getter;
import org.caffinitas.ohc.CacheSerializer;
import org.caffinitas.ohc.Eviction;
import org.caffinitas.ohc.OHCache;
import org.caffinitas.ohc.OHCacheBuilder;

import java.io.Serializable;
import java.nio.ByteBuffer;
import java.util.function.Function;

public class KafkaOffHeapCacheDeduper<T, K extends Serializable> {

    private final Function<T, K> keySelector;
    private final CacheSerializer<K> keySerializer;
    private final long expirationTimeHour;
    /**
     * size in bytes
     */
    private final long maxSize;
    @Getter
    private OHCache<K, Boolean> cache;

    public KafkaOffHeapCacheDeduper(Function<T, K> keySelector, CacheSerializer<K> keySerializer,
                                    long expirationTimeHour, long maxSize) {
        this.keySelector = keySelector;
        this.keySerializer = keySerializer;
        this.expirationTimeHour = expirationTimeHour;
        this.maxSize = maxSize;
        init();
    }

    private void init() {
        this.cache = OHCacheBuilder
                .<K, Boolean>newBuilder()
                .timeouts(true)
                .keySerializer(keySerializer)
                .valueSerializer(new CacheSerializer<>() {
                    @Override
                    public void serialize(Boolean value, ByteBuffer buf) {
                        buf.put(value ? (byte) 1 : (byte) 0);
                    }

                    @Override
                    public Boolean deserialize(ByteBuffer buf) {
                        byte val = buf.get();
                        return val == 1;
                    }

                    @Override
                    public int serializedSize(Boolean value) {
                        return 1;
                    }
                })
                .defaultTTLmillis(expirationTimeHour * 3600 * 1000L)
                .capacity(maxSize)
                .eviction(Eviction.LRU)
                .build();
    }

    public boolean filter(T value) {
        K key = keySelector.apply(value);
        return cache.putIfAbsent(key, true);
    }
}
