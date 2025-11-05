#ifndef _COMMON_H_
#define _COMMON_H_

//STL
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <span>
#include <memory>
#include <functional>
#include <concepts>
#include <format>
#include <chrono>
#include <numeric>
#include <numbers>
#include <random>
#include <string_view>
#include <type_traits>
#include <mutex>
#include <stdexcept>
#include <future>

#ifdef NDEBUG
	#include <Python.h>
#endif

//GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Eigen
#include "Eigen/Dense"

//stb_image
#include <stb_image/stb_image.h>

//magic_enum
#include <magic_enum/magic_enum.hpp>

//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


//Vulkan
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 
#define NOMINMAX
#endif
#include <vulkan/vulkan.h>
#include <vulkan/utility/vk_format_utils.h>

//Marco
#define DestroyHandleBy(Func) if (handle) { Func(VulkanBase::Base().Device(), handle, nullptr); handle = VK_NULL_HANDLE; }
#define MoveHandle handle = other.handle; other.handle = VK_NULL_HANDLE;
#define DefineHandleTypeOperator operator decltype(handle)() const { return handle; }
#define DefineAddressFunction const decltype(handle)* Address() const { return &handle; }

#define ExecuteOnce(...) { static bool executed = false; if (executed) return __VA_ARGS__; executed = true; }

#define XS_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*(_ARR)))) 

#define M_INVALID_ID -1

#endif // !_COMMON_H_

