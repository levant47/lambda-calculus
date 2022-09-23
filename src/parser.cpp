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
            // is_bound = true
            u32 bounded_id;
            u32 bound_index;
            // is_bound = false
            String global_name;
        };
        // ExpressionTypeFunction
        struct
        {
            // this is just a simple way to refer to a function in an expression
            // (used for expression to string conversion)
            u32 parameter_id;
            String parameter_name;
            Expression* body;
        };
        // ExpressionTypeApplication
        struct { Expression* left; Expression* right; };
    };

    void deallocate();
    String to_string();
};

void Expression::deallocate()
{
    switch (type)
    {
        case ExpressionTypeVariable:
            if (!is_bound) { global_name.deallocate(); }
            break;
        case ExpressionTypeFunction:
            parameter_name.deallocate();
            body->deallocate();
            default_deallocate(body);
            break;
        case ExpressionTypeApplication:
            left->deallocate();
            right->deallocate();
            default_deallocate(left);
            default_deallocate(right);
            break;
        default: assert(false);
    }
}

Expression copy(Expression source)
{
    Expression result;
    result.type = source.type;
    switch (source.type)
    {
        case ExpressionTypeVariable:
            result.is_bound = source.is_bound;
            if (source.is_bound)
            {
                result.bounded_id = source.bounded_id;
                result.bound_index = source.bound_index;
            }
            else
            {
                result.global_name = source.global_name.copy();
            }
            return result;
        case ExpressionTypeFunction:
            result.parameter_id = source.parameter_id;
            result.parameter_name = source.parameter_name.copy();
            result.body = copy_to_heap(copy(*source.body));
            return result;
        case ExpressionTypeApplication:
            result.left = copy_to_heap(copy(*source.left));
            result.right = copy_to_heap(copy(*source.right));
            return result;
        default: assert(false); return {};
    }
}

static bool operator==(Expression left, Expression right)
{
    if (left.type != right.type) { return false; }
    switch (left.type)
    {
        case ExpressionTypeVariable:
            return left.is_bound == right.is_bound && (
                left.is_bound
                    ? left.bound_index == right.bound_index
                    : left.global_name == right.global_name
            );
        case ExpressionTypeFunction: return *left.body == *right.body;
        case ExpressionTypeApplication: return *left.left == *right.left && *left.right == *right.right;
        default: assert(false, "Unknown expression type encountered"); return {};
    }
}

static bool operator!=(Expression left, Expression right) { return !(left == right); }

struct Statement
{
    String name;
    Expression expression;

    void deallocate()
    {
        name.deallocate();
        expression.deallocate();
    }
};

String to_string(List<Statement> statements)
{
    auto result = String::allocate();
    for (u64 i = 0; i < statements.size; i++)
    {
        auto statement = statements.data[i];
        result.push(statement.name);
        result.push(" = ");

        auto expression_string = statement.expression.to_string();
        result.push(expression_string);
        expression_string.deallocate();

        result.push(";\n");
    }
    return result;
}

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

    void deallocate() { entries.deallocate(); }

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

void print(Expression expression)
{
    auto string_representation = expression.to_string();
    print(string_representation);
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
    global_variable_names_map.deallocate();
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
}

