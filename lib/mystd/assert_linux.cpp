void assert(bool condition)
{
    if (!condition)
    {
        print("Assertion failed\n");
        exit(1);
    }
}

template <typename T>
void assert(bool condition, T message)
{
    if (!condition)
    {
        print("Assertion failed: ", message);
        exit(1);
    }
}

template <typename T1, typename T2>
void assert(bool condition, T1 message1, T2 message2)
{
    if (!condition)
    {
        print("Assertion failed: ", message1, message2, "\n");
        exit(1);
    }
}

template <typename T1, typename T2, typename T3>
void assert(bool condition, T1 message1, T2 message2, T3 message3)
{
    if (!condition)
    {
        print("Assertion failed: ", message1, message2, message3, "\n");
        exit(1);
    }
}

template <typename T1, typename T2, typename T3, typename T4>
void assert(bool condition, T1 message1, T2 message2, T3 message3, T4 message4)
{
    if (!condition)
    {
        print("Assertion failed: ", message1, message2, message3, message4, "\n");
        exit(1);
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
void assert(bool condition, T1 message1, T2 message2, T3 message3, T4 message4, T5 message5)
{
    if (!condition)
    {
        print("Assertion failed: ", message1, message2, message3, message4, message5);
        print("\n");
        exit(1);
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void assert(bool condition, T1 message1, T2 message2, T3 message3, T4 message4, T5 message5, T6 message6)
{
    if (!condition)
    {
        print("Assertion failed: ");
        print(message1, message2, message3, message4, message5, message6);
        print("\n");
        exit(1);
    }
}
