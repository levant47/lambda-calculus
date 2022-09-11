struct InterpreterResult
{
    bool success;
    Expression expression;
    String error;

    static InterpreterResult make_fail(String error)
    {
        InterpreterResult result;
        result.success = false;
        result.error = error;
        return result;
    }

    void deallocate()
    {
        if (!success) { error.deallocate(); }
    }
};

Expression resolve_names(List<Statement> definitions, Expression source)
{
    switch (source.type)
    {
        case ExpressionTypeVariable:
            if (source.is_bound) { return source; }
            for (u64 i = 0; i < definitions.size; i++)
            {
                auto definition = definitions.data[i];
                if (definition.name == source.global_name)
                {
                    return definition.expression;
                }
            }
            return source;
        case ExpressionTypeFunction:
            source.body = copy_to_heap(resolve_names(definitions, *source.body));
            return source;
        case ExpressionTypeApplication:
            source.left = copy_to_heap(resolve_names(definitions, *source.left));
            source.right = copy_to_heap(resolve_names(definitions, *source.right));
            return source;
        default: assert(false, "Encountered an unknown expression type"); return {};
    }
}

InterpreterResult interpret(List<Statement> definitions, Expression main_expression)
{
    while (true)
    {
        auto new_expression = reduce(main_expression);
        new_expression = resolve_names(definitions, new_expression);
        if (new_expression == main_expression)
        { // we're done
            break;
        }
        main_expression = new_expression;
    }

    InterpreterResult result;
    result.success = true;
    result.expression = main_expression;
    return result;
}

InterpreterResult interpret(List<Statement> program)
{
    Expression main_expression;
    bool main_expression_found = false;
    for (u64 i = 0; i < program.size; i++)
    {
        auto statement = program.data[i];
        if (statement.name == "main")
        {
            main_expression = statement.expression;
            main_expression_found = true;
            break;
        }
    }
    if (!main_expression_found)
    {
        return InterpreterResult::make_fail(String::copy_from_c_string("Failed to find definition of 'main'"));
    }

    return interpret(program, main_expression);
}
