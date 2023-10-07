#version 450

#include "../common/common.inl"

//* VK_CULL_MODE_BACK_BIT
//* VK_FRONT_FACE_COUNTER_CLOCKWISE
Vertex cube_vertices[6*6] =
{
  // Top
  Vertex(vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)),
  Vertex(vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)),
  // Front
  Vertex(vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)),
  Vertex(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)),
  // Left
  Vertex(vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)),
  Vertex(vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)),
  // Back
  Vertex(vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)),
  Vertex(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)),
  // Right
  Vertex(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)),
  Vertex(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)),
  // Bottom
  Vertex(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)),
  Vertex(vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)),
};

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform Push {
  CameraMatrices camera_matrices;
  FaceBuffer faces;
  ShaderInfo shader_info;
  mat4 model_matrix;
  vec4 color;
};

void main() {
  CameraMatrices matrices = camera_matrices;
  uint i_div = gl_VertexIndex/6;
  uint i_mod = gl_VertexIndex%6;
  ivec3 voxel_pos = faces.faces[i_div].position;
  uint index = faces.faces[i_div].index;

  vec4 pos = vec4(cube_vertices[index*6 + i_mod].position.xyz, 1.0) + vec4(voxel_pos, 0.0);

  // model space to world space
  vec4 worldspace = model_matrix * pos;

  // Camera space. Inverse of camera transform.
  // Also called camera transformation matrix
  vec4 viewspace = matrices.view_matrix * worldspace;

  // view space to clip space
  gl_Position = matrices.projection_matrix * viewspace;

  //* Fragment shader input
  float light_intensity;
  switch(index) {
    case 0:
      light_intensity = 1.0;
      //color = vec3(1.0, 1.0, 1.0);
      break;
    case 1:
      light_intensity = 0.7;
      //color = vec3(1.0, 0.0, 0.0);
      break;
    case 2:
      light_intensity = 0.6;
      //color = vec3(0.0, 0.0, 1.0);
      break;
    case 3:
      light_intensity = 0.5;
      //color = vec3(0.0, 1.0, 0.0);
      break;
    case 4:
      light_intensity = 0.8;
      //color = vec3(1.0, 0.0, 1.0);
      break;
    case 5:
      light_intensity = 0.3;
      //color = vec3(0.0, 0.0, 0.0);
      break;
    default:
      light_intensity = 0.0;
      //color = vec3(1.0, 1.0, 0.0);
      break;
  };

  out_color = vec4(color.xyz * light_intensity, color.w);
}