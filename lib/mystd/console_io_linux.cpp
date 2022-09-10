void print_buffer(CStringView data, u64 size)
{
    write(STDOUT, data, size);
}
