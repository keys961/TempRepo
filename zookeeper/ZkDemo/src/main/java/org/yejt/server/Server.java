package org.yejt.server;

import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.ZooDefs;
import org.apache.zookeeper.ZooKeeper;
import org.apache.zookeeper.data.Stat;
import org.yejt.zk.ZookeeperConnection;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Server implements Runnable
{
    private static final String HOST = "192.168.50.20:2181";

    private int port;

    private ExecutorService threadPool = Executors.newCachedThreadPool();

    private ZooKeeper zooKeeper;

    private ZookeeperConnection zookeeperConnection;

    public Server(int port)
    {
        this.port = port;
        zookeeperConnection = new ZookeeperConnection();
    }

    private void regServer() throws IOException, InterruptedException, KeeperException
    {
        zooKeeper = zookeeperConnection.connect(HOST);
        String path = "/test/server" + port;
        String data = "127.0.0.1:" + port;
        Stat stat = null;
        if((stat = zooKeeper.exists(path, true)) != null)
        {
            int version = stat.getVersion();
            zooKeeper.delete(path, version);
        }
        // Create emphemeral node
        zooKeeper.create(path, data.getBytes(), ZooDefs.Ids.OPEN_ACL_UNSAFE, CreateMode.EPHEMERAL);
    }

    public void run()
    {
        ServerSocket server = null;
        try
        {
            server = new ServerSocket(port);
            regServer();
            System.out.println("Server started");
            Socket socket = null;
            while (true)
            {
                socket = server.accept();
                threadPool.submit(new ServerHandler(socket));
            }
        }
        catch(IOException | InterruptedException | KeeperException ex)
        {
            ex.printStackTrace();
        }
        finally
        {
            if (server != null)
            {
                try
                {
                    server.close();
                    zookeeperConnection.close();
                }
                catch (IOException | InterruptedException e)
                {
                    e.printStackTrace();
                }
            }
        }

    }

    public static void main(String[] args)
    {
        int port = Integer.parseInt(args[0]);
        Server server = new Server(port);
        Thread thread = new Thread(server);
        thread.start();
    }
}
