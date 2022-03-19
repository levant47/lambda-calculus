enum ExpressionType
{
    ExpressionTypeVariable,
    ExpressionTypeFunction,
    ExpressionTypeApplication,
};

struct BoundedVariableUsageMapEntry
{
    String variable_name;
    u32 variable_id;
};

struct BoundedVariableUsageMap
{
    List<BoundedVariableUsageMapEntry> entries;

    static BoundedVariableUsageMap allocate()
    {
        BoundedVariableUsageMap result;
        result.entries = List<BoundedVariableUsageMapEntry>::allocate();
        return result;
    }

    void deallocate()
    {
        entries.deallocate();
    }

    void add(String variable_name, u32 variable_id)
    {
        BoundedVariableUsageMapEntry entry;
        entry.variable_name = variable_name;
        entry.variable_id = variable_id;
        entries.push(entry);
    }

    String get_name(u32 variable_id)
    {
        for (u64 i = 0; i < entries.size; i++)
        {
            if (entries.data[i].variable_id == variable_id)
            {
                return entries.data[i].variable_name;
            }
        }
        assert(false);
        return {};
    }

    u32 get_duplicates_count(u32 variable_id, String variable_name)
    {
        auto duplicates_count = 0;
        for (u64 i = 0; i < entries.size; i++)
        {
            if (entries.data[i].variable_id == variable_id)
            {
                return duplicates_count;
            }
            if (entries.data[i].variable_name == variable_name)
            {
                duplicates_count++;
            }
        }
        assert(false);
        return {};
    }
};

struct Expression
{
    ExpressionType type;
    u32 depth; // for keeping track of the nestedness level

    union
    {
        // ExpressionTypeVariable
        struct
        {
            bool is_bound;
            union
            {
                // is_bound = true
                u32 bounded_id; // TODO: rename to bound_id
                // is_bound = false
                String global_name;
            };
        };
        // ExpressionTypeFunction
        struct { u32 parameter_id; String parameter_name; Expression* body; };
        // ExpressionTypeApplication
        struct { Expression* left; Expression* right; };
    };

    String to_string()
    {
        auto map = BoundedVariableUsageMap::allocate();
        auto result = to_string(map);
        map.deallocate();
        return result;
    }

