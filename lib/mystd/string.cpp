static char to_lower(char c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; }

struct StringView
{
    u64 size;
    const char* data;

    static StringView construct(u64 size, CStringView data)
    {
        StringView result;
        result.size = size;
        result.data = data;
        return result;
    }

    static StringView from_c_string(CStringView source)
    {
        StringView result;
        result.size = get_c_string_length(source);
        result.data = source;
        return result;
    }
};

struct String
{
    u64 capacity;
    u64 size;
    char* data;

    static const u64 DEFAULT_CAPACITY = 16;

    static String allocate(u64 capacity = DEFAULT_CAPACITY)
    {
        String result;
        result.capacity = capacity;
        result.size = 0;
        result.data = (char*)default_allocate(sizeof(char) * result.capacity);
        return result;
    }

    static String copy_from_c_string(const char* source)
    {
        auto source_length = get_c_string_length(source);
        auto result = allocate(source_length);
        for (u64 i = 0; i < source_length; i++)
        {
            result.push(source[i]);
        }
        return result;
    }

    static String from_number(u64 value, u64 base = 10)
    {
        auto result = allocate(64);
        result.size = number_to_string(value, result.data, base);
        return result;
    }

    String copy()
    {
        String result;
        result.capacity = capacity;
        result.size = size;
        result.data = (char*)default_allocate(sizeof(char) * capacity);
        for (u64 i = 0; i < size; i++)
        {
            result.data[i] = data[i];
        }
        return result;
    }

    void deallocate()
    {
        assert(capacity != 0);
        default_deallocate(data);
        capacity = 0;
    }

    void push(char c)
    {
        if (size == capacity)
        {
            assert(capacity != 0);
            auto new_capacity = capacity * 2;
            data = (char*)default_reallocate(data, capacity * sizeof(char), new_capacity * sizeof(char));
            capacity *= 2;
        }
        data[size] = c;
        size++;
    }

    void push(const char* c_string)
    {
        for (u64 i = 0; c_string[i] != '\0'; i++)
        {
            push(c_string[i]);
        }
    }

    void push(String other_string)
    {
        for (u64 i = 0; i < other_string.size; i++)
        {
            push(other_string.data[i]);
        }
    }

    void push(u64 number, u64 base = 10)
    {
        char buffer[20];
        auto digits_count = number_to_string(number, buffer, base);
        for (u64 i = 0; i < digits_count; i++)
        {
            push(buffer[i]);
        }
    }

    void push(StringView source)
    {
        for (u64 i = 0; i < source.size; i++)
        {
            push(source.data[i]);
        }
    }

    void pop(u64 count = 1)
    {
        assert(size >= count);
        size -= count;
    }

    void reverse() { reverse_memory(data, size); }

    void make_c_string_compatible() { push('\0'); pop(); }

    void clear() { size = 0; }

    StringView to_string_view() { return StringView::construct(size, data); }

    void reserve_at_least(u64 new_capacity)
    {
        data = (char*)default_reallocate(data, capacity * sizeof(char), new_capacity * sizeof(char));
        capacity = new_capacity;
    }
};

static bool operator==(String left, String right)
{
    if (left.size != right.size)
    {
        return false;
    }
    for (u64 i = 0; i < left.size; i++)
    {
        if (left.data[i] != right.data[i])
        {
            return false;
        }
    }
    return true;
}

static bool operator==(String left, const char* right)
{
    u64 i = 0;
    while (i != left.size && right[i] != '\0')
    {
        if (left.data[i] != right[i])
        {
            return false;
        }
        i++;
    }
    return i == left.size && right[i] == '\0';
}

static bool operator!=(String left, const char* right)
{
    return !(left == right);
}

static void print(String string)
{
    print_buffer(string.data, string.size);
}
