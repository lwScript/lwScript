#include "Library.h"
#include "VM.h"
#include <stdio.h>
namespace lws
{

	Library::Library(VM* vm)
		: mVMHandle(vm)
	{
	}
	Library::~Library()
	{
		std::unordered_map<std::wstring, std::function<Object* (std::vector<Object*>)>>().swap(mNativeFunctions);
	}

	void Library::AddNativeFunction(std::wstring_view name, std::function<Object* (std::vector<Object*>)> fn)
	{
		auto iter = mNativeFunctions.find(name.data());
		if (iter != mNativeFunctions.end())
			Assert(std::wstring(L"Already exists native function:") + name.data());
		mNativeFunctions[name.data()] = fn;
	}
	std::function<Object* (std::vector<Object*>)> Library::GetNativeFunction(std::wstring_view fnName)
	{
		auto iter = mNativeFunctions.find(fnName.data());
		if (iter != mNativeFunctions.end())
			return iter->second;
		Assert(std::wstring(L"No native function:") + fnName.data());

		return nullptr;
	}
	bool Library::HasNativeFunction(std::wstring_view name)
	{
		auto iter = mNativeFunctions.find(name.data());
		if (iter != mNativeFunctions.end())
			return true;
		return false;
	}

	IO::IO(VM* vm)
		: Library(vm)
	{
		mNativeFunctions[L"print"] = [this](std::vector<Object*> args) -> Object*
		{
			if (args.empty())
				return nullptr;

			if (args.size() == 1)
			{
				std::wcout << args[0]->Stringify();
				return nullptr;
			}

			if (args[0]->Type() != OBJECT_STR)
			{
				for (const auto& arg : args)
					std::wcout << arg->Stringify();
				return nullptr;
			}

			std::wstring content = TO_STR_OBJ(args[0])->value;

			if (args.size() != 1) //formatting output
			{
				size_t pos = content.find(L"{}");
				size_t argpos = 1;
				while (pos != std::wstring::npos)
				{
					if (argpos < args.size())
						content.replace(pos, 2, args[argpos++]->Stringify());
					else
						content.replace(pos, 2, L"null");
					pos = content.find(L"{}");
				}
			}

			size_t pos = content.find(L"\\n");
			while (pos != std::wstring::npos)
			{
				content[pos] = '\n';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\n");
			}

			pos = content.find(L"\\t");
			while (pos != std::wstring::npos)
			{
				content[pos] = '\t';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\t");
			}

			pos = content.find(L"\\r");
			while (pos != std::wstring::npos)
			{
				content[pos] = '\r';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\r");
			}

			std::wcout << content;
			return nullptr;
		};

		mNativeFunctions[L"println"] = [this](std::vector<Object*> args) -> Object*
		{
			if (args.empty())
				return nullptr;

			if (args.size() == 1)
			{
				std::wcout << args[0]->Stringify() << std::endl;
				return nullptr;
			}

			if (args[0]->Type() != OBJECT_STR)
			{
				for (const auto& arg : args)
					std::wcout << arg->Stringify() << std::endl;
				return nullptr;
			}

			std::wstring content = TO_STR_OBJ(args[0])->value;

			if (args.size() != 1) //formatting output
			{
				size_t pos = (int32_t)content.find(L"{}");
				size_t argpos = 1;
				while (pos != std::wstring::npos)
				{
					if (argpos < args.size())
						content.replace(pos, 2, args[argpos++]->Stringify());
					else
						content.replace(pos, 2, L"null");
					pos = content.find(L"{}");
				}
			}

			size_t pos = content.find(L"\\n");
			while (pos != std::wstring::npos)
			{
				content[pos] = '\n';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\n");
			}

			pos = content.find(L"\\t");
			while (pos != std::wstring::npos)
			{
				content[pos] = '\t';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\t");
			}

			pos = content.find(L"\\r");
			while (pos != std::wstring::npos)
			{
				content[pos] = L'\r';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\r");
			}

			std::wcout << content << std::endl;
			return nullptr;
		};
	}

