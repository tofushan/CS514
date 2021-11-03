#ifndef __PROXY_H_
#define __PROXY_H_
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

using namespace std;

static string bad_request = "HTTP/1.1 400 Bad Request\r\n\r\n";
static string bad_request_1 = "HTTP/1.1 400 Bad Request";
static string bad_gateway = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
static string bad_gateway_1 = "HTTP/1.1 502 Bad Gateway";

class http {
private:
     int id;
     string status_line;
     unordered_map<string, string> header_value_map;
     string body, orig_string;
     vector<char> orig_vec;

public:
     int get_id() {
          return id;
     }
     void set_id (int _id) {
          id = _id;
     }

     int parse_http_by_line(string temp);

     string get_header_value (string key) {
          return header_value_map.find(key) == header_value_map.end() ? "" : header_value_map[key];
     }
     string get_status_line() {
          return status_line;
     }
     bool check_header_exist(string s) {
          return get_header_value(s) == "" ? false : true;
     }
     string get_orig_string() {
          return orig_string;
     }
     void save_to_vec(char c){
          orig_vec.push_back(c);
     }
     vector<char> get_vec() {
          return orig_vec;
     }
};


class response : public http{
private:
     string http_version, status_num, status_str;
public:
     int setup();

     string get_http_version() {
          return http_version;
     }
     string get_status_num() {
          return status_num;
     }
     string get_status_str() {
          return status_str;
     }
     string get_exp_time() {
          return get_header_value("Expires");
     }
     string get_date() {
          return get_header_value("Date");
     }
     string get_last_modified() {
          return get_header_value("Last-Modified");
     }
};

class request : public http {
private:
     string method, host, port, url, http_version;

public:
     int setup();

     string get_request_method() {
          return method;
     }
     string get_request_host() {
          return host;
     }
     string get_request_port() {
          return port;
     }
     string get_request_url() {
          return url;
     }

};

#endif
