#pragma once
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

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
NAMESPACE_END(eldr::vk::wr)
