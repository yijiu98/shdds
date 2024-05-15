#include <iostream>
#include "shdds.h"
int main(int argc, char **argv)
{
    shdds::init(true);
    shdds::publish();
    exit(1);
}