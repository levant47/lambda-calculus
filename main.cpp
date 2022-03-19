#include "include.h"

const char* source =
"(\\ x z . x y z) (\\ x y z . z x)"
// "(\\ x y z . x y z) (\\ x y z . z x) (\\ z . z)"
;

int main()
{
    auto tokenization_result = tokenize(String::copy_from_c_string(source));
    if (!tokenization_result.success)
    {
        printf("Tokenization failed at character %lu\n", tokenization_result.failed_at_index);
        return 1;
    }
    printf("Tokenization succeeded\n");

    auto parsing_result = parse(tokenization_result.tokens);
    if (!parsing_result.has_data)
    {
        printf("Parsing failed\n");
        return 1;
    }
    printf("Parsing succeeded: ");
    parsing_result.value.print();
    printf("\n");

    auto reduced_result = reduce(parsing_result.value);
    reduced_result.print();
    printf("\n");
}

