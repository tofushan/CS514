#include "cache.h"
#define http_struct_size 65536

int global_id = 0;
ofstream f_write;
static pthread_mutex_t id_lock;
static pthread_mutex_t cs_lock;
static pthread_mutex_t wt_lock;

int socket_setup(const char* hostname, const char* port_num) {
     //hint init
     struct addrinfo hints, *head;
     memset(&hints, 0, sizeof(hints));
     hints.ai_family = AF_UNSPEC;         // sockaddr_in family requirement
     hints.ai_socktype = SOCK_STREAM;   // tcp reliable connetion
     hints.ai_flags = AI_PASSIVE;       // system will pick

     if (getaddrinfo(hostname, port_num, &hints, &head) != 0) {
          perror("getaddrinfo error");
          // exit(EXIT_FAILURE);
          throw exception();
     }

     int socket_fd = socket(head->ai_family, head->ai_socktype, head->ai_protocol);
     if (socket_fd == -1){
          perror("socket creation failed");
          // exit(EXIT_FAILURE);
          throw exception();
     }

     // ref Beej's page 24
     // usage: to reuse a port
     int yes = 1;
     if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
          perror("setsockopt failed");
          // exit(EXIT_FAILURE);
          throw exception();
     }     

     // bind
     bind(socket_fd, head->ai_addr, head->ai_addrlen);

     // listen for connection
     if (listen(socket_fd, 100) == -1){
          perror("listen failed");
          // exit(EXIT_FAILURE);
          throw exception();
     }

     // free the memory getaddrinfo allocated
     freeaddrinfo(head);

     return socket_fd;
}

int accept_setup(int socket_fd, string &ip_addr) {
     struct sockaddr_in sock_addr_str;
     socklen_t t = sizeof(sock_addr_str);
     
     // accept calling convention
     int accept_fd = accept(socket_fd, (struct sockaddr*)&sock_addr_str, &t);
     if(accept_fd == -1) {
          perror("accpet failed");
          exit(EXIT_FAILURE);
     }
     ip_addr = inet_ntoa(sock_addr_str.sin_addr);
     return accept_fd;
}

void connect_to_server(const char* host, const char* port_num, int &fd){
     //hint init
     struct addrinfo hints, *head;
     memset(&hints, 0, sizeof(hints));
     hints.ai_family = AF_UNSPEC;         // sockaddr_in family requirement
     hints.ai_socktype = SOCK_STREAM;     // tcp reliable connetion

     if (getaddrinfo(host, port_num, &hints, &head) != 0) {
          perror("getaddrinfo error");
          // exit(EXIT_FAILURE);
          throw exception();
     }

     fd = socket(head->ai_family, head->ai_socktype, head->ai_protocol);

     if (fd == -1){
          perror("socket creation failed");
          // exit(EXIT_FAILURE);
          throw exception();
     }

     if(connect(fd, head->ai_addr, head->ai_addrlen) == -1) {
          perror("connection to server failed");
          // exit(EXIT_FAILURE);
          throw exception();
     }

     freeaddrinfo(head);
}

void build_request(int accept_fd, request& rqst){
     char request_msg[http_struct_size];
     if(recv(accept_fd, &request_msg, sizeof(request_msg), 0) <= 0) {
          return;
     }
     string str(request_msg);
     if(rqst.parse_http_by_line(request_msg) != 0) {
          send(accept_fd, &bad_request.data()[0], bad_request.length(), 0);
          pthread_mutex_lock(&wt_lock);
          f_write << rqst.get_id() << ": ERROR bad request, sending \"" << bad_request_1 << "\" to client" << endl;
          pthread_mutex_unlock(&wt_lock);
     }
     rqst.set_id(global_id);
     if(rqst.setup() != 0) {
          send(accept_fd, &bad_request.data()[0], bad_request.length(), 0);
          pthread_mutex_lock(&wt_lock);
          f_write << rqst.get_id() << ": ERROR bad request, sending \"" << bad_request_1 << "\" to client" << endl;
          pthread_mutex_unlock(&wt_lock);
     }
     pthread_mutex_lock(&id_lock);
     global_id++;
     pthread_mutex_unlock(&id_lock);
}

