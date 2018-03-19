#pragma once

#include <condition_variable>
#include <queue>
#include <thread>

#include "../util/util_mutex.h"
#include "../util/util_condition_variable.h"

#include "dxvk_cmdlist.h"
#include "dxvk_sync.h"

namespace dxvk {
  
  class DxvkDevice;
  
  /**
   * \brief Submission queue
   * 
   * 
   */
  class DxvkSubmissionQueue {
    
  public:
    
    DxvkSubmissionQueue(DxvkDevice* device);
    ~DxvkSubmissionQueue();
    
    void submit(const Rc<DxvkCommandList>& cmdList);
    
  private:
    
    DxvkDevice*             m_device;
    
    std::atomic<bool>       m_stopped = { false };
    
    util::CriticalMutex              m_mutex;
    util::ConditionVariable m_condOnAdd;
    util::ConditionVariable m_condOnTake;
    std::queue<Rc<DxvkCommandList>> m_entries;
    std::thread             m_thread;
    
    void threadFunc();
    
  };
  
}
