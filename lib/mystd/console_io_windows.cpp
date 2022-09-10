bool is_console_io_initialized = false;
HANDLE STDOUT;

static void print_buffer(CStringView data, u64 size)
{
    if (!is_console_io_initialized)
    {
        STDOUT = GetStdHandle(STD_OUTPUT_HANDLE);
        if (STDOUT == INVALID_HANDLE_VALUE)
        {
            ExitProcess(2);
        }
        is_console_io_initialized = true;
    }
    WriteConsoleA(STDOUT, data, size, NULL, NULL);
}
