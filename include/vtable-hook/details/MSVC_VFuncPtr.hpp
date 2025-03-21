#pragma once

#include <cstdint>
#include <cstring>
#include <bit>
#include <stdexcept>

//Force full info in pointer to member function representation
//It is recomended to include this header in separate translation unit and after all other includes
#pragma pointers_to_members(full_generality, virtual_inheritance)

namespace vthk::details{
namespace {

//There is no official doc, but there is good article
//https://rants.vastheman.com/2021/09/21/msvc/
struct MemberFunctionPointer
{
  std::uintptr_t fnptr;
  int adj;    // this pointer displacement in bytes
  int vadj;   // offset to vptr or undefined
  int vindex; // byte offset to base class offset in vtable or zero
};

template<typename T, typename R, typename... Args>
struct VFuncPtr
{
  std::size_t vtable_slot;
  std::ptrdiff_t this_adj;

  VFuncPtr(const T* instance, R(T::*vfuncptr)(Args...))
  {
    auto mfp = std::bit_cast<MemberFunctionPointer>(vfuncptr);

    std::uintptr_t fnptr = mfp.fnptr;

    this_adj = 0;
    if(mfp.vindex != 0)
    {
      std::uintptr_t vptr_addr = reinterpret_cast<std::uintptr_t>(instance) + mfp.vadj;
      std::uintptr_t vtable_addr;
      std::memcpy(&vtable_addr, reinterpret_cast<const void*>(vptr_addr), sizeof(vtable_addr));

      int vbase_offset;
      std::memcpy(&vbase_offset, reinterpret_cast<const void*>(vtable_addr + mfp.vindex), sizeof(vbase_offset));

      this_adj += mfp.vadj + vbase_offset;
    }
    this_adj += mfp.adj;

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
