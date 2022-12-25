package web;


import java.io.*;
import java.net.*;
import java.util.*;

class Worker extends Thread implements HttpConstants {

    final static int BUF_SIZE = 2048;
    static final byte[] EOL = {(byte) '\r', (byte) '\n'};

    /* buffer to use for requests */
    byte[] buffer;
    /* Socket to client we're handling */
    private Socket socket;
    /* WebServer the worker works for */
    WebServer webServer;

    Worker(WebServer webServer, String name) {
        super(name);
        this.webServer = webServer;
        buffer = new byte[BUF_SIZE];
        this.start();
    }

    synchronized void setSocket(Socket socket) {
        this.socket = socket;
        notify();
    }

    public synchronized void run() {

        webServer.workerHasStarted();

        while (true) {
            /* nothing to do */
            try {
                wait();
            } catch (InterruptedException e) {
                /* should not happen */
                continue;
            }

            try {
                handleClient();
            } catch (Exception e) {
                e.printStackTrace();
            }
            /* go back in wait queue if there's fewer
             * than numHandler connections.
             */

            Vector pool = webServer.threads;
            synchronized (pool) {
                if (pool.size() >= webServer.workers) {
                    /* too many threads, exit this one */
                    return;
                } else {
                    pool.addElement(this);
                }
            }
        }
    }

    void handleClient() throws IOException {
      // input and output streams
        InputStream is = new BufferedInputStream(socket.getInputStream());
        PrintStream ps = new PrintStream(socket.getOutputStream());
        System.out.println("Waiting for input ...");

        // read an integer
        is.read(buffer);

        // set a number and steps
        int number = buffer[0];
        int steps = 0;
        int workingNumber = number;

        // if the number is not one perform the algorithm
         if(number != 1)
         {
            // while tempNUm
            while(workingNumber > 1)
            {
               if(workingNumber % 2 == 0)
               {
                  workingNumber /= 2;
               }
               else
               {
                  workingNumber = (3 * workingNumber) + 1;
               }
               steps++;
            }
         }
         System.out.println("Number of steps: " + steps);
         ps.write(steps);

         socket.close();
    }

}
