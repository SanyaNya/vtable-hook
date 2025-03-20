#pragma once

#include <cstdint>
#include <bit>
#include <stdexcept>

namespace vthk::details{
namespace {

//There is no official doc, but there is good article
//https://rants.vastheman.com/2021/09/21/msvc/
struct MemberFunctionPointer1
{
  std::uintptr_t fnptr;
};

struct MemberFunctionPointer2
{
  std::uintptr_t fnptr;
  int this_adj;
};

struct MemberFunctionPointer3
{
  std::uintptr_t fnptr;
  int this_adj;
  int vbptr_off;
};

struct MemberFunctionPointer4
{
  std::uintptr_t fnptr;
  int this_adj;
  int vbptr_off;
  int vb_adj;
};

template<typename >
constexpr bool always_false_v = false;

template<typename T, typename R, typename... Args>
struct VFuncPtr
{
  std::size_t vtable_slot;
  std::ptrdiff_t this_adj;

  VFuncPtr(R(T::*vfuncptr)(Args...))
  {
    std::uintptr_t fnptr;

    if constexpr(sizeof(vfuncptr) == sizeof(MemberFunctionPointer1))
    {
      fnptr = std::bit_cast<MemberFunctionPointer1>(vfuncptr).fnptr;
      this_adj = 0;
    }
    else if constexpr(sizeof(vfuncptr) == sizeof(MemberFunctionPointer2))
    {
      auto mfp = std::bit_cast<MemberFunctionPointer2>(vfuncptr);
      fnptr = mfp.fnptr;
      this_adj = mfp.this_adj;
    }
    else if constexpr(sizeof(vfuncptr) == sizeof(MemberFunctionPointer3))
    {
      auto mfp = std::bit_cast<MemberFunctionPointer3>(vfuncptr);
      fnptr = mfp.fnptr;
      this_adj = mfp.this_adj;
    }
    else if constexpr(sizeof(vfuncptr) == sizeof(MemberFunctionPointer4))
    {
      auto mfp = std::bit_cast<MemberFunctionPointer4>(vfuncptr);
      fnptr = mfp.fnptr;
      this_adj = mfp.this_adj;
    }
    else
    {
      static_assert(always_false_v<T>, "Unknown MemberFunctionPointer layout");
    }

    std::uint8_t* mem = reinterpret_cast<std::uint8_t*>(fnptr);

  #if defined(_M_IX86) || defined(_M_X64) && !defined(_M_ARM64EC)
    // on x86 code for thunk is always this:
    // MOV RAX,QWORD PTR [RCX]
    // JMP QWORD PTR [RAX+OFFSET_FROM_VTABLE]
    constexpr std::size_t i = sizeof(void*) == 8 ? 1 : 0;
    if(!((i == 1 ? mem[0] == 0x48 : true) && mem[i+0] == 0x8b && mem[i+1] == 0x01 && mem[i+2] == 0xff && (mem[i+3] == 0x60 || mem[i+3] == 0x20)))
      throw std::runtime_error("Unknown thunk code or function is not virtual");
    
    if(mem[i+3] == 0x60) vtable_slot = mem[i+4] / sizeof(void*);
    else vtable_slot = 0;
  #else
    #error "Arm architecture is not yet supported"
  #endif
  }
};

}} //namespace vthk::details::anonymous
