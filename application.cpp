#include "application.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "input.hpp"
#include "ecs/entity.hpp"
#include "ecs/entity_manager.hpp"
#include "ecs/event_bus.hpp"

#include "app_systems_and_renderers.hpp"

#include "compute/compute_pipeline.hpp"

#include "buffer.hpp"
#include "image.hpp"
#include "shared/render.hpp"

#include <iostream>
#include <array>
#include <cstring>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <utility>

#include <unistd.h>

#define DEFAULT_INIT 0

namespace sve {
  Directions set_dirs(float aspect, float fov, glm::vec3 rotation) {
    auto v = glm::vec3{ 1.0, 1.0 / aspect, cos(fov) };
  
    auto yaw = rotation.y;
    auto pitch = rotation.x;
    auto roll = rotation.z;
    glm::mat4 m{1.};
    auto rot = glm::rotate(m, yaw, glm::vec3(0.0, 1.0, 0.0)) * glm::rotate(m, pitch, glm::vec3(1.0, 0.0, 0.0)) * glm::rotate(m, roll, glm::vec3(0.0, 0.0, 1.0));

    auto tl = glm::mat3(rot) * glm::vec3{-v.x,  v.y, v.z};
    auto tr = glm::mat3(rot) * glm::vec3{ v.x,  v.y, v.z};
    auto bl = glm::mat3(rot) * glm::vec3{-v.x, -v.y, v.z};
    auto br = glm::mat3(rot) * glm::vec3{ v.x, -v.y, v.z};

    Directions dirs_struct = {
      .bl = glm::vec4(bl, 1.0),
      .tl = glm::vec4(tl, 1.0),
      .br = glm::vec4(br, 1.0),
      .tr = glm::vec4(tr, 1.0)
    };
    return dirs_struct;
  }

  void Application::create_descriptor_resources(void) {
    static const u32 max_bindless_resources = 16536;

    //////////////////////////////////*
    // DESCRIPTOR POOL
    //////////////////////////////////*

    VkDescriptorPoolSize pool_sizes_bindless[] = {
      { 
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = max_bindless_resources
      },
      { 
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = max_bindless_resources
      }
    };
    
    // Update after bind is needed here, for each binding and in the descriptor set layout creation.
    VkDescriptorPoolCreateInfo descriptor_pool_info {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
      .maxSets = static_cast<u32>(2),
      .poolSizeCount = static_cast<u32>(2),
      .pPoolSizes = pool_sizes_bindless
    };
    auto result = vkCreateDescriptorPool( renderer.get_device(), &descriptor_pool_info, nullptr, &descriptor_pool );
    VK_CHECK("vkCreateDescriptorPool");

    ////////////////////////////////////////*
    // DESCRIPTOR SET LAYOUT
    ////////////////////////////////////////*

    VkDescriptorSetLayoutBinding texture_set_layout_binding {
      .binding = static_cast<u32>(0),
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = max_bindless_resources,
      .stageFlags = VK_SHADER_STAGE_ALL,
      .pImmutableSamplers = nullptr
    };

    VkDescriptorSetLayoutBinding animation_set_layout_binding {
      .binding = static_cast<u32>(0),
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = max_bindless_resources,
      .stageFlags = VK_SHADER_STAGE_ALL,
      .pImmutableSamplers = nullptr
    };

    VkDescriptorBindingFlags binding_flags_bindless =
    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info_EXT {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
      .pNext = nullptr,
      .bindingCount = static_cast<u32>(1),
      .pBindingFlags = &binding_flags_bindless
    };

    VkDescriptorSetLayoutCreateInfo texture_layout_info {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = &extended_info_EXT,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
      .bindingCount = static_cast<u32>(1),
      .pBindings = &texture_set_layout_binding,
    };

    VkDescriptorSetLayoutCreateInfo animation_layout_info {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = &extended_info_EXT,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
      .bindingCount = static_cast<u32>(1),
      .pBindings = &animation_set_layout_binding,
    };

    result = vkCreateDescriptorSetLayout(
      renderer.get_device(),
      &texture_layout_info,
      nullptr,
      &texture_set_layout
    );
    VK_CHECK("vkCreateDescriptorSetLayout");

    result = vkCreateDescriptorSetLayout(
      renderer.get_device(),
      &animation_layout_info,
      nullptr,
      &animation_set_layout
    );
    VK_CHECK("vkCreateDescriptorSetLayout");

    ////////////////////////////////////*
    // ALLOCATE DESCRIPTOR SETS
    ////////////////////////////////////*

    u32 greatest_binding_slot = static_cast<u32>(max_bindless_resources - 1);
    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT count_info {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
      .descriptorSetCount = static_cast<u32>(1),
      .pDescriptorCounts = &greatest_binding_slot
    };

    VkDescriptorSetAllocateInfo texture_allocate_info {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = &count_info,
      .descriptorPool = descriptor_pool,
      .descriptorSetCount = static_cast<uint32_t>(1),
      .pSetLayouts = &texture_set_layout
    };

    VkDescriptorSetAllocateInfo animation_allocate_info {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = &count_info,
      .descriptorPool = descriptor_pool,
      .descriptorSetCount = static_cast<uint32_t>(1),
      .pSetLayouts = &animation_set_layout
    };

    result = vkAllocateDescriptorSets(renderer.get_device(), &texture_allocate_info, &texture_descriptor_set);
    VK_CHECK("vkAllocateDescriptorSets");

    result = vkAllocateDescriptorSets(renderer.get_device(), &animation_allocate_info, &animation_descriptor_set);
    VK_CHECK("vkAllocateDescriptorSets");
  }

