package org.example;

import io.siddhi.core.SiddhiAppRuntime;
import io.siddhi.core.SiddhiManager;
import io.siddhi.core.event.Event;
import io.siddhi.core.stream.input.InputHandler;
import io.siddhi.core.stream.output.StreamCallback;
import io.siddhi.core.util.EventPrinter;
import org.apache.log4j.BasicConfigurator;

/**
 * Hello world!
 *
 */
public class SiddhiApp {
    public static void main(String[] args) throws InterruptedException {
        BasicConfigurator.configure();

        // Create Siddhi Manager
        SiddhiManager siddhiManager = new SiddhiManager();

        //Siddhi Application
        String siddhiApp = "" +
                "define stream StockStream (symbol string, price float, volume int); " +
                "" +
                "@info(name = 'query1') " +
                "from every e1=StockStream[e1.volume < 150] -> every e2=StockStream[e2.volume >= 200]<1:> -> every e3=StockStream[e3.volume < 150] " +
                "select e1.symbol as e1, e2.symbol as e2, e3.symbol as e3 " +
                "insert into OutputStream;";

        //Generate runtime
        SiddhiAppRuntime siddhiAppRuntime = siddhiManager.createSiddhiAppRuntime(siddhiApp);

        //Adding callback to retrieve output events from stream
        siddhiAppRuntime.addCallback("OutputStream", new StreamCallback() {
            @Override
            public void receive(Event[] events) {
                EventPrinter.print(events);
                //To convert and print event as a map
                //EventPrinter.print(toMap(events));
            }
        });

        //Get InputHandler to push events into Siddhi
        InputHandler inputHandler = siddhiAppRuntime.getInputHandler("StockStream");

        //Start processing
        siddhiAppRuntime.start();

        //Sending events to Siddhi
        inputHandler.send(new Object[]{"IBM", 700f, 100});
        inputHandler.send(new Object[]{"WSO2", 60.5f, 200});
        inputHandler.send(new Object[]{"IBM", 76.6f, 400});
        inputHandler.send(new Object[]{"GOOG", 50f, 30});
        inputHandler.send(new Object[]{"IBM", 76.6f, 400});
        inputHandler.send(new Object[]{"WSO2", 45.6f, 50});
        Thread.sleep(500);

        //Shutdown runtime
        siddhiAppRuntime.shutdown();

        //Shutdown Siddhi Manager
        siddhiManager.shutdown();
    }
}
