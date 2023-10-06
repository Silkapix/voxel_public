#version 450

#include "../common/common.inl"

layout(location = 0) in vec4 out_color;
layout(location = 0) out vec4 swapchain_color;

void main() {
	swapchain_color = out_color;
}