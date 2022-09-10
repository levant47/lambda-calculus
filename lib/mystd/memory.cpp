static void copy_memory(const void* passed_source, u64 size, void* passed_destination)
{
    auto source = (byte*)passed_source;
    auto destination = (byte*)passed_destination;
    for (u64 i = 0; i < size; i++)
    {
        destination[i] = source[i];
    }
}

static void reverse_memory(void* passed_source, u64 size)
{
    auto source = (byte*)passed_source;
    for (u64 i = 0; i < size / 2; i++)
    {
        byte temp = source[i];
        source[i] = source[size - i - 1];
        source[size - i - 1] = temp;
    }
}

static void set_memory(byte value, u64 size, void* passed_destination)
{
    auto destination = (byte*)passed_destination;
    for (u64 i = 0; i < size; i++)
    {
        destination[i] = value;
    }
}