size_t send_data(int from_fd, int to_fd) {
     char s[http_struct_size];
     size_t data_size = -1;
     memset(&s, 0, http_struct_size);
     data_size = recv(from_fd, &s, sizeof(s), 0);
     if(data_size == 0) {
          return 0;
     }
     send(to_fd, &s, data_size, 0);
     return data_size;
}

void forward_to_server(int &forward_fd, request &rqst) {
     string hostname = rqst.get_request_host();
     try{
          connect_to_server(hostname.c_str(), "80", forward_fd);
     } catch(exception &e) {
          cout << "connect to server failed" << endl;
          return;
     }
     string to_send = rqst.get_orig_string();
     vector<char> res;
     for(size_t i=0; i<to_send.size(); ++i) {
          res.push_back(to_send[i]);
     }
     if(send(forward_fd, &res.data()[0], to_send.size(), 0) != (int)to_send.size()) {
          perror("send failed");
          return;
     }
}


string receive_from_server(int accept_fd, int forward_fd, response &rsps) {
     char buffer[http_struct_size];
     memset(&buffer, 0, http_struct_size);
     string empty_string = "";
     vector<char> res;
     size_t data_received = recv(forward_fd, &buffer, sizeof(buffer), 0);
     
     if(data_received == 0) {
          return empty_string;
     }
     for(size_t i=0; i<data_received; ++i) {
          res.push_back(buffer[i]);
          rsps.save_to_vec(buffer[i]);
     }
     size_t sent = 0;
     while(sent < data_received) {
          size_t temp = send(accept_fd, &res.data()[0], data_received, 0);
          sent += temp;
     }
     string str(res.begin(), res.end());
     if(rsps.parse_http_by_line(str) != 0) {
          send(forward_fd, &bad_gateway.data()[0], bad_gateway.length(), 0);
          pthread_mutex_lock(&wt_lock);
          f_write << "(no-id): ERROR bad gateway, sending \"" << bad_gateway_1 << "\" to server" << endl;
          pthread_mutex_unlock(&wt_lock);
     }
     if(rsps.setup() != 0) {
          send(forward_fd, &bad_gateway.data()[0], bad_gateway.length(), 0);
          pthread_mutex_lock(&wt_lock);
          f_write << "(no-id): ERROR bad gateway, sending \"" << bad_gateway_1 << "\" to server" << endl;
          pthread_mutex_unlock(&wt_lock);
     }
     if(rsps.get_header_value("Content-Length") != "") {
          int content_length = stoi(rsps.get_header_value("Content-Length"));
          // cout << "Content-length: " << content_length << endl;
          int size_left = content_length - (int)data_received;
          while(size_left > 0) {
               // cout << "size_left: " << size_left << endl;
               res.clear();
               data_received = recv(forward_fd, &buffer, sizeof(buffer), 0);
               for(size_t i=0; i<data_received; ++i) {
                    res.push_back(buffer[i]);
                    rsps.save_to_vec(buffer[i]);
               }
               if(send(accept_fd, &res.data()[0], data_received, 0) != (int)data_received) {
                    perror("send error");
               }
               size_left -= data_received;
          }
     }
     return str;
}

