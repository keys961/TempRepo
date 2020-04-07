package com.example.springsourcecode;

import com.example.springsourcecode.dedup.KafkaOffHeapCacheDeduper;
import org.apache.kafka.common.serialization.Serdes;
import org.apache.kafka.streams.*;
import org.apache.kafka.streams.kstream.KStream;
import org.apache.kafka.streams.kstream.KTable;
import org.apache.kafka.streams.kstream.Materialized;
import org.apache.kafka.streams.kstream.Produced;
import org.caffinitas.ohc.CacheSerializer;

import java.nio.ByteBuffer;
import java.util.Properties;
import java.util.concurrent.CountDownLatch;

import static com.example.springsourcecode.KafkaConstants.*;

public class KafkaStreamApp {

    public static void main(String[] args) {
        Properties props = new Properties();
        props.put(StreamsConfig.APPLICATION_ID_CONFIG, "streams-pipe");
        props.put(StreamsConfig.BOOTSTRAP_SERVERS_CONFIG, "192.168.211.135:9092");
        props.put(StreamsConfig.DEFAULT_KEY_SERDE_CLASS_CONFIG, Serdes.String().getClass());
        props.put(StreamsConfig.DEFAULT_VALUE_SERDE_CLASS_CONFIG, Serdes.String().getClass());
        props.put(StreamsConfig.NUM_STREAM_THREADS_CONFIG, 2);

        final StreamsBuilder builder = new StreamsBuilder();
        final KafkaOffHeapCacheDeduper<String, String> deduper
                = new KafkaOffHeapCacheDeduper<>(
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

        KStream<String, String> stream = builder.<String, String>stream(PIPE_INPUT);
        //.filter((key, value) -> deduper.filter((String) key));
        KStream<String, String> stream1 = stream.filter((key, value) -> "0".equals(key));
        stream.map((o, o2) -> {
            System.out.println(Thread.currentThread().getName() + ": " + o + " " + o2);
            return new KeyValue<>(o, o2);
        }).to(PIPE_OUTPUT);
        stream1.map(((key, value) -> {
            System.err.println(Thread.currentThread().getName() + ": " + key + " " + value);
            return new KeyValue<>(key, value);
        })).to(PIPE_OUTPUT_1);
        KTable<String, Long> table = stream
                .groupBy((key, value) -> value)
                .count(Materialized.as("count-store"));
        table.toStream()
                .map((key, val) -> new KeyValue<>(key, String.valueOf(val)))
                .to(WC_OUTPUT, Produced.with(Serdes.String(), Serdes.String()));

        final Topology topology = builder.build();

        final KafkaStreams streams = new KafkaStreams(topology, props);
        final CountDownLatch latch = new CountDownLatch(1);

        // attach shutdown handler to catch control-c
        Runtime.getRuntime().addShutdownHook(new Thread("streams-shutdown-hook") {
            @Override
            public void run() {
                streams.close();
                latch.countDown();
            }
        });

        try {
            streams.start();
            latch.await();
        } catch (Throwable e) {
            System.exit(1);
        }
        System.exit(0);
    }
}
