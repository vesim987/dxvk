#pragma once

#include <memory>

#include <windows.h>

namespace dxvk::util {

class ConditionVariable;

class CriticalMutex {
    friend ConditionVariable;
public:
    CriticalMutex();
    CriticalMutex(std::uint32_t spinCount);
    
    CriticalMutex(const CriticalMutex&) = delete;
    ~CriticalMutex();
    
    void lock();
    void unlock();
    
private:
   CRITICAL_SECTION criticalSection; 
};

using RecursiveCriticalMutex = CriticalMutex;

}
