#pragma once
#include "Token.h"
namespace lwscript
{
    enum class TypeKind
    {
        I8,
        U8,
        I16,
        U16,
        I32,
        U32,
        I64,
        U64,
        
        F32,
        F64,

        BOOL,
        CHAR,
        STRING,
        ANY,

        STRUCT,
        CLASS,
    };

    class Type
    {
    public:
        Type() noexcept;
        Type(STD_STRING_VIEW name, const SourceLocation &scl = {}) noexcept;
        ~Type() noexcept = default;

        TypeKind GetKind() const noexcept;

        bool IsNumeric() const noexcept;
        bool IsInteger() const noexcept;
        bool IsFloating() const noexcept;

        bool Is(TypeKind kind)
        {
            return mKind == kind;
        }

        STD_STRING_VIEW GetName() const noexcept;

        bool IsPrimitiveType() const noexcept;
        bool IsCompositeType() const noexcept;

    private:
        STD_STRING_VIEW mName;
        SourceLocation mSourceLocation;
        TypeKind mKind;
    };
};