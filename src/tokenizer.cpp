// had to prefix this enum with Lc because on Windows one of the system headers already defines TokenType
enum LcTokenType
{
    LcTokenTypeOpenParen,
    LcTokenTypeCloseParen,
    LcTokenTypeLambdaHeadStart,
    LcTokenTypeLambdaHeadEnd,
    LcTokenTypeWhitespace,
    LcTokenTypeName,
    LcTokenTypeEquals,
    LcTokenTypeSemicolon,
};

const char* to_string(LcTokenType token)
{
    switch (token)
    {
        case LcTokenTypeOpenParen: return "Opening parenthesis";
        case LcTokenTypeCloseParen: return "Closing parenthesis";
        case LcTokenTypeLambdaHeadStart: return "Lambda head start";
        case LcTokenTypeLambdaHeadEnd: return "Lambda head end";
        case LcTokenTypeWhitespace: return "Whitespace";
        case LcTokenTypeName: return "Name";
        case LcTokenTypeEquals: return "Equals sign";
        case LcTokenTypeSemicolon: return "Semicolon";
        default: return "Unknown token";
    }
}

struct Token
{
    LcTokenType type;

    union
    {
        // LcTokenTypeName
        struct { String name; };
    };
};

bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
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

    Option<Token> tokenize_open_paren() { return tokenize_character('(', LcTokenTypeOpenParen); }

    Option<Token> tokenize_close_paren() { return tokenize_character(')', LcTokenTypeCloseParen); }

    Option<Token> tokenize_lambda_head_start() { return tokenize_character('\\', LcTokenTypeLambdaHeadStart); }

    Option<Token> tokenize_lambda_head_end() { return tokenize_character('.', LcTokenTypeLambdaHeadEnd); }

    Option<Token> tokenize_equals() { return tokenize_character('=', LcTokenTypeEquals); }

    Option<Token> tokenize_semicolon() { return tokenize_character(';', LcTokenTypeSemicolon); }

    Option<Token> tokenize_whitespace()
    {
        if (is_done() || !is_whitespace(current()))
        {
            return Option<Token>::empty();
        }

        index++;
        while (!is_done() && is_whitespace(current())) { index++; }

        Token token;
        token.type = LcTokenTypeWhitespace;
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
        token.type = LcTokenTypeName;
        token.name = name;
        return Option<Token>::construct(token);
    }

private:
    Option<Token> tokenize_character(char target_character, LcTokenType target_type)
    {
        if (is_done() || current() != target_character) { return Option<Token>::empty(); }
        index++;
        Token token;
        token.type = target_type;
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

        maybe_token = tokenizer.tokenize_equals();
        if (maybe_token.has_data) { tokens.push(maybe_token.value); continue; }

        maybe_token = tokenizer.tokenize_semicolon();
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
