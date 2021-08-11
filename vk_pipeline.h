#pragma once

#define USE_VOLK
#include "vk_include.h"

#include <unordered_map>
#include <vector>

namespace vk_utils
{
  VkPipelineInputAssemblyStateCreateInfo IA_TList();
  VkPipelineInputAssemblyStateCreateInfo IA_PList();
  VkPipelineInputAssemblyStateCreateInfo IA_LList();
  VkPipelineInputAssemblyStateCreateInfo IA_LSList();

  struct GraphicsPipelineMaker
  {
    static constexpr uint32_t MAX_STAGES = 5;
    VkShaderModule                  shaderModules[MAX_STAGES] = { VK_NULL_HANDLE };
    VkPipelineShaderStageCreateInfo shaderStageInfos[MAX_STAGES] = { };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
    VkViewport                             viewport {};
    VkRect2D                               scissor {};
    VkPipelineViewportStateCreateInfo      viewportState {};
    VkPipelineRasterizationStateCreateInfo rasterizer {};
    VkPipelineMultisampleStateCreateInfo   multisampling {};
    VkPipelineColorBlendAttachmentState    colorBlendAttachment {};
    VkPipelineColorBlendStateCreateInfo    colorBlending {};
    VkPushConstantRange                    pcRange {};
    VkPipelineLayoutCreateInfo             pipelineLayoutInfo {};
    VkPipelineDepthStencilStateCreateInfo  depthStencilTest {};
    VkGraphicsPipelineCreateInfo           pipelineInfo {};

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void             LoadShaders(VkDevice a_device, const std::unordered_map<VkShaderStageFlagBits, std::string> &shader_paths);
    void             SetDefaultState(uint32_t a_width, uint32_t a_height);
    VkPipelineLayout MakeLayout(VkDevice a_device, std::vector<VkDescriptorSetLayout> a_dslayouts, uint32_t a_pcRangeSize);
    VkPipeline       MakePipeline(VkDevice a_device, VkPipelineVertexInputStateCreateInfo a_vertexLayout, VkRenderPass a_renderPass,
                                  std::vector<VkDynamicState> a_dynamicStates = {},
                                  VkPipelineInputAssemblyStateCreateInfo a_inputAssembly = IA_TList());
  private:
    int              m_stagesNum = 0;
    VkPipeline       m_pipeline  = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
  };
}

