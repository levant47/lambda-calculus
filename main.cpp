#include <mystd/include.h>

#include "node.cpp"
#include "parsing.cpp"

int main()
{
    auto parsing_result = parse_lambda_calculus(String::copy_from_c_string("\\ x . \\ y . x y z"));
    if (!parsing_result.is_successful)
    {
        printf("Parsing failed\n");
        return 1;
    }
    parsing_result.syntax_tree->print();
    printf("\n");
}

