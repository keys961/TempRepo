package org.yejt.client;

import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.Watcher;
import org.apache.zookeeper.ZooKeeper;
import org.apache.zookeeper.data.Stat;
import org.yejt.zk.ZookeeperConnection;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.*;
import java.util.concurrent.CountDownLatch;

public class Client
{
    private static List<String> servers = new ArrayList<>(16);

    private static CountDownLatch countDownLatch = new CountDownLatch(1);

    private static ZooKeeper zooKeeper;

    private static ZookeeperConnection connection = new ZookeeperConnection();

    public static void main(String[] args) throws IOException, InterruptedException, KeeperException
    {
        String host = "192.168.50.20:2181";

        initServerList(host);

        Client client = new Client();
        BufferedReader console = new BufferedReader(new InputStreamReader(System.in));
        while (true)
        {
            String name;
            try
            {
                name = console.readLine();
                if("exit".equals(name))
                    System.exit(0);
                client.send(name);
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
        }
    }

    private static void initServerList(String host) throws IOException, InterruptedException, KeeperException
    {
        String path = "/test";
        connectToZookeeper(host);
        servers.clear();
        Stat stat = zooKeeper.exists(path, true);
        if(stat != null)
        {
            List<String> children = zooKeeper.getChildren(path, event ->
            {
                if (event.getType() == Watcher.Event.EventType.NodeChildrenChanged)
                {
                    servers.clear();
                    System.out.println("Watcher called...");
                    List<String> childrens = null;
                    try
                    {
                        childrens = zooKeeper.getChildren(path, true);
                    }
                    catch (KeeperException | InterruptedException e)
                    {
                        e.printStackTrace();
                        return;
                    }

                    for (String child : childrens)
                    {
                        byte[] data;
                        try
                        {
                            data = zooKeeper.getData(path + "/" + child,
                                    null, null);
                        }
                        catch (KeeperException | InterruptedException e)
                        {
                            e.printStackTrace();
                            continue;
                        }
                        servers.add(new String(data));
                    }
                }
            });

            for(String child : children)
            {
                byte[] data = zooKeeper.getData(path + "/" + child,
                        null, null);
                servers.add(new String(data));
            }
        }
    }

    private static void connectToZookeeper(String host) throws IOException, InterruptedException
    {
        zooKeeper = connection.connect(host);
    }

    public static String getServer()
    {
        if(servers.size() == 0)
            return null;
        Random r = new Random(System.currentTimeMillis());
        return servers.get(r.nextInt(servers.size()));
    }

    public Client()
    { }

    public void send(String name)
    {
        String server = Client.getServer();
        String[] cfg = server.split(":");

        Socket socket = null;
        BufferedReader in = null;
        PrintWriter out = null;
        try
        {
            socket = new Socket(cfg[0], Integer.parseInt(cfg[1]));
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintWriter(socket.getOutputStream(), true);

            out.println(name);
            while(true)
            {
                String resp = in.readLine();
                if(resp == null)
                    break;
                else if(resp.length() > 0)
                {
                    System.out.println("Receive : " + resp);
                    break;
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        finally
        {
            if (out != null)
                out.close();
            if (in != null)
                try
                {
                    in.close();
                }
                catch (IOException e)
                {
                    e.printStackTrace();
                }
            }
            if (socket != null)
            {
                try
                {
                    socket.close();
                }
                catch (IOException e)
                {
                    e.printStackTrace();
                }
            }
        }
}
