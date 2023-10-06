#pragma once

#include <vulkan/vk_enum_string_helper.h>

#include <iostream>

#pragma diag_suppress 20
#define VK_CHECK(strfunction) \
{ \
  if (result != VK_SUCCESS) { \
    std::string color = "\n\033[1;31m"; \
    std::string colour = "\033[0m"; \
    std::string str = strfunction; \
    std::string a = color + str; \
    std::string b = "() returned "; \
    std::string c = string_VkResult(result); \
    std::string d = "!" + colour; \
    std::string msg = a + b + c + d ; \
    throw std::runtime_error(msg); \
  } \
}
#pragma diag_default 20

#define PRINT(x) std::cout << x << std::endl
#define PRINTVEC3(v) std::cout << v.x << " " << v.y << " " << v.z << " " << std::end
#define DEBUG(x) PRINT(x)

#define assert_abort(cond, str) assert(cond && str); std::abort();