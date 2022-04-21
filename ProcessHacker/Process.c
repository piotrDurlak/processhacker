#include "Process.h"
#include <phapp.h>
#include <symprv.h>

#include <hndlprv.h>
#include <mainwnd.h>


void ProcessCollectionStart()
{
    if (!InitializationForProcessesCollection())
    {
        PhShowError(NULL, L"%s", L"Unable to initialize collection of processes data.");
    }
}