  void Application::load_textures(std::vector<const char *> texture_paths) {
    u32 size = texture_paths.size();
    VkWriteDescriptorSet bindless_image_descriptor_set_write;
    VkDescriptorImageInfo descriptor_image_infos[size];

    u32 i{0};
    for (; i < size; i++) {
      Image texture_image = Image{renderer.get_sve_device()};
      texture_image.create_texture_image(texture_paths[i], VK_IMAGE_ASPECT_COLOR_BIT, true);

      VkDescriptorImageInfo descriptor_image_info {
        .sampler = texture_image.get_sampler(),
        .imageView = texture_image.get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      };
      descriptor_image_infos[i] = descriptor_image_info;
      textures.emplace_back(texture_image);
    }

    VkWriteDescriptorSet descriptor_write {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = texture_descriptor_set,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = size,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = descriptor_image_infos,
      .pBufferInfo = nullptr
    };

    vkUpdateDescriptorSets(
      renderer.get_device(),
      1,
      &descriptor_write,
      0,
      nullptr
    );
  }

  void Application::load_animations(std::vector<const char *> animation_paths) {
    u32 num_animations = animation_paths.size();

    VkWriteDescriptorSet bindless_image_descriptor_set_write;
    VkDescriptorImageInfo descriptor_image_infos[num_animations];
    
    for(u32 i{0}; i < num_animations; i++) {
      std::vector<std::string> frame_paths;
      u32 frame_count{0};
      while(true) {
        std::string prepend = "../../../animations/";
        std::string a = animation_paths[i];
        std::string b = std::to_string(frame_count) + ".png";
        // Doesn't account for when I have 100+ frames in an animation.
        // Why would I have that many anyway?
        std::string c = frame_count<=9 ? "000" + b : "00" + b;
        std::string path = a + c;
        std::string path_exists = prepend + path;
        if(std::filesystem::exists(path_exists))
          frame_paths.push_back(std::move(path));
        else
          break;
        frame_count++;
      }
      Device &device = renderer.get_sve_device();
      // The 3D image for the animation.
      // Depth is time/frame of animation.
      Image animation_atlas = Image{renderer.get_sve_device()};
      animation_atlas.create_atlas3D(frame_paths, VK_IMAGE_ASPECT_COLOR_BIT, true);

      VkDescriptorImageInfo descriptor_image_info {
        .sampler = animation_atlas.get_sampler(),
        .imageView = animation_atlas.get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      };

      descriptor_image_infos[i] = descriptor_image_info;
      anims.emplace_back(animation_atlas);
    }


    VkWriteDescriptorSet descriptor_write {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = animation_descriptor_set,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = num_animations,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = descriptor_image_infos,
      .pBufferInfo = nullptr
    };

    vkUpdateDescriptorSets(
      renderer.get_device(),
      1,
      &descriptor_write,
      0,
      nullptr
    );
  }

  //////////////////*
  // APPLICATION RUN
  //////////////////*

