#pragma once
#include <eldr/vulkan/vulkan.hpp>

namespace eldr::vk::wr {

class Semaphore {
public:
  Semaphore();
  Semaphore(const Device& device, VkSemaphoreCreateFlags flags = 0);
  Semaphore(Semaphore&&) noexcept;
  ~Semaphore();

  [[nodiscard]] VkSemaphore        vk() const;
  [[nodiscard]] const VkSemaphore* vkp() const;

private:
  class SemaphoreImpl;
  std::unique_ptr<SemaphoreImpl> d_;
};
} // namespace eldr::vk::wr
