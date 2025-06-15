#pragma once
NAMESPACE_BEGIN(eldr::vk::wr)
template <typename T> class VkObject {
public:
  VkObject() = default;
  VkObject(VkObject&& o) noexcept
    : name_(std::move(o.name_)),
      object_(std::exchange(o.object_, VK_NULL_HANDLE))
  {
  }
  VkObject& operator=(VkObject&& o) noexcept
  {
    if (this != &o) {
      name_   = std::move(o.name_);
      object_ = std::exchange(o.object_, VK_NULL_HANDLE);
    }
    return *this;
  }

  /// Return the underlying vulkan object
  [[nodiscard]] EL_INLINE T& vk()
  {
    Assert(object_);
    return object_;
  }

  /// Return the underlying vulkan object (const)
  [[nodiscard]] EL_INLINE const T& vk() const
  {
    Assert(object_);
    return object_;
  }

  /// Return a pointer to the underlying vulkan object
  [[nodiscard]] T* vkp() { return &object_; }

  /// Return a pointer to the underlying vulkan object (const)
  [[nodiscard]] EL_INLINE const T* vkp() const
  {
    Assert(object_);
    return &object_;
  }

  /// Return the name of this object (const)
  [[nodiscard]] const std::string& name() const { return name_; }

protected:
  VkObject(std::string_view name) : name_(name) {}
  std::string name_{ "UNDEFINED" };

private:
  T object_{ VK_NULL_HANDLE };
};

template <typename T> class VkDeviceObject : public VkObject<T> {
  using Base = VkObject<T>;

public:
  VkDeviceObject()                                     = default;
  VkDeviceObject(VkDeviceObject&&) noexcept            = default;
  VkDeviceObject& operator=(VkDeviceObject&&) noexcept = default;

protected:
  VkDeviceObject(std::string_view name, const Device& device)
    : Base(name), device_(&device)
  {
  }

  EL_INLINE const Device& device() const
  {
    Assert(device_);
    return *device_;
  }
  EL_INLINE Device& device()
  {
    Assert(device_);
    return *device_;
  }

private:
  const Device* device_{ nullptr };
};

template <typename T> class VkAllocatedObject : public VkDeviceObject<T> {
  using Base = VkDeviceObject<T>;

public:
  VkAllocatedObject() = default;
  VkAllocatedObject(VkAllocatedObject&& o) noexcept
    : Base(o.name_, o.device()),
      allocation_(std::exchange(o.allocation_, VK_NULL_HANDLE)),
      alloc_info_(std::move(o.alloc_info_)), mem_flags_(std::move(o.mem_flags_))
  {
  }

  VkAllocatedObject& operator=(VkAllocatedObject&& o) noexcept
  {
    if (this != &o) {
      Base::operator=(o);
      allocation_ = std::exchange(o.allocation_, VK_NULL_HANDLE);
    }
    return *this;
  }

protected:
  VkAllocatedObject(std::string_view name, const Device& device)
    : Base(name, device)
  {
  }

  VmaAllocation         allocation_{ VK_NULL_HANDLE };
  VmaAllocationInfo     alloc_info_;
  VkMemoryPropertyFlags mem_flags_;
};

NAMESPACE_END(eldr::vk)
