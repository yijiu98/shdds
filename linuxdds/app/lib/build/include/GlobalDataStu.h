#pragma once

typedef struct CutMotor
{
    long long int timestamp;
 	unsigned int state;
    unsigned int rpm;
}CutMotor;
typedef struct Battery
{
 	unsigned int soc;
    unsigned int soh;
}Battery;
typedef struct LeftMotor
{
    unsigned int state;
    unsigned int rpm;
}LeftMotor;