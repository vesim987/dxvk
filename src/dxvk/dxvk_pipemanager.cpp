#include "dxvk_pipemanager.h"

namespace dxvk {
  
  size_t DxvkPipelineKeyHash::operator () (const DxvkComputePipelineKey& key) const {
    std::hash<DxvkShader*> hash;
    return hash(key.cs.ptr());
  }
  
  
  size_t DxvkPipelineKeyHash::operator () (const DxvkGraphicsPipelineKey& key) const {
    DxvkHashState state;
    
    std::hash<DxvkShader*> hash;
    state.add(hash(key.vs.ptr()));
    state.add(hash(key.tcs.ptr()));
    state.add(hash(key.tes.ptr()));
    state.add(hash(key.gs.ptr()));
    state.add(hash(key.fs.ptr()));
    return state;
  }
  
  
  bool DxvkPipelineKeyEq::operator () (const DxvkComputePipelineKey& a, const DxvkComputePipelineKey& b) const {
    return a.cs == b.cs;
  }
  
  
  bool DxvkPipelineKeyEq::operator () (const DxvkGraphicsPipelineKey& a, const DxvkGraphicsPipelineKey& b) const {
    return a.vs  == b.vs
        && a.tcs == b.tcs
        && a.tes == b.tes
        && a.gs  == b.gs
        && a.fs  == b.fs;
  }
  
  
  DxvkPipelineManager::DxvkPipelineManager(const DxvkDevice* device)
  : m_device(device) {
    
  }
  
  
  DxvkPipelineManager::~DxvkPipelineManager() {
    
  }
  
  
  Rc<DxvkComputePipeline> DxvkPipelineManager::createComputePipeline(
    const Rc<DxvkPipelineCache>&  cache,
    const Rc<DxvkShader>&         cs) {
    if (cs == nullptr)
      return nullptr;
    
    DxvkComputePipelineKey key;
    key.cs = cs;
    
    auto pair = m_computePipelines.find(key);
    if (pair != m_computePipelines.end())
      return pair->second;
    
    const Rc<DxvkComputePipeline> pipeline
      = new DxvkComputePipeline(m_device, cache, cs);
    
    m_computePipelines.insert(std::make_pair(key, pipeline));
    return pipeline;
  }
  
  
  Rc<DxvkGraphicsPipeline> DxvkPipelineManager::createGraphicsPipeline(
    const Rc<DxvkPipelineCache>&  cache,
    const Rc<DxvkShader>&         vs,
    const Rc<DxvkShader>&         tcs,
    const Rc<DxvkShader>&         tes,
    const Rc<DxvkShader>&         gs,
    const Rc<DxvkShader>&         fs) {
    if (vs == nullptr)
      return nullptr;
    
    DxvkGraphicsPipelineKey key;
    key.vs  = vs;
    key.tcs = tcs;
    key.tes = tes;
    key.gs  = gs;
    key.fs  = fs;
    
    auto pair = m_graphicsPipelines.find(key);
    if (pair != m_graphicsPipelines.end())
      return pair->second;
    
    const Rc<DxvkGraphicsPipeline> pipeline
      = new DxvkGraphicsPipeline(m_device, cache, vs, tcs, tes, gs, fs);
    
    m_graphicsPipelines.insert(std::make_pair(key, pipeline));
    return pipeline;
  }
  
}