void handle_connect(int accept_fd, request &rqst) {
     int forward_fd = 0;
     connect_to_server(rqst.get_request_host().c_str(), rqst.get_request_port().c_str(), forward_fd);
     string valid = "HTTP/1.1 200 OK\r\n\r\n";

     pthread_mutex_lock(&wt_lock);
     f_write << rqst.get_id() << ": Responding \"HTTP/1.1 200 OK\"" << endl;
     pthread_mutex_unlock(&wt_lock);

     if(send(accept_fd, valid.c_str(), valid.size(), 0) < 0) {
          perror("send error");
          throw exception();
          return;
     }
     
     fd_set readfds;
     while(1) {      
          FD_ZERO(&readfds);
          int max_fd = accept_fd > forward_fd ? accept_fd : forward_fd;
          FD_SET(accept_fd, &readfds);
          FD_SET(forward_fd, &readfds);
          if(select(max_fd+1, &readfds, NULL, NULL, NULL) == -1) {
               perror("select = -1");
               throw exception();
               break;
          }
          if(FD_ISSET(accept_fd, &readfds)) {
               if(send_data(accept_fd, forward_fd) == 0) {
                    break;
               }
          }
          if (FD_ISSET(forward_fd, &readfds)) {
               if(send_data(forward_fd, accept_fd) == 0){
                    break;
               }
          }
     }
     pthread_mutex_lock(&wt_lock);
     f_write << rqst.get_id() << ": Tunnel closed" << endl;
     pthread_mutex_unlock(&wt_lock);

     close(accept_fd);
     close(forward_fd);
}

bool ok_to_add_cache(request rqst, response rsps) {
     string rqst_cc = rqst.get_header_value("Cache-Control");
     if(rqst_cc != "") {
          if(rqst_cc == "no-store" || rqst_cc == "no-stale") {
               pthread_mutex_lock(&wt_lock);
               f_write << rqst.get_id() << ": NOTE detecting request Cache-Control header, not adding to cache" << endl;
               pthread_mutex_unlock(&wt_lock);
               return false;
          }
     }
     string rsps_cc = rsps.get_header_value("Cache-Control");
     if(rsps_cc != "") {
          if(rsps_cc == "no-store" || rsps_cc == "no-stale") {
               pthread_mutex_lock(&wt_lock);
               f_write << rqst.get_id() << ": NOTE detecting request Cache-Control header, not adding to cache" << endl;
               pthread_mutex_unlock(&wt_lock);
               return false;
          }
     }
     return true;
}

void validate(request rqst, response &rsps) {
     string if_none_match = "\r\nIf-None-Match: ";
     string if_modified_since = "\r\nIf-Modified-Since: ";
     string etag = rsps.get_header_value("ETag");
     string last_modified = rsps.get_header_value("LAST-MODIFIED");
     string new_header = "";

     if(etag != "" &&rqst.get_header_value("If-None-Match") == "" ) {
          new_header += if_none_match+etag;
     }
     if(last_modified != "" && rqst.get_header_value("If-Modified-Since") == "") {
          new_header += if_modified_since+last_modified;
     }     

     
     string orig_string = rqst.get_orig_string();

     size_t insert_idx = orig_string.find("\r\n\r\n");
     orig_string.insert(insert_idx, new_header);

     vector<char> to_send;
     // to_send.resize(orig_string.size());
     
     for(size_t i=0; i<orig_string.size(); ++i) {
          to_send.push_back(orig_string[i]);
     }
     int forward_fd = 0;
     string hostname = rqst.get_request_host();
     connect_to_server(hostname.c_str(), "80", forward_fd);

     if(send(forward_fd, &to_send.data()[0], to_send.size(), 0) != (int)to_send.size()) {
          perror("send failed");
          throw exception();
          return;
     }
     char buffer[65536];
     memset(&buffer, 0, 65536);
     size_t data_received = recv(forward_fd, &buffer[0], 65536, 0);
     // cout << buffer << endl;
     if(data_received == 0){
          return;
     }
     string str(buffer);
     size_t idx_space = str.find(" ");
     if(str.substr(idx_space+1, 3) == "304") {
          return;
     }
     else {
          if(rsps.parse_http_by_line(str) != 0) {
               send(forward_fd, &bad_gateway.data()[0], bad_gateway.length(), 0);
               pthread_mutex_lock(&wt_lock);
               f_write << rqst.get_id() << ": ERROR bad gateway, sending \"" << bad_gateway_1 << "\" to server" << endl;
               pthread_mutex_unlock(&wt_lock);
          }
          if(rsps.setup() != 0) {
               send(forward_fd, &bad_gateway.data()[0], bad_gateway.length(), 0);
               pthread_mutex_lock(&wt_lock);
               f_write << rqst.get_id() << ": ERROR bad gateway, sending \"" << bad_gateway_1 << "\" to server" << endl;
               pthread_mutex_unlock(&wt_lock);
          }
     }
}

