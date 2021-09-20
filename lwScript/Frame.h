#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "Object.h"
namespace lws
{
	enum OpCode
	{
		OP_PUSH,
		OP_POP,
		OP_NEG,
		OP_RETURN,
		OP_ADD,
		OP_SUB,
		OP_MUL,
		OP_DIV,
		OP_GT,
		OP_LE,
		OP_GTEQ,
		OP_LEEQ,
		OP_EQ,
		OP_NEQ,
		OP_OR,
		OP_AND,
		OP_GET_VAR,
		OP_SET_VAR,
		OP_DEFINE_VAR,
		OP_DEFINE_ARRAY,
		OP_GET_INDEX_VAR,
		OP_SET_INDEX_VAR,
		OP_ENTER_SCOPE,
		OP_EXIT_SCOPE,
		OP_JUMP,
		OP_JUMP_IF_FALSE,
		OP_FUNCTION_CALL,
	};

	class Frame
	{
	public:
		void AddOpCode(uint8_t code);
		size_t GetOpCodeSize() const;

		uint8_t AddObject(Object *object);

		void AddFunctionFrame(const Frame &frame);
		size_t GetFunctionFrameSize() const;

		std::string Stringify(int depth=0);

		void Clear();

	private:

		friend class VM;

		std::vector<uint8_t> m_Codes;
		std::vector<Object *> m_Objects;
		std::vector<Frame> m_FunctionFrames;
	};


	void Frame::AddOpCode(uint8_t code)
	{
		m_Codes.emplace_back(code);
	}

	size_t Frame::GetOpCodeSize() const
	{
		return m_Codes.size();
	}

	uint8_t Frame::AddObject(Object *object)
	{
		m_Objects.emplace_back(object);
		return (uint8_t)(m_Objects.size() - 1);
	}

	void Frame::AddFunctionFrame(const Frame &frame)
	{
		m_FunctionFrames.emplace_back(frame);
	}

			size_t Frame::GetFunctionFrameSize() const
			{
				return m_FunctionFrames.size();
			}


	std::string Frame::Stringify(int depth)
	{
		std::string interval;
		for(size_t i=0;i<depth;++i)
			interval+="\t";

#define SINGLE_INSTR_STRINGIFY(op) \
	result <<interval<<"\t" <<std::setfill('0') << std::setw(8) << i << "     " << (#op) << "\n"

#define CONSTANT_INSTR_STRINGIFY(op) \
	result <<interval<< "\t"<<std::setfill('0') << std::setw(8) << i << "     " << (#op) << "     " << std::to_string(m_Codes[++i]) << "    " << m_Objects[m_Codes[i]]->Stringify() << "\n"

		std::stringstream result;

		for(size_t i=0;i<m_FunctionFrames.size();++i)
			{
				result<<interval<<"Frame "<<i<<":\n";
				result<<m_FunctionFrames[i].Stringify(depth+1);
			}

		result <<interval<<"Frame root:\n"<<interval<< "OpCodes:\n";

		for (size_t i = 0; i < m_Codes.size(); ++i)
		{
			switch (m_Codes[i])
			{
			case OP_RETURN:
				SINGLE_INSTR_STRINGIFY(OP_RETURN);
				break;
			case OP_PUSH:
				CONSTANT_INSTR_STRINGIFY(OP_PUSH);
				break;
			case OP_POP:
				SINGLE_INSTR_STRINGIFY(OP_POP);
				break;
			case OP_NEG:
				SINGLE_INSTR_STRINGIFY(OP_NEG);
				break;
			case OP_ADD:
				SINGLE_INSTR_STRINGIFY(OP_ADD);
				break;
			case OP_SUB:
				SINGLE_INSTR_STRINGIFY(OP_SUB);
				break;
			case OP_MUL:
				SINGLE_INSTR_STRINGIFY(OP_MUL);
				break;
			case OP_DIV:
				SINGLE_INSTR_STRINGIFY(OP_DIV);
				break;
			case OP_GT:
				SINGLE_INSTR_STRINGIFY(OP_GT);
				break;
			case OP_LE:
				SINGLE_INSTR_STRINGIFY(OP_LE);
				break;
			case OP_GTEQ:
				SINGLE_INSTR_STRINGIFY(OP_GTEQ);
				break;
			case OP_LEEQ:
				SINGLE_INSTR_STRINGIFY(OP_LEEQ);
				break;
			case OP_EQ:
				SINGLE_INSTR_STRINGIFY(OP_EQ);
				break;
			case OP_AND:
				SINGLE_INSTR_STRINGIFY(OP_AND);
				break;
			case OP_OR:
				SINGLE_INSTR_STRINGIFY(OP_OR);
				break;
			case OP_GET_VAR:
				SINGLE_INSTR_STRINGIFY(OP_GET_VAR);
				break;
			case OP_DEFINE_VAR:
				SINGLE_INSTR_STRINGIFY(OP_DEFINE_VAR);
				break;
			case OP_SET_VAR:
				SINGLE_INSTR_STRINGIFY(OP_SET_VAR);
				break;
			case OP_DEFINE_ARRAY:
				SINGLE_INSTR_STRINGIFY(OP_DEFINE_ARRAY);
				break;
			case OP_GET_INDEX_VAR:
				SINGLE_INSTR_STRINGIFY(OP_GET_INDEX_VAR);
				break;
			case OP_SET_INDEX_VAR:
				SINGLE_INSTR_STRINGIFY(OP_SET_INDEX_VAR);
				break;
			case OP_ENTER_SCOPE:
				SINGLE_INSTR_STRINGIFY(OP_ENTER_SCOPE);
				break;
			case OP_EXIT_SCOPE:
				SINGLE_INSTR_STRINGIFY(OP_EXIT_SCOPE);
				break;
			case OP_JUMP:
				SINGLE_INSTR_STRINGIFY(OP_JUMP);
				break;
			case OP_JUMP_IF_FALSE:
				SINGLE_INSTR_STRINGIFY(OP_JUMP_IF_FALSE);
				break;
			case OP_FUNCTION_CALL:
				SINGLE_INSTR_STRINGIFY(OP_FUNCTION_CALL);
				break;
			default:
				SINGLE_INSTR_STRINGIFY(UNKNOWN);
				break;
			}
		}

		result << interval<<"Objects:\n";

		for (size_t i = 0; i < m_Objects.size(); ++i)
			result <<interval<<"\t" <<std::setfill('0') << std::setw(8) << i << "     " << m_Objects[i]->Stringify() << "\n";
		return result.str();
	}
	void Frame::Clear()
	{
		std::vector<uint8_t>().swap(m_Codes);
		std::vector<Object *>().swap(m_Objects);
		std::vector<Frame>().swap(m_FunctionFrames);
	}
} 
