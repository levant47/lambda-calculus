enum ExpressionType
{
    ExpressionTypeVariable,
    ExpressionTypeFunction,
    ExpressionTypeApplication,
};

struct Expression
{
    ExpressionType type;
    u32 depth; // for keeping track of the nestedness level

    union
    {
        // ExpressionTypeVariable
        struct { String name; };
        // ExpressionTypeFunction
        struct { String parameter_name; Expression* body; };
        // ExpressionTypeApplication
        struct { Expression* left; Expression* right; };
    };

    String to_string()
    {
        switch (type)
        {
            case ExpressionTypeVariable:
                return name.copy();
            case ExpressionTypeFunction: {
                auto result = String::allocate();
                result.push("\\ ");
                auto node = this;
                do
                {
                    result.push(node->parameter_name);
                    result.push(" ");
                    node = node->body;
                }
                while (node->type == ExpressionTypeFunction);
                result.push(". ");
                auto body_string = node->to_string();
                result.push(body_string);
                body_string.deallocate();
                return result;
            }
            case ExpressionTypeApplication: {
                auto result = String::allocate();

                if (left->type == ExpressionTypeFunction)
                {
                    result.push('(');
                }
                auto left_string = left->to_string();
                result.push(left_string);
                left_string.deallocate();
                if (left->type == ExpressionTypeFunction)
                {
                    result.push(')');
                }

                result.push(' ');

                if (right->type == ExpressionTypeFunction || right->type == ExpressionTypeApplication)
                {
                    result.push('(');
                }
                auto right_string = right->to_string();
                result.push(right_string);
                right_string.deallocate();
                if (right->type == ExpressionTypeFunction || right->type == ExpressionTypeApplication)
                {
                    result.push(')');
                }
                return result;
            }
        }
        assert(false);
        return {};
    }

    void print()
    {
        auto string_representation = to_string();
        string_representation.print();
        string_representation.deallocate();
    }
};

Expression append_left_to_application(u32 depth, Expression node_to_append, Expression* application_tree)
{
    if (application_tree->left->type == ExpressionTypeApplication && application_tree->left->depth == depth)
    {
        append_left_to_application(depth, node_to_append, application_tree->left);
    }
    else
    {
        Expression new_left;
        new_left.type = ExpressionTypeApplication;
        new_left.depth = depth;
        new_left.left = copy_to_heap(node_to_append);
        new_left.right = application_tree->left;

        application_tree->left = copy_to_heap(new_left);
    }
    return *application_tree;
}

struct ExpressionParser
{
    List<Token> tokens;
    u64 index;
    u32 depth;
    List<String> bounded_variable_names;

    static ExpressionParser allocate(List<Token> tokens)
    {
        ExpressionParser result;
        result.tokens = tokens;
        result.index = 0;
        result.depth = 0;
        result.bounded_variable_names = List<String>::allocate();
        return result;
    }

    void deallocate()
    {
        bounded_variable_names.deallocate();
    }

    bool is_done() { return index == tokens.size; }

    Token current() { return tokens.data[index]; }

    void skip_whitespace()
    {
        while (!is_done() && current().type == TokenTypeWhitespace)
        {
            index++;
        }
    }

    bool expect_token_type(TokenType token_type)
    {
        if (is_done() || current().type != token_type)
        {
            return false;
        }
        index++;
        return true;
    }

    Option<Expression> parse_variable()
    {
        if (is_done() || current().type != TokenTypeName) { return Option<Expression>::empty(); }

        Expression expression;
        expression.type = ExpressionTypeVariable;
        expression.depth = depth;
        expression.name = current().name;

        index++;

        return Option<Expression>::construct(expression);
    }

