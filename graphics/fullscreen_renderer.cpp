#include "fullscreen_renderer.hpp"

#include <array>
#include <cassert>
#include <map>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace sve {
  FullscreenRenderer::FullscreenRenderer(VkDevice device, VkFormat swapchain_image_format, VkFormat depth_format) : device{device} {
    create_raster_pipeline(device, swapchain_image_format, depth_format);
  }

  FullscreenRenderer::~FullscreenRenderer() {

  }

  void FullscreenRenderer::create_raster_pipeline(VkDevice &device, VkFormat &swapchain_image_format, VkFormat depth_format) {
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    // Descriptor set layouts
    pipeline_layout_create_info.pSetLayouts = nullptr;
    pipeline_layout_create_info.setLayoutCount = 0;
    // Push constant
    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_ALL;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(FullscreenPush);

    pipeline_layout_create_info.pushConstantRangeCount = 1;
    pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;
    auto result = vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout);
    VK_CHECK("vkCreatePipelineLayout");
    
    raster_pipeline = std::make_unique<RasterPipeline>(
      device,
      "clouds", 
      pipeline_layout, 
      swapchain_image_format, 
      depth_format,
      true);
  }

  void FullscreenRenderer::cmd_draw(VkCommandBuffer &command_buffer, FullscreenDrawInfo &draw_info) {
    raster_pipeline->cmd_bind_pipeline(command_buffer);

    // w is iTime
    FullscreenPush push{draw_info.cam_pos, draw_info.pcamera_matrices, draw_info.pdirs};
    vkCmdPushConstants(command_buffer,
                       pipeline_layout,
                       VK_SHADER_STAGE_ALL,
                       0,
                       sizeof(FullscreenPush),
                       &push
                      );

    vkCmdDraw(command_buffer, 3, 1, 0, 0);
  }
}