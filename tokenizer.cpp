enum TokenType
{
    TokenTypeOpenParen,
    TokenTypeCloseParen,
    TokenTypeLambdaHeadStart,
    TokenTypeLambdaHeadEnd,
    TokenTypeNewLine,
    TokenTypeWhitespace,
    TokenTypeName,
};

struct Token
{
    TokenType type;

    union
    {
        // TokenTypeName
        struct { String name; };
    };
};

bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

bool is_name_start(char c)
{
    return c >= 'A' && c <= 'Z'
        || c >= 'a' && c <= 'z'
        || c == '_';
}

bool is_name_tail(char c)
{
    return c >= 'A' && c <= 'Z'
        || c >= 'a' && c <= 'z'
        || c >= '0' && c <= '9'
        || c == '_';
}

struct Tokenizer
{
    String source;
    u64 index;

    static Tokenizer construct(String source)
    {
        Tokenizer result;
        result.source = source;
        result.index = 0;
        return result;
    }

    bool is_done() { return index == source.size; }

    char current() { return source.data[index]; }

    Option<Token> tokenize_open_paren()
    {
        if (is_done() || current() != '(')
        {
            return Option<Token>::empty();
        }
        index++;
        Token token;
        token.type = TokenTypeOpenParen;
        return Option<Token>::construct(token);
    }

    Option<Token> tokenize_close_paren()
    {
        if (is_done() || current() != ')')
        {
            return Option<Token>::empty();
        }
        index++;
        Token token;
        token.type = TokenTypeCloseParen;
        return Option<Token>::construct(token);
    }

    Option<Token> tokenize_lambda_head_start()
    {
        if (is_done() || current() != '\\')
        {
            return Option<Token>::empty();
        }
        index++;
        Token token;
        token.type = TokenTypeLambdaHeadStart;
        return Option<Token>::construct(token);
    }

    Option<Token> tokenize_lambda_head_end()
    {
        if (is_done() || current() != '.')
        {
            return Option<Token>::empty();
        }
        index++;
        Token token;
        token.type = TokenTypeLambdaHeadEnd;
        return Option<Token>::construct(token);
    }

    Option<Token> tokenize_new_line()
    {
        if (is_done() || current() != '\n')
        {
            return Option<Token>::empty();
        }
        index++;
        Token token;
        token.type = TokenTypeNewLine;
        return Option<Token>::construct(token);
    }

    Option<Token> tokenize_whitespace()
    {
        if (is_done() || !is_whitespace(current()))
        {
            return Option<Token>::empty();
        }

        index++;
        while (!is_done() && is_whitespace(current())) { index++; }

        Token token;
        token.type = TokenTypeWhitespace;
        return Option<Token>::construct(token);
    }

    Option<Token> tokenize_name()
    {
        if (is_done() || !is_name_start(current()))
        {
            return Option<Token>::empty();
        }

        auto name = String::allocate();
        name.push(current());
        index++;

        while (!is_done() && is_name_tail(current()))
        {
            name.push(current());
            index++;
        }

        Token token;
        token.type = TokenTypeName;
        token.name = name;
        return Option<Token>::construct(token);
    }
};

struct TokenizationResult
{
    bool success;
    List<Token> tokens;
    u64 failed_at_index;
};

TokenizationResult tokenize(String source)
{
    auto tokenizer = Tokenizer::construct(source);

    auto tokens = List<Token>::allocate();
    while (!tokenizer.is_done())
    {
        Option<Token> maybe_token;

        maybe_token = tokenizer.tokenize_open_paren();
        if (maybe_token.has_data) { tokens.push(maybe_token.value); continue; }

        maybe_token = tokenizer.tokenize_close_paren();
        if (maybe_token.has_data) { tokens.push(maybe_token.value); continue; }

        maybe_token = tokenizer.tokenize_lambda_head_start();
        if (maybe_token.has_data) { tokens.push(maybe_token.value); continue; }

        maybe_token = tokenizer.tokenize_lambda_head_end();
        if (maybe_token.has_data) { tokens.push(maybe_token.value); continue; }

        maybe_token = tokenizer.tokenize_new_line();
        if (maybe_token.has_data) { tokens.push(maybe_token.value); continue; }

        maybe_token = tokenizer.tokenize_whitespace();
        if (maybe_token.has_data) { tokens.push(maybe_token.value); continue; }

        maybe_token = tokenizer.tokenize_name();
        if (maybe_token.has_data) { tokens.push(maybe_token.value); continue; }

        tokens.deallocate();

        TokenizationResult result;
        result.success = false;
        result.failed_at_index = tokenizer.index;
        return result;
    }

    TokenizationResult result;
    result.success = true;
    result.tokens = tokens;
    return result;
}
