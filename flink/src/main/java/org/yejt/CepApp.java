package org.yejt;

import org.apache.flink.cep.CEP;
import org.apache.flink.cep.PatternSelectFunction;
import org.apache.flink.cep.PatternStream;
import org.apache.flink.cep.pattern.Pattern;
import org.apache.flink.cep.pattern.conditions.SimpleCondition;
import org.apache.flink.streaming.api.TimeCharacteristic;
import org.apache.flink.streaming.api.datastream.DataStream;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.streaming.api.functions.AssignerWithPeriodicWatermarks;
import org.apache.flink.streaming.api.watermark.Watermark;
import org.apache.flink.streaming.api.windowing.time.Time;
import org.yejt.pojo.ComplexEvent;
import org.yejt.pojo.Event;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;


public class CepApp {

    private static final AtomicInteger ID_GENERATOR = new AtomicInteger(0);

    public static void main(String[] args) throws Exception {
        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
        env.setStreamTimeCharacteristic(TimeCharacteristic.EventTime);
        env.setParallelism(1);

        DataStream<Event> input = env.readTextFile("cep_data.txt")
                .map(line -> {
                    String[] elements = line.split("\\s+");
                    return new Event(Integer.parseInt(elements[0]), System.currentTimeMillis(), elements[1]);
                })
                .assignTimestampsAndWatermarks(new AssignerWithPeriodicWatermarks<Event>() {
                    private long currentMaxTime = 0L;
                    @Override
                    public Watermark getCurrentWatermark() {
                        // Perfect watermarks
                        return new Watermark(currentMaxTime);
                    }
                    @Override
                    public long extractTimestamp(Event element, long previousElementTimestamp) {
                        this.currentMaxTime = Math.max(currentMaxTime, element.getTime());
                        return element.getTime();
                    }
                });

        DataStream<Event> partitionedInput = input.keyBy(Event::getId);

        Pattern<Event, ?> pattern = Pattern.<Event>begin("start")
                .next("middle").where(new SimpleCondition<Event>() {
                    @Override
                    public boolean filter(Event value) throws Exception {
                        return value.getName().equals("error");
                    }
                }).followedBy("end").where(new SimpleCondition<Event>() {
                    @Override
                    public boolean filter(Event value) throws Exception {
                        return value.getName().equals("critical");
                    }
                }).within(Time.seconds(10));

        PatternStream<Event> patternStream = CEP.pattern(partitionedInput, pattern);
        DataStream<ComplexEvent> ceStream = patternStream.select(new PatternSelectFunction<Event, ComplexEvent>() {
            @Override
            public ComplexEvent select(Map<String, List<Event>> map) throws Exception {
                System.out.println(map);
                List<Event> eventList = new ArrayList<>();
                map.forEach((k, v) -> eventList.addAll(v));
                eventList.sort((e1, e2) -> (int) (e1.getTime() - e2.getTime()));
                return new ComplexEvent(eventList.get(0).getId(), eventList);
            }
        });
        ceStream.print();
        env.execute("cep-pattern");
    }
}
