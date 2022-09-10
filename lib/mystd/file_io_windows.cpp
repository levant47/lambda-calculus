static Option<u64> get_file_size(CStringView path)
{
    auto target_file_handle = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (target_file_handle == INVALID_HANDLE_VALUE)
    {
        return Option<u64>::empty();
    }
    auto target_file_size = GetFileSize(target_file_handle, NULL);
    CloseHandle(target_file_handle);
    return Option<u64>::construct(target_file_size);
}

static Option<String> read_whole_file(CStringView path)
{
    auto target_file_handle = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (target_file_handle == INVALID_HANDLE_VALUE)
    {
        return Option<String>::empty();
    }
    auto target_file_size = GetFileSize(target_file_handle, NULL);
    auto contents = String::allocate(target_file_size);
    ReadFile(target_file_handle, contents.data, target_file_size, NULL, NULL);
    contents.size = target_file_size;
    CloseHandle(target_file_handle);
    return Option<String>::construct(contents);
}

static bool write_whole_file(String data, CStringView path)
{
    auto target_file_handle = CreateFileA(
        path,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (target_file_handle == INVALID_HANDLE_VALUE) { return false; }
    u64 bytes_written;
    auto write_success = WriteFile(
        target_file_handle,
        data.data,
        data.size,
        &bytes_written,
        nullptr
    );
    auto close_success = CloseHandle(target_file_handle);
    return write_success && close_success && bytes_written == data.size;
}

// argument is fully qualified name of the directory being deleted, without
// trailing backslash and double null-terminated;
// returns true if successful and false otherwise
static bool delete_directory(CStringView path)
{ // https://stackoverflow.com/a/218636
    // TODO: explain
    SHFILEOPSTRUCT operation;
    operation.hwnd = nullptr;
    operation.wFunc = FO_DELETE;
    operation.pFrom = path;
    operation.pTo = "";
    operation.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    operation.fAnyOperationsAborted = false;
    operation.hNameMappings = nullptr;
    operation.lpszProgressTitle = "";
    auto error = SHFileOperation(&operation);
    return error == 0;
}

static bool delete_file(CStringView path) { return DeleteFileA(path); }

// both arguments have to be double null-terminated
static bool copy_directory(CString from, CString to)
{ // https://stackoverflow.com/a/4725137
    // TODO: explain
    SHFILEOPSTRUCT operation;
    operation.hwnd = nullptr;
    operation.wFunc = FO_COPY;
    operation.fFlags = FOF_SILENT;
    operation.pFrom = from;
    operation.pTo = to;
    operation.fAnyOperationsAborted = false;
    operation.hNameMappings = nullptr;
    operation.lpszProgressTitle = "";
    auto error = SHFileOperation(&operation);
    return error == 0;
}

static bool directory_exists(CStringView path)
{
    auto dwAttrib = GetFileAttributes(path);
    return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

static bool file_exists(CStringView path)
{
    auto dwAttrib = GetFileAttributes(path);
    return dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
