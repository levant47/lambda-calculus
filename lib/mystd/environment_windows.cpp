static String get_environment_strings()
{
    auto result = String::allocate();
    auto environment = GetEnvironmentStringsA();
    for (u64 i = 0; i < 2 || environment[i - 2] != '\0' || environment[i - 1] != '\0'; i++) { result.push(environment[i]); }
    FreeEnvironmentStringsA(environment);
    return result;
}

static Option<String> get_environment_variable(CString environment_variable_name)
{
    // this size includes null-terminator
    u64 environment_variable_size = GetEnvironmentVariableA(environment_variable_name, nullptr, 0);
    if (environment_variable_size == 0) { return Option<String>::empty(); }
    auto environment_variable = String::allocate(environment_variable_size);
    // here the size of the string WITHOUT the null-terminator is returned, for details check the docs
    environment_variable.size = GetEnvironmentVariableA(
        environment_variable_name,
        environment_variable.data,
        environment_variable_size
    );
    assert(environment_variable.size + 1 == environment_variable_size);
    return Option<String>::construct(environment_variable);
}
