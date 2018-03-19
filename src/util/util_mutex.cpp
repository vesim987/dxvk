#include "util_mutex.h"

#include <windows.h>

namespace dxvk::util {

CriticalMutex::CriticalMutex() {
    InitializeCriticalSection(&criticalSection);
}
    
CriticalMutex::CriticalMutex(std::uint32_t spinCount)  {
    InitializeCriticalSectionAndSpinCount(&criticalSection, spinCount);
}
   
CriticalMutex::~CriticalMutex() {
    DeleteCriticalSection(&criticalSection);
}
    
void CriticalMutex::lock() {
    EnterCriticalSection(&criticalSection);
}
   
void CriticalMutex::unlock() {
    LeaveCriticalSection(&criticalSection);
}


}

