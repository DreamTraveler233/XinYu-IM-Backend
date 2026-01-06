#include "core/base/memory_pool.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {

#define CHECK(cond)                                                                                 \
    do {                                                                                             \
        if (!(cond)) {                                                                               \
            std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << " CHECK(" #cond ")\n"; \
            std::abort();                                                                            \
        }                                                                                            \
    } while (0)

static void cleanup_set_flag(void *data)
{
    auto *flag = static_cast<int *>(data);
    *flag = 1;
}

static void test_small_alloc_alignment()
{
    IM::NgxMemPool pool(256);

    void *p1 = pool.palloc(1);
    CHECK(p1 != nullptr);
    CHECK((reinterpret_cast<std::uintptr_t>(p1) % IM::NGX_ALIGNMENT) == 0);

    void *p2 = pool.palloc(15);
    CHECK(p2 != nullptr);
    CHECK((reinterpret_cast<std::uintptr_t>(p2) % IM::NGX_ALIGNMENT) == 0);

    void *p3 = pool.pnalloc(7);
    CHECK(p3 != nullptr);
}

static void test_pcalloc_zeroed()
{
    IM::NgxMemPool pool;

    constexpr std::size_t n = 64;
    auto *p = static_cast<unsigned char *>(pool.pcalloc(n));
    CHECK(p != nullptr);

    for (std::size_t i = 0; i < n; ++i) {
        CHECK(p[i] == 0);
    }
}

static void test_large_alloc_and_free()
{
    IM::NgxMemPool pool;

    const std::size_t largeSize = static_cast<std::size_t>(IM::NGX_MAX_ALLOC_FROM_POOL) + 1024;
    auto *buf = static_cast<unsigned char *>(pool.palloc(largeSize));
    CHECK(buf != nullptr);

    buf[0] = 0xAB;
    buf[largeSize - 1] = 0xCD;

    pool.pfree(buf);
    // double pfree should be a no-op
    pool.pfree(buf);

    // reset after large alloc should also be safe
    auto *buf2 = static_cast<unsigned char *>(pool.palloc(largeSize));
    CHECK(buf2 != nullptr);
    pool.resetPool();

    auto *buf3 = static_cast<unsigned char *>(pool.palloc(largeSize));
    CHECK(buf3 != nullptr);
}

static void test_block_growth_small_allocs()
{
    IM::NgxMemPool pool(IM::NGX_MIN_POOL_SIZE);

    // Allocate enough small chunks to force additional blocks.
    for (int i = 0; i < 2000; ++i) {
        void *p = pool.palloc(128);
        CHECK(p != nullptr);
    }
}

static void test_cleanup_called_on_destruct()
{
    int flag = 0;
    {
        IM::NgxMemPool pool;
        IM::NgxPoolCleanup_t *c = pool.cleanupAdd(0);
        CHECK(c != nullptr);
        c->handler = cleanup_set_flag;
        c->data = &flag;
        CHECK(flag == 0);
    }
    CHECK(flag == 1);
}

static void test_move_semantics()
{
    IM::NgxMemPool pool;
    void *p = pool.palloc(32);
    CHECK(p != nullptr);

    IM::NgxMemPool moved(std::move(pool));
    void *p2 = moved.palloc(32);
    CHECK(p2 != nullptr);

    // moved-from object should be safe to use (returns nullptr) and safe to destruct.
    CHECK(pool.palloc(16) == nullptr);
}

} // namespace

int main()
{
    test_small_alloc_alignment();
    test_pcalloc_zeroed();
    test_large_alloc_and_free();
    test_block_growth_small_allocs();
    test_cleanup_called_on_destruct();
    test_move_semantics();

    std::cout << "[OK] test_memory_pool\n";
    return 0;
}
