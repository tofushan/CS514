#include "socket.hpp"

bool check(int argc, char** argv) {
     // ideal command: ./server thread_method number_of_buckets number_of_cores 
     if(argc != 3) {
          cout << "invalid arguments" << endl;
          return false;
     }

     // 0:per-threaded, 1:pre-created
     int thread_method = atoi(argv[1]);
     if(thread_method != 0 && thread_method != 1) {
          cout << "invalid thread method" << endl;
          return false;
     }

     //bucket size should be: 32, 128, 512, 2048
     int bucket_size = atoi(argv[2]);
     if(bucket_size != 32 && bucket_size != 128 && bucket_size != 512 && bucket_size != 2048) {
          cout << "invalid bucket size" << endl;
          return false;
     }

     //core number should be: 1, 2, 4
     // int core_num = atoi(argv[3]);
     // if(core_num != 1 && core_num != 2 && core_num != 4) {
     //      cout << "invalid number of cores" << endl;
     //      return false;
     // }
     
     //valid arguments
     return true;
}

int main(int argc, char** argv) {
     if(!check(argc, argv)) {
          return EXIT_FAILURE;
     }

     Server sv(atoi(argv[2]));
     int thread_method = atoi(argv[1]);

     //debug purpose
     cout << "thread method: ";
     if(atoi(argv[1]) == 0) {
          cout << "per-request thread" << endl;
     }
     else {
          cout << "pre-created thread" << endl;
     }
     cout << "number of buckets: " << atoi(argv[2]) << endl;

     sv.run(thread_method);

     return EXIT_SUCCESS;
}