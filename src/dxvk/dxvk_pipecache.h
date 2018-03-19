#pragma once

#include <atomic>
#include <chrono>
#include <fstream>
#include <thread>

#include "dxvk_include.h"

#include "../util/util_mutex.h"
#include "../util/util_condition_variable.h"

#include "../util/sha1/sha1_util.h"
#include "../util/util_env.h"

namespace dxvk {
  
  /**
   * \brief Pipeline cache
   * 
   * Allows the Vulkan implementation to
   * re-use previously compiled pipelines.
   */
  class DxvkPipelineCache : public RcObject {
    
  public:
    
    DxvkPipelineCache(const Rc<vk::DeviceFn>& vkd);
    ~DxvkPipelineCache();
    
    /**
     * \brief Pipeline cache handle
     * \returns Pipeline cache handle
     */
    VkPipelineCache handle() const {
      return m_handle;
    }
    
    /**
     * \brief Sends an update signal
     * 
     * Notifies the update thread that the
     * pipeline cache should be updated.
     */
    void update();
    
  private:
    
    Rc<vk::DeviceFn>        m_vkd;
    VkPipelineCache         m_handle;
    
    std::string             m_fileName;
    
    std::atomic<uint32_t>   m_updateStop;
    std::atomic<uint32_t>   m_updateCounter;
    
    util::CriticalMutex     m_updateMutex;
    util::ConditionVariable m_updateCond;
    std::thread             m_updateThread;
    
    void runThread();
    
    std::vector<char> getPipelineCache() const;
    
    std::vector<char> loadPipelineCache() const;
    
    void storePipelineCache(
      const std::vector<char>& cacheData) const;
    
    static std::string getFileName();
    
  };
  
}
