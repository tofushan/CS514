#ifndef __CACHE_H_
#define __CACHE_H_
#include "server.cpp"
#define AGE_INF pow(2,20)

class cache {
private:
     unordered_map<string, response> caches;
     queue<string> lru;

public:
     bool cache_exist(string url) {
          return caches.find(url) == caches.end() ? false : true;
     }
     response get_cache_response(string url) {
          return caches[url];
     }
     void add_to_cache(string url, response rsps) {
          if(lru.size() >= 20){
               string to_delete = lru.front();
               cout << "remove cache for url: " << to_delete<< " due to LRU" << endl;
               lru.pop();
               caches.erase(to_delete);
          }
          lru.push(url);
          // cout << lru.size() << endl;
          caches[url] = rsps;
     }
     string get_current_time() {
          time_t cur_time = time(0);
          tm *cur_time_tm = gmtime(&cur_time);
          cur_time = mktime(cur_time_tm);
          string ts = change_time_to_string(cur_time);
          return ts;
     }
     time_t change_string_to_time(string v) {
          v = v.substr(0, v.size()-4); // get rid of GMT
          tm x;
          strptime(v.c_str(), "%a, %d %b %Y %H:%M:%S", &x);
          time_t t = mktime(&x);
          return t;
     }
     string change_time_to_string(time_t time) {
          tm *gmt = localtime(&time);
          char *t = asctime(gmt);
          string str = string(t);
          str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
          return str;
     }
     string time_in_utc(time_t time) {
          tm *tm = localtime(&time);
          string str = string(asctime(tm));
          str.erase(remove(str.begin(), str.end(), '\n'), str.end());
          return str;
     }
     bool check_cache_control(http s, string &msg) {
          string cache_control_string = s.get_header_value("Cache-Control");
          // cout << "Checking cache-control for request with id = " << s.get_id() << " and get value: " << cache_control_string << endl;
          if(cache_control_string == "") {
               return false;
          }
          if (cache_control_string == "no-store") {
               msg = "NOTE detect Cache-Control: no-store, sending re-validation request to original server";
               return true;
          }
          else if (cache_control_string == "no-cache") {
               msg = "NOTE detect Cache-Control: no-cache, sending re-validation request to original server";
               return true;
          }
          else if(cache_control_string == "must-revalidate") {
               msg = "NOTE detect Cache-Control: must-revalidate, sending re-validation request to original server";
               return true;
          }
          else if (cache_control_string == "max-age=0") {
               msg = "NOTE detect Cache-Control: max-age = 0, sending re-validation request to original server";
               return true;
          }
          return false;
     }
     bool check_age(http s, int &max_age, int &fresh_age, double &time_left, string &msg) {
          string maxAge = s.get_header_value("max-age");
          if(maxAge != "") {
               max_age = atoi(maxAge.c_str());
               if(max_age == 0) {
                    msg = "in cache, but requires re-validation because max_age = 0";
                    return true;
               }
          }
          string freshAge = s.get_header_value("min-fresh");

          if(freshAge != "") {
               fresh_age = atoi(freshAge.c_str());
          }
          else fresh_age = 0;

          time_t orig_time = change_string_to_time(s.get_header_value("Date"));

          time_t cur_time = time(0);
          tm *cur_time_tm = gmtime(&cur_time);
          cur_time = mktime(cur_time_tm);

          // cout << "current time: " << cur_time << endl;
          // cout << "original time: " << cur_time << endl;
          // cout << "max age: " << max_age << endl;
          // cout << "min-fresh age: " << fresh_age << endl;

          if (difftime(cur_time, orig_time) > max_age-fresh_age) {
               msg = "in cache, but expired @ " + time_in_utc(orig_time+max_age-fresh_age);
               return true;
          }

          return false;
     }

     bool check_exipre_in_cache(http s, string &msg) {
          time_t exp_time = change_string_to_time(s.get_header_value("Expires"));
          time_t cur_time = change_string_to_time(get_current_time());
          if (difftime(exp_time, cur_time) < 0) {
               return true;
          }
          else return false;
     }

     bool valid_in_cache(request &rqst, response& rsps, string &msg) {
          string url = rqst.get_request_url();
          int max_age = AGE_INF;
          int fresh_age = 0;
          double time_left = 0.0;

          if(cache_exist(url) == false) {
               msg = "not in cache";
               return false;
          }

          // check request headers
          if(check_cache_control(rqst, msg) ==  true) {
               return false;
          }
          
          // check response headers
          rsps = get_cache_response(url);
          if(rsps.check_header_exist("Cache-Control") == true) {
               if (check_cache_control(rsps,  msg) == true) {
                    return false;
               }
               if (check_age(rsps, max_age, fresh_age, time_left, msg) == true) {
                    return false;
               }
               if (check_exipre_in_cache(rsps, msg) == true) {
                    return false;
               }
          }
          
          else if (check_exipre_in_cache(rsps, msg) == true) {
               return false;
          }

          msg = "in cache, valid";
          return true;
     }





};


#endif