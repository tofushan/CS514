import java.util.*;
import java.io.*;
import java.net.*;




public class Server{
    
    // initialize socket and scanner 
    private Socket socket = null;
    private ServerSocket server = null;
    private DataInputStream in = null;

    public Server (int port) {
        try {
            // start a server and wait for connection
            server = new ServerSocket(port);
            System.out.println("Server started");

            System.out.println("Waiting for a client ...");

            socket = server.accept();
            System.out.println("Client accepted");
            
            // take input from the client socket 
            in = new DataInputStream(new BufferedInputStream(socket.getInputStream()));

            String line = "";

            while (!line.equals("over")) {
                try {
                    line = in.readUTF();
                    System.out.println(line);
                }
                catch (IOException i){
                    System.out.println(i);
                    System.exit(1);
                }
            }
            System.out.println("Closing connection");

            socket.close();
            in.close();
        }
        catch (IOException i) {
            System.out.println(i);
            System.exit(1);
        }
    }

    public static void main(String[] args) {
        Server server = new Server(5000);
    }
}