    String to_string(BoundedVariableUsageMap bounded_variable_usage_map)
    {
        switch (type)
        {
            case ExpressionTypeVariable: {
                auto name = is_bound
                    ? bounded_variable_usage_map.get_name(bounded_id)
                    : global_name;

                auto result = String::allocate(name.size + 2);
                result.push(name);

                if (is_bound) // global variables aren't going to have an entry
                {
                    auto duplicates_count = bounded_variable_usage_map.get_duplicates_count(bounded_id, name);
                    if (duplicates_count != 0)
                    {
                        result.push('_');
                        auto count_string = number_to_string(duplicates_count);
                        result.push(count_string);
                        count_string.deallocate();
                    }
                }

                return result;
            }
            case ExpressionTypeFunction: {
                auto original_bounded_variable_usage_map_size = bounded_variable_usage_map.entries.size;

                auto result = String::allocate();
                result.push("\\ ");
                auto node = this;
                do
                {
                    result.push(node->parameter_name);

                    bounded_variable_usage_map.add(node->parameter_name, node->parameter_id);

                    auto duplicates_count = bounded_variable_usage_map.get_duplicates_count(node->parameter_id, node->parameter_name);
                    if (duplicates_count != 0)
                    {
                        result.push('_');
                        auto count_string = number_to_string(duplicates_count);
                        result.push(count_string);
                        count_string.deallocate();
                    }

                    result.push(" ");

                    node = node->body;
                }
                while (node->type == ExpressionTypeFunction);
                result.push(". ");
                auto body_string = node->to_string(bounded_variable_usage_map);
                result.push(body_string);
                body_string.deallocate();

                bounded_variable_usage_map.entries.size = original_bounded_variable_usage_map_size;

                return result;
            }
            case ExpressionTypeApplication: {
                auto result = String::allocate();

                if (left->type == ExpressionTypeFunction)
                {
                    result.push('(');
                }
                auto left_string = left->to_string(bounded_variable_usage_map);
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
                auto right_string = right->to_string(bounded_variable_usage_map);
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

struct BoundedVariableMapEntry
{
    u32 id;
    String name;
};

struct BoundedVariableMap
{
    List<BoundedVariableMapEntry> list;

    static BoundedVariableMap allocate()
    {
        BoundedVariableMap result;
        result.list = List<BoundedVariableMapEntry>::allocate();
        return result;
    }

    void deallocate()
    {
        list.deallocate();
    }

    void push(u32 id, String name)
    {
        BoundedVariableMapEntry entry;
        entry.id = id;
        entry.name = name;
        list.push(entry);
    }

    bool has(String name)
    {
        for (u64 i = 0; i < list.size; i++)
        {
            if (list.data[i].name == name)
            {
                return true;
            }
        }
        return false;
    }

    u32 get(String name)
    {
        for (u64 i = 0; i < list.size; i++)
        {
            if (list.data[i].name == name)
            {
                return list.data[i].id;
            }
        }
        assert(false);
        return {};
    }
};

struct ExpressionParser
{
    List<Token> tokens;
    u64 index;
    u32 depth;
    u32 next_id;
    BoundedVariableMap bounded_variable_map;

    static ExpressionParser allocate(List<Token> tokens)
    {
        ExpressionParser result;
        result.tokens = tokens;
        result.index = 0;
        result.depth = 0;
        result.next_id = 0;
        result.bounded_variable_map = BoundedVariableMap::allocate();
        return result;
    }

    void deallocate()
    {
        bounded_variable_map.deallocate();
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
        expression.is_bound = bounded_variable_map.has(current().name);
        if (expression.is_bound)
        {
            expression.bounded_id = bounded_variable_map.get(current().name);
        }
        else
        {
            expression.global_name = current().name;
        }

        index++;

        return Option<Expression>::construct(expression);
    }

    Option<Expression> parse_function()
    {
        if (is_done()) { return Option<Expression>::empty(); }

        auto original_index = index;
        auto original_bounded_variable_map_size = bounded_variable_map.list.size;
        if (false)
        {
            fail:
            index = original_index;
            bounded_variable_map.list.size = original_bounded_variable_map_size;
            return Option<Expression>::empty();
        }

        bool head_start_found = expect_token_type(TokenTypeLambdaHeadStart);
        if (!head_start_found) { goto fail; }

        skip_whitespace();

        if (current().type != TokenTypeName || bounded_variable_map.has(current().name)) { goto fail; }
        Expression function;
        function.type = ExpressionTypeFunction;
        function.depth = depth;
        function.parameter_id = next_id++;
        function.parameter_name = current().name;
        bounded_variable_map.push(function.parameter_id, function.parameter_name);
        index++;

        Expression** next_body = &function.body;
        while (true)
        {
            skip_whitespace();

            if (current().type != TokenTypeName) { break; }
            if (bounded_variable_map.has(current().name)) { goto fail; }

            Expression next_function;
            next_function.type = ExpressionTypeFunction;
            next_function.depth = depth;
            next_function.parameter_id = next_id++;
            next_function.parameter_name = current().name;

            *next_body = copy_to_heap(next_function); // #leak in case of parsing error down the line
            next_body = &(*next_body)->body;

            bounded_variable_map.push(next_function.parameter_id, next_function.parameter_name);

            index++;
        }

        bool head_end_found = expect_token_type(TokenTypeLambdaHeadEnd);
        if (!head_end_found) { goto fail; }

        skip_whitespace();

        auto maybe_body = parse_expression();
        if (!maybe_body.has_data) { goto fail; }
        *next_body = copy_to_heap(maybe_body.value);

        bounded_variable_map.list.size = original_bounded_variable_map_size;

        return Option<Expression>::construct(function);
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
