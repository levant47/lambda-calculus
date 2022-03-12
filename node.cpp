enum NodeType
{
    NodeTypeVariable,
    NodeTypeFunction,
    NodeTypeApplication,
};

struct Node
{
    NodeType type;
    union
    {
        // NodeTypeVariable
        struct { String name; };
        // NodeTypeFunction
        struct { String parameter_name; Node* body; };
        // NodeTypeApplication
        struct { Node* left; Node* right; };
    };

    void print()
    {
        switch (type)
        {
            case NodeTypeVariable:
                name.print();
                return;
            case NodeTypeFunction:
                printf("\\ ");
                parameter_name.print();
                printf(" . ");
                body->print();
                return;
            case NodeTypeApplication:
                printf("(");
                left->print();
                printf(" ");
                right->print();
                printf(")");
                return;
        }
    }
};

