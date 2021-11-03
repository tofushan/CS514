import java.net.*;
import java.io.*;
import java.util.*;


public class Client {
    
    // initialize socket and input output stream
    private Socket socket = null;
    private DataOutputStream out = null;    
    private Scanner console = null;


    public Client(String address, int port) {
        
        // try a connection
        try {
            socket = new Socket(address, port);
            System.out.println("Connected");
            
            // take input from terminal 
            console = new Scanner(System.in);
            
            // send output to the socket 
            out = new DataOutputStream(socket.getOutputStream());

        }
        catch (UnknownHostException u){
            System.out.println(u);
            System.exit(1);
        }
        catch (IOException i) {
            System.out.println(i);
            System.exit(1);
        }

        String line = "";

        while (!line.equals("over")) {
            try {
                line = console.nextLine();
                out.writeUTF(line);
            }
            catch (IOException i) { 
                System.out.println(i);
                System.exit(1);
            }
        }
        try {
            console.close();
            out.close();
            socket.close();
        }
        catch (IOException i) {
            System.out.println(i);
        }
    }

    public static void main (String[] args) {
        Client client = new Client("127.168.0.1", 5000);
    }
}





