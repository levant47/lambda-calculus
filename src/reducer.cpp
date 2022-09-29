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

Result<Expression, String> reduce(Expression expression, u64 recursion_counter = 0)
{
    u64 recursion_limit = 300;
    if (recursion_counter == recursion_limit)
    {
        auto error = String::allocate();
        error.push("Recursion limit of ");
        error.push(recursion_limit);
        error.push(" reached");
        return Result<Expression, String>::fail(error);
    }
    recursion_counter++;

    Expression result;
    switch (expression.type)
    {
        case ExpressionTypeVariable:
        {
            result = copy(expression);
            break;
        }
        case ExpressionTypeFunction:
        {
            auto reducing_body_result = reduce(*expression.body, recursion_counter);
            if (!reducing_body_result.is_success) { return reducing_body_result; }
            auto reduced_body = copy_to_heap(reducing_body_result.value);
            expression.body = reduced_body;
            auto eta_reduced_expression = eta_reduce(expression);
            reduced_body->deallocate();
            default_deallocate(reduced_body);
            result = eta_reduced_expression;
            break;
        }
        case ExpressionTypeApplication:
        {
            auto reducing_left_result = reduce(*expression.left, recursion_counter);
            if (!reducing_left_result.is_success) { return reducing_left_result; }
            auto reducing_right_result = reduce(*expression.right, recursion_counter);
            if (!reducing_right_result.is_success)
            {
                reducing_left_result.value.deallocate();
                return reducing_right_result;
            }
            auto reduced_left = reducing_left_result.value;
            auto reduced_right = reducing_right_result.value;
            if (reduced_left.type == ExpressionTypeFunction)
            {
                auto beta_reduced = beta_reduce(0, reduced_right, *reduced_left.body);
                reduced_left.deallocate();
                reduced_right.deallocate();
                auto reducing_application_result = reduce(beta_reduced, recursion_counter);
                beta_reduced.deallocate();
                return reducing_application_result;
            }
            expression.left = copy_to_heap(reduced_left);
            expression.right = copy_to_heap(reduced_right);
            result = expression;
            break;
        }
        default: assert(false); return {};
    }
    return Result<Expression, String>::success(result);
}
