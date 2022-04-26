#pragma once
#ifndef PROCESS
#define PROCESS
typedef struct _KYP_PROCESS 
{
    unsigned long PID;
    const wchar_t * name;
    //char* name;
    float CPUUsage;
    float IOUsage;
    float MemoryUsage;


}KYP_PROCESS, * KYP_PROCESS_P;

extern int counter;
__declspec(dllexport) int InitProcess();
__declspec(dllexport) KYP_PROCESS* GetProcesses();

void GetProcessesFromPH(KYP_PROCESS* processes);




#endif 



