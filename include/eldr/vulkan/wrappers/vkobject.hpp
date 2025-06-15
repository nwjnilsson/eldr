#pragma once
#include <eldr/core/logger.hpp>
#include <utility>
NAMESPACE_BEGIN(eldr::vk::wr)

#define EL_VK_IMPORT_DEFAULTS(Name)                                            \
  Name();                                                                      \
  Name(Name&&) noexcept;                                                       \
  Name& operator=(Name&&);                                                     \
  ~Name();

#define EL_VK_IMPL_DEFAULTS(Name)                                              \
  Name::Name()                  = default;                                     \
  Name::Name(Name&&) noexcept   = default;                                     \
  Name& Name::operator=(Name&&) = default;

#define EL_VK_IMPL_DESTRUCTOR(Name)                                            \
  Name::~Name()                                                                \
  {                                                                            \
    if (vk()) {                                                                \
      vkDestroy##Name(device().logical(), object_, nullptr);                   \
    }                                                                          \
  }

template <typename T> class VkObject {
public:
  VkObject() = default;
  VkObject(VkObject&& o) noexcept
    : name_(std::move(o.name_)),
      object_(std::exchange(o.object_, VK_NULL_HANDLE))
  {
  }
  VkObject& operator=(VkObject&& o)
  {
    if (this != &o) {
      name_   = std::move(o.name_);
      object_ = std::exchange(o.object_, VK_NULL_HANDLE);
    }
    return *this;
  }

  /// Return the underlying vulkan object
  [[nodiscard]] const T& vk() const { return object_; }

  /// Return the name of this object (const)
  [[nodiscard]] const std::string& name() const { return name_; }

protected:
  VkObject(std::string_view name) : name_(name) {}
  std::string name_{ "UNDEFINED" };
  T           object_{ VK_NULL_HANDLE };
};

template <typename T> class VkDeviceObject : public VkObject<T> {
  using Base = VkObject<T>;

public:
  VkDeviceObject() = default;
  VkDeviceObject(VkDeviceObject&& o) noexcept : Base(std::move(o))
  {
    if (device_) {
      Assert(device_ == o.device_);
    }
    device_ = std::exchange(o.device_, nullptr);
  }
  VkDeviceObject& operator=(VkDeviceObject&&) = default;

  const Device& device() const
  {
    Assert(device_);
    return *device_;
  }

protected:
  VkDeviceObject(std::string_view name, const Device& device)
    : Base(name), device_(&device)
  {
  }

private:
  const Device* device_{ nullptr };
};

template <typename T> class VkAllocatedObject : public VkDeviceObject<T> {
  using Base = VkDeviceObject<T>;

public:
  VkAllocatedObject() = default;
  VkAllocatedObject(VkAllocatedObject&& o) noexcept
    : Base(std::move(o)),
      allocation_(std::exchange(o.allocation_, VK_NULL_HANDLE)),
      alloc_info_(std::move(o.alloc_info_)), mem_flags_(std::move(o.mem_flags_))
  {
  }

  VkAllocatedObject& operator=(VkAllocatedObject&& o)
  {
    if (this != &o) {
      Base::operator=(std::move(o));
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
