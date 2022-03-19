Expression beta_reduce(u32 parameter_id, Expression argument, Expression body)
{
    switch (body.type)
    {
        case ExpressionTypeVariable:
            if (body.is_bound && body.bounded_id == parameter_id)
            {
                return argument;
            }
            else
            {
                return body;
            }
        case ExpressionTypeFunction:
            body.body = copy_to_heap(beta_reduce(parameter_id, argument, *body.body));
            return body;
        case ExpressionTypeApplication:
            body.left = copy_to_heap(beta_reduce(parameter_id, argument, *body.left));
            body.right = copy_to_heap(beta_reduce(parameter_id, argument, *body.right));
            return body;
    }
    assert(false);
    return {};
}

bool has_usages(u32 variable_id, Expression expression)
{
    switch (expression.type)
    {
        case ExpressionTypeVariable:
            return expression.is_bound && expression.bounded_id == variable_id;
        case ExpressionTypeFunction:
            return has_usages(variable_id, *expression.body);
        case ExpressionTypeApplication:
            return has_usages(variable_id, *expression.left)
                || has_usages(variable_id, *expression.right);
    }
    assert(false);
    return {};
}

Expression eta_reduce(Expression function)
{
    assert(function.type == ExpressionTypeFunction);

    if (function.body->type == ExpressionTypeApplication
        && function.body->right->type == ExpressionTypeVariable
        && function.body->right->bounded_id == function.parameter_id
        && !has_usages(function.parameter_id, *function.body->left))
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
                return reduce(beta_reduce(expression.left->parameter_id, *expression.right, *expression.left->body));
            }
            return expression;
    }
    assert(false);
    return {};
}
