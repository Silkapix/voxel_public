#pragma once
#include "../../common/types.inl"
#include "../ecs/scene.hpp"
namespace sve {

struct DrawInfo {
  BasicPush push{};
  uint32_t vertex_count{};
};

struct DrawIndirectInfo {
  BasicPush push{};
  VkBuffer indirect_commands_buffer;
  uint32_t draw_count;
};

struct FullscreenDrawInfo {
  f32vec4 cam_pos; // w is iTime
  VkDeviceAddress pcamera_matrices;
  VkDeviceAddress pdirs;
};

}