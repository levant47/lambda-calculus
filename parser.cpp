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

    String to_string();
    void print();
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

struct GlobalVariableNamesMapEntry
{
    String name;
    u32 function_id; // use function's parameter ID
};

struct GlobalVariableNamesMap
{
    List<GlobalVariableNamesMapEntry> entries;

    static GlobalVariableNamesMap allocate()
    {
        GlobalVariableNamesMap result;
        result.entries = List<GlobalVariableNamesMapEntry>::allocate();
        return result;
    }

    void deallocate()
    {
        entries.deallocate();
    }

    void push(String name, u32 function_id)
    {
        if (!has(name, function_id))
        {
            GlobalVariableNamesMapEntry entry;
            entry.name = name;
            entry.function_id = function_id;
            entries.push(entry);
        }
    }

    bool has(String name, u32 function_id)
    {
        for (u64 i = 0; i < entries.size; i++)
        {
            if (entries.data[i].name == name && entries.data[i].function_id == function_id)
            {
                return true;
            }
        }
        return false;
    }
};

struct ExpressionToStringConverter
{
    String result;
    BoundedVariableUsageMap bounded_variable_usage_map;
    GlobalVariableNamesMap global_variable_names_map;
    Option<u32> last_function_id;

    static ExpressionToStringConverter allocate();

    void deallocate();
    String convert(Expression expression);
    void collect_global_variables(List<u32>* function_ids, Expression expression);
    void internal_convert(Expression expression);
};

String Expression::to_string()
{
    auto converter = ExpressionToStringConverter::allocate();
    auto result = converter.convert(*this);
    converter.deallocate();
    return result;
}

void Expression::print()
{
    auto string_representation = to_string();
    ::print(string_representation);
    string_representation.deallocate();
}

ExpressionToStringConverter ExpressionToStringConverter::allocate()
{
    ExpressionToStringConverter result;
    result.result = String::allocate();
    result.bounded_variable_usage_map = BoundedVariableUsageMap::allocate();
    result.global_variable_names_map = GlobalVariableNamesMap::allocate();
    result.last_function_id = Option<u32>::empty();
    return result;
}

void ExpressionToStringConverter::deallocate()
{
    // don't deallocate result!
    bounded_variable_usage_map.deallocate();
}

String ExpressionToStringConverter::convert(Expression expression)
{
    auto function_ids = List<u32>::allocate();
    collect_global_variables(&function_ids, expression);
    function_ids.deallocate();

    internal_convert(expression);
    return result;
}

void ExpressionToStringConverter::collect_global_variables(List<u32>* function_ids, Expression expression)
{
    switch (expression.type)
    {
        case ExpressionTypeVariable:
            if (!expression.is_bound)
            {
                for (u64 i = 0; i < function_ids->size; i++)
                {
                    global_variable_names_map.push(expression.global_name, function_ids->data[i]);
                }
            }
            return;
        case ExpressionTypeFunction:
            function_ids->push(expression.parameter_id);
            collect_global_variables(function_ids, *expression.body);
            function_ids->pop();
            return;
        case ExpressionTypeApplication:
            collect_global_variables(function_ids, *expression.left);
            collect_global_variables(function_ids, *expression.right);
            return;
    }
    assert(false);
    return;
}

void ExpressionToStringConverter::internal_convert(Expression expression)
{
    switch (expression.type)
    {
        case ExpressionTypeVariable: {
            auto name = expression.is_bound
                ? bounded_variable_usage_map.get_name(expression.bounded_id)
                : expression.global_name;

            result.push(name);

            if (expression.is_bound) // global variables are going to be prioritized so that they always have their name printed unchanged
            {
                auto duplicates_count = bounded_variable_usage_map.get_duplicates_count(expression.bounded_id, name);
                if (last_function_id.has_data && global_variable_names_map.has(name, last_function_id.value))
                {
                    duplicates_count++;
                }
                if (duplicates_count != 0)
                {
                    result.push('_');
                    auto count_string = number_to_string(duplicates_count);
                    result.push(count_string);
                    count_string.deallocate();
                }
            }
            return;
        }
        case ExpressionTypeFunction: {
            auto original_bounded_variable_usage_map_size = bounded_variable_usage_map.entries.size;

            last_function_id = Option<u32>::construct(expression.parameter_id);

            result.push("\\ ");
            auto next_node = &expression;
            do
            {
                result.push(next_node->parameter_name);

                bounded_variable_usage_map.add(next_node->parameter_name, next_node->parameter_id);

                auto duplicates_count = bounded_variable_usage_map.get_duplicates_count(next_node->parameter_id, next_node->parameter_name)
                    + (global_variable_names_map.has(next_node->parameter_name, next_node->parameter_id) ? 1 : 0);
                if (duplicates_count != 0)
                {
                    result.push('_');
                    auto count_string = number_to_string(duplicates_count);
                    result.push(count_string);
                    count_string.deallocate();
                }

                result.push(" ");

                next_node = next_node->body;
            }
            while (next_node->type == ExpressionTypeFunction);

            result.push(". ");
            internal_convert(*next_node);

            bounded_variable_usage_map.entries.size = original_bounded_variable_usage_map_size;

            return;
        }
        case ExpressionTypeApplication: {
            if (expression.left->type == ExpressionTypeFunction)
            {
                result.push('(');
            }
            internal_convert(*expression.left);
            if (expression.left->type == ExpressionTypeFunction)
            {
                result.push(')');
            }

            result.push(' ');

            if (expression.right->type == ExpressionTypeFunction || expression.right->type == ExpressionTypeApplication)
            {
                result.push('(');
            }
            internal_convert(*expression.right);
            if (expression.right->type == ExpressionTypeFunction || expression.right->type == ExpressionTypeApplication)
            {
                result.push(')');
            }
            return;
        }
    }
    assert(false);
    return;
}

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
