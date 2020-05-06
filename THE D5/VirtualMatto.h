#ifndef _TESTC_H_
#define _TESTC_H_

//README FIRST
//PLEASE DO NOT MODIFY THE H FILE.
//IF YOU NEED TO DECLARE YOUR OWN FUNCTION, PLEASE DO SO FROM THE C FILE

typedef struct {
    double Ai1;
    double Ai2;
    double Ai3;
    double Ai4;
    double Di1;
    double Di2;
    double Di3;
} inputVector;

typedef struct {
    double Ao1;
    double Ao2;
    double Do1;
    double Do2;
    double Do3;
} outputVector;

void update(inputVector* input, double clk, outputVector* output);

#endif