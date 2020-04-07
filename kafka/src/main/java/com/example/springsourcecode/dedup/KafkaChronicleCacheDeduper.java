package com.example.springsourcecode.dedup;

import lombok.Getter;
import net.openhft.chronicle.map.ChronicleMap;
import net.openhft.chronicle.map.ChronicleMapBuilder;

public class KafkaChronicleCacheDeduper {

    @Getter
    private ChronicleMap<String, Boolean> set;

    public KafkaChronicleCacheDeduper() {
        set = ChronicleMapBuilder.of(String.class, Boolean.class)
                .averageKeySize(50.0)
                .name("kafka-deduper")
                .entries(5)
                .create();

    }

    public boolean filter(String key) {
        return set.putIfAbsent(key, true) == null;
    }
}
