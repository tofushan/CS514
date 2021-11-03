#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <assert.h>
#include <vector>
#include <algorithm> 
#include <map>
#include <fstream>
#include <thread>
#include <queue>
#include <exception>
#include <unordered_map>
#include <mutex>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <queue>

using namespace std;
#define MSG_LEN 10

mutex cout_sever_lock, cout_client_lock;

class Socket{
public:
     struct addrinfo hints, *head;
     
     // return current port
     int get_port(int socket_fd) {
          struct sockaddr_in sin;
          socklen_t len = sizeof(sin);
          if (getsockname(socket_fd, (struct sockaddr *)&sin, &len) == -1){
               throw runtime_error("get sock name error");
          }
          int port = ntohs(sin.sin_port);
          return port;
     }

     vector<char> parse_string (string s) {
          vector<char> res;
          for(size_t i=0; i<s.size();++i) {
               res.push_back(s[i]);
          }
          return res;
     }

     void send_msg(int dst_fd, string msg) {
          vector<char> to_send = parse_string(msg);
          if(send(dst_fd, &to_send.data()[0], to_send.size(), 0) == -1) {
               throw runtime_error("send failure");
          }
     }

     string recv_msg(int from_fd){
          char buffer[MSG_LEN];
          memset(&buffer, 0, MSG_LEN);
          if(recv(from_fd, &buffer, MSG_LEN, 0) == -1) {
               throw runtime_error("receive failure");
          }
          return string(buffer);
     } 

     Socket(){}
     ~Socket(){}
};


class Server : public Socket {
private:
     const char* listen_port = "12345";
     int socket_fd;
     vector<unsigned> buckets;
     mutex bucket_lock, queue_lock;

public:
     int socket_setup(const char* port_num) {

          //hint init
          memset(&hints, 0, sizeof(hints));
          hints.ai_family = AF_UNSPEC;         // sockaddr_in family requirement
          hints.ai_socktype = SOCK_STREAM;   // tcp reliable connetion
          hints.ai_flags = AI_PASSIVE;       // system will pick

          if (getaddrinfo(NULL, port_num, &hints, &head) != 0) {
               cout << port_num << endl;
               throw runtime_error("getaddrinfo error");
          }

          int socket_fd = socket(head->ai_family, head->ai_socktype, head->ai_protocol);
          if (socket_fd == -1){
               throw runtime_error("socket creation failed");
          }

          // ref Beej's page 24
          // usage: to reuse a port
          int yes = 1;
          if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
               throw runtime_error("setsockopt failed");
          }     

          // bind to port
          bind(socket_fd, head->ai_addr, head->ai_addrlen);

          // listen for connection
          if (listen(socket_fd, 10) == -1){
               throw runtime_error("listen failed");
          }

          freeaddrinfo(head);

