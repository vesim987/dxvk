#pragma once

#include "dxvk_binding.h"
#include "dxvk_pipecache.h"
#include "dxvk_pipelayout.h"
#include "dxvk_resource.h"
#include "dxvk_shader.h"

namespace dxvk {
  
  class DxvkDevice;
  
  /**
   * \brief Compute pipeline state info
   */
  struct DxvkComputePipelineStateInfo {
    bool operator == (const DxvkComputePipelineStateInfo& other) const;
    bool operator != (const DxvkComputePipelineStateInfo& other) const;
    
    DxvkBindingState bsBindingState;
  };
  
  
  /**
   * \brief Compute pipeline
   * 
   * Stores a compute pipeline object and the corresponding
   * pipeline layout. Unlike graphics pipelines, compute
   * pipelines do not need to be recompiled against any sort
   * of pipeline state.
   */
  class DxvkComputePipeline : public DxvkResource {
    
  public:
    
    DxvkComputePipeline(
      const DxvkDevice*             device,
      const Rc<DxvkPipelineCache>&  cache,
      const Rc<DxvkShader>&         cs);
    ~DxvkComputePipeline();
    
    /**
     * \brief Pipeline layout
     * 
     * Stores the pipeline layout and the descriptor set
     * layout, as well as information on the resource
     * slots used by the pipeline.
     * \returns Pipeline layout
     */
    Rc<DxvkPipelineLayout> layout() const {
      return m_layout;
    }
    
    /**
     * \brief Pipeline handle
     * 
     * \param [in] state Pipeline state
     * \returns Pipeline handle
     */
    VkPipeline getPipelineHandle(
      const DxvkComputePipelineStateInfo& state);
    
  private:
    
    struct PipelineStruct {
      DxvkComputePipelineStateInfo stateVector;
      VkPipeline                   pipeline;
    };
    
    const DxvkDevice* const m_device;
    const Rc<vk::DeviceFn>  m_vkd;
    
    Rc<DxvkPipelineCache>   m_cache;
    Rc<DxvkPipelineLayout>  m_layout;
    Rc<DxvkShaderModule>    m_cs;
    
    std::vector<PipelineStruct> m_pipelines;
    
    VkPipeline m_basePipeline = VK_NULL_HANDLE;
    
    VkPipeline compilePipeline(
      const DxvkComputePipelineStateInfo& state,
            VkPipeline                    baseHandle) const;
    
    void destroyPipelines();
    
  };
  
}
