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
