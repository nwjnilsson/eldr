#pragma once
#include <eldr/vulkan/common.hpp>

namespace eldr::vk::wr {

class Semaphore {
public:
  Semaphore() = default;
  Semaphore(const Device& device, VkSemaphoreCreateFlags flags = 0);

  [[nodiscard]] VkSemaphore        vk() const;
  [[nodiscard]] const VkSemaphore* vkp() const;

private:
  class SemaphoreImpl;
  std::shared_ptr<SemaphoreImpl> s_data_;
};
} // namespace eldr::vk::wr
