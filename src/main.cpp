#include "include.h"

const char* source =
"(\\ g y x . y x g) x (\\ a b x . a x b)"
;

extern "C" void _start()
{
    auto tokenization_result = tokenize(String::copy_from_c_string(source));
    if (!tokenization_result.success)
    {
        print("Tokenization failed at character ");
        print(tokenization_result.failed_at_index);
        print("\n");
        exit(1);
    }
    print("Tokenization succeeded\n");

    auto parsing_result = parse_terminal_expression(tokenization_result.tokens);
    if (!parsing_result.has_data)
    {
        print("Parsing failed\n");
        exit(1);
    }
    print("Parsing succeeded: ");
    parsing_result.value.print();
    print("\n");

    auto reduced_result = reduce(parsing_result.value);
    reduced_result.print();
    print("\n");
    exit(0);
}