  void Application::run(void) {
    subgroup_size = renderer.subgroup_size;

    ChunkRenderer chunk_renderer {
      renderer.get_device(),
      renderer.get_swapchain_image_format(),
      renderer.get_depth_format(),
      false
    };

    ComputePipeline terrain_gen_pipeline{renderer.get_device(), "terrain_gen", sizeof(TerrainComputePush)};
    ComputePipeline terrain_meshing_pipeline{renderer.get_device(), "terrain_meshing", sizeof(TerrainComputePush)};

    static_assert(WORLD_CHUNK_NVOX%32==0);
    std::unique_ptr<Buffer> voxel_buffer;
    voxel_buffer = std::make_unique<Buffer>(
                        renderer.get_sve_device(),
                        WORLD_CHUNK_NVOX * sizeof(u32),
                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                        );

    std::unique_ptr<Buffer> face_buffer;
    face_buffer = std::make_unique<Buffer>(
                      renderer.get_sve_device(),
                      sizeof(i32vec4) * 6/*6 faces*/ * WORLD_CHUNK_NVOX * WORLD_NCHUNKS, 
                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                    );

    std::unique_ptr<Buffer> host_info_buffer;
      host_info_buffer = std::make_unique<Buffer>(
                        renderer.get_sve_device(),
                        sizeof(HostDrawInfo), 
                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                      | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                      );
      host_info_buffer->map_memory();

    std::cout << "\nCREATING CHUNKS\n" << std::endl;
    for(int y{0}; y < RENDER_HEIGHT; y++) {
    for(int z{0}; z < RENDER_DISTANCE; z++) {
    for(int x{0}; x < RENDER_DISTANCE; x++) {
      i32vec3 chunk_pos{x, y, z};
      u32 i = chunk_index_from_coords(chunk_pos);

      static_assert(WORLD_CHUNK_NVOX == CHUNK_GENERATION_DISPATCH_INVOCATIONS);

      TerrainComputePush gen_push {
        .chunk_pos = f32vec4{chunk_pos, 0.0},
        .pvoxels = voxel_buffer->get_buffer_device_address(),
        // Not needed for this dispatch
        .pface_buffer = (VkDeviceAddress)0,
        .phost_draw_info = (VkDeviceAddress)0
      };

      VkCommandBuffer gen_command_buffer = renderer.get_sve_device().begin_single_time_command_buffer();
      
      //* Each thread does 1 voxel
      terrain_gen_pipeline.dispatch(
        gen_command_buffer,
        CHUNK_GENERATION_WORKGROUP_NX,
        CHUNK_GENERATION_WORKGROUP_NY,
        CHUNK_GENERATION_WORKGROUP_NZ,
        &gen_push
      );

      renderer
        .get_sve_device()
        .end_single_time_compute_command_buffer_and_queue_submit_then_queue_wait_idle(gen_command_buffer, 0);

       //***********             *************//
      //*********** CHUNK MESHING ***********//
      //************             *************//

      static_assert(WORLD_CHUNK_NVOX == CHUNK_GENERATION_DISPATCH_INVOCATIONS);

      TerrainComputePush meshing_push {
        .chunk_pos = f32vec4{chunk_pos, 0.0},
        .pvoxels = voxel_buffer->get_buffer_device_address(),
        .pface_buffer = face_buffer->get_buffer_device_address(),
        .phost_draw_info = host_info_buffer->get_buffer_device_address()
      };

      VkCommandBuffer meshing_command_buffer = renderer.get_sve_device().begin_single_time_command_buffer();

      //* Each thread does 1 voxel
      terrain_meshing_pipeline.dispatch(
        meshing_command_buffer,
        CHUNK_MESHING_WORKGROUP_NX,
        CHUNK_MESHING_WORKGROUP_NY,
        CHUNK_MESHING_WORKGROUP_NZ,
        &meshing_push
      );
      
      renderer
        .get_sve_device()
        .end_single_time_compute_command_buffer_and_queue_submit_then_queue_wait_idle(meshing_command_buffer, 0);
    }
    }
    }

    PRINT("Wrote vertices.");

    EventBus event_bus{};
    Input input{window.get_glfw_window(), &event_bus};
    Scene scene{};
    EntityManager manager{&scene};

    Entity &default_entity = manager.create_entity("default_entity");
    default_entity.add_component<TransformComponent>();

    constexpr f32vec3 PLAYER_STARTING_POS = {0.0, 0.0, 0.0};

    Camera camera {
      TransformComponent{PLAYER_STARTING_POS},
      &event_bus,
      0.1,
      10000.0
    };

    camera.set_view_yxz();

    std::unique_ptr<Buffer> camera_matrices_buffers[in_flight_frames_count];
    for(int i{0}; i < in_flight_frames_count; i++) {
      camera_matrices_buffers[i] = std::make_unique<Buffer>(
                          renderer.get_sve_device(),
                          sizeof(CameraMatrices),
                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                          );
      camera_matrices_buffers[i]->map_memory();
    }

    auto initial = std::chrono::steady_clock::now();
    bool first_frame{true};
    f64 f{0.0};
    f32 iTime{0.0}; // in seconds
    u32 vertices_to_render{0};

    PRINT("STARTING PROGRAM LOOP!");
    /*************************************************/
    /*************************************************/
    /*************************************************/
    /*************************************************/
    /*************************************************/
    while(!window.should_close()) {
    auto final = std::chrono::steady_clock::now();
    f64 dt = std::chrono::duration<f64, std::chrono::milliseconds::period>(final - initial).count();
    initial = final;
    //PRINT(std::to_string(dt) + " ms");
    iTime += dt/1000.0;
    //PRINT("iTime " + std::to_string(iTime));

    //* GLFW callbacks *//
    input.poll_events(dt);

    auto &camera_viewer_transform = camera.transform;
    input.move_in_xz_plane(dt, camera_viewer_transform);
    
    f32 fov = glm::radians(60.0);
    camera.set_perspective_projection(fov, window.get_aspect_ratio());
    //* view matrix
    camera.set_view_yxz();


    VkCommandBuffer command_buffer = renderer.begin_command_buffer();
    renderer.cmd_begin_rendering(command_buffer);
    uint32_t current_frame = renderer.get_current_frame();
    
    ////////*
    //* BEGIN
    ////////*
    camera_matrices_buffers[current_frame]->write(&camera.get_matrices(), sizeof(CameraMatrices));
    camera.set_window_dim(window.get_dim_f32());


    constexpr u32 SETS_SIZE = 2;
    VkDescriptorSet descriptor_sets[SETS_SIZE] = {texture_descriptor_set, animation_descriptor_set};

    
    u32 vc = reinterpret_cast<HostDrawInfo*>(host_info_buffer->get_mapped_address())->face_count*6;
    PRINT(vc);

    DrawInfo draw_info{
      .push = BasicPush {
        .pcamera_matrices = camera_matrices_buffers[current_frame]->get_buffer_device_address(),
        .pvertices = (VkDeviceAddress)0,
        .phost_draw_info = host_info_buffer->get_buffer_device_address(),
        .pfaces = face_buffer->get_buffer_device_address(),
        .model_matrix = default_entity.get_component<TransformComponent>().mat4(),
        .color = {1.0, 1.0, 1.0, 1.0},
      },
      .vertex_count = vc
    };
    
    chunk_renderer.cmd_draw(command_buffer, draw_info);

    //////*
    //* END
    //////*


    renderer.cmd_end_rendering(command_buffer);
    renderer.end_command_buffer(command_buffer);
    renderer.queue_submit_and_present(command_buffer);
    first_frame = false;
    if (glfwGetKey(window.get_glfw_window(), GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window.get_glfw_window(), true);
    }

    } // while() loop exit
    
    auto result = vkDeviceWaitIdle(renderer.get_device());

    if(result != VK_SUCCESS) {
      if(result == VK_ERROR_DEVICE_LOST) throw std::runtime_error("[PROGRAM EXIT] Device lost at vkDeviceWaitIdle()!");
      else if(result == VK_ERROR_OUT_OF_HOST_MEMORY) throw std::runtime_error("[PROGRAM EXIT] Ran out of host memory at vkDeviceWaitIdle()!");
      else if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) throw std::runtime_error("[PROGRAM EXIT] Ran out of device memory at vkDeviceWaitIdle()!");
      else {
        std::string a = "[PROGRAM EXIT] vkDeviceWaitIdle() failed with error ";
        std::string b = string_VkResult(result);
        std::string c = "!";
        std::string msg = a + b + c;
        throw std::runtime_error(msg);
      }
    }
  } // application::run() exit
} // program exit