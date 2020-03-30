package org.yejt;

import org.apache.flink.api.common.functions.AggregateFunction;
import org.apache.flink.api.common.functions.FilterFunction;
import org.apache.flink.api.common.functions.MapFunction;
import org.apache.flink.api.java.tuple.Tuple1;
import org.apache.flink.api.java.tuple.Tuple2;
import org.apache.flink.api.java.tuple.Tuple3;
import org.apache.flink.streaming.api.TimeCharacteristic;
import org.apache.flink.streaming.api.datastream.DataStream;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.streaming.api.functions.AssignerWithPeriodicWatermarks;
import org.apache.flink.streaming.api.watermark.Watermark;
import org.apache.flink.streaming.api.windowing.time.Time;
import org.apache.flink.util.StringUtils;

/**
 * Hello world!
 *
 */
@SuppressWarnings("unchecked")
public class App {
    public static void main(String[] args) throws Exception {
        //Local env if started at IDE
        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
        //Set EventTime
        env.setStreamTimeCharacteristic(TimeCharacteristic.EventTime);
        env.setParallelism(1);
        //Emit watermark per 9 sec
        env.getConfig().setAutoWatermarkInterval(9000);
        DataStream<String> text = env.readTextFile("data.txt");
        DataStream<Tuple2<String, Integer>> counts = text.filter(new FilterClass()).map(new LineSplitter())
                .assignTimestampsAndWatermarks(new AssignerWithPeriodicWatermarks<Tuple3<String, Long, Integer>>() {
                    private long currentMaxTimestamp = 0L;

                    // get event time
                    @Override
                    public long extractTimestamp(Tuple3<String, Long, Integer> element, long previousElementTimestamp) {
                        long timestamp = element.f1;
                        currentMaxTimestamp = Math.max(timestamp, currentMaxTimestamp);
                        return timestamp;
                    }

                    // emit watermark
                @Override
                public Watermark getCurrentWatermark() {
                    long maxOutOfOrder = 10000L;
                    System.out.println("Wall clock = " + System.currentTimeMillis() + "; New watermark = "
                            + (currentMaxTimestamp - maxOutOfOrder));
                    return new Watermark(currentMaxTimestamp - maxOutOfOrder);
                }
                })
                // key by tuple.f0
                .keyBy(0)
                // 20 sec window size without slicing
                .timeWindow(Time.seconds(20))
                // sum by tuple.f2
                .aggregate(new AggregateFunction<Tuple3<String, Long, Integer>, Integer, Integer>() {
                    @Override
                    public Integer createAccumulator() {
                        return 0;
                    }

                    @Override
                    public Integer add(Tuple3<String, Long, Integer> stringLongIntegerTuple3, Integer o) {
                        return o + stringLongIntegerTuple3.f2;
                    }

                    @Override
                    public Integer getResult(Integer o) {
                        return o;
                    }

                    @Override
                    public Integer merge(Integer o, Integer acc1) {
                        return o + acc1;
                    }
                }, (key, window, aggregateResult, out) -> out.collect(new Tuple2<>(((Tuple1<String>) key).f0, aggregateResult.iterator().next())));
        counts.print();
        env.execute("Windowed WordCount");

    }

    public static final class LineSplitter implements MapFunction<String, Tuple3<String, Long, Integer>> {

        @Override
        public Tuple3<String, Long, Integer> map(String value) {
            // <key, event_time, count>
            String[] tokens = value.toLowerCase().split("\\W+");
            long eventTime = Long.parseLong(tokens[1]);
            return new Tuple3<>(tokens[0], eventTime, 1);
        }
    }

    public static final class FilterClass implements FilterFunction<String> {
        @Override
        public boolean filter(String value) {
            return !StringUtils.isNullOrWhitespaceOnly(value);
        }
    }
}
