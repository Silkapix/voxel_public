#ifdef __cplusplus
#pragma once
#define STATIC static
#define INLINE inline

#else
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_buffer_reference_uvec2 : require
#extension GL_KHR_shader_subgroup_arithmetic : require
#extension GL_KHR_shader_subgroup_ballot : require

#define gl_GlobalInvocationIndex int( gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * (gl_WorkGroupSize.x) + gl_GlobalInvocationID.z * (gl_WorkGroupSize.x*gl_WorkGroupSize.y) )
#define sizeof(Type) (uint( uint64_t(Type(uint64_t(0))+1) ))

#define STATIC
#define INLINE
#endif