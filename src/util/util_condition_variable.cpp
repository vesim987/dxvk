#include "util_condition_variable.h"

#include "util_mutex.h"

namespace dxvk::util {

ConditionVariable::ConditionVariable() {
    InitializeConditionVariable(&conditionVariable);
}

//ConditionVariable::~ConditionVariable() {
    //Nothing to do here??
//}

void ConditionVariable::notify_one() { 
    WakeConditionVariable(&conditionVariable);
}

void ConditionVariable::wait(std::unique_lock<CriticalMutex>& lock) {
    SleepConditionVariableCS(&conditionVariable, &lock.mutex()->criticalSection, INFINITE); 
}



}


