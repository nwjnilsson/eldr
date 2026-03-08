#include "arraygeneric.hpp"
#include "packetintrin.hpp"

#include <eldr.hpp>

NAMESPACE_BEGIN(eldr::embr)
template <typename _Val, size_t _Sz>
struct Packet : StaticArray<_Val, _Sz, false, Packet<_Val, _Sz>> {
  using Base = StaticArray<_Val, _Sz, false, Packet<_Val, _Sz>>;
  using ArrayType = Packet;
  // using MaskType  = PacketMask<_Val, _Sz>;

  /// Type alias for creating a similar-shaped array over a different type
  template <typename T> using ReplaceValue = Packet<T, _Sz>;

  EL_ARRAY_IMPORT(Packet, Base)
};
NAMESPACE_END(eldr::embr)
