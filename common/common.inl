#ifdef __cplusplus
#pragma once
#include "push.inl"
#endif

#include "defines.inl"
#include "world_defines.inl"
#include "types.inl"
#include "voxel_types.inl"

STATIC INLINE int voxel_index_from_coords(int x, int y, int z) {
  int index = x + y * WORLD_CHUNK_NX + z * WORLD_CHUNK_NX*WORLD_CHUNK_NY;
  return index;
}

STATIC INLINE int voxel_index_from_coords(i32vec3 pos) {
  int index = pos.x + pos.y * WORLD_CHUNK_NX + pos.z * WORLD_CHUNK_NX*WORLD_CHUNK_NY;
  return index;
}

STATIC INLINE int chunk_index_from_coords(i32vec2 pos) {
  int index = pos.x + pos.y * RENDER_DISTANCE;
  return index;
}

STATIC INLINE int chunk_index_from_coords(i32vec3 pos) {
  int index = pos.x + pos.y * RENDER_DISTANCE + pos.z * RENDER_DISTANCE*RENDER_DISTANCE;
  return index;
}

#if !defined(__cplusplus)
f32 saturate(f32 x) {
  return clamp(x, 0.0, 1.0);
}
#endif

DEVICE_ADDRESS(CameraMatrices, 16) {
  f32mat4 projection_matrix;
  f32mat4 view_matrix;
  f32mat4 model_matrix;
};

DEVICE_ADDRESS(Directions, 16) {
  f32vec4 bl;
  f32vec4 tl;
  f32vec4 br;
  f32vec4 tr;
};

#define TERRAIN_GENERATION_DEVICE (1)
#define TERRAIN_GENERATION_HOST (!TERRAIN_GENERATION_DEVICE)

#if TERRAIN_GENERATION_DEVICE

//*******************************
#define TERRAIN_MESHING_DEVICE (1)
//*******************************

#define TERRAIN_MESHING_HOST (!TERRAIN_MESHING_DEVICE)
#else
#define TERRAIN_MESHING_DEVICE (0)
#define TERRAIN_MESHING_HOST (!TERRAIN_MESHING_DEVICE)
#endif

struct Vertex {
  f32vec4 position;
  f32vec4 normal;
};

struct VoxelFace {
  Vertex vertices[6];
};

struct VoxelCube {
  VoxelFace faces[6]; 
};

#define sizeof_vertex (sizeof(f32vec4)+sizeof(f32vec4))
#define sizeof_voxel_face (sizeof_vertex*6)
#define sizeof_voxel_cube (sizeof_voxel_face*6)

#if !defined(__cplusplus)
DEVICE_ADDRESS(VertexBuffer, 16) {
  Vertex vertices[];
};

struct Face {
  i32vec3 position;
  uint index;
};

DEVICE_ADDRESS(FaceBuffer, 16) {
  Face faces[];
};
#endif

DEVICE_ADDRESS(VoxelBuffer, 16) {
  u32 voxels[1];
};

DEVICE_ADDRESS(HostDrawInfo, 16) {
  u32 face_count;
};

//DESCRIPTOR_ARRAY(0, 0, sampler2D, textures);
//DESCRIPTOR_ARRAY(1, 0, sampler3D, animations);