#include <cstdint>
#include <RED4ext/Detail/Memory.hpp>
namespace RED4ext::Detail {
template<> struct AddressResolverOverride<uint32_t> : std::true_type {
    static uintptr_t Resolve(uint32_t hash);
};
}
#include <RED4ext/RED4ext.hpp>
#include <cassert>
#include <cstring>
static uintptr_t allocator[8]{};
static int destroyed = 0;
static void destroy(RED4ext::CStackFrame* frame) {
    assert(frame->unk98 == frame->unk68);
    ++destroyed;
}
uintptr_t RED4ext::Detail::AddressResolverOverride<uint32_t>::Resolve(uint32_t hash) {
    if (hash == RED4ext::Detail::AddressHashes::Memory_PoolScript_vtbl)
        return reinterpret_cast<uintptr_t>(allocator);
    if (hash == RED4ext::Detail::AddressHashes::CStackFrame_dtor)
        return reinterpret_cast<uintptr_t>(&destroy);
    assert(false && "unexpected engine dependency");
    return 0;
}
int main() {
    char code[] = {0};
    {
        RED4ext::CStackFrame frame(nullptr, code);
        assert(frame.code == code && frame.context == nullptr && frame.func == nullptr);
        assert(frame.isRoot && frame.unk98 == frame.unk68 && frame.unkA0 == 0);
        for (unsigned i = 0; i < 0x20; ++i) assert(frame.unk68[i] == 0);
        uint32_t empty; std::memcpy(&empty, frame.unk68 + 0x20, sizeof(empty));
        assert(empty == 0xFFFFFFFF);
        uintptr_t pool; std::memcpy(&pool, frame.unk68 + 0x28, sizeof(pool));
        assert(pool == reinterpret_cast<uintptr_t>(allocator));
        assert(destroyed == 0);
    }
    assert(destroyed == 1);
}
