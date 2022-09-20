#include "include.h"

struct CliArguments
{
    CStringView source_file_path;
};

Result<CliArguments, String> parse_cli_arguments(CStringView cli_arguments_string)
{
    u64 index = 0;
    auto cli_arguments_string_length = get_c_string_length(cli_arguments_string);

    // skip the first word, which is the program name
    while (true)
    {
        if (index == cli_arguments_string_length)
        {
            return Result<CliArguments, String>::fail(String::copy_from_c_string("Missing source file path"));
        }
        if (cli_arguments_string[index] == ' ')
        {
            while (cli_arguments_string[index] == ' ') { index++; } // skip spaces as there can be multiple
            CliArguments result;
            result.source_file_path = cli_arguments_string + index;
            return Result<CliArguments, String>::success(result);
        }
        index++;
    }
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

    auto maybe_source = read_whole_file(cli_arguments.source_file_path);
    if (!maybe_source.has_data)
    {
        auto error = String::allocate();
        error.push("File '");
        error.push(cli_arguments.source_file_path);
        error.push("' not found\n");
        print(error);
        return 1;
    }
    auto source = maybe_source.value;

    auto tokenization_result = tokenize(source);
    if (!tokenization_result.success)
    {
        print("Tokenization failed at character ", tokenization_result.failed_at_index, '\n');
        return 1;
    }

    auto parsing_result = parse_statements(tokenization_result.tokens);
    if (!parsing_result.success)
    {
        print("Parsing failed: ", parsing_result.error, "\n");
        return 1;
    }

    auto interpretation_result = interpret(parsing_result.statements);
    if (!interpretation_result.success)
    {
        print("Interpretation failed: ", interpretation_result.error, "\n");
        return 1;
    }
    print(interpretation_result.expression);

    return 0;
}
