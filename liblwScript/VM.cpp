#include "VM.h"
#include <iostream>
#include "Utils.h"
#include "Object.h"
namespace lws
{
	Value VM::sNullValue = Value();

	VM::VM()
	{
		ResetStatus();
	}
	VM::~VM()
	{
	}

	void VM::ResetStatus()
	{
		mFrameCount = 0;
		mStackTop = mValueStack;
		objectChain = nullptr;

		for (int32_t i = 0; i < mLibraryManager.mLibraries.size(); ++i)
			mGlobalVariables[i] = mLibraryManager.mLibraries[i];
	}

	std::vector<Value> VM::Run(FunctionObject *mainFunc)
	{
		ResetStatus();

		Push(mainFunc);

		CallFrame *mainCallFrame = &mFrames[mFrameCount++];
		mainCallFrame->function = mainFunc;
		mainCallFrame->ip = mainFunc->chunk.opCodes.data();
		mainCallFrame->slots = mStackTop - 1;

		Execute();

		std::vector<Value> returnValues;

		while (mStackTop != mValueStack + 1)
			returnValues.emplace_back(Pop());

		Pop();

		return returnValues;
	}

	void VM::Execute()
	{
		//  - * /
#define COMMON_BINARY(op)                                                                  \
	do                                                                                     \
	{                                                                                      \
		Value right = Pop();                                                               \
		Value left = Pop();                                                                \
		if (IS_REF_VALUE(left))                                                            \
			left = *TO_REF_VALUE(left)->pointer;                                           \
		if (IS_REF_VALUE(right))                                                           \
			right = *TO_REF_VALUE(right)->pointer;                                         \
		if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                                     \
			Push(TO_INT_VALUE(left) op TO_INT_VALUE(right));                               \
		else if (IS_REAL_VALUE(left) && IS_REAL_VALUE(right))                              \
			Push(TO_REAL_VALUE(left) op TO_REAL_VALUE(right));                             \
		else if (IS_INT_VALUE(left) && IS_REAL_VALUE(right))                               \
			Push(TO_INT_VALUE(left) op TO_REAL_VALUE(right));                              \
		else if (IS_REAL_VALUE(left) && IS_INT_VALUE(right))                               \
			Push(TO_REAL_VALUE(left) op TO_INT_VALUE(right));                              \
		else                                                                               \
			ASSERT(L"Invalid binary op:" + left.Stringify() + (L#op) + right.Stringify()); \
	} while (0);

// & | << >>
#define INTEGER_BINARY(op)                                                                 \
	do                                                                                     \
	{                                                                                      \
		Value right = Pop();                                                               \
		Value left = Pop();                                                                \
		if (IS_REF_VALUE(left))                                                            \
			left = *TO_REF_VALUE(left)->pointer;                                           \
		if (IS_REF_VALUE(right))                                                           \
			right = *TO_REF_VALUE(right)->pointer;                                         \
		if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                                     \
			Push(TO_INT_VALUE(left) op TO_INT_VALUE(right));                               \
		else                                                                               \
			ASSERT(L"Invalid binary op:" + left.Stringify() + (L#op) + right.Stringify()); \
	} while (0);

// > <
#define COMPARE_BINARY(op)                                                    \
	do                                                                        \
	{                                                                         \
		Value right = Pop();                                                  \
		Value left = Pop();                                                   \
		if (IS_REF_VALUE(left))                                               \
			left = *TO_REF_VALUE(left)->pointer;                              \
		if (IS_REF_VALUE(right))                                              \
			right = *TO_REF_VALUE(right)->pointer;                            \
		if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                        \
			Push(TO_INT_VALUE(left) op TO_INT_VALUE(right) ? true : false);   \
		else if (IS_REAL_VALUE(left) && IS_REAL_VALUE(right))                 \
			Push(TO_REAL_VALUE(left) op TO_REAL_VALUE(right) ? true : false); \
		else if (IS_INT_VALUE(left) && IS_REAL_VALUE(right))                  \
			Push(TO_INT_VALUE(left) op TO_REAL_VALUE(right) ? true : false);  \
		else if (IS_REAL_VALUE(left) && IS_INT_VALUE(right))                  \
			Push(TO_REAL_VALUE(left) op TO_INT_VALUE(right) ? true : false);  \
		else                                                                  \
			Push(false);                                                      \
	} while (0);

// && ||
#define LOGIC_BINARY(op)                                                                    \
	do                                                                                      \
	{                                                                                       \
		Value right = Pop();                                                                \
		Value left = Pop();                                                                 \
		if (IS_REF_VALUE(left))                                                             \
			left = *TO_REF_VALUE(left)->pointer;                                            \
		if (IS_REF_VALUE(right))                                                            \
			right = *TO_REF_VALUE(right)->pointer;                                          \
		if (IS_BOOL_VALUE(left) && IS_BOOL_VALUE(right))                                    \
			Push(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false)); \
		else                                                                                \
			ASSERT("Invalid op:" + left.Stringify() + (L#op) + right.Stringify());          \
	} while (0);

		CallFrame *frame = &mFrames[mFrameCount - 1];

		while (1)
		{
			auto instruction = *frame->ip++;
			switch (instruction)
			{
			case OP_RETURN:
			{
				auto retCount = *frame->ip++;
				Value *retValues;
				retValues = mStackTop - retCount;

				mFrameCount--;
				if (mFrameCount == 0)
					return;

				mStackTop = frame->slots;

				if (retCount == 0)
					Push(sNullValue);
				else
				{
					uint8_t i = 0;
					while (i < retCount)
					{
						auto value = *(retValues + i);
						Push(value);
						i++;
					}
				}

				frame = &mFrames[mFrameCount - 1];
				break;
			}
			case OP_CONSTANT:
			{
				auto pos = *frame->ip++;
				Push(frame->function->chunk.constants[pos]);
				break;
			}
			case OP_NULL:
			{
				Push(sNullValue);
				break;
			}
			case OP_SET_GLOBAL:
			{
				auto pos = *frame->ip++;
				auto v = Pop();
				if (IS_REF_VALUE(mGlobalVariables[pos]))
					*TO_REF_VALUE(mGlobalVariables[pos])->pointer = v;
				else
					mGlobalVariables[pos] = v;
				break;
			}
			case OP_GET_GLOBAL:
			{
				auto pos = *frame->ip++;
				Push(mGlobalVariables[pos]);
				break;
			}
			case OP_SET_LOCAL:
			{
				auto pos = *frame->ip++;
				auto value = Peek(0);

				auto slot = frame->slots + pos;

				if (IS_REF_VALUE((*slot)))
					*TO_REF_VALUE((*slot))->pointer = value;
				else
					*slot = value; // now assume base ptr on the stack bottom
				break;
			}
			case OP_GET_LOCAL:
			{
				auto pos = *frame->ip++;
				Push(frame->slots[pos]); // now assume base ptr on the stack bottom
				break;
			}
			case OP_ADD:
			{
				Value left = Pop();
				Value right = Pop();
				if (IS_REF_VALUE(left))
					left = *TO_REF_VALUE(left)->pointer;
				if (IS_REF_VALUE(right))
					right = *TO_REF_VALUE(right)->pointer;
				if (IS_INT_VALUE(left) && IS_INT_VALUE(right))
					Push(TO_INT_VALUE(left) + TO_INT_VALUE(right));
				else if (IS_REAL_VALUE(left) && IS_REAL_VALUE(right))
					Push(TO_REAL_VALUE(left) + TO_REAL_VALUE(right));
				else if (IS_INT_VALUE(left) && IS_REAL_VALUE(right))
					Push(TO_INT_VALUE(left) + TO_REAL_VALUE(right));
				else if (IS_REAL_VALUE(left) && IS_INT_VALUE(right))
					Push(TO_REAL_VALUE(left) + TO_INT_VALUE(right));
				else if (IS_STR_VALUE(left) && IS_STR_VALUE(right))
					Push(CreateObject<StrObject>(TO_STR_VALUE(left) + TO_STR_VALUE(right)));
				else
					ASSERT(L"Invalid binary op:" + left.Stringify() + L" + " + right.Stringify());
				break;
			}
			case OP_SUB:
			{
				COMMON_BINARY(-);
				break;
			}
			case OP_MUL:
			{
				COMMON_BINARY(*);
				break;
			}
			case OP_DIV:
			{
				COMMON_BINARY(/);
				break;
			}
			case OP_MOD:
			{
				INTEGER_BINARY(%);
				break;
			}
			case OP_BIT_AND:
			{
				INTEGER_BINARY(&);
				break;
			}
			case OP_BIT_OR:
			{
				INTEGER_BINARY(|);
				break;
			}
			case OP_BIT_LEFT_SHIFT:
			{
				INTEGER_BINARY(<<);
				break;
			}
			case OP_BIT_RIGHT_SHIFT:
			{
				INTEGER_BINARY(>>);
				break;
			}
			case OP_LESS:
			{
				COMPARE_BINARY(<);
				break;
			}
			case OP_GREATER:
			{
				COMPARE_BINARY(>);
				break;
			}
			case OP_NOT:
			{
				auto value = Pop();
				if (IS_REF_VALUE(value))
					value = *TO_REF_VALUE(value)->pointer;
				if (!IS_BOOL_VALUE(value))
					ASSERT(L"Invalid op:! " + value.Stringify() + L", only !(bool expr) is available.");
				Push(!TO_BOOL_VALUE(value));
				break;
			}
			case OP_EQUAL:
			{
				Value left = Pop();
				Value right = Pop();
				if (IS_REF_VALUE(left))
					left = *TO_REF_VALUE(left)->pointer;
				if (IS_REF_VALUE(right))
					right = *TO_REF_VALUE(right)->pointer;
				Push(left == right);
				break;
			}
			case OP_MINUS:
			{
				auto value = Pop();
				if (IS_REF_VALUE(value))
					value = *TO_REF_VALUE(value)->pointer;
				if (IS_INT_VALUE(value))
					Push(-TO_INT_VALUE(value));
				else if (IS_REAL_VALUE(value))
					Push(-TO_REAL_VALUE(value));
				else
					ASSERT(L"Invalid op:-" + value.Stringify() + L", only -(int||real expr) is available.");
				break;
			}
			case OP_FACTORIAL:
			{
				auto value = Pop();
				if (IS_REF_VALUE(value))
					value = *TO_REF_VALUE(value)->pointer;
				if (IS_INT_VALUE(value))
					Push(Factorial(TO_INT_VALUE(value)));
				else
					ASSERT(L"Invalid op:" + value.Stringify() + L"!, only (int expr)! is available.");
				break;
			}
			case OP_ARRAY:
			{
				auto count = *frame->ip++;

				std::vector<Value> elements;
				auto prePtr = mStackTop - count;
				for (auto i = 0; i < count; ++i)
					elements.emplace_back(*prePtr++);
				mStackTop -= count;
				auto arrayObject = CreateObject<ArrayObject>(elements);
				Push(arrayObject);
				break;
			}
			case OP_TABLE:
			{
				auto eCount = *frame->ip++;
				ValueUnorderedMap elements;
				for (int64_t i = 0; i < (int64_t)eCount; ++i)
				{
					auto key = Pop();
					auto value = Pop();
					elements[key] = value;
				}
				Push(CreateObject<TableObject>(elements));
				break;
			}
			case OP_GET_INDEX:
			{
				auto idxValue = Pop();
				auto dsValue = Pop();
				if (IS_ARRAY_VALUE(dsValue))
				{
					auto array = TO_ARRAY_VALUE(dsValue);
					if (!IS_INT_VALUE(idxValue))
						ASSERT("Invalid idx for array,only integer is available.");
					auto intIdx = TO_INT_VALUE(idxValue);
					if (intIdx < 0 || intIdx >= (int64_t)array->elements.size())
						ASSERT("Idx out of range.");
					Push(array->elements[intIdx]);
				}
				else if (IS_STR_VALUE(dsValue))
				{
					auto str = TO_STR_VALUE(dsValue);
					if (!IS_INT_VALUE(idxValue))
						ASSERT("Invalid idx for array,only integer is available.");
					auto intIdx = TO_INT_VALUE(idxValue);
					if (intIdx < 0 || intIdx >= (int64_t)str.size())
						ASSERT("Idx out of range.");
					Push(CreateObject<StrObject>(str.substr(intIdx, 1)));
				}
				else if (IS_TABLE_VALUE(dsValue))
				{
					auto table = TO_TABLE_VALUE(dsValue);

					auto iter = table->elements.find(idxValue);

					if (iter != table->elements.end())
						Push(iter->second);
					else
						ASSERT("No key in table.");
				}
				break;
			}
			case OP_SET_INDEX:
			{
				auto idxValue = Pop();
				auto dsValue = Pop();
				auto newValue = Pop();
				if (IS_ARRAY_VALUE(dsValue))
				{
					auto array = TO_ARRAY_VALUE(dsValue);
					if (!IS_INT_VALUE(idxValue))
						ASSERT(L"Invalid idx for array,only integer is available.");
					auto intIdx = TO_INT_VALUE(idxValue);
					if (intIdx < 0 || intIdx >= (int64_t)array->elements.size())
						ASSERT(L"Idx out of range.");
					array->elements[intIdx] = newValue;
				}
				else if (IS_STR_VALUE(dsValue))
				{
					auto str = TO_STR_VALUE(dsValue);
					if (!IS_INT_VALUE(idxValue))
						ASSERT(L"Invalid idx for array,only integer is available.");
					auto intIdx = TO_INT_VALUE(idxValue);
					if (intIdx < 0 || intIdx >= (int64_t)str.size())
						ASSERT("Idx out of range.");

					if (!IS_STR_VALUE(newValue))
						ASSERT(L"Cannot insert a non string clip:" + newValue.Stringify() + L" to string:" + str);

					str.append(TO_STR_VALUE(newValue), intIdx, TO_STR_VALUE(newValue).size());
				}
				else if (IS_TABLE_VALUE(dsValue))
				{
					auto table = TO_TABLE_VALUE(dsValue);
					table->elements[idxValue] = newValue;
				}
				break;
			}
			case OP_POP:
			{
				Pop();
				break;
			}
			case OP_JUMP_IF_FALSE:
			{
				uint16_t address = (*(frame->ip++) << 8) | (*(frame->ip++));
				if (IsFalsey(Peek(0)))
					frame->ip += address;
				break;
			}
			case OP_JUMP:
			{
				uint16_t address = (*(frame->ip++) << 8) | (*(frame->ip++));
				frame->ip += address;
				break;
			}
			case OP_LOOP:
			{
				uint16_t address = (*(frame->ip++) << 8) | (*(frame->ip++));
				frame->ip -= address;
				break;
			}
			case OP_REF_GLOBAL:
			{
				auto index = *frame->ip++;
				Push(CreateObject<RefObject>(&mGlobalVariables[index]));
				break;
			}
			case OP_REF_LOCAL:
			{
				auto index = *frame->ip++;
				Push(CreateObject<RefObject>(frame->slots + index));
				break;
			}
			case OP_CALL:
			{
				auto argCount = *frame->ip++;
				auto callee = Peek(argCount);
				if (IS_FUNCTION_VALUE(callee) || IS_CLASS_FUNCTION_BIND_VALUE(callee)) // normal function
				{

					if (IS_CLASS_FUNCTION_BIND_VALUE(callee))
					{
						auto binding = TO_CLASS_FUNCTION_BIND_VALUE(callee);
						mStackTop[-argCount - 1] = binding->receiver;
						callee = binding->function;
					}

					if (argCount != TO_FUNCTION_VALUE(callee)->arity)
						ASSERT(L"No matching argument count.");
					// init a new frame
					CallFrame *newframe = &mFrames[mFrameCount++];
					newframe->function = TO_FUNCTION_VALUE(callee);
					newframe->ip = newframe->function->chunk.opCodes.data();
					newframe->slots = mStackTop - argCount - 1;

					frame = &mFrames[mFrameCount - 1];
				}
				else if (IS_NATIVE_FUNCTION_VALUE(callee)) // native function
				{
					std::vector<Value> args(argCount);

					int32_t j = 0;
					for (Value *slot = mStackTop - argCount; slot < mStackTop && j < argCount; ++slot, ++j)
						args[j] = *slot;

					mStackTop -= argCount + 1;

					auto retV = TO_NATIVE_FUNCTION_VALUE(callee)->fn(args);
					Push(retV);
				}
				else
					ASSERT(L"Invalid callee,Only function is available.");
				break;
			}
			case OP_CLASS:
			{
				auto name = Pop();
				auto memCount = *frame->ip++;
				auto parentClassCount = *frame->ip++;

				auto classObj = CreateObject<ClassObject>();
				classObj->name = TO_STR_VALUE(name);

				for (int i = 0; i < parentClassCount; ++i)
				{
					name = Pop();
					auto parentClass = Pop();
					classObj->parents[TO_STR_VALUE(name)] = TO_CLASS_VALUE(parentClass);
				}

				for (int i = 0; i < memCount; ++i)
				{
					name = Pop();
					auto v = Pop();
					classObj->members[TO_STR_VALUE(name)] = v;
				}

				Push(classObj);
				break;
			}
			case OP_GET_PROPERTY:
			{
				if (!IS_CLASS_VALUE(Peek(1)))
					ASSERT(L"Invalid class call:not a valid class instance.");
				ClassObject *klass = TO_CLASS_VALUE(Peek(1));
				auto propName = TO_STR_VALUE(Pop());

				Value member;
				if (klass->GetMember(propName, member))
				{
					Pop(); //pop class object
					if (IS_FUNCTION_VALUE(member))
					{
						ClassFunctionBindObject *binding = CreateObject<ClassFunctionBindObject>(klass, TO_FUNCTION_VALUE(member));
						member = Value(binding);
					}
					Push(member);
					break;
				}
				else
					ASSERT(L"No member:" + propName + L" in class object" + klass->name);

				break;
			}
			case OP_SET_PROPERTY:
			{
				if (!IS_CLASS_VALUE(Peek(1)))
					ASSERT(L"Invalid class call:not a valid class instance.");
				auto propName = TO_STR_VALUE(Pop());
				auto klass = TO_CLASS_VALUE(Pop());
				klass->members[propName] = Peek(0);
				break;
			}
			case OP_GET_BASE:
			{
				if (!IS_CLASS_VALUE(Peek(1)))
					ASSERT(L"Invalid class call:not a valid class instance.");
				auto propName = TO_STR_VALUE(Pop());
				auto klass = TO_CLASS_VALUE(Pop());
				Value member;
				bool hasValue = klass->GetParentMember(propName, member);
				if (!hasValue)
					ASSERT(L"No member:" + propName + L"in class:" + klass->name + L" 's parent class(es).");
				Push(member);
				break;
			}
			default:
				break;
			}
		}
	}

	bool VM::IsFalsey(const Value &v)
	{
		return IS_NULL_VALUE(v) || (IS_BOOL_VALUE(v) && !TO_BOOL_VALUE(v));
	}

	void VM::Push(const Value &value)
	{
		*(mStackTop++) = value;
	}
	Value VM::Pop()
	{
		return *(--mStackTop);
	}

	Value VM::Peek(int32_t distance)
	{
		return *(mStackTop - distance - 1);
	}
}