#include "server.h"

int http::parse_http_by_line(string temp) {
     orig_string = temp;

     // setup first line
     if (temp.size() < 4) return 1;
     size_t first_line_idx = temp.find("\r\n");
     status_line = temp.substr(0, first_line_idx);
     temp = temp.substr(first_line_idx + 2);

     // deal with the rest
     // two situation: header+msg, head+msg+body
     size_t idx = 0;
     if(temp.find("\r\n\r\n") == temp.npos) {
          return 1;
     }
     while(idx < temp.find("\r\n\r\n")) {
          size_t idx_end = temp.find("\r\n", idx);
          size_t idx_colon = temp.find(":", idx);
          
          string header = temp.substr(idx, idx_colon-idx);
          string msg = temp.substr(idx_colon + 2, idx_end-idx_colon-2);
          header_value_map[header] = msg;   
          idx = idx_end+2;
     }
     return 0;
}


int response::setup() {
     string first_line = get_status_line();
     size_t idx_space = first_line.find(" ",0);
     if(idx_space == first_line.npos) {
          // cout << get_id() << ": ERROR invalid response structure, sending \"" << bad_gateway << "\" to server" << endl;
          return 1;
     }
     http_version = first_line.substr(0, idx_space);
     size_t idx_space2 = first_line.find(" ", idx_space+1);
     status_num = first_line.substr(idx_space+1, idx_space2-idx_space-1);
     if(idx_space2 == first_line.npos) {
          // cout << get_id() << ": ERROR invalid response structure, sending \"" << bad_gateway << "\" to server" << endl;
          return 1;
     }
     status_str = first_line.substr(idx_space2+1);
     return 0;
}

int request::setup() {
     string first_line = get_status_line();
     size_t idx_space = first_line.find(" ");
     if(idx_space == first_line.npos) {
          // cout << get_id() << ": ERROR invalid request structure, sending \"" << bad_request << "\" to client" << endl;
          return 1;
     }
     method = first_line.substr(0, idx_space);
     if(method != "GET" && method != "POST" && method != "CONNECT") {
          // cout << get_id() << ": ERROR invalid request structure, sending \"" << bad_request << "\" to client" << endl;
          return 1;
     }
     // first line example: GET /awesome.txt HTTP/1.1
     size_t idx_space2 = first_line.find(" ", idx_space+1);
     url = first_line.substr(idx_space+1, idx_space2-idx_space-1);

     // test if user specifies a port number
     size_t idx_colon = first_line.find(":");
     if (idx_colon != first_line.npos) {
          port = first_line.substr(idx_colon + 1, idx_space2-idx_colon-1);
     }
     else {
          size_t idx_end = first_line.find("\r");
          http_version = first_line.substr(idx_space2+1, idx_end-idx_space2-1);
          cout << http_version << endl;
          if(http_version.size() >= 4 && http_version.substr(0,4) == "HTTP"){
               port = "80";
          }
          else if(http_version.size() >= 5 && http_version.substr(0,5) == "HTTPS") {
               port = "443";
          }
          else {
               // cout << get_id() << ": ERROR invalid request http version, sending \"" << bad_request << "\" to client" << endl;
               return 1;
          }
     }

     // complete url
     host = get_header_value("Host");
     if(host == "") {
          // cout << get_id() << ": ERROR invalid request hostname, sending \"" << bad_request << "\" to client" << endl;
          return 1;
     }
     idx_colon = host.find(":");
     if(idx_colon != host.npos) {
          host = host.substr(0, idx_colon);
     }
     return 0;
}