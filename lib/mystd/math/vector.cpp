template <typename T>
struct Vector2
{
    T x;
    T y;

    static Vector2 construct(T x, T y)
    {
        Vector2 result;
        result.x = x;
        result.y = y;
        return result;
    }
};

template <typename T>
static void print(Vector2<T> vector)
{
    print("(", vector.x, ", ", vector.y, ")");
}

template <typename T>
static Vector2<T> operator+(Vector2<T> left, Vector2<T> right)
{
    Vector2<T> result;
    result.x = left.x + right.x;
    result.y = left.y + right.y;
    return result;
}

template <typename T>
static Vector2<T> operator+(Vector2<T> left, T right)
{
    return left + Vector2<T>::construct(right, right);
}

template <typename T>
static Vector2<T> operator+(T left, Vector2<T> right)
{
    return Vector2<T>::construct(left, left) + right;
}
