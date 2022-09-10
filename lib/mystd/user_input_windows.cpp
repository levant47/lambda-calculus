static void hold(int code)
{
    INPUT key_press;
    set_memory(0, sizeof(key_press), &key_press);
    key_press.type = INPUT_KEYBOARD;
    key_press.ki.wVk = code;
    SendInput(1, &key_press, sizeof(INPUT));
}

static void release(int code)
{
    INPUT key_release;
    set_memory(0, sizeof(key_release), &key_release);
    key_release.type = INPUT_KEYBOARD;
    key_release.ki.wVk = code;
    key_release.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &key_release, sizeof(INPUT));
}

static void press(int code)
{
    hold(code);
    Sleep(100);
    release(code);
}

static void click_on(u16 x, u16 y)
{
    u16 SCREEN_WIDTH = 1920;
    u16 SCREEN_HEIGHT = 1080;

    INPUT inputs[3];
    set_memory(0, sizeof(inputs), inputs);

    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dx = (u64)x * 65535 / SCREEN_WIDTH;
    inputs[0].mi.dy = (u64)y * 65535 / SCREEN_HEIGHT;
    inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    inputs[2].type = INPUT_MOUSE;
    inputs[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    auto send_input_result = SendInput(ARRAY_SIZE(inputs), inputs, sizeof(INPUT));
    assert_winapi(send_input_result == ARRAY_SIZE(inputs), "SendInput");
}

static bool char_requires_shift(char c)
{
    return c >= 'A' && c <= 'Z'
        || c == '!'
        || c == '#'
        || c == '$'
        || c == ':'
        || c == '>'
        || c == '&'
        || c == '?'
        || c == '%'
    ;
}

static u16 char_to_vk_code(char c)
{
    if (c >= 'A' && c <= 'Z' || c >= '1' && c <= '9')
    {
        return c; // upper case ASCII letters and digits correspond exactly to VK codes
    }
    if (c >= 'a' && c <= 'z')
    {
        return c - 0x20; // convert to upper case
    }
    if (c == '!' || c == '#' || c == '$')
    {
        return c + 0x10;
    }
    if (c == '\n')
    {
        return VK_RETURN;
    }
    if (c == ' ')
    {
        return VK_SPACE;
    }
    if (c == '\\')
    {
        return VK_OEM_5; // backslash
    }
    if (c == ':')
    {
        return VK_OEM_1;
    }
    if (c == '-')
    {
        return VK_OEM_MINUS;
    }
    if (c == '\b')
    {
        return VK_BACK;
    }
    if (c == '\t')
    {
        return VK_TAB;
    }
    if (c == '.' || c == '>')
    {
        return VK_OEM_PERIOD;
    }
    if (c == '%')
    {
        return '5';
    }
    if (c == '&')
    {
        return '7';
    }
    if (c == '/' || c == '?')
    {
        return VK_OEM_2;
    }
    auto error_message = String::allocate();
    error_message.push("char_to_vk_code: invalid argument: '");
    error_message.push(c);
    error_message.push("'");
    error_message.push('\0');
    assert_gui(false, error_message.data);
    return {};
}

static void type_in(CStringView text)
{
    for (u64 i = 0; text[i] != '\0'; i++)
    {
        if (char_requires_shift(text[i]))
        {
            INPUT shift_press;
            set_memory(0, sizeof(shift_press), &shift_press);
            shift_press.type = INPUT_KEYBOARD;
            shift_press.ki.wVk = VK_SHIFT;

            SendInput(1, &shift_press, sizeof(INPUT));
            Sleep(30);
        }

        INPUT key_press;
        set_memory(0, sizeof(key_press), &key_press);
        key_press.type = INPUT_KEYBOARD;
        key_press.ki.wVk = char_to_vk_code(text[i]);

        SendInput(1, &key_press, sizeof(INPUT));
        Sleep(30);

        INPUT key_release;
        set_memory(0, sizeof(key_release), &key_release);
        key_release.type = INPUT_KEYBOARD;
        key_release.ki.wVk = char_to_vk_code(text[i]);
        key_release.ki.dwFlags = KEYEVENTF_KEYUP;

        SendInput(1, &key_release, sizeof(INPUT));
        Sleep(30);

        if (char_requires_shift(text[i]))
        {
            INPUT shift_release;
            set_memory(0, sizeof(shift_release), &shift_release);
            shift_release.type = INPUT_KEYBOARD;
            shift_release.ki.wVk = VK_SHIFT;
            shift_release.ki.dwFlags = KEYEVENTF_KEYUP;

            SendInput(1, &shift_release, sizeof(INPUT));
            Sleep(30);
        }
    }
}

static void type_in(char c)
{
    auto string = String::allocate();
    string.push(c);
    string.push('\0');
    type_in(string.data);
    string.deallocate();
    Sleep(50);
}