void ExpressionToStringConverter::internal_convert(Expression expression)
{
    switch (expression.type)
    {
        case ExpressionTypeVariable:
        {
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
                    result.push((u64)duplicates_count);
                }
            }
            return;
        }
        case ExpressionTypeFunction:
        {
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
                    result.push((u64)duplicates_count);
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
        case ExpressionTypeApplication:
        {
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

    u32 get_index(String name)
    {
        for (u64 i = 0; i < list.size; i++)
        {
            if (list.data[i].name == name)
            {
                return i;
            }
        }
        assert(false);
        return {};
    }

    void clear() { list.clear(); }
};

u32 next_id = 0;

struct ExpressionParser
{
    List<Token> tokens;
    u64 index;
    u32 depth;
    BoundedVariableMap bounded_variable_map;
    List<String> global_names;

    static ExpressionParser allocate(List<Token> tokens)
    {
        ExpressionParser result;
        result.tokens = tokens;
        result.index = 0;
        result.depth = 0;
        result.bounded_variable_map = BoundedVariableMap::allocate();
        result.global_names = List<String>::allocate();
        return result;
    }

    void deallocate()
    {
        bounded_variable_map.deallocate();
        global_names.deallocate();
    }

    bool is_done() { return index == tokens.size; }

    Token current() { return tokens.data[index]; }

    void next() { index++; }

    void skip_whitespace() { while (!is_done() && current().type == LcTokenTypeWhitespace) { next(); } }

    bool expect_token_type(LcTokenType token_type)
    {
        if (is_done() || current().type != token_type) { return false; }
        next();
        return true;
    }

    Option<Expression> parse_variable()
    {
        if (is_done() || current().type != LcTokenTypeName) { return Option<Expression>::empty(); }

        Expression expression;
        expression.type = ExpressionTypeVariable;
        expression.depth = depth;
        expression.is_bound = bounded_variable_map.has(current().name);
        if (expression.is_bound)
        {
            expression.bounded_id = bounded_variable_map.get(current().name);
            expression.bound_index =
                bounded_variable_map.list.size - bounded_variable_map.get_index(current().name) - 1;
        }
        else
        {
            expression.global_name = current().name.copy();
        }

        next();

        return Option<Expression>::construct(expression);
    }

    Option<Expression> parse_function()
    {
        if (is_done()) { return Option<Expression>::empty(); }

        auto original_index = index;
        auto original_bounded_variable_map_size = bounded_variable_map.list.size;

        bool head_start_found = expect_token_type(LcTokenTypeLambdaHeadStart);
        skip_whitespace(); // do it here just to avoid an extra nestedness level
        if (
            head_start_found
                && current().type == LcTokenTypeName
                && !bounded_variable_map.has(current().name)
                && !global_names.contains(current().name)
        )
        {
            Expression function;
            function.type = ExpressionTypeFunction;
            function.depth = depth;
            function.parameter_id = next_id++;
            function.parameter_name = current().name.copy();
            function.body = nullptr;
            bounded_variable_map.push(function.parameter_id, function.parameter_name);
            next();

            Expression** next_body = &function.body;
            bool success = true;
            while (true)
            {
                skip_whitespace();

                if (current().type != LcTokenTypeName) { break; }
                if (bounded_variable_map.has(current().name)) { success = false; break; }

                Expression next_function;
                next_function.type = ExpressionTypeFunction;
                next_function.depth = depth;
                next_function.parameter_id = next_id++;
                next_function.parameter_name = current().name.copy();
                next_function.body = nullptr;

                *next_body = copy_to_heap(next_function);
                next_body = &(*next_body)->body;

                bounded_variable_map.push(next_function.parameter_id, next_function.parameter_name);

                next();
            }

            if (success && expect_token_type(LcTokenTypeLambdaHeadEnd))
            {
                skip_whitespace();
                auto maybe_body = parse_expression();
                if (maybe_body.has_data)
                {
                    *next_body = copy_to_heap(maybe_body.value);
                    bounded_variable_map.list.size = original_bounded_variable_map_size;
                    return Option<Expression>::construct(function);
                }
            }

            function.parameter_name.deallocate();
            auto node = function.body;
            while (node != nullptr)
            {
                auto next_node = node->body;
                node->parameter_name.deallocate();
                default_deallocate(node);
                node = next_node;
            }
        }

        index = original_index;
        bounded_variable_map.list.size = original_bounded_variable_map_size;
        return Option<Expression>::empty();
    }

    Option<Expression> parse_application()
    {
        if (is_done()) { return Option<Expression>::empty(); }

        auto original_index = index;

        auto maybe_left = parse_variable();
        if (!maybe_left.has_data) { maybe_left = parse_parenthesized_expression(); }

        if (maybe_left.has_data)
        {
            skip_whitespace();

            auto maybe_right = parse_expression();
            if (maybe_right.has_data)
            {
                if (maybe_right.value.type == ExpressionTypeApplication && maybe_right.value.depth == depth)
                {
                    return Option<Expression>::construct(
                        append_left_to_application(depth, maybe_left.value, &maybe_right.value)
                    );
                }
                Expression expression;
                expression.type = ExpressionTypeApplication;
                expression.depth = depth;
                expression.left = copy_to_heap(maybe_left.value);
                expression.right = copy_to_heap(maybe_right.value);
                return Option<Expression>::construct(expression);
            }

            maybe_left.value.deallocate();
        }

        index = original_index;
        return Option<Expression>::empty();
    }

    Option<Expression> parse_parenthesized_expression()
    {
        auto original_index = index;
        if (expect_token_type(LcTokenTypeOpenParen))
        {
            auto original_depth = depth;
            depth++;
            auto maybe_expression = parse_expression();
            if (maybe_expression.has_data)
            {
                if (expect_token_type(LcTokenTypeCloseParen))
                {
                    depth--;
                    return Option<Expression>::construct(maybe_expression.value);
                }
                maybe_expression.value.deallocate();
            }
            depth = original_depth;
        }
        index = original_index;
        return Option<Expression>::empty();
    }

    Option<Expression> parse_expression()
    {
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

    Result<Statement, String> parse_statement()
    {
        if (is_done())
        {
            return Result<Statement, String>::fail(String::copy_from_c_string(
                "Expected a statement, encountered end of file"
            ));
        }
        if (current().type != LcTokenTypeName)
        {
            auto error = String::allocate();
            error.push("Expected a name as start of a statement, encountered ");
            error.push(to_string(current().type));
            return Result<Statement, String>::fail(error);
        }
        auto name = current().name;
        if (global_names.contains(name))
        {
            auto error = String::allocate();
            error.push("Encountered duplicate definition: ");
            error.push(name);
            return Result<Statement, String>::fail(error);
        }
        global_names.push(name);
        // we don't need to restore global names in case of failure as long as we know that source can only be a list of
        // statements, and therefore failure to parse a statement will lead to termination of parsing and deallocation
        // of all parser resources

        auto original_index = index;
        next();
        skip_whitespace();

        if (!expect_token_type(LcTokenTypeEquals))
        {
            index = original_index;
            auto error = String::allocate();
            error.push("Expected an equals sign as part of a statement, encountered ");
            error.push(to_string(current().type));
            return Result<Statement, String>::fail(error);
        }

        skip_whitespace();

        auto maybe_expression = parse_expression();
        if (!maybe_expression.has_data)
        {
            index = original_index;
            auto error = String::allocate();
            error.push("Failed to parse expression associated with definition ");
            error.push(name);
            return Result<Statement, String>::fail(error);
        }

        skip_whitespace();

        if (!expect_token_type(LcTokenTypeSemicolon))
        {
            maybe_expression.value.deallocate();
            index = original_index;
            auto error = String::allocate();
            error.push("Expected a semicolon at the end of a statement, encountered ");
            error.push(to_string(current().type));
            return Result<Statement, String>::fail(error);
        }

        // we don't want local variables from one definition to affect local variables in another definition in any way
        bounded_variable_map.clear();

        Statement result;
        result.name = name.copy();
        result.expression = maybe_expression.value;
        return Result<Statement, String>::success(result);
    }
};

Option<Expression> parse_terminal_expression(List<Token> tokens)
{
    auto parser = ExpressionParser::allocate(tokens);
    auto result = parser.parse_expression();
    if (result.has_data)
    {
        parser.skip_whitespace();
        if (parser.is_done())
        {
            parser.deallocate();
            return result;
        }
        result.value.deallocate();
    }
    parser.deallocate();
    return Option<Expression>::empty();
}

struct ParseStatementsResult
{
    bool success;
    union
    {
        // success = true
        List<Statement> statements;
        // success = false
        String error;
    };

    static ParseStatementsResult make_success(List<Statement> statements)
    {
        ParseStatementsResult result;
        result.success = true;
        result.statements = statements;
        return result;
    }

    static ParseStatementsResult make_fail(String error)
    {
        ParseStatementsResult result;
        result.success = false;
        result.error = error;
        return result;
    }

    void deallocate()
    {
        if (!success) { error.deallocate(); return; }
        for (u64 i = 0; i < statements.size; i++) { statements.data[i].deallocate(); }
        statements.deallocate();
    }
};

ParseStatementsResult parse_statements(List<Token> tokens)
{
    auto parser = ExpressionParser::allocate(tokens);
    auto statements = List<Statement>::allocate();
    parser.skip_whitespace();
    bool fail = false;
    String error;
    while (!parser.is_done())
    {
        auto statement_result = parser.parse_statement();
        if (!statement_result.is_success)
        {
            fail = true;
            error = statement_result.error;
            break;
        }
        statements.push(statement_result.value);

        parser.skip_whitespace();
    }
    parser.deallocate();
    if (fail)
    {
        for (u64 i = 0; i < statements.size; i++) { statements.data[i].deallocate(); }
        statements.deallocate();
        return ParseStatementsResult::make_fail(error);
    }
    return ParseStatementsResult::make_success(statements);
}
