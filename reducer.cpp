Expression beta_reduce(String parameter_name, Expression argument, Expression body)
{
    switch (body.type)
    {
        case ExpressionTypeVariable:
            if (body.name == parameter_name)
            {
                return argument;
            }
            else
            {
                return body;
            }
        case ExpressionTypeFunction:
            body.body = copy_to_heap(beta_reduce(parameter_name, argument, *body.body));
            return body;
        case ExpressionTypeApplication:
            body.left = copy_to_heap(beta_reduce(parameter_name, argument, *body.left));
            body.right = copy_to_heap(beta_reduce(parameter_name, argument, *body.right));
            return body;
    }
    assert(false);
    return {};
}

bool has_usages(String variable_name, Expression expression)
{
    switch (expression.type)
    {
        case ExpressionTypeVariable:
            return expression.name == variable_name;
        case ExpressionTypeFunction:
            return has_usages(variable_name, *expression.body);
        case ExpressionTypeApplication:
            return has_usages(variable_name, *expression.left)
                || has_usages(variable_name, *expression.right);
    }
    assert(false);
    return {};
}

Expression eta_reduce(Expression function)
{
    assert(function.type == ExpressionTypeFunction);

    if (function.body->type == ExpressionTypeApplication
        && function.body->right->type == ExpressionTypeVariable
        && function.body->right->name == function.parameter_name
        && !has_usages(function.parameter_name, *function.body->left))
    {
        return *function.body->left;
    }
    return function;
}

Expression reduce(Expression expression)
{
    switch (expression.type)
    {
        case ExpressionTypeVariable:
            return expression;
        case ExpressionTypeFunction:
            expression.body = copy_to_heap(reduce(*expression.body));
            expression = eta_reduce(expression);
            return expression;
        case ExpressionTypeApplication:
            expression.left = copy_to_heap(reduce(*expression.left));
            expression.right = copy_to_heap(reduce(*expression.right));
            if (expression.left->type == ExpressionTypeFunction)
            {
                return reduce(beta_reduce(expression.left->parameter_name, *expression.right, *expression.left->body));
            }
            return expression;
    }
    assert(false);
    return {};
}
