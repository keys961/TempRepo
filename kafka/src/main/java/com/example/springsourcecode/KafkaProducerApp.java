package com.example.springsourcecode;

import org.apache.kafka.clients.producer.KafkaProducer;
import org.apache.kafka.clients.producer.ProducerRecord;
import org.apache.kafka.common.serialization.StringSerializer;

import java.util.Properties;
import java.util.Random;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.atomic.AtomicInteger;

import static com.example.springsourcecode.KafkaConstants.PIPE_INPUT;

public class KafkaProducerApp {

    public static void main(String[] args) throws InterruptedException, ExecutionException {
        Properties properties = new Properties();
        properties.put("bootstrap.servers", "192.168.211.135:9092");
        properties.put("key.serializer", StringSerializer.class);
        properties.put("value.serializer", StringSerializer.class);

        AtomicInteger integer = new AtomicInteger();
        KafkaProducer<String, String> producer = new KafkaProducer<>(properties);
        Random r = new Random(1);
        Runtime.getRuntime().addShutdownHook(new Thread("streams-shutdown-hook") {
            @Override
            public void run() {
                producer.close();
            }
        });
        while (true) {
            producer.send(new ProducerRecord<>(PIPE_INPUT, String.valueOf(integer.getAndIncrement() % 10), String.valueOf(r.nextInt(10)))).get();
            Thread.sleep(2000);
        }
    }
}
