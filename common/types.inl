#ifdef __cplusplus
#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
typedef glm::ivec2 i32vec2;
typedef glm::ivec3 i32vec3;
typedef glm::ivec4 i32vec4;
typedef glm::uvec2 u32vec2;
typedef glm::uvec3 u32vec3;
typedef glm::uvec4 u32vec4;

typedef glm::vec2 f32vec2;
typedef glm::vec3 f32vec3;
typedef glm::vec4 f32vec4;
typedef glm::dvec2 f64vec2;
typedef glm::dvec3 f64vec3;
typedef glm::dvec4 f64vec4;

typedef glm::mat3 f32mat3;
typedef glm::mat4 f32mat4;
typedef std::int16_t i16;
typedef std::uint16_t u16;
typedef std::int32_t i32;
typedef std::uint32_t u32;
typedef std::int64_t i64;
typedef std::uint64_t u64;
typedef float f32;
typedef double f64;
#define DEVICE_ADDRESS(NAME, x) struct NAME
#define DESCRIPTOR_ARRAY(SET, BINDING, TYPE, NAME)

#else

struct VkDrawIndirectCommand {
  uint vertexCount;
  uint instanceCount;
  uint firstVertex;
  uint firstInstance;
};

#define VkDeviceAddress uint64_t
#define i32vec2 ivec2
#define i32vec3 ivec3
#define i32vec4 ivec4
#define u32vec2 uvec2
#define u32vec3 uvec3
#define u32vec4 uvec4
#define f32vec2 vec2
#define f32vec3 vec3
#define f32vec4 vec4
#define f32mat3 mat3
#define f32mat4 mat4
#define i32 int
#define u32 uint
#define i64 int64_t
#define u64 uint64_t
#define f32 float
#define f64 double
#define DEVICE_ADDRESS(NAME, x) layout(buffer_reference, scalar, buffer_reference_align = x) buffer NAME
#define DESCRIPTOR_ARRAY(SET, BINDING, TYPE, NAME) layout (set = SET, binding = BINDING) uniform TYPE NAME[]

#endif