#include "compute_pipeline.hpp"

#include "../../common/push.inl"
#include "../shared/utils.hpp"

#include <fstream>

namespace sve {

  std::vector<char> read_file(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      std::string a = "Failed to open file ";
      std::string f = filename;
      std::string c = " !";
      std::string msg = a + f + c;
      throw std::runtime_error(msg);
    }

    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();

    return buffer;
  }

  void ComputePipeline::create_shader_module(const std::vector<char> &code, VkShaderModule *module) {
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = code.size();
    shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    auto result = vkCreateShaderModule(device, &shader_module_create_info, nullptr, module);
    VK_CHECK("vkCreateShaderModule");
  }

  ComputePipeline::ComputePipeline(VkDevice device, std::string shader_file_name, size_t push_constant_size)
                  : device{device}, push_constant_size{push_constant_size} {
    std::string f = "../../../spv/" + shader_file_name + ".comp.spv";
    auto compute_code = read_file(f);
    VkShaderModule compute_module{VK_NULL_HANDLE};
    create_shader_module(compute_code, &compute_module);
    
    VkPipelineShaderStageCreateInfo shader_stage_create_info {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = compute_module,
      .pName = "main",
      .pSpecializationInfo = nullptr
    };

    VkPushConstantRange range {
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .offset = 0,
      .size = static_cast<u32>(push_constant_size)
    };

    VkPipelineLayoutCreateInfo layout_create_info {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &range
    };

    auto result = vkCreatePipelineLayout(device, &layout_create_info, nullptr, &pipeline_layout);
    VK_CHECK("Compute::vkCreatePipelineLayout");

    VkComputePipelineCreateInfo create_info {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stage = shader_stage_create_info,
      .layout = pipeline_layout,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = 0
    };

    result = vkCreateComputePipelines(
              device,
              nullptr,
              1,
              &create_info,
              nullptr,
              &pipeline);
    VK_CHECK("vkCreateComputePipelines");

  }

  void ComputePipeline::dispatch(
    VkCommandBuffer command_buffer,
    const u32 x,
    const u32 y,
    const u32 z,
    const void *push) {
      
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    vkCmdPushConstants(
      command_buffer,
      pipeline_layout,
      VK_SHADER_STAGE_COMPUTE_BIT,
      0,
      push_constant_size,
      push
    );

    vkCmdDispatch(command_buffer, x, y, z);
  }
}