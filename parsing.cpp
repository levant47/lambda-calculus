struct ParsingState
{
    String source;
    uint64_t current_index;
};

struct ParsingResult
{
    bool is_successful;
    Node* syntax_tree;

    static ParsingResult success(Node* syntax_tree)
    {
        ParsingResult result;
        result.is_successful = true;
        result.syntax_tree = syntax_tree;
        return result;
    };

    static ParsingResult fail()
    {
        ParsingResult result;
        result.is_successful = false;
        return result;
    };
};

bool is_whitespace(char value)
{
    return value == ' ' || value == '\t' || value == '\n';
}

bool is_valid_name_character(char character)
{
    return character >= 'a' && character <= 'z'
        || character >= 'A' && character <= 'Z'
        || character == '_'
        || character >= '0' && character <= '9';
}

void skip_whitespace(ParsingState* state)
{
    while (state->current_index < state->source.size && is_whitespace(state->source.data[state->current_index]))
    {
        state->current_index++;
    }
}

ParsingResult parse_variable(ParsingState* state)
{
    if (state->current_index == state->source.size)
    {
        return ParsingResult::fail();
    }

    String name;
    name.data = state->source.data + state->current_index;
    name.size = 0;
    for (uint64_t i = state->current_index; i < state->source.size; i++)
    {
        if (is_valid_name_character(state->source.data[i]))
        {
            name.size++;
        }
        else
        {
            break;
        }
    }

    if (name.size == 0)
    {
        return ParsingResult::fail();
    }
    state->current_index += name.size;
    Node* variable = new Node;
    variable->type = NodeTypeVariable;
    variable->name = name;
    return ParsingResult::success(variable);
}

ParsingResult parse_lambda_calculus(ParsingState* state);

// app A B -> app B A
Node* mirror_applications(Node* node)
{
    if (node->type != NodeTypeApplication)
    {
        return node;
    }

    auto new_node = new Node;
    new_node->type = NodeTypeApplication;
    new_node->left = mirror_applications(node->right);
    new_node->right = mirror_applications(node->left);
    return new_node;
}

Node* left_prepend_applications(Node* tree, Node* new_leaf)
{
    if (tree->type != NodeTypeApplication)
    {
        auto new_branch = new Node;
        new_branch->type = NodeTypeApplication;
        new_branch->right = tree;
        new_branch->left = new_leaf;
        return new_branch;
    }

    auto new_branch = new Node;
    new_branch->type = NodeTypeApplication;
    new_branch->right = tree->right;
    new_branch->left = left_prepend_applications(tree->left, new_leaf);
    return new_branch;
}

// app (app c b) a -> app (app a b) c
Node* left_reverse_applications(Node* node)
{
    if (node->type != NodeTypeApplication)
    {
        return node;
    }

    return left_prepend_applications(left_reverse_applications(node->left), node->right);
}

ParsingResult parse_application(ParsingState* state)
{
    auto original_index = state->current_index;
    auto left_result = parse_variable(state);
    if (!left_result.is_successful)
    {
        state->current_index = original_index;
        return ParsingResult::fail();
    }

    skip_whitespace(state);

    auto right_result = parse_lambda_calculus(state);
    if (!right_result.is_successful)
    {
        state->current_index = original_index;
        return ParsingResult::fail();
    }

    // make sure the tree is left associative
    return ParsingResult::success(left_prepend_applications(right_result.syntax_tree, left_result.syntax_tree));
}

// TODO: support for multiple parameters in a single function declaration
ParsingResult parse_function(ParsingState* state)
{
    if (state->current_index == state->source.size || state->source.data[state->current_index] != '\\')
    {
        return ParsingResult::fail();
    }
    auto original_index = state->current_index;
    state->current_index++;

    skip_whitespace(state);

    String parameter_name;
    parameter_name.data = state->source.data + state->current_index;
    parameter_name.size = 0;
    for (uint64_t i = state->current_index; i < state->source.size; i++)
    {
        if (is_valid_name_character(state->source.data[i]))
        {
            parameter_name.size++;
        }
        else
        {
            break;
        }
    }

    if (parameter_name.size == 0)
    {
        state->current_index = original_index;
        return ParsingResult::fail();
    }
    state->current_index += parameter_name.size;

    skip_whitespace(state);

    if (state->current_index == state->source.size || state->source.data[state->current_index] != '.')
    {
        state->current_index = original_index;
        return ParsingResult::fail();
    }
    state->current_index++;

    skip_whitespace(state);

    auto body_parsing_result = parse_lambda_calculus(state);
    if (!body_parsing_result.is_successful)
    {
        state->current_index = original_index;
        return ParsingResult::fail();
    }

    auto function_node = new Node;
    function_node->type = NodeTypeFunction;
    function_node->parameter_name = parameter_name;
    function_node->body = body_parsing_result.syntax_tree;

    return ParsingResult::success(function_node);
}

ParsingResult parse_lambda_calculus(ParsingState* state)
{
    skip_whitespace(state);

    auto original_index = state->current_index;

    auto variable_parsing_result = parse_variable(state);
    if (variable_parsing_result.is_successful)
    {
        skip_whitespace(state);
        if (state->current_index == state->source.size)
        {
            return variable_parsing_result;
        }
        else
        {
            state->current_index = original_index;
        }
    }

    auto function_parsing_result = parse_function(state);
    if (function_parsing_result.is_successful)
    {
        skip_whitespace(state);
        if (state->current_index == state->source.size)
        {
            return function_parsing_result;
        }
        else
        {
            state->current_index = original_index;
        }
    }

    auto application_parsing_result = parse_application(state);
    if (application_parsing_result.is_successful)
    {
        skip_whitespace(state);
        if (state->current_index == state->source.size)
        {
            return application_parsing_result;
        }
        else
        {
            state->current_index = original_index;
        }
    }

    return ParsingResult::fail();
}

ParsingResult parse_lambda_calculus(String source)
{
    ParsingState state;
    state.source = source;
    state.current_index = 0;
    return parse_lambda_calculus(&state);
}

