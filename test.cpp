#include "include.h"

Option<Expression> tokenize_and_parse(const char* source_c_string)
{
    auto source = String::copy_from_c_string(source_c_string);
    auto tokenization_result = tokenize(source);
    source.deallocate();
    if (!tokenization_result.success)
    {
        return Option<Expression>::empty();
    }
    return parse_expression(tokenization_result.tokens);
}

void test_parser_success(const char* source, const char* expected)
{
    auto maybe_expression = tokenize_and_parse(source);
    if (!maybe_expression.has_data)
    {
        printf("Test failed, original expression: %s, expected result: %s, actual result: parsing failed\n", source, expected);
        return;
    }

    auto expression_string = maybe_expression.value.to_string();
    if (expression_string != expected)
    {
        printf("Test failed, original expression: %s, expected result: %s, actual result: ", source, expected);
        expression_string.print();
        printf("\n");
        return;
    }
}

void test_parser_success(const char* source)
{
    test_parser_success(source, source);
}

int main()
{
    test_parser_success("a");
    test_parser_success("a b");
    test_parser_success("a b c");
    test_parser_success("a (b c)");
    test_parser_success("(a b) (c d) (e f)", "a b (c d) (e f)");
    test_parser_success("\\ x . x");
    test_parser_success("(\\ x . x)", "\\ x . x");
    test_parser_success("(\\ x . x) y");
    test_parser_success("(\\ x . x) (\\ x . x)");
    test_parser_success("\\ x . \\ y . x y z", "\\ x y . x y z");
    test_parser_success("\\ x y . x y z");
    test_parser_success("(\\ x y z w . x y z) a b c d");
    test_parser_success("(\\f x . f x)(\\f x . f x)", "(\\ f x . f x) (\\ f x . f x)");
}
