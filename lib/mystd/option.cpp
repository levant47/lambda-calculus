template <typename T>
struct Option
{
    bool has_data;
    T value;

    static Option<T> empty()
    {
        Option<T> result;
        result.has_data = false;
        return result;
    }

    static Option<T> construct(T value)
    {
        Option<T> result;
        result.has_data = true;
        result.value = value;
        return result;
    }

    T unwrap(CStringView error = nullptr)
    {
        assert(has_data, error ? error : "Option::unwrap failed");
        return value;
    }
};

template <typename TResult, typename TError>
struct Result
{
    bool is_success;
    union
    {
        TResult value;
        TError error;
    };

    static Result<TResult, TError> fail(TError error)
    {
        Result<TResult, TError> result;
        result.is_success = false;
        result.error = error;
        return result;
    }

    static Result<TResult, TError> success(TResult value)
    {
        Result<TResult, TError> result;
        result.is_success = true;
        result.value = value;
        return result;
    }
};
