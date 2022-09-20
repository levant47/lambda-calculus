#include "lib/mystd/include_windows.h"

enum Mode
{
    ModeRunTests,
    ModeBuildExecutable,
};

struct CliArguments
{
    Mode mode;
};

Result<CliArguments, String> parse_cli_arguments(CStringView cli_arguments_string)
{
    u64 index = 0;
    auto cli_arguments_string_length = get_c_string_length(cli_arguments_string);

    // skip the first word, which is the program name
    while (true)
    {
        if (index == cli_arguments_string_length)
        { // default behavior when no mode is provided on the CLI is to run tests
            CliArguments result;
            result.mode = ModeRunTests;
            return Result<CliArguments, String>::success(result);
        }
        if (cli_arguments_string[index] == ' ')
        {
            while (cli_arguments_string[index] == ' ') { index++; } // skip spaces as there can be multiple
            if (index == cli_arguments_string_length) { continue; } // go to the default mode branch
            break;
        }
        index++;
    }

    // parse the mode
    Mode mode;
    if (c_string_starts_with_case_insensitive("tests", cli_arguments_string + index))
    {
        mode = ModeRunTests;
    }
    else if (c_string_starts_with_case_insensitive("build", cli_arguments_string + index))
    {
        mode = ModeBuildExecutable;
    }
    else
    {
        auto error = String::allocate();
        error.push("Invalid mode parameter: '");
        error.push(cli_arguments_string + index);
        error.push('\'');
        return Result<CliArguments, String>::fail(error);
    }

    CliArguments result;
    result.mode = mode;
    return Result<CliArguments, String>::success(result);
}

int run_tests()
{
    // have to do this because otherwise cl will not be able to compile
    if (!directory_exists("temp"))
    {
        auto success = CreateDirectoryA("temp", nullptr);
        if (!success)
        {
            print("Failed to create temp directory\n");
            return 1;
        }
    }

    auto command = String::allocate();
    command.push("cl"); // MSVC compiler

    // compiler options:
    command.push(" /nologo");
    command.push(" /Fo.\\temp\\"); // directory for temporary build files
    command.push(" /Fe.\\temp\\lci_tests.exe"); // output path
    command.push(" /Gy"); // collapse identical functions, https://stackoverflow.com/a/629978
    command.push(" /GS-"); // disable buffer security checks
    command.push(" /Zl"); // ignore CRT when compiling object files
    command.push(" /I."); // use current directory for includes
    command.push(" test\\test.cpp");

    // linker options:
    command.push(" /link");
    command.push(" /NODEFAULTLIB"); // ignore CRT when linking
    command.push(" /ENTRY:main");
    command.push(" /SUBSYSTEM:console");
    command.push(" kernel32.lib");

    command.make_c_string_compatible();

    print(command, "\n");
    auto exit_code = (int)complete(start_cmd(command.data));
    command.deallocate();
    if (exit_code != 0) { return exit_code; }

    print("Running tests...\n");
    auto tests_command = "temp\\lci_tests";
    print(tests_command, "\n");
    return (int)complete(start_cmd(tests_command));
}

int build_executable()
{
    // have to do this because otherwise cl will not be able to compile
    if (!directory_exists("build"))
    {
        auto success = CreateDirectoryA("build", nullptr);
        if (!success)
        {
            print("Failed to create build directory\n");
            return 1;
        }
    }

    auto command = String::allocate();
    command.push("cl"); // MSVC compiler

    // compiler options:
    command.push(" /nologo");
    command.push(" /Fo.\\temp\\"); // directory for temporary build files
    command.push(" /Fe.\\build\\lci.exe"); // output path
    command.push(" /Gy"); // collapse identical functions, https://stackoverflow.com/a/629978
    command.push(" /GS-"); // disable buffer security checks
    command.push(" /Zl"); // ignore CRT when compiling object files
    command.push(" /I."); // use current directory for includes
    command.push(" src\\main.cpp");

    // linker options:
    command.push(" /link");
    command.push(" /NODEFAULTLIB"); // ignore CRT when linking
    command.push(" /ENTRY:main");
    command.push(" /SUBSYSTEM:console");
    command.push(" kernel32.lib");

    command.make_c_string_compatible();

    print(command, "\n");
    auto exit_code = (int)complete(start_cmd(command.data));
    command.deallocate();
    if (exit_code != 0) { return exit_code; }

    auto success = copy_directory("data\\samples\0", "build\0");
    if (!success)
    {
        print("Failed to copy samples to the build directory\n");
        return 1;
    }

    return 0;
}

int main()
{
    auto cli_arguments_parsing_result = parse_cli_arguments(GetCommandLineA());
    if (!cli_arguments_parsing_result.is_success)
    {
        print(cli_arguments_parsing_result.error, "\n");
        return 1;
    }
    auto cli_arguments = cli_arguments_parsing_result.value;

    switch (cli_arguments.mode)
    {
        case ModeRunTests: return run_tests();
        // I don't know why, but just returning from main after calling build_executable doesn't actually exit the
        // program, it just stays there hanging, so we have to force it to quit with ExitProcess
        case ModeBuildExecutable: ExitProcess(build_executable());
        default: assert(false);
    }
}
