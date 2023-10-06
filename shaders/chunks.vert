#version 450

#include "../common/common.inl"

//* VK_CULL_MODE_BACK_BIT
//* VK_FRONT_FACE_COUNTER_CLOCKWISE
Vertex cube_vertices[6*6] =
{ // Top face
  Vertex(vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)),
  Vertex(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f)),
  // Front Face
  Vertex(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)),
  Vertex(vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f)),
  // Left face
  Vertex(vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)),
  Vertex(vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f)),
  // Back face
  Vertex(vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)),
  Vertex(vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)), Vertex(vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f)),
  // Right face
  Vertex(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)),
  Vertex(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f)),
  // Bottom face
  Vertex(vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)),
  Vertex(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)), Vertex(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)), Vertex(vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, -1.0f, 0.0f)),
};

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform Push {
  VkDeviceAddress pcamera_matrices;
  VkDeviceAddress pvertices;
  VkDeviceAddress phost_draw_info;
  VkDeviceAddress pfaces;
  mat4 model_matrix;
  vec4 color;
};

void main() {
  CameraMatrices matrices = CameraMatrices(pcamera_matrices);
  uint i_div = gl_VertexIndex/6;
  uint i_mod = gl_VertexIndex%6;
  ivec3 voxel_pos = FaceBuffer(pfaces).faces[i_div].position;
  uint index = FaceBuffer(pfaces).faces[i_div].index;

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
    // Top
    case 0:
      light_intensity = 1.0;
      //color = vec3(1.0, 1.0, 1.0);
      break;
    // Front
    case 1:
      light_intensity = 0.8;
      //color = vec3(1.0, 0.0, 0.0);
      break;
    // Left
    case 2:
      light_intensity = 0.4;
      //color = vec3(0.0, 0.0, 1.0);
      break;
    // Back
    case 3:
      light_intensity = 0.5;
      //color = vec3(0.0, 1.0, 0.0);
      break;
    // Right
    case 4:
      light_intensity = 0.65;
      //color = vec3(1.0, 0.0, 1.0);
      break;
    // Bottom
    case 5:
      light_intensity = 0.3;
      //color = vec3(0.0, 0.0, 0.0);
      break;
    default:
      light_intensity = 0.1;
      //color = vec3(1.0, 1.0, 0.0);
      break;
  };
  out_color = vec4(color.xyz * light_intensity, color.w);
}