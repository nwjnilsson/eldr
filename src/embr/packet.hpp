#include <eldr.hpp>
#include <embr/arraygeneric.hpp>

NAMESPACE_BEGIN(eldr::embr)
template <typename _Val, size_t _Size>
struct Packet : StaticArray<_Val, _Size, Packet<_Val, _Size>> {
  using Base      = StaticArray<_Val, _Size, Packet<_Val, _Size>>;
  using ArrayType = Packet;
  // using MaskType  = PacketMask<_Val, _Size>;

  /// Type alias for creating a similar-shaped array over a different type
  template <typename T> using ReplaceValue = Packet<T, _Size>;

  EL_ARRAY_IMPORT(Packet, Base)
};
NAMESPACE_END(eldr::embr)
