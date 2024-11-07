#include "Server.hpp"
#include "MSTFactory.hpp"
#include "MSTAlgorithmType.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <map>

#define NUM_THREADS 4 // Number of threads in LFP

using namespace std;

LFP lfp(NUM_THREADS); // Create an instance of LFP
struct pollfd *pfds;  // Set of file descriptors for poll() management
int fd_count = 0;
