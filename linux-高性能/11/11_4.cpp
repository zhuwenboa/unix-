#include"head.hpp"
#include<time.h>
#include<signal.h>

const int TIMEOUT = 5000;
int timeout = TIMEOUT;
time_t start = time(NULL);
time_t end = time(NULL);
