typedef char* CString;
typedef const char* CStringView;

static char to_lower(char c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; }

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

static bool c_string_equal_case_insensitive(CStringView left, CStringView right)
{
    u64 i = 0;
    while (true)
    {
        if (to_lower(left[i]) != to_lower(right[i]))
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

static bool c_string_starts_with_case_insensitive(CStringView target, CStringView source)
{
    u64 i = 0;
    while (target[i] != '\0')
    {
        if (to_lower(target[i]) != to_lower(source[i]))
        {
            return false;
        }
        i++;
    }
    return true;
}
