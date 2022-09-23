Expression beta_reduce(u32 bound_index, Expression argument, Expression body)
{
    switch (body.type)
    {
        case ExpressionTypeVariable:
            if (!body.is_bound || body.bound_index < bound_index) { return copy(body); }
            if (body.bound_index == bound_index) { return copy(argument); }
            // else if (body.bound_index > bound_index)
            body.bound_index--;
            return copy(body);
        case ExpressionTypeFunction:
            body.parameter_name = body.parameter_name.copy();
            body.body = copy_to_heap(beta_reduce(bound_index + 1, argument, *body.body));
            return body;
        case ExpressionTypeApplication:
            body.left = copy_to_heap(beta_reduce(bound_index, argument, *body.left));
            body.right = copy_to_heap(beta_reduce(bound_index, argument, *body.right));
            return body;
        default: assert(false); return {};
    }
}

bool has_usages(u32 bound_index, Expression expression)
{
    switch (expression.type)
    {
        case ExpressionTypeVariable:
            return expression.is_bound && expression.bound_index == bound_index;
        case ExpressionTypeFunction:
            return has_usages(bound_index + 1, *expression.body);
        case ExpressionTypeApplication:
            return has_usages(bound_index, *expression.left)
                || has_usages(bound_index, *expression.right);
        default: assert(false); return {};
    }
}

Expression fix_bound_indices_after_eta_reduction(u32 bound_index, Expression source)
{
    switch (source.type)
    {
        case ExpressionTypeVariable:
            if (source.bound_index > bound_index) { source.bound_index--; }
            return copy(source);
        case ExpressionTypeFunction:
            source.parameter_name = source.parameter_name.copy();
            source.body = copy_to_heap(fix_bound_indices_after_eta_reduction(bound_index + 1, *source.body));
            return source;
        case ExpressionTypeApplication:
            source.left = copy_to_heap(fix_bound_indices_after_eta_reduction(bound_index, *source.left));
            source.right = copy_to_heap(fix_bound_indices_after_eta_reduction(bound_index, *source.right));
            return source;
        default: assert(false); return {};
    }
}

Expression eta_reduce(Expression function)
{
    assert(function.type == ExpressionTypeFunction);

    if (function.body->type == ExpressionTypeApplication
        && function.body->right->type == ExpressionTypeVariable
        && function.body->right->bound_index == 0
        && !has_usages(0, *function.body->left))
    {
        return fix_bound_indices_after_eta_reduction(0, *function.body->left);
    }
    return copy(function);
}

Expression reduce(Expression expression)
{
    switch (expression.type)
    {
        case ExpressionTypeVariable: return copy(expression);
        case ExpressionTypeFunction:
        {
            auto reduced_body = copy_to_heap(reduce(*expression.body));
            expression.body = reduced_body;
            auto eta_reduced_expression = eta_reduce(expression);
            reduced_body->deallocate();
            default_deallocate(reduced_body);
            return eta_reduced_expression;
        }
        case ExpressionTypeApplication:
        {
            auto reduced_left = reduce(*expression.left);
            auto reduced_right = reduce(*expression.right);
            if (reduced_left.type == ExpressionTypeFunction)
            {
                auto beta_reduced = beta_reduce(0, reduced_right, *reduced_left.body);
                reduced_left.deallocate();
                reduced_right.deallocate();
                auto reduced_application = reduce(beta_reduced);
                beta_reduced.deallocate();
                return reduced_application;
            }
            expression.left = copy_to_heap(reduced_left);
            expression.right = copy_to_heap(reduced_right);
            return expression;
        }
        default: assert(false); return {};
    }
}
