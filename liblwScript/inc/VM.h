#pragma once
#include <iostream>
#include <vector>
#include "Config.h"
#include "Chunk.h"
#include "Object.h"
#include "Library.h"
namespace lwscript
{
    struct CallFrame
    {
        ClosureObject *closure = nullptr;
        uint8_t *ip = nullptr;
        Value *slots = nullptr;
#ifdef USE_FUNCTION_CACHE
        std::vector<Value> arguments;
#endif
    };

#ifdef USE_FUNCTION_CACHE
    class FunctionReturnResultCache
    {
    public:
        void Set(const std::wstring& name, const std::vector<Value>& arguments, const std::vector<Value>& result);
		bool Get(const std::wstring& name, const std::vector<Value>& arguments, std::vector<Value>& result) const;
#ifdef PRINT_FUNCTION_CACHE
        void Print();
#endif
    private:
        std::unordered_map<std::wstring, ValueVecUnorderedMap> mCaches;
    };
#endif

    class LWSCRIPT_API VM
    {
    public:
        VM();
        ~VM();

        void ResetStatus();

        std::vector<Value> Run(FunctionObject *mainFunc);

    private:
        void Execute();

        bool IsFalsey(const Value &v);

        void Push(const Value &value);
        Value Pop();
        Value Peek(int32_t distance=0);

        template <class T, typename... Args>
        T *CreateObject(Args &&...params);

        template <class T>
        void FreeObject(T *object);
        void FreeObjects();

        UpValueObject *CaptureUpValue(Value *location);
        void ClosedUpValues(Value *end);

        void RegisterToGCRecordChain(const Value &value);
        void GC();
        void MarkRootObjects();
        void MarkGrayObjects();
        void Sweep();

        UpValueObject *mOpenUpValues;

        static Value sNullValue;

        Value mGlobalVariables[GLOBAL_VARIABLE_MAX];

        Value *mStackTop;
        Value mValueStack[STACK_MAX];

        CallFrame mFrames[STACK_MAX];
        int32_t mFrameCount;

#ifdef USE_FUNCTION_CACHE
        FunctionReturnResultCache mFunctionCache;
#endif

        friend struct Object;

        Object *mObjectChain;
        std::vector<Object *> mGrayObjects;
        size_t mBytesAllocated;
        size_t mNextGCByteSize;
    };

    template <class T, typename... Args>
    inline T *VM::CreateObject(Args &&...params)
    {
        T *object = new T(std::forward<Args>(params)...);
        object->next = mObjectChain;
        object->marked = false;
        mObjectChain = object;

        mBytesAllocated += sizeof(object);

        return object;
    }

    template <class T>
    inline void VM::FreeObject(T *object)
    {
#ifdef GC_DEBUG
        std::wcout << L"delete object(0x" << (void *)object << L")" << std::endl;
#endif
        mBytesAllocated -= sizeof(object);
        delete object;
    }

}