    Option<Expression> parse_function()
    {
        if (is_done()) { return Option<Expression>::empty(); }

        auto original_index = index;
        auto original_bounded_variable_names_size = bounded_variable_names.size;
        if (false)
        {
            fail:
            index = original_index;
            bounded_variable_names.size = original_bounded_variable_names_size;
            return Option<Expression>::empty();
        }

        bool head_start_found = expect_token_type(TokenTypeLambdaHeadStart);
        if (!head_start_found) { goto fail; }

        skip_whitespace();

        if (current().type != TokenTypeName || bounded_variable_names.contains(current().name)) { goto fail; }
        auto parameter_name = current().name;
        bounded_variable_names.push(parameter_name);
        index++;

        auto parameters = List<String>::allocate(); // #leak
        parameters.push(parameter_name);
        while (true)
        {
            skip_whitespace();

            if (current().type != TokenTypeName) { break; }
            if (bounded_variable_names.contains(current().name)) { goto fail; }
            parameters.push(current().name);
            bounded_variable_names.push(current().name);
            index++;
        }

        bool head_end_found = expect_token_type(TokenTypeLambdaHeadEnd);
        if (!head_end_found) { goto fail; }

        skip_whitespace();

        auto maybe_body = parse_expression();
        if (!maybe_body.has_data) { goto fail; }

        Expression expression;
        expression.type = ExpressionTypeFunction;
        expression.depth = depth;
        expression.parameter_name = parameters.data[0];
        Expression** next_body = &expression.body;
        for (u64 i = 1; i < parameters.size; i++)
        {
            Expression next_function;
            next_function.type = ExpressionTypeFunction;
            next_function.depth = depth;
            next_function.parameter_name = parameters.data[i];

            *next_body = copy_to_heap(next_function);
            next_body = &(*next_body)->body;
        }
        *next_body = copy_to_heap(maybe_body.value);

        bounded_variable_names.size = original_bounded_variable_names_size;

        return Option<Expression>::construct(expression);
    }

    Option<Expression> parse_application()
    {
        if (is_done()) { return Option<Expression>::empty(); }

        auto original_index = index;
        if (false)
        {
            fail:
            index = original_index;
            return Option<Expression>::empty();
        }

        Option<Expression> maybe_left;

        maybe_left = parse_variable();
        if (!maybe_left.has_data)
        {
            index = original_index;
            goto try_parenthesized;
        }
        goto continue_to_right;

        try_parenthesized:
        maybe_left = parse_parenthesized_expression();
        if (!maybe_left.has_data) { goto fail; }

        continue_to_right:
        skip_whitespace();

        auto maybe_right = parse_expression();
        if (!maybe_right.has_data) { goto fail; }

        if (maybe_right.value.type == ExpressionTypeApplication && maybe_right.value.depth == depth)
        {
            return Option<Expression>::construct(append_left_to_application(depth, maybe_left.value, &maybe_right.value));
        }
        else
        {
            Expression expression;
            expression.type = ExpressionTypeApplication;
            expression.depth = depth;
            expression.left = copy_to_heap(maybe_left.value);
            expression.right = copy_to_heap(maybe_right.value);
            return Option<Expression>::construct(expression);
        }
    }

    Option<Expression> parse_parenthesized_expression()
    {
        if (is_done()) { return Option<Expression>::empty(); }

        auto original_index = index;
        auto original_depth = depth;
        if (false)
        {
            fail:
            index = original_index;
            depth = original_depth;
            return Option<Expression>::empty();
        }

        auto open_paren_found = expect_token_type(TokenTypeOpenParen);
        if (!open_paren_found) { goto fail; }

        depth++;
        auto maybe_expression = parse_expression();
        if (!maybe_expression.has_data) { goto fail; }
        depth--;

        auto close_paren_found = expect_token_type(TokenTypeCloseParen);
        if (!close_paren_found) { goto fail; }

        return Option<Expression>::construct(maybe_expression.value);
    }

    Option<Expression> parse_expression()
    {
        if (is_done()) { return Option<Expression>::empty(); }

        auto maybe_application = parse_application();
        if (maybe_application.has_data) { return maybe_application; }

        auto maybe_variable = parse_variable();
        if (maybe_variable.has_data) { return maybe_variable; }

        auto maybe_function = parse_function();
        if (maybe_function.has_data) { return maybe_function; }

        auto maybe_parenthesized = parse_parenthesized_expression();
        if (maybe_parenthesized.has_data) { return maybe_parenthesized; }

        return Option<Expression>::empty();
    }

    Option<Expression> parse_terminal_expression()
    {
        if (is_done()) { return Option<Expression>::empty(); }

        auto original_index = index;
        if (false)
        {
            fail:
            index = original_index;
            return Option<Expression>::empty();
        }

        auto maybe_expression = parse_expression();
        if (!maybe_expression.has_data) { goto fail; }

        skip_whitespace();

        if (is_done()) { return maybe_expression; }

        bool new_line_found = expect_token_type(TokenTypeNewLine);
        if (!new_line_found) { goto fail; }

        return maybe_expression;
    }
};

Option<Expression> parse(List<Token> tokens)
{
    auto parser = ExpressionParser::allocate(tokens);
    auto maybe_expression = parser.parse_terminal_expression();
    if (!maybe_expression.has_data) { return Option<Expression>::empty(); }

    parser.skip_whitespace();

    if (!parser.is_done())
    {
        parser.deallocate();
        return Option<Expression>::empty();
    }

    parser.deallocate();
    return maybe_expression;
}
