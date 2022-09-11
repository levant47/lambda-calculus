#include "src/include.h"

Option<Expression> tokenize_and_parse(const char* source_c_string)
{
    auto source = String::copy_from_c_string(source_c_string);
    auto tokenization_result = tokenize(source);
    source.deallocate();
    if (!tokenization_result.success) { return Option<Expression>::empty(); }
    return parse_terminal_expression(tokenization_result.tokens);
}

ParseStatementsResult tokenize_and_parse_statements(const char* source_c_string)
{
    auto source = String::copy_from_c_string(source_c_string);
    auto tokenization_result = tokenize(source);
    source.deallocate();
    if (!tokenization_result.success)
    {
        return ParseStatementsResult::make_fail(String::copy_from_c_string("Tokenization failed"));
    }
    return parse_statements(tokenization_result.tokens);
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
        print("Test failed, original expression: ");
        print(source);
        print(", expected result: ");
        print(expected);
        print(", actual result: parsing failed\n");
        return;
    }

    auto expression_string = maybe_expression.value.to_string();
    if (expression_string != expected)
    {
        print("Test failed, original expression: ");
        print(source);
        print(", expected result: ");
        print(expected);
        print(", actual result: ");
        print(expression_string);
        print("\n");
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
        print("Test failed, original expression: ");
        print(source);
        print(", expected result: parsing failed, actual result: ");
        print("\n");
        expression_string.deallocate();
    }
}

void test_statements_parser_success(const char* source, const char* expected)
{
    auto statements_result = tokenize_and_parse_statements(source);
    if (statements_result.success)
    {
        auto statements_string = to_string(statements_result.statements);
        if (statements_string != expected)
        {
            print("Test failed, original statements:\n");
            print(source);
            print("expected result:\n");
            print(expected);
            print("actual result:\n");
            print(statements_string);
            print("\n");
        }
        statements_string.deallocate();
    }
    else
    {
        print("Test failed, original statements:\n");
        print(source);
        print("expected result:\n");
        print(expected);
        print("actual result: parsing failed\n");
    }
    statements_result.deallocate();
}

void test_statements_parser_success(const char* source)
{
    test_statements_parser_success(source, source);
}

void test_statements_parser_fail(const char* source, const char* expected_error)
{
    auto statements_result = tokenize_and_parse_statements(source);
    if (statements_result.success)
    {
        auto statements_string = to_string(statements_result.statements);
        print(
            "Test failed, original statements:\n",
            source,
            "expected the following parsing error: ",
            expected_error,
            "\nactual result: parsing succeeded:\n",
            statements_string
        );
        statements_string.deallocate();
    }
    else if (!contains_case_insensitive(
        StringView::from_c_string(expected_error),
        statements_result.error.to_string_view()
    ))
    {
        print(
            "Test failed, original statements:\n",
            source,
            "expected the following parsing error: ",
            expected_error,
            "\nactual error received: ",
            statements_result.error,
            "\n"
        );
    }
    statements_result.deallocate();
}

void test_reducer(const char* source, const char* expected)
{
    auto maybe_reduced = tokenize_parse_and_reduce(source);
    if (!maybe_reduced.has_data)
    {
        print("Test failed, original expression: ");
        print(source);
        print(", expected result: ");
        print(expected);
        print(", actual result: reduction failed\n");
        return;
    }

    auto reduced_string = maybe_reduced.value.to_string();
    if (reduced_string != expected)
    {
        print("Test failed, original expression: ");
        print(source);
        print(", expected result: ");
        print(expected);
        print(", actual result: ");
        print(reduced_string);
        print("\n");
        return;
    }
}

void test_interpreter(const char* c_string_source, const char* expected)
{
    auto source = String::copy_from_c_string(c_string_source);
    auto tokenization_result = tokenize(source);
    if (tokenization_result.success)
    {
        auto parse_result = parse_statements(tokenization_result.tokens);
        if (parse_result.success)
        {
            auto interpreter_result = interpret(parse_result.statements);
            if (interpreter_result.success)
            {
                auto maybe_expected_expression = tokenize_and_parse(expected);
                assert(maybe_expected_expression.has_data);

                auto interpreted_expected_expression = interpret(
                    parse_result.statements,
                    maybe_expected_expression.value
                );
                assert(interpreted_expected_expression.success);

                if (interpreter_result.expression != interpreted_expected_expression.expression)
                {
                    auto resulting_expression_string = interpreter_result.expression.to_string();
                    auto expected_expression_string = interpreted_expected_expression.expression.to_string();
                    print(
                        "Test failed, original program:\n",
                        c_string_source,
                        "Expected result: ",
                        expected_expression_string,
                        "\nActual result: ",
                        resulting_expression_string,
                        "\n"
                    );
                    expected_expression_string.deallocate();
                    resulting_expression_string.deallocate();
                }
            }
            else
            {
                print(
                    "Test failed, original program:\n",
                    c_string_source,
                    "Expected result: ",
                    expected,
                    "\nActual result: interpretation failed: ",
                    interpreter_result.error,
                    "\n"
                );
            }
            interpreter_result.deallocate();
        }
        else
        {
            print(
                "Test failed, original program:\n",
                c_string_source,
                "Expected result: ",
                expected,
                "\nActual result: parsing failed: ",
                parse_result.error,
                "\n"
            );
        }
        parse_result.deallocate();
        tokenization_result.tokens.deallocate();
    }
    else
    {
        print(
            "Test failed, original program:\n",
            c_string_source,
            "Expected result: ",
            expected,
            "\nActual result: tokenization failed at index ",
            tokenization_result.failed_at_index,
            "\n"
        );
    }
    source.deallocate();
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

    test_statements_parser_success("zero = \\ f x . x;\n");
    test_statements_parser_success(
        "zero = \\ f x . x;\n"
        "succ = \\ n f x . f (n f x);\n"
    );
    test_statements_parser_success(
        "zero =\n\t\\ f x . x;\n"
        "succ =\n\t\\ n f x . f (n f x);\n",
        "zero = \\ f x . x;\n"
        "succ = \\ n f x . f (n f x);\n"
    );
    test_statements_parser_fail(
        "zero = \\ f x . x;\n"
        "zero = hey hey;\n",
        "duplicate definition"
    );

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

    test_interpreter(
        "main = hey hey;\n",
        "hey hey"
    );
    test_interpreter(
        "zero = \\ f x . x;\n"
        "succ = \\ n . \\ f x . f (n f x);\n"
        "main = succ zero;\n",
        "\\ f x . f x"
    );

    print("Done\n");
}
