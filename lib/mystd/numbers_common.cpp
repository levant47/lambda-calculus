typedef float f32;
typedef double f64;

static u64 align_to(u64 alignment, u64 value)
{
    if (value % alignment == 0)
    {
        return value;
    }
    return (value / alignment + 1) * alignment;
}

static char digit_value_to_character(u64 value)
{
    if (value < 10) { return value + '0'; }
    if (value < 36) { return value - 10 + 'a'; }
    ExitProcess(3); // I don't even...
}

static u64 number_to_string(u64 value, char* buffer, u64 base = 10)
{
    u64 size = 0;
    do
    {
        buffer[size] = digit_value_to_character(value % base);
        value /= base;
        size++;
    }
    while (value != 0);

    for (u64 i = 0; i < size / 2; i++)
    {
        auto temp = buffer[i];
        buffer[i] = buffer[size - i - 1];
        buffer[size - i - 1] = temp;
    }

    return size;
}

static bool is_between(u64 start, u64 end, u64 value) { return value >= start && value < end; }
