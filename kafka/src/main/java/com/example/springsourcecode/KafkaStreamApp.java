package com.example.springsourcecode;

import com.google.common.primitives.Bytes;
import org.apache.kafka.common.serialization.Serdes;
import org.apache.kafka.streams.*;
import org.apache.kafka.streams.kstream.KStream;
import org.apache.kafka.streams.kstream.KTable;
import org.apache.kafka.streams.kstream.Materialized;
import org.apache.kafka.streams.kstream.Produced;
import org.apache.kafka.streams.state.KeyValueStore;

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

        final StreamsBuilder builder = new StreamsBuilder();

        KStream<String, String> stream = builder.stream(PIPE_INPUT)
                .map((o, o2) -> {
                    System.out.println(o.toString() + " " + o2.toString());
                    return new KeyValue<>((String)o, (String)o2);
                });
        stream.to(PIPE_OUTPUT);
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
