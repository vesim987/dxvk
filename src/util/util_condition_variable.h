#pragma once

//TODO: mingw is reporting wrong windows version
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601

#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "util_mutex.h"
#include <windows.h>

namespace dxvk::util {

class ConditionVariable {
public:
    ConditionVariable();
    ConditionVariable(const ConditionVariable&) = delete;

    void notify_one();

    void wait(std::unique_lock<CriticalMutex>& lock);
    
    template<class Predicate>
    void wait(std::unique_lock<CriticalMutex>& lock, Predicate pred) {
    	while(!pred()) {
	    wait(lock);
	}
    }
   

   //TODO: Implement in in the other way, wait_until should use wait_for
   template< class Clock, class Duration >
   std::cv_status wait_until( std::unique_lock<CriticalMutex>& lock, const std::chrono::time_point<Clock, Duration>& timeout_time ) {
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(timeout_time - std::chrono::steady_clock::now()).count();
	auto result = SleepConditionVariableCS(&conditionVariable, &lock.mutex()->criticalSection, millis);
   	return result == 0 ? std::cv_status::timeout : std::cv_status::no_timeout; //TODO: handle ERROR_TIMEOUT in a proper way 
   }

   template< class Clock, class Duration, class Predicate >
   bool wait_until(std::unique_lock<CriticalMutex>& lock, const std::chrono::time_point<Clock, Duration>& timeout_time, Predicate pred) {
	while (!pred()) {
	    if (wait_until(lock, timeout_time) == std::cv_status::timeout) {
                return pred();
            }
        }
   	return true;
   }
   
   template< class Clock, class Duration, class Predicate >
   bool wait_for(std::unique_lock<CriticalMutex>& lock, const std::chrono::duration<Clock, Duration>& rel_time) {
   	return wait_until(lock, std::chrono::steady_clock::now() + rel_time);
   }

   template< class Clock, class Duration, class Predicate >
   bool wait_for(std::unique_lock<CriticalMutex>& lock, const std::chrono::duration<Clock, Duration>& rel_time, Predicate pred) {
   	return wait_until(lock, std::chrono::steady_clock::now() + rel_time, std::move(pred));
   }

private:
    CONDITION_VARIABLE conditionVariable;
};

}
