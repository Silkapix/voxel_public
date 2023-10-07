#pragma once

#include "../../common/common.inl"
#include "../shared/render.hpp"
#include "../raster_pipeline.hpp"
#include "../camera.hpp"

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

namespace sve {
  struct FullscreenRenderer {
    public:
    FullscreenRenderer(VkDevice device, VkFormat swapchain_image_format, VkFormat depth_format);
    ~FullscreenRenderer();

    FullscreenRenderer(const FullscreenRenderer &) = delete;
    FullscreenRenderer &operator=(const FullscreenRenderer &) = delete;

    void cmd_draw(VkCommandBuffer &command_buffer, FullscreenDrawInfo &draw_info);

    VkPipelineLayout &get_pipeline_layout() { return pipeline_layout; }

    private:
    void create_raster_pipeline(VkDevice &device, VkFormat &swapchain_image_format, VkFormat depth_format);

    VkDevice &device;
    VkPipelineLayout pipeline_layout;
    std::unique_ptr<RasterPipeline> raster_pipeline;
  };
}