import java.util.*;
import java.io.*;
import java.text.*;
import java.net.*;




public class Server {
    
    public static void main(String[] args) throws IOException {
        
        // esstablish a server listening on port 5056
        ServerSocket ss = new ServerSocket(5056);
        
        
        while (true) {
            
            Socket s = null;
            
            try {
                
                // accepting state : socket object receive incoming client request
                s = ss.accept();

                System.out.println("A new client is connected : " + s);

                DataInputStream datain = new DataInputStream(s.getInputStream());
                DataOutputStream dataout = new DataOutputStream(s.getOutputStream());

                System.out.println("Assigning new thread for this client");
                
                // create a new thread object 
                Thread t = new ClientHandler(s, datain, dataout);

                t.start();
            }
            catch (Exception e) {
                s.close();
                e.printStackTrace();
            }
        }
    }
}


class ClientHandler extends Thread {

    DateFormat fordate = new SimpleDateFormat("yyyy/MM/dd");
    DateFormat fortime = new SimpleDateFormat("hh:mm:ss");
    private Socket s = null;
    private DataInputStream datain = null;
    private DataOutputStream dataout = null;


    // constructor 
    public ClientHandler( Socket s, DataInputStream datain, DataOutputStream dataout ) {
        this.s = s;
        this.datain = datain;
        this.dataout = dataout;
    }

    @Override
    public void run() {
        String received;
        String toreturn;
        
        while (true) {

            try {
                dataout.writeUTF("What do you want? [Data | Time]...\n"+
                                "Type Exit to terminate connection.");
                received = datain.readUTF();

                if (received.equals("Exit")) {
                    System.out.println("Client " + this.s + " sends exit...");
                    System.out.println("Closing this connection.");
                    s.close();
                    System.out.println("Connection closed");
                    break;
                }

                Date date = new Date();

                switch (received) {

                    case "Date":
                        toreturn = fordate.format(date);
                        dataout.writeUTF(toreturn);
                        break;
                    
                    case "Time":
                        toreturn = fortime.format(date);
                        dataout.writeUTF(toreturn);
                        break;

                    default:
                        dataout.writeUTF("Invalid input"); 
                        break;
                }
            }
            catch (IOException e) {
                e.printStackTrace();
            }
        }

        try {
            this.datain.close();
            this.dataout.close();
        }
        catch (IOException e) {
            e.printStackTrace();
        }

    }


}
