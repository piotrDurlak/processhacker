#pragma once
#ifndef PROCESS
#define PROCESS
typedef struct _KYP_PROCESS {
    unsigned long PID;
    char* name;
    float CPUUsage;
    unsigned long IOUsage;
    unsigned long MemoryUsage;


}KYP_PROCESS, * KYP_PROCESS_P;

extern int counter;
__declspec(dllexport) int InitProcess();
//__declspec(dllexport) KYP_PROCESS DisplayHelloFromDLL();

void testDisplay(int counter);




#endif 



