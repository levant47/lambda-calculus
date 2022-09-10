static void assert(bool condition, const char* user_message = nullptr)
{
    if (condition)
    {
        return;
    }
    if (IsDebuggerPresent()) { __debugbreak(); }
    if (user_message == nullptr)
    {
        print("Assertion failed\n");
    }
    else
    {
        print("Assertion failed: ", user_message, "\n");
    }
    ExitProcess(1);
}

static void assert_gui(bool condition, const char* user_message)
{
    if (condition)
    {
        return;
    }
    if (IsDebuggerPresent()) { __debugbreak(); }
    MessageBox(NULL, user_message, "Assertion failed", MB_OK);
    ExitProcess(1);
}

static void assert_winapi(bool condition, CStringView function_name)
{
    if (condition)
    {
        return;
    }

    CString error_message;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&error_message,
        0,
        NULL
    );
    if (IsDebuggerPresent()) { __debugbreak(); }
    print(function_name, " failed: ", error_message, "\n");
    ExitProcess(1);
}

static void assert_winapi_gui(bool condition, CString function_name = nullptr)
{
    if (condition)
    {
        return;
    }

    CString error_message;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&error_message,
        0,
        NULL
    );
    if (IsDebuggerPresent()) { __debugbreak(); }
    MessageBox(NULL, error_message, "WinAPI error", MB_OK);
    ExitProcess(1);
}
