#pragma once

#include "../../common/common.inl"
#include "../shared/render.hpp"
#include "../raster_pipeline.hpp"
#include "../camera.hpp"

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

namespace sve {
  struct ChunkRenderer {
    public:
    ChunkRenderer(
      VkDevice device,
      VkFormat swapchain_image_format,
      VkFormat depth_format,
      bool enable_blending);
    ~ChunkRenderer();

    ChunkRenderer(const ChunkRenderer &) = delete;
    ChunkRenderer &operator=(const ChunkRenderer &) = delete;

    void cmd_draw(VkCommandBuffer &command_buffer, DrawInfo &draw_info);
    void cmd_draw_indirect(VkCommandBuffer &command_buffer, DrawIndirectInfo &draw_info);

    private:
    void create_raster_pipeline(
      VkDevice &device, 
      VkFormat &swapchain_image_format,
      VkFormat depth_format,
      bool enable_blending);

    VkDevice &device;
    VkPipelineLayout pipeline_layout;
    std::unique_ptr<RasterPipeline> raster_pipeline;
  };
}