	DataStructure::DataStructure(VM* vm)
		: Library(vm)
	{
		mNativeFunctions[L"sizeof"] = [this](std::vector<Object*> args) -> Object*
		{
			if (args.empty() || args.size() > 1)
				Assert(L"[Native function 'sizeof']:Expect a argument.");

			if (IS_ARRAY_OBJ(args[0]))
				return mVMHandle->CreateIntNumObject(TO_ARRAY_OBJ(args[0])->elements.size());
			else if (IS_TABLE_OBJ(args[0]))
				return mVMHandle->CreateIntNumObject(TO_TABLE_OBJ(args[0])->elements.size());
			else if (IS_STR_OBJ(args[0]))
				return mVMHandle->CreateIntNumObject(TO_STR_OBJ(args[0])->value.size());
			else
				Assert(L"[Native function 'sizeof']:Expect a array,table ot string argument.");
			return nullptr;
		};

		mNativeFunctions[L"insert"] = [this](std::vector<Object*> args) -> Object*
		{
			if (args.empty() || args.size() != 3)
				Assert(L"[Native function 'insert']:Expect 3 arguments,the arg0 must be array,table or string object.The arg1 is the index object.The arg2 is the value object.");

			if (IS_ARRAY_OBJ(args[0]))
			{
				ArrayObject* array = TO_ARRAY_OBJ(args[0]);
				if (!IS_INT_OBJ(args[1]))
					Assert(L"[Native function 'insert']:Arg1 must be integer type while insert to a array");

				int64_t iIndex = TO_INT_OBJ(args[1])->value;

				if (iIndex < 0 || iIndex >= (int64_t)array->elements.size())
					Assert(L"[Native function 'insert']:Index out of array's range");

				array->elements.insert(array->elements.begin() + iIndex, 1, args[2]);
			}
			else if (IS_TABLE_OBJ(args[0]))
			{
				TableObject* table = TO_TABLE_OBJ(args[0]);

				for (auto [key, value] : table->elements)
					if (key->IsEqualTo(args[1]))
						Assert(L"[Native function 'insert']:Already exist value in the table object of arg1" + args[1]->Stringify());

				table->elements[args[1]] = args[2];
			}
			else if (IS_STR_OBJ(args[0]))
			{
				StrObject* string = TO_STR_OBJ(args[0]);
				if (!IS_INT_OBJ(args[1]))
					Assert(L"[Native function 'insert']:Arg1 must be integer type while insert to a array");

				int64_t iIndex = TO_INT_OBJ(args[1])->value;

				if (iIndex < 0 || iIndex >= (int64_t)string->value.size())
					Assert(L"[Native function 'insert']:Index out of array's range");

				string->value.insert(iIndex, args[2]->Stringify());
			}
			else
				Assert(L"[Native function 'insert']:Expect a array,table ot string argument.");
			return nullptr;
		};

		mNativeFunctions[L"erase"] = [this](std::vector<Object*> args) -> Object*
		{
			if (args.empty() || args.size() != 2)
				Assert(L"[Native function 'erase']:Expect 2 arguments,the arg0 must be array,table or string object.The arg1 is the corresponding index object.");

			if (IS_ARRAY_OBJ(args[0]))
			{
				ArrayObject* array = TO_ARRAY_OBJ(args[0]);
				if (!IS_INT_OBJ(args[1]))
					Assert(L"[Native function 'erase']:Arg1 must be integer type while insert to a array");

				int64_t iIndex = TO_INT_OBJ(args[1])->value;

				if (iIndex < 0 || iIndex >= (int64_t)array->elements.size())
					Assert(L"[Native function 'erase']:Index out of array's range");

				array->elements.erase(array->elements.begin() + iIndex);
			}
			else if (IS_TABLE_OBJ(args[0]))
			{
				TableObject* table = TO_TABLE_OBJ(args[0]);

				bool hasValue = false;

				for (auto it = table->elements.begin(); it != table->elements.end(); ++it)
					if (it->first->IsEqualTo(args[1]))
					{
						table->elements.erase(it);
						hasValue = true;
						break;
					}

				if (!hasValue)
					Assert(L"[Native function 'erase']:No corresponding index in table.");
			}
			else if (IS_STR_OBJ(args[0]))
			{
				StrObject* string = TO_STR_OBJ(args[0]);
				if (!IS_INT_OBJ(args[1]))
					Assert(L"[Native function 'erase']:Arg1 must be integer type while insert to a array");

				int64_t iIndex = TO_INT_OBJ(args[1])->value;

				if (iIndex < 0 || iIndex >= (int64_t)string->value.size())
					Assert(L"[Native function 'erase']:Index out of array's range");

				string->value.erase(string->value.begin() + iIndex);
			}
			else
				Assert(L"[Native function 'erase']:Expect a array,table ot string argument.");
			return nullptr;
		};
	}

	Memory::Memory(VM *vm)
		: Library(vm)
	{
		mNativeFunctions[L"addressof"] = [this](std::vector<Object *> args) -> Object *
		{
			if (args.empty() || args.size() != 1)
				Assert(L"[Native function 'addressof']:Expect 1 arguments.");

			return mVMHandle->CreateStrObject(PointerAddressToString(args[0]));
		};
	}

	std::unordered_map<std::wstring, Library*> LibraryManager::mLibraries;

	void LibraryManager::RegisterLibrary(std::wstring_view name, Library* lib)
	{
		auto iter = mLibraries.find(name.data());
		if (iter != mLibraries.end())
			Assert(L"Already exists a native function library:" + std::wstring(name));
		mLibraries[name.data()] = lib;
	}
	bool LibraryManager::HasNativeFunction(std::wstring_view name)
	{
		for (const auto& lib : mLibraries)
			if (lib.second->HasNativeFunction(name))
				return true;
		return false;
	}
}