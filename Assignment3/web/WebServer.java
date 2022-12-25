package web;

import java.io.*;
import java.net.*;
import java.util.*;


class WebServer {

    // where to print log
    static PrintStream log = null;

    // properties
    protected static Properties properties = new Properties();

    // takes idle worker threads
    static Vector threads = new Vector();

    // the web server's root directory
    static File root = new File(System.getProperty("user.dir") + File.separator + "docs");

    // web servers port it is listening on
    static int port = 23657;

    // timeout on client connections
    static int timeout = 5000;

    // max # worker threads
    static int workers = 5;

    // default constructor
    WebServer() {}
        
    // print to stdout
    protected static void print(String s) {
        System.out.println(s);
    }

    // print to the log file
    static void log(String s) {
        synchronized (log) {
            log.println(s);
            log.flush();
        }
    }
    
    // load properties from directory config
    static void loadProps() throws IOException {
        String fileString = System.getProperty("user.dir") + File.separator + "config" + File.separator + "www-server.properties";
        File file = new File(fileString);
        
        if (file.exists()) {
            InputStream propertiesInputStream = new BufferedInputStream(new FileInputStream(file));
            properties.load(propertiesInputStream);
            propertiesInputStream.close();
            
            String propertyString = properties.getProperty("root");
            if (propertyString != null) {
                root = new File(propertyString);
                if (!root.exists()) {
                    throw new Error(root + " doesn't exist");
                }
            }
            
            propertyString = properties.getProperty("timeout");
            if (propertyString != null) {
                timeout = Integer.parseInt(propertyString);
            }

            propertyString = properties.getProperty("port");
            if (propertyString != null) {
                port = Integer.parseInt(propertyString);
            }

            propertyString = properties.getProperty("workers");
            if (propertyString != null) {
                workers = Integer.parseInt(propertyString);
            }

            propertyString = properties.getProperty("log");
            if (propertyString != null) {
                print("opening log file: " + propertyString);
                log = new PrintStream(new BufferedOutputStream(
                        new FileOutputStream(propertyString)));
            } else {
                log = System.out;                
            }
        }
    }

    static void printProps() {
        print("doc root=" + root);
        print("port=" + port);
        print("timeout=" + timeout);
        print("workers=" + workers);
    }
     
    synchronized void workerHasStarted() {
        notify();
    }

    synchronized void run() throws Exception {

        /* start worker threads */
        for (int i = 0; i < workers; ++i) {
            Worker w = new Worker(this, "worker #" + i);
            wait();
            threads.addElement(w);
        }


        ServerSocket ss = new ServerSocket(port);
        Worker w = null;

        print("Start server loop");
        while (true) {

            Socket s = ss.accept();

            synchronized (threads) {
                if (threads.isEmpty()) {
                    w = new Worker(this, "additional worker");
                    wait();
                    w.setSocket(s);
                } else {
                    w = (Worker) threads.elementAt(0);
                    threads.removeElementAt(0);
                    w.setSocket(s);
                }
            }
        }

    }

    public static void main(String[] args) throws Exception {

        loadProps();
        printProps();

        (new WebServer()).run();

    }
}
