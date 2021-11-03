#include "socket.hpp"
#include <chrono>
#include <ctime>
#include <unordered_map>

using namespace std;


mutex lck;

// void send_request(int num_commands, const char*hostname, const char* port_num, int bucket_size) {
//     int delay = 1;
//     while (num_commands > 0) {
//         Client* ptr = new Client(hostname, port_num);
//         if (num_commands > 0)
//         {
//             //there is commands needed to be sent                                                                                                      
//             lck.lock();
//             if (num_commands > 0) {
//                 //double check                                                                                                                         
//                 int bucket_idx = rand()%bucket_size;
//                 string msg = to_string(delay) + "," + to_string(bucket_idx);
//                 num_commands--;
//                 lck.unlock();
//                 ptr->run(msg);
//             }
//             else {
//                 lck.unlock();
//             }
//         }
//     }
// }

void send_request(queue<string> &commands, const char*hostname, const char* port_num) {
    bool need_send = true;
    while (need_send) {
        if (!commands.empty())
        {
            //there is commands needed to be sent
            lck.lock();
            if (!commands.empty()) {
                Client* ptr = new Client(hostname, port_num);
                //double check
                string msg = commands.front();
                commands.pop();
                lck.unlock();
                ptr->run(msg);
                delete(ptr);
            }
            else {
                lck.unlock();
                need_send = false;
            }
        }
        else {
            //no more command to send
            need_send = false;
        }
    }
}

int main(int argc, char** argv) {

    if(argc != 4) {
        cout << "invalid arguments" << endl;
        cout << "command: ./client   bucket_size delay_amount num_requests" << endl;
        return EXIT_FAILURE;
    }

    const char* hostname = "127.0.0.1";
    const char* port_num = "12345";
    int bucket_size = atoi(argv[1]);
    
    int max_delay = atoi(argv[2]);
    int num_commands = atoi(argv[3]);
    unordered_map<int, std::thread> threads;
    srand(time(NULL));
    //make all commands
    queue<string> commands;
    for (int i = 0; i < num_commands; ++i) {
        int cur_delay = rand() % max_delay;
        int bucket_idx = rand() % bucket_size ;
        string msg = to_string(cur_delay) + "," + to_string(bucket_idx);
        commands.push(msg);
    } 
    
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    int num_threads = 512;
    for(int i=0; i < num_threads; i++) {
            threads[i] = thread(
            [&]() {
              send_request(commands, hostname, port_num);
            });
            // threads[i].detach();
    }

    for(int i=0; i < num_threads; i++){
            threads[i].join();
    }

    

    // int commands_done = 0;
    // while(commands_done < num_commands) {
    //     for(int i=0; i<commands_limit; i++) {
    //         threads[i] = thread(
    //         [&]() {
    //         send_request(commands, hostname, port_num);
    //         });
    //     }
    //     for(int i=0; i<commands_limit; i++){
    //         threads[i].join();
    //     }
    //     commands_done += commands_limit;
    // }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    cout << "Time taken for random " << num_commands <<" commands with max delay "
            << max_delay << "s and bucket size of " << bucket_size << " is: " 
            << chrono::duration_cast<chrono::milliseconds>(end-begin).count() << " milliseconds" << endl;

    

    return EXIT_SUCCESS;
}
