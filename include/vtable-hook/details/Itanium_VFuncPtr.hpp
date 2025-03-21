#pragma once

#include <cstdint>
#include <bit>
#include <stdexcept>

namespace vthk::details{
namespace {

//https://itanium-cxx-abi.github.io/cxx-abi/abi.html#member-function-pointers
struct MemberFunctionPointer
{
  std::uintptr_t fnptr;
  std::ptrdiff_t this_adj;
};

template<typename T, typename R, typename... Args>
struct VFuncPtr
{
  std::size_t vtable_slot;
  std::ptrdiff_t this_adj;

  constexpr VFuncPtr(R(T::*vfuncptr)(Args...))
  {
    MemberFunctionPointer mfp = std::bit_cast<MemberFunctionPointer>(vfuncptr);

    bool is_virtual = 
    #if defined(__arm__) && !defined(__aarch64__)
      mfp.this_adj & 1;
    #else
      mfp.fnptr & 1;
    #endif

    if(!is_virtual)
      throw std::invalid_argument("Pointer to member function is not virtual");

    vtable_slot = mfp.fnptr;
    this_adj = mfp.this_adj;

    //normalize fields removing is_virtual flag
  #if defined(__arm__) && !defined(__aarch64__)
    vtable_slot /= sizeof(void*);
    this_adj >>= 1;
  #else
    vtable_slot &= ~1;
    vtable_slot /= sizeof(void*);
  #endif
  }
};

}} //namespace vthk::details::anonymous
