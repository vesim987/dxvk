#include "dxvk_event.h"

namespace dxvk {
  
  DxvkEvent:: DxvkEvent() { }
  DxvkEvent::~DxvkEvent() { }
  
  
  uint32_t DxvkEvent::reset() {
    std::unique_lock<util::CriticalMutex> lock(m_mutex);
    
    m_status = DxvkEventStatus::Reset;
    return ++m_revision;
  }
  
  
  void DxvkEvent::signal(uint32_t revision) {
    std::unique_lock<util::CriticalMutex> lock(m_mutex);
    
    if (m_revision == revision)
      m_status = DxvkEventStatus::Signaled;
  }
  
  
  DxvkEventStatus DxvkEvent::getStatus() {
    std::unique_lock<util::CriticalMutex> lock(m_mutex);
    return m_status;
  }
  
}
