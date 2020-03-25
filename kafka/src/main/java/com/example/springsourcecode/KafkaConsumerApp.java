package com.example.springsourcecode;

import com.google.common.collect.Lists;
import org.apache.kafka.clients.consumer.ConsumerRecords;
import org.apache.kafka.clients.consumer.KafkaConsumer;
import org.apache.kafka.common.errors.WakeupException;
import org.apache.kafka.common.serialization.StringDeserializer;
import org.apache.kafka.common.serialization.StringSerializer;

import java.time.Duration;
import java.util.Collections;
import java.util.Properties;

import static com.example.springsourcecode.KafkaConstants.PIPE_OUTPUT;
import static com.example.springsourcecode.KafkaConstants.WC_OUTPUT;

public class KafkaConsumerApp {

    public static void main(String[] args) {
        Properties properties = new Properties();
        properties.put("bootstrap.servers", "192.168.211.135:9092");
        properties.put("group.id", "consumer");
        properties.put("key.deserializer", StringDeserializer.class);
        properties.put("value.deserializer", StringDeserializer.class);

        KafkaConsumer<String, String> consumer = new KafkaConsumer<>(properties);
        consumer.subscribe(Lists.newArrayList(PIPE_OUTPUT, WC_OUTPUT));
        Runtime.getRuntime().addShutdownHook(new Thread("streams-shutdown-hook") {
            @Override
            public void run() {
                consumer.wakeup();
            }
        });

        try {
            while (true) {
                ConsumerRecords<String, String> records = consumer.poll(Duration.ofSeconds(10L));
                records.forEach(record -> {
                    System.out.printf("topic = %s, partition = %d, offset = %d, key = %s, value = %s.\n",
                            record.topic(), record.partition(), record.offset(), record.key(), record.value());
                    consumer.commitAsync();
                });

            }
        } catch (WakeupException e) {
            // Ignored
        } finally {
            consumer.close();
        }

    }
}
