#pragma once
NAMESPACE_BEGIN(eldr::vk::wr)
template <typename T> class VkObject {
public:
  VkObject()                               = default;
  VkObject(VkObject&&) noexcept            = default;
  VkObject& operator=(VkObject&&) noexcept = default;

  /// Return the underlying vulkan object
  [[nodiscard]] T vk();

  /// Return the underlying vulkan object (const)
  [[nodiscard]] const T vk() const;

  /// Return a pointer to the underlying vulkan object
  [[nodiscard]] T* vkp();

  /// Return a pointer to the underlying vulkan object (const)
  [[nodiscard]] const T* vkp() const;

  /// Return the name of this object
  [[nodiscard]] const std::string& name() const { return name_; }

protected:
  VkObject(std::string_view name) : name_(name) {}

private:
  std::string name_{ "UNDEFINED" };
  T           object_{ VK_NULL_HANDLE };
};

template <typename T> class VkDeviceObject : public VkObject<T> {
public:
  VkDeviceObject()                                     = default;
  VkDeviceObject(VkDeviceObject&&) noexcept            = default;
  VkDeviceObject& operator=(VkDeviceObject&&) noexcept = default;

protected:
  VkDeviceObject(std::string_view name, const Device* device)
    : VkObject(name), device_(device)
  {
  }
  const Device* device_{ nullptr };
};
NAMESPACE_END(eldr::vk)
