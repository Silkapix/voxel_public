#include "chunk_renderer.hpp"

#include <array>
#include <cassert>
#include <map>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace sve {

  ChunkRenderer::ChunkRenderer(VkDevice device, VkFormat swapchain_image_format, VkFormat depth_format, bool enable_blending) : device{device} {
    create_raster_pipeline(device, swapchain_image_format, depth_format, enable_blending);
  }

  ChunkRenderer::~ChunkRenderer() {

  }

  void ChunkRenderer::create_raster_pipeline(VkDevice &device, VkFormat &swapchain_image_format, VkFormat depth_format, bool enable_blending) {    
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // Descriptor set layouts
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pSetLayouts = nullptr;
    // Push constant
    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(BasicPush);

    pipeline_layout_create_info.pushConstantRangeCount = 1;
    pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;
    auto result = vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout);
    VK_CHECK("vkCreatePipelineLayout");
    
    raster_pipeline = std::make_unique<RasterPipeline>(
      device,
      "chunks",
      pipeline_layout,
      swapchain_image_format,
      depth_format,
      false);
  }

  void ChunkRenderer::cmd_draw(VkCommandBuffer &command_buffer, DrawInfo &draw_info) {
    raster_pipeline->cmd_bind_pipeline(command_buffer);
    
    BasicPush push{draw_info.push};
    vkCmdPushConstants(command_buffer,
                       pipeline_layout,
                       VK_SHADER_STAGE_VERTEX_BIT,
                       0,
                       sizeof(BasicPush),
                       &push
                      );

    vkCmdDraw(command_buffer, draw_info.vertex_count, 1, 0, 0);
  }

  void ChunkRenderer::cmd_draw_indirect(VkCommandBuffer &command_buffer, DrawIndirectInfo &draw_info) {
    raster_pipeline->cmd_bind_pipeline(command_buffer);

    BasicPush push{draw_info.push};
    vkCmdPushConstants(command_buffer,
                       pipeline_layout,
                       VK_SHADER_STAGE_VERTEX_BIT,
                       0,
                       sizeof(BasicPush),
                       &push
                      );

    assert(draw_info.draw_count != 0 && "Why are you doing draw indirect with 0 draws??");
    vkCmdDrawIndirect(command_buffer,
                      draw_info.indirect_commands_buffer,
                      (VkDeviceSize)0,
                      draw_info.draw_count,
                      sizeof(VkDrawIndirectCommand)
                     );
  }
}