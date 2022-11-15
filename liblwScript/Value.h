#pragma once
#include <string>
#include <unordered_map>
namespace lws
{
    enum ValueType
    {
        VALUE_NULL,
        VALUE_INT,
        VALUE_REAL,
        VALUE_BOOL,
        VALUE_OBJECT,
    };

    enum ValueDesc
    {   
        DESC_VARIABLE,
        DESC_CONSTANT
    };

    struct Value
    {
        Value();
        Value(int64_t integer);
        Value(double number);
        Value(bool boolean);
        Value(struct Object *object);
        ~Value();

        ValueType Type() const;
        std::wstring Stringify() const;
        void Mark() const;
        void UnMark() const;

        ValueType type;
        ValueDesc desc;
        
        union
        {
            int64_t integer;
            double realnum;
            bool boolean;
            struct Object *object;
        };
    };

    bool operator==(const Value &left, const Value &right);
    bool operator!=(const Value &left, const Value &right);
}