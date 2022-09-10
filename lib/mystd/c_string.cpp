typedef char* CString;
typedef const char* CStringView;

static u64 get_c_string_length(CStringView source)
{
    u64 i = 0;
    while (source[i] != '\0')
    {
        i++;
    }
    return i;
}

static bool c_string_equal(CStringView left, CStringView right)
{
    u64 i = 0;
    while (true)
    {
        if (left[i] != right[i])
        {
            return false;
        }
        if (left[i] == '\0')
        {
            return true;
        }
        i++;
    }
}

static bool starts_with(CString target, CString source)
{
    for (u64 i = 0; target[i] != '\0'; i++)
    {
        if (source[i] != target[i])
        {
            return false;
        }
    }
    return true;
}

static bool contains(CString target, CString source)
{
    auto target_length = get_c_string_length(target);
    auto source_length = get_c_string_length(source);
    if (target_length > source_length) { return false; }
    for (auto i = 0; i <= source_length - target_length; i++)
    {
        if (starts_with(target, source + i))
        {
            return true;
        }
    }
    return false;
}
