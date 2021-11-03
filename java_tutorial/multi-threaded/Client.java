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


            while (true) {
                System.out.println(datain.readUTF());
                String tosend = scn.nextLine();
                dataout.writeUTF(tosend);


                if (tosend.equals("Exit")) {
                    System.out.println("Closing this connection : " + s);
                    s.close();
                    System.out.println("Connection closed");
                    break;
                }

                String received = datain.readUTF();
                System.out.println(received);
            }

            scn.close();
            datain.close();
            dataout.close();
        }
        catch (IOException e) {
            e.printStackTrace();
        }

    }

}





