#include <RED4ext/RED4ext.hpp>
#include <RED4ext/Callback.hpp>
#include <type_traits>
using namespace RED4ext;
#ifdef RED4EXT_GAME_VERSION_25
static_assert(sizeof(CRTTISystem) == 0x308);
static_assert(__builtin_offsetof(CRTTISystem, rttiRegistratorMutex) == 0x228);
static_assert(sizeof(CBaseFunction) == 0xF0);
static_assert(sizeof(CGlobalFunction) == 0xF8);
static_assert(__builtin_offsetof(CGlobalFunction, regIndex) == 0xF0);
static_assert(sizeof(CClassFunction) == 0x100);
static_assert(__builtin_offsetof(CClassFunction, parent) == 0xF0);
static_assert(sizeof(CStackFrame) == 0xA8);
static_assert(!std::is_copy_constructible_v<CStackFrame>);
static_assert(!std::is_move_constructible_v<CStackFrame>);
static_assert(sizeof(Callback<void (*)()>) == 0x48);
#else
static_assert(sizeof(CBaseFunction) == 0xB0);
static_assert(sizeof(CGlobalFunction) == 0xB8);
static_assert(sizeof(CClassFunction) == 0xC0);
static_assert(sizeof(CStackFrame) == 0x68);
static_assert(sizeof(Callback<void (*)()>) == 0x28);
#endif
int main() {}
