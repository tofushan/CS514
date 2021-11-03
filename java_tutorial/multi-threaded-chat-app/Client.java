import java.net.*;
import java.io.*;
import java.util.*;


public class Client {
    
    // initialize socket and input output stream
    private Socket socket = null;
    private DataOutputStream out = null;    
    private Scanner console = null;
    
    public static void main(String[] args) throws IOException {
        
        try {
            Scanner scn = new Scanner(System.in);

            InetAddress ip = InetAddress.getByName("localhost");

            Socket s = new Socket(ip, 5056);

            DataInputStream datain = new DataInputStream(s.getInputStream());
            DataOutputStream dataout = new DataOutputStream(s.getOutputStream());

            // create two threads for client; one for receiving message one for sending message
            
            Thread send = new Thread(new Runnable() {
                String msg_to_send;
                @Override
                public void run() {

                    while (true) {
                        msg_to_send = scn.nextLine();
                        try {
                            dataout.writeUTF(msg_to_send);
                        }
                        catch ( IOException e ) {
                            e.printStackTrace();
                        }
                    }
                }
            });

            Thread receive = new Thread(new Runnable() {
                String received;
                @Override
                public void run() {

                    while (true)  {
                        try {
                            received = datain.readUTF();
                            System.out.println(received);
                        }
                        catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }
            });


            send.start();
            receive.start();

        }
        catch (IOException e) {
            e.printStackTrace();
        }

    }

}