          return socket_fd;
     }

     int accept_setup(int socket_fd) {
          struct sockaddr_in sock_addr_str;
          socklen_t t = sizeof(sock_addr_str);
          
          // accept calling convention
          int accept_fd = accept(socket_fd, (struct sockaddr*)&sock_addr_str, &t);
          if(accept_fd == -1) {
               throw runtime_error("accpet failed");
          }
          return accept_fd;
     }

     void do_delay(unsigned delay_value)
     {
          struct timeval start, check;
          double elapsed_seconds;
          gettimeofday(&start, NULL);
          do
          {
               gettimeofday(&check, NULL);
               elapsed_seconds = (check.tv_sec + (check.tv_usec / 1000000.0)) - (start.tv_sec + (start.tv_usec / 1000000.0));
          } while(elapsed_seconds < delay_value);
     }

     void handler_1(queue<int> &fds) {
          while(true) {
	          if(!fds.empty()) {
          	     queue_lock.lock();
		          if (!fds.empty()) {
                         int accept_fd = fds.front();
                         fds.pop();
                         queue_lock.unlock();
                    
                         // cout << accept_fd << endl;

                         string request = recv_msg(accept_fd);
                         unsigned delay = atoi(&request[0]);
                         unsigned bucket_idx = atoi(&request[2]);
                         if(bucket_idx < 0 || bucket_idx >= buckets.size()) {
                              throw runtime_error("invalid bucket index: out of range");
                         }

                         do_delay(delay);

                         bucket_lock.lock();
                         buckets[bucket_idx] += delay;
                         bucket_lock.unlock();

                         stringstream msg;
                         msg << to_string(buckets[bucket_idx]) << '\n';
                         send_msg(accept_fd, msg.str());
                    }
                    else {
                         queue_lock.unlock();
                    }
               }
	     }// cout << "responded" << endl;
     }

     void handler_2(int accept_fd) {
          string request = recv_msg(accept_fd);
          unsigned delay = atoi(&request[0]);
          unsigned bucket_idx = atoi(&request[2]);
          if(bucket_idx < 0 || bucket_idx >= buckets.size()) {
               throw runtime_error("invalid bucket index: out of range");
          }

          do_delay(delay);

          bucket_lock.lock();
          buckets[bucket_idx] += delay;
          bucket_lock.unlock();

          stringstream msg;
          msg << to_string(buckets[bucket_idx]) << '\n';
          send_msg(accept_fd, msg.str());
          // cout << "responded" << endl;
     }

     void run(int thread_method) {
          map<thread::id, int> children;
          mutex params_lock;
          queue<int> fds;

          if(thread_method == 0) {
               while(1) {
                    int accept_fd = accept_setup(socket_fd);
                    fds.push(accept_fd);
                    try{
                         thread t([&]() {
                                        handler_1(fds);
                                   }
                              );
                         t.detach();
                    } catch (exception &e) {
                         cout << e.what() << endl;
                         close(socket_fd);
                         exit(EXIT_FAILURE);
                    }
               }
          }
	  else {
          //pre-thread
               int num_threads = 1024;
               for (int i = 0; i < num_threads; ++i) {
                    thread t([&]() {
                         process(fds);
                    });
                    t.detach();
               }
               
               while (true) {
                    int accept_fd = accept_setup(socket_fd);
		    fds.push(accept_fd);
               }
          }
     }
    
     void process(queue<int> &fds) {
          while (true) {
	     // cout <<"Current Param: " << accept_fd << endl;
               try {
                    // cout << "handling request" << endl;
                    handler_1(fds);
                    
               } catch (exception &e) {
                    cout << e.what() << endl;
                    close(socket_fd);
                    exit(EXIT_FAILURE);
               }
                   
          }
     }

     Server(int bucket_num):buckets(vector<unsigned>(bucket_num, 0)) {
          socket_fd = socket_setup(listen_port);
     }

     ~Server() {
          close(socket_fd);
     }
};


class Client : public Socket {
private:
     int socket_fd;

public:
     int init(const char* host, const char* port_num) {
          //hint init
          memset(&hints, 0, sizeof(hints));
          hints.ai_family = AF_UNSPEC;         // sockaddr_in family requirement
          hints.ai_socktype = SOCK_STREAM;     // tcp reliable connetion
          hints.ai_protocol = 0;

          if (getaddrinfo(host, port_num, &hints, &head) != 0) {
               throw runtime_error("getaddrinfo error");
          }

          int fd = socket(head->ai_family, head->ai_socktype, head->ai_protocol);

          if (fd == -1){
               throw runtime_error("socket creation failed");
          }

          // connect to server
          if(connect(fd, head->ai_addr, head->ai_addrlen) == -1) {
               throw runtime_error("connection to server failed");
          }

          freeaddrinfo(head);
          return fd;
     }

     void run(string msg) {
          // cout_client_lock.lock();
          // cout << "sending msg: " << msg << endl;
          // cout_client_lock.unlock();
          send_msg(socket_fd, msg);
          string returned_msg = recv_msg(socket_fd);
          // cout_client_lock.lock();
          // cout << "received from server: " << returned_msg;
          // cout_client_lock.unlock();
     }


     Client(const char* host, const char* port_num) {
          socket_fd = init(host, port_num);
          // run(socket_fd, msg);
     }

     ~Client() {
          close(socket_fd);
     }
};
