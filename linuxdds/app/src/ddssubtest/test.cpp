#include <iostream>
#include "shdds.h"
#include <thread>
int main(int argc, char **argv)
{
    shdds::init(false);
    // shdds::publish();
    while(1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    exit(1);
}