void handle_get(int accept_fd, request &rqst, response &rsps, cache* cs) {
     string url = rqst.get_request_url();
     string msg = "";

     if(cs->cache_exist(url) == false) {
          int forward_fd = 0;
          pthread_mutex_lock(&wt_lock);
          f_write << rqst.get_id() << ": Requesting \"" << rqst.get_status_line() << "\" from " << rqst.get_header_value("Host") << endl;
          pthread_mutex_unlock(&wt_lock);
          
          forward_to_server(forward_fd, rqst);
          string s = receive_from_server(accept_fd, forward_fd, rsps);
          if(s.size() == 0) {
               perror("recv failed");
               return;
               // exit(EXIT_FAILURE);
          }
          pthread_mutex_lock(&wt_lock);
          f_write << rqst.get_id() << ": Received \"" << rsps.get_status_line() <<"\" from " << rqst.get_header_value("Host") << endl;
          pthread_mutex_unlock(&wt_lock);
          
          if(ok_to_add_cache(rqst, rsps) && rqst.get_request_method() == "GET") {
               cs->add_to_cache(url, rsps);
          }
          // cout << "adding cache for request id:" << rqst.get_id() << endl;
     }
     else {
          rsps = cs->get_cache_response(url);
          if (cs->valid_in_cache(rqst, rsps, msg) == false) {
               validate(rqst, rsps);
               pthread_mutex_lock(&cs_lock);
               cs->add_to_cache(url, rsps);
               pthread_mutex_unlock(&cs_lock);
          }
          vector<char> to_send = rsps.get_vec();
          size_t sent = 0;
          while(sent < to_send.size()) {
               sent += send(accept_fd, &to_send.data()[0], to_send.size(), 0);
          }
          
     }
     if(msg != ""){
          pthread_mutex_lock(&wt_lock);
          f_write << rqst.get_id() << ": " << msg << endl;
          pthread_mutex_unlock(&wt_lock);
     }
}


void handle_request(int accept_fd, cache* cs, string ip_from) {
     request rqst;
     response rsps;
     build_request(accept_fd, rqst);
     string status_line = rqst.get_status_line();
     string cur_time = cs->get_current_time();
     // cout << cur_time << endl;
     string method = rqst.get_request_method();
     

     if(status_line != "" && cur_time != "" && ip_from != ""){
          pthread_mutex_lock(&wt_lock);
          f_write << rqst.get_id() << ": Receving \""<< status_line << "\" from \"" << ip_from <<"\" @ " << cur_time;
          pthread_mutex_unlock(&wt_lock);
     }

     if(method == "GET" || method == "POST") {
          handle_get(accept_fd, rqst, rsps, cs);
     }
     else if(method == "CONNECT") {
          handle_connect(accept_fd, rqst);
     }
     else {
          int id = rqst.get_id();
          send(accept_fd, &bad_request.data()[0], bad_request.length(), 0);
          pthread_mutex_lock(&wt_lock);
          f_write << id << ": ERROR invalid http method, Sending \"" << bad_request_1 << "\" to client" << endl;
          pthread_mutex_unlock(&wt_lock);
     }

}

int main(int argc, char** argv) {
     f_write.open("/var/log/erss/proxy.log");
     cache cs;
     const char* port_num = "12345";
     int socket_fd = socket_setup(NULL, port_num);
     string ip_from;
     
     while(1) {
          int accept_fd = accept_setup(socket_fd, ip_from);
          try{
               thread t(handle_request, accept_fd, &cs, ip_from);
               t.detach();
          }catch (exception &e){
               return EXIT_FAILURE;
          }
     }
     
     f_write.close();
     return EXIT_SUCCESS;
}