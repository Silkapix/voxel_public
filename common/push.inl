#pragma once

#include "defines.inl"
#include "world_defines.inl"
#include "types.inl"

//* Remember to update them in the shader!

struct BasicPush {
  VkDeviceAddress pcamera_matrices;
  VkDeviceAddress pvertices;
  VkDeviceAddress phost_draw_info;
  VkDeviceAddress pfaces;
  f32mat4 model_matrix;
  f32vec4 color;
};
static_assert(sizeof(BasicPush) <= 128);


struct TerrainComputePush {
  i32vec4 chunk_pos;
  VkDeviceAddress pvoxels;
  VkDeviceAddress pface_buffer;
  VkDeviceAddress phost_draw_info;
};
static_assert(sizeof(TerrainComputePush) <= 128);


struct FullscreenPush {
  f32vec4 cam_pos; // w is iTime
  VkDeviceAddress pcamera_matrices;
  VkDeviceAddress pdirs;
  VkDeviceAddress reserved;
};
static_assert(sizeof(FullscreenPush) <= 128);
