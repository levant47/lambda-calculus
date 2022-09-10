template <typename T>
struct ListView
{
    u64 size;
    T* data;

    static ListView<T> construct(u64 size, T* data)
    {
        ListView result;
        result.size = size;
        result.data = data;
        return result;
    }
};

template <typename T>
struct List
{
    u64 capacity;
    u64 size;
    T* data;

    static const u64 DEFAULT_CAPACITY = 16;

    static List<T> allocate(u64 capacity = DEFAULT_CAPACITY)
    {
        List<T> result;
        result.capacity = capacity;
        result.size = 0;
        result.data = (T*)default_allocate(sizeof(T) * result.capacity);
        return result;
    }

    void deallocate()
    {
        assert(capacity != 0);
        default_deallocate(data);
        capacity = 0;
    }

    void push(T element)
    {
        if (size == capacity)
        {
            auto new_capacity = capacity * 2;
            data = (T*)default_reallocate(data, capacity * sizeof(T), new_capacity * sizeof(T));
            capacity = new_capacity;
        }
        data[size] = element;
        size++;
    }

    void push(ListView<T> source)
    {
        for (u64 i = 0; i < source.size; i++)
        {
            push(source.data[i]);
        }
    }

    void pop()
    {
        assert(size != 0);
        size--;
    }

    bool contains(T element)
    {
        for (u64 i = 0; i < size; i++)
        {
            if (data[i] == element)
            {
                return true;
            }
        }
        return false;
    }

    void clear()
    {
        size = 0;
    }
};
