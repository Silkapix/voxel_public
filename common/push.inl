#pragma once

#include "defines.inl"
#include "world_defines.inl"
#include "types.inl"

//* Remember to update them in the shader!

struct BasicPush {
  VkDeviceAddress camera_matrices;
  VkDeviceAddress faces;
  VkDeviceAddress shader_info;
  f32mat4 model_matrix;
  f32vec4 color;
};
static_assert(sizeof(BasicPush) <= 128);


struct TerrainComputePush {
  VkDeviceAddress chunk_updates;
  VkDeviceAddress scratch_voxel_buffer_pointers;
  VkDeviceAddress voxel_buffer_pointers;
  VkDeviceAddress face_buffer;
  VkDeviceAddress shader_info;
};
static_assert(sizeof(TerrainComputePush) <= 128);

struct FullscreenPush {
  f32vec4 cam_pos; // w is iTime
  VkDeviceAddress pcamera_matrices;
  VkDeviceAddress pdirs;
  VkDeviceAddress reserved;
};
static_assert(sizeof(FullscreenPush) <= 128);
