bool default_allocator_initialized = false;
HANDLE process_heap;

static byte* default_allocate(u64 size)
{
    if (!default_allocator_initialized)
    {
        process_heap = GetProcessHeap();
        assert_winapi(process_heap != nullptr, "GetProcessHeap");
        default_allocator_initialized = true;
    }
    auto result = (byte*)HeapAlloc(process_heap, 0, size);
    assert(result != nullptr, "default_allocate: failed to allocate memory");
    return result;
}

static void default_deallocate(void* address)
{
    assert(default_allocator_initialized, "default_deallocate: default allocator has not been initialized");

    auto free_result = HeapFree(process_heap, 0, address);
    assert(free_result != 0, "default_deallocate: failed to deallocate memory");
}

static byte* default_reallocate(void* old_address, u64 old_size, u64 new_size)
{
    assert(default_allocator_initialized, "default_reallocate: default allocator has not been initialized");

    auto result = (byte*)HeapReAlloc(process_heap, 0, old_address, new_size);
    assert(result != nullptr, "default_reallocate: failed to reallocate memory");
    return result;
}
