#pragma once
#include <string>
#include <array>
#include "Config.h"
#include "Value.h"
#include "Token.h"
namespace lws
{
    enum class SymbolType
    {
        GLOBAL,
        LOCAL,
        UPVALUE,
    };

    struct UpValue
    {
        uint8_t index = 0;
        uint8_t location = 0;
        uint8_t depth = -1;
    };

    struct FunctionSymbolInfo
    {
        int8_t paramCount = -1;
        uint8_t varArgParamType = 0;//0:no,1:varArgWithoutName(...),2:varArgWithName(...args)
    };

    struct Symbol
    {
        std::wstring name;
        SymbolType type = SymbolType::GLOBAL;
        ValueDesc descType = ValueDesc::VARIABLE;
        uint8_t index = 0;
        int8_t scopeDepth = -1;
        FunctionSymbolInfo functionSymInfo;
        UpValue upvalue; // available only while type is SymbolType::UPVALUE
        bool isCaptured = false;
        Token relatedToken;
    };
    class SymbolTable
    {
    public:
        SymbolTable();
        SymbolTable(SymbolTable *enclosing);

        Symbol Define(Token relatedToken, ValueDesc descType, const std::wstring &name,const FunctionSymbolInfo& functionInfo={});

        Symbol Resolve(Token relatedToken, const std::wstring &name, int8_t paramCount = -1 , int8_t d = 0);

        std::array<Symbol, UINT8_COUNT> mSymbols;
        uint8_t mSymbolCount;
        uint8_t mGlobalSymbolCount;
        uint8_t mLocalSymbolCount;
        std::array<UpValue, UINT8_COUNT> mUpValues;
        int32_t mUpValueCount;
        uint8_t mScopeDepth; // Depth of scope nesting(related to code {} scope)
        SymbolTable *enclosing;

    private:
        UpValue AddUpValue(Token relatedToken, uint8_t location, uint8_t depth);
        uint8_t mTableDepth; // Depth of symbol table nesting(related to symboltable's enclosing)
    };
}
