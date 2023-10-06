#pragma once

#include "../../common/types.inl"

#include <vulkan/vulkan.h>

#include <string>

namespace sve {
  struct ComputePipeline {
    public:
    ComputePipeline(VkDevice device, std::string shader_file_name, size_t push_constant_size);
    ~ComputePipeline() = default;

    void dispatch(VkCommandBuffer command_buffer, const u32 x, const u32 y, const u32 z, const void *push);

    private:
    void create_shader_module(const std::vector<char> &code, VkShaderModule *module);

    VkDevice device;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    size_t push_constant_size;
  };
}