template <typename T>
static T* copy_to_heap(T value)
{
    auto value_on_heap = (T*)default_allocate(sizeof(T));
    copy_memory(&value, sizeof(T), value_on_heap);
    return value_on_heap;
}
