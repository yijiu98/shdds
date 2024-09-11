#pragma once
typedef signed char S8;
typedef unsigned char U8;

typedef signed short int S16;
typedef unsigned short int U16;

typedef signed int S32;
typedef unsigned int U32;
typedef float F32;

typedef  unsigned long long U64;
typedef  double D64;


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