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
    return parse(tokenization_result.tokens);
}

Option<Expression> tokenize_parse_and_reduce(const char* source_c_string)
{
    auto maybe_expression = tokenize_and_parse(source_c_string);
    if (!maybe_expression.has_data) { return Option<Expression>::empty(); }

    return Option<Expression>::construct(reduce(maybe_expression.value));
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

void test_parser_fail(const char* source)
{
    auto maybe_expression = tokenize_and_parse(source);
    if (maybe_expression.has_data)
    {
        auto expression_string = maybe_expression.value.to_string();
        printf("Test failed, original expression: %s, expected result: parsing failed, actual result: ", source);
        expression_string.print();
        printf("\n");
        expression_string.deallocate();
    }
}

void test_reducer(const char* source, const char* expected)
{
    auto maybe_reduced = tokenize_parse_and_reduce(source);
    if (!maybe_reduced.has_data)
    {
        printf("Test failed, original expression: %s, expected result: %s, actual result: reduction failed\n", source, expected);
        return;
    }

    auto reduced_string = maybe_reduced.value.to_string();
    if (reduced_string != expected)
    {
        printf("Test failed, original expression: %s, expected result: %s, actual result: ", source, expected);
        reduced_string.print();
        printf("\n");
        return;
    }
}

int main()
{
    test_parser_success("a");
    test_parser_success("a b");
    test_parser_success("a b c");
    test_parser_success("a (b c)");
    test_parser_success("(a b) (c d) (e f)", "a b (c d) (e f)");
    test_parser_success("((a b)) (((c d))) (e f)", "a b (c d) (e f)");
    test_parser_success("\\ x . x");
    test_parser_success("(\\ x . x)", "\\ x . x");
    test_parser_success("(\\ x . x) y");
    test_parser_success("(\\ x . x) (\\ x . x)");
    test_parser_success("\\ x . \\ y . x y z", "\\ x y . x y z");
    test_parser_success("\\ x y . x y z");
    test_parser_success("(\\ x y z w . x y z) a b c d");
    test_parser_success("(\\f x . f x)(\\f x . f x)", "(\\ f x . f x) (\\ f x . f x)");
    test_parser_success("(\\ y x . x y) x");

    test_parser_fail("\\ x x . x");
    test_parser_fail("\\ x y x . z");
    test_parser_fail("\\ x y x z . z");
    test_parser_fail("\\ a b c b . z");
    test_parser_fail("\\ a b b c . z");
    test_parser_fail("\\ x . \\ y . \\ x . z");
    test_parser_fail("\\ x . \\ y . \\ y . z");

    test_reducer("x", "x");
    test_reducer("(\\ x . x) value", "value");
    test_reducer("\\ x . y x", "y");
    test_reducer("\\ a b . global a b", "global");
    test_reducer("\\ x y z . x y z", "\\ x . x");
    test_reducer("(\\ x . x x) y z", "y y z");
    test_reducer("(\\ x y . x y) one two", "one two");
    test_reducer("(\\ x y . y x) one two", "two one");
    test_reducer("(\\ x y z . z y x) one two three", "three two one");
    test_reducer("(\\ x y . y x) (\\ y . y)", "\\ y . y (\\ y_1 . y_1)");
    test_reducer("(\\ x y z . z (y x)) (\\ z . z) (\\ x z . z x)", "\\ z . z (\\ z_1 . z_1 (\\ z_2 . z_2))");
    test_reducer("(\\ x y . y x) (\\ y . y) z", "z (\\ y . y)");
    test_reducer("(\\ y x . x y) x", "\\ x_1 . x_1 x");
    test_reducer("(\\ y x . x y) x y", "y x");
    test_reducer("(\\ g y x . y x g) x (\\ a b x . a x b)", "\\ x_1 x_2 . x_1 x_2 x");
}
