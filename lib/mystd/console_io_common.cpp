static void print(u64 value)
{
    char message[20];
    auto number_length = number_to_string(value, message);
    print_buffer(message, number_length);
}

static void print(CStringView string)
{
    print_buffer(string, get_c_string_length(string));
}

static void print(char c)
{
    print_buffer(&c, 1);
}

template <typename T1, typename T2>
static void print(T1 arg1, T2 arg2)
{
    print(arg1);
    print(arg2);
}

template <typename T1, typename T2, typename T3>
static void print(T1 arg1, T2 arg2, T3 arg3)
{
    print(arg1);
    print(arg2);
    print(arg3);
}

template <typename T1, typename T2, typename T3, typename T4>
static void print(T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
    print(arg1);
    print(arg2);
    print(arg3);
    print(arg4);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
static void print(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
    print(arg1);
    print(arg2);
    print(arg3);
    print(arg4);
    print(arg5);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
static void print(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
{
    print(arg1);
    print(arg2);
    print(arg3);
    print(arg4);
    print(arg5);
    print(arg6);
}
