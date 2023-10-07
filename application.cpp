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

    //* Buffer for scratch voxels
    std::unique_ptr<Buffer> scratch_voxel_buffer_pointers;
    scratch_voxel_buffer_pointers = std::make_unique<Buffer>(
                              renderer.get_sve_device(),
                              MAX_CHUNK_UPDATES_PER_DISPATCH * sizeof(VkDeviceAddress),
                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                            | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                            );

    //* Scratch buffers used when generating chunks
    //* Their contents are not persistent.
    std::unique_ptr<Buffer> scratch_voxel_buffers[MAX_CHUNK_UPDATES_PER_DISPATCH];
    for(int i{0}; i < MAX_CHUNK_UPDATES_PER_DISPATCH; i++) {
      scratch_voxel_buffers[i] = std::make_unique<Buffer>(
                            renderer.get_sve_device(),
                            WORLD_CHUNK_NVOX * sizeof(u32),
                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                          );
    }

    // Write addresses
    scratch_voxel_buffer_pointers->map_memory();
    for(int i{0}; i < MAX_CHUNK_UPDATES_PER_DISPATCH; i++) {
      VkDeviceAddress a = scratch_voxel_buffers[i]->get_buffer_device_address();
      scratch_voxel_buffer_pointers->write(&a, sizeof(VkDeviceAddress), sizeof(VkDeviceAddress)*i);
    }
    scratch_voxel_buffer_pointers->unmap_memory();

    std::unique_ptr<Buffer> voxel_buffer_pointers;
    voxel_buffer_pointers = std::make_unique<Buffer>(
                              renderer.get_sve_device(),
                              WORLD_NCHUNKS * sizeof(VkDeviceAddress),
                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                            | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                            );

    // Here chunks are stored
    std::unique_ptr<Buffer> voxel_buffers[WORLD_NCHUNKS];
    for(int i{0}; i < WORLD_NCHUNKS; i++) {
      voxel_buffers[i] = std::make_unique<Buffer>(
                            renderer.get_sve_device(),
                            WORLD_CHUNK_NVOX * sizeof(u32),
                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                          );
    }

    // Write addresses
    voxel_buffer_pointers->map_memory();
    for(int i{0}; i < MAX_CHUNK_UPDATES_PER_DISPATCH; i++) {
      VkDeviceAddress a = voxel_buffers[i]->get_buffer_device_address();
      voxel_buffer_pointers->write(&a, sizeof(VkDeviceAddress), sizeof(VkDeviceAddress)*i);
    }
    voxel_buffer_pointers->unmap_memory();

    std::unique_ptr<Buffer> face_buffer;
    face_buffer = std::make_unique<Buffer>(
                    renderer.get_sve_device(),
                    sizeof(i32vec4) * 6 * WORLD_CHUNK_NVOX * WORLD_NCHUNKS,
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                  );

    std::unique_ptr<Buffer> shader_info_buffer;
      shader_info_buffer = std::make_unique<Buffer>(
                          renderer.get_sve_device(),
                          sizeof(ShaderInfo), 
                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                        | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                        );
    shader_info_buffer->map_memory();

    std::unique_ptr<Buffer> chunk_updates_buffer;
      chunk_updates_buffer = std::make_unique<Buffer>(
                          renderer.get_sve_device(),
                          sizeof(ChunkUpdate), 
                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                        | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                        );
    chunk_updates_buffer->map_memory();

    // w is chunk index
    std::vector<i32vec4> chunk_update_requests_generation;
    std::vector<i32vec4> chunk_update_requests_meshing;

    //* At startup, no chunks exist.
    //* So all must be requested.
    for(int y{0}; y < RENDER_HEIGHT; y++) {
    for(int z{0}; z < RENDER_DISTANCE; z++) {
    for(int x{0}; x < RENDER_DISTANCE; x++) {
      i32vec3 chunk_pos{x, y, z};
      i32 i = chunk_index_from_coords(chunk_pos);
      i32vec4 pos = i32vec4{chunk_pos, i};

      chunk_update_requests_generation.push_back(pos);
      chunk_update_requests_meshing.push_back(pos);
    }
    }
    }

    PRINT("GENERATING CHUNKS");

    while(!chunk_update_requests_generation.empty()) {

    //* Gather the chunk updates for generation
    int updates{0};
    for(; updates < MAX_CHUNK_UPDATES_PER_DISPATCH; updates++) {
      // Any updates left?
      if(chunk_update_requests_generation.empty()) break;

      // get update
      chunk_updates_buffer->write(chunk_update_requests_generation.data(), sizeof(i32vec4), updates*sizeof(i32vec4));
      // remove from requests
      chunk_update_requests_generation.erase(chunk_update_requests_generation.begin());
    }
    shader_info_buffer->write(&updates, sizeof(u32), 0);

    TerrainComputePush gen_push {
      .chunk_updates = chunk_updates_buffer->get_buffer_device_address(),
      .scratch_voxel_buffer_pointers = scratch_voxel_buffer_pointers->get_buffer_device_address(),
      .voxel_buffer_pointers = voxel_buffer_pointers->get_buffer_device_address(),
      .face_buffer = face_buffer->get_buffer_device_address(),
      .shader_info = shader_info_buffer->get_buffer_device_address(),
    };

    VkCommandBuffer gen_command_buffer = renderer.get_sve_device().begin_single_time_command_buffer();
    
    // Each thread does 1 voxel
    PRINT("Chunk gen " << updates);
    terrain_gen_pipeline.dispatch(
      gen_command_buffer,
      CHUNK_GENERATION_WORKGROUP_NX * updates,
      CHUNK_GENERATION_WORKGROUP_NY * updates,
      CHUNK_GENERATION_WORKGROUP_NZ * updates,
      &gen_push
    );

    renderer
      .get_sve_device()
      .end_single_time_compute_command_buffer_and_queue_submit_then_queue_wait_idle(gen_command_buffer, 0);
    }
    //* end of while loop


    //***********             *************//
    //*********** CHUNK MESHING ***********//
    //************             *************//

    while(!chunk_update_requests_meshing.empty()) {

    //* Gather the chunk updates for meshing
    int updates{0};
    for(; updates < MAX_CHUNK_UPDATES_PER_DISPATCH; updates++) {
      // Any updates left?
      if(chunk_update_requests_meshing.empty()) break;

      // get update
      chunk_updates_buffer->write(chunk_update_requests_meshing.data(), sizeof(i32vec4), updates*sizeof(i32vec4));
      // remove from requests
      chunk_update_requests_meshing.erase(chunk_update_requests_meshing.begin());
    }
    shader_info_buffer->write(&updates, sizeof(u32), 0);

    TerrainComputePush meshing_push {
      .chunk_updates = chunk_updates_buffer->get_buffer_device_address(),
      .scratch_voxel_buffer_pointers = scratch_voxel_buffer_pointers->get_buffer_device_address(),
      .voxel_buffer_pointers = voxel_buffer_pointers->get_buffer_device_address(),
      .face_buffer = face_buffer->get_buffer_device_address(),
      .shader_info = shader_info_buffer->get_buffer_device_address(),
    };

    VkCommandBuffer meshing_command_buffer = 
      renderer
        .get_sve_device()
        .begin_single_time_command_buffer();

    // Compute shader for meshing
    // Each thread does 1 voxel
    PRINT("Chunk meshing " << updates);
    terrain_meshing_pipeline.dispatch(
      meshing_command_buffer,
      CHUNK_MESHING_WORKGROUP_NX * updates,
      CHUNK_MESHING_WORKGROUP_NY * updates,
      CHUNK_MESHING_WORKGROUP_NZ * updates,
      &meshing_push
    );
    
    renderer
      .get_sve_device()
      .end_single_time_compute_command_buffer_and_queue_submit_then_queue_wait_idle(meshing_command_buffer, 0);
    }
    //* end of while loop

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

    u32 fc = reinterpret_cast<ShaderInfo*>(shader_info_buffer->get_mapped_address())->face_count;

    DrawInfo draw_info{
      .push = BasicPush {
        .camera_matrices = camera_matrices_buffers[current_frame]->get_buffer_device_address(),
        .faces = face_buffer->get_buffer_device_address(),
        .shader_info = shader_info_buffer->get_buffer_device_address(),
        .model_matrix = default_entity.get_component<TransformComponent>().mat4(),
        .color = {1.0, 1.0, 1.0, 1.0},
      },
      .vertex_count = fc*6
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