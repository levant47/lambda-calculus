bool default_allocator_initialized = false;
byte* current_break = nullptr;
byte* current_allocated = nullptr;
byte* previous_allocated;
const u64 ALLOCATION_BLOCK_SIZE = 4096;

// TODO: do the linked list bookeping thing, malloc-style

byte* default_allocate(u64 size)
{
    assert(size != 0, "default_allocate: size was 0");

    if (!default_allocator_initialized)
    {
        current_break = (byte*)brk(nullptr);
        current_allocated = current_break;
        default_allocator_initialized = true;
    }

    if (current_allocated + size > current_break)
    { // allocate more memory if necessary
        auto amount_to_allocate = align_to(ALLOCATION_BLOCK_SIZE, size);
        auto new_break = current_break + amount_to_allocate;
        auto resulting_break = (byte*)brk(new_break);
        assert(resulting_break == new_break, "default_allocate: failed to allocate memory");
        current_break = resulting_break;
    }

    previous_allocated = current_allocated;
    current_allocated += size;

    return previous_allocated;
}

byte* default_reallocate(void* passed_address, u64 old_size, u64 size)
{
    assert(default_allocator_initialized, "default_reallocate: default allocator was not initialized");

    auto address = (byte*)passed_address;
    if (address == previous_allocated)
    {
        if (previous_allocated + size > current_break)
        {
            default_allocate(size - (current_allocated - previous_allocated));
        }
        current_allocated = previous_allocated + size;
        return previous_allocated;
    }
    else
    {
        auto new_address = default_allocate(size);
        for (u64 i = 0; i < old_size; i++)
        {
            new_address[i] = address[i];
        }
        return new_address;
    }
}

void default_deallocate(void* address)
{
    assert(default_allocator_initialized, "default_deallocate: default allocator was not initialized");

    if (address == previous_allocated)
    {
        current_allocated = previous_allocated;
    }
}
