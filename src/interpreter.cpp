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
        else { expression.deallocate(); }
    }
};

Expression resolve_names(List<Statement> definitions, Expression source)
{
    switch (source.type)
    {
        case ExpressionTypeVariable:
            if (source.is_bound) { return copy(source); }
            for (u64 i = 0; i < definitions.size; i++)
            {
                auto definition = definitions.data[i];
                if (definition.name == source.global_name)
                {
                    return copy(definition.expression);
                }
            }
            return copy(source);
        case ExpressionTypeFunction:
            source.parameter_name = source.parameter_name.copy();
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
    Expression previous_expression = copy(main_expression);
    while (true)
    {
        auto reduced_expression = reduce(previous_expression);
        auto resolved_expression = resolve_names(definitions, reduced_expression);
        reduced_expression.deallocate();
        if (resolved_expression == previous_expression)
        { // we're done
            previous_expression.deallocate();

            InterpreterResult result;
            result.success = true;
            result.expression = resolved_expression;
            return result;
        }
        previous_expression.deallocate();
        previous_expression = resolved_expression;
    }
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
