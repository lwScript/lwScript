#include "Library.h"
#include "Utils.h"

#include <stdio.h>
#include <ctime>
#include <iostream>
namespace lws
{
    LibraryManager &LibraryManager::Instance() noexcept
    {
        static LibraryManager instance;
        return instance;
    }

    LibraryManager::LibraryManager()
    {
        auto ioClass = new ClassObject(L"io");
        auto dsClass = new ClassObject(L"ds");
        auto memClass = new ClassObject(L"mem");
        auto timeClass = new ClassObject(L"time");

        ioClass->members[L"print"] = new NativeFunctionObject([](const std::vector<Value> &args, const Token* relatedToken) -> Value
                                                              {
                                                                  if (args.empty())
                                                                      return Value();

                                                                  if (args.size() == 1)
                                                                  {
                                                                      std::wcout << args[0].ToString();
                                                                      return Value();
                                                                  }

                                                                  if (!IS_STR_VALUE(args[0]))
                                                                  {
                                                                      for (const auto &arg : args)
                                                                          std::wcout << arg.ToString();
                                                                      return Value();
                                                                  }

                                                                  std::wstring content = TO_STR_VALUE(args[0]);

                                                                  if (args.size() != 1) // formatting output
                                                                  {
                                                                      size_t pos = content.find(L"{}");
                                                                      size_t argpos = 1;
                                                                      while (pos != std::wstring::npos)
                                                                      {
                                                                          if (argpos < args.size())
                                                                              content.replace(pos, 2, args[argpos++].ToString());
                                                                          else
                                                                              content.replace(pos, 2, L"null");
                                                                          pos = content.find(L"{}");
                                                                      }
                                                                  }

                                                                  size_t pos = content.find(L"\\n");
                                                                  while (pos != std::wstring::npos)
                                                                  {
                                                                      content[pos] = '\n';
                                                                      content.replace(pos + 1, 1, L""); // erase a char
                                                                      pos = content.find(L"\\n");
                                                                  }

                                                                  pos = content.find(L"\\t");
                                                                  while (pos != std::wstring::npos)
                                                                  {
                                                                      content[pos] = '\t';
                                                                      content.replace(pos + 1, 1, L""); // erase a char
                                                                      pos = content.find(L"\\t");
                                                                  }

                                                                  pos = content.find(L"\\r");
                                                                  while (pos != std::wstring::npos)
                                                                  {
                                                                      content[pos] = '\r';
                                                                      content.replace(pos + 1, 1, L""); // erase a char
                                                                      pos = content.find(L"\\r");
                                                                  }

                                                                  std::wcout << content;
                                                                  return Value();
                                                              });

        ioClass->members[L"println"] = new NativeFunctionObject([](const std::vector<Value> &args, const Token* relatedToken) -> Value
                                                                {
                                                                    if (args.empty())
                                                                        return Value();

                                                                    if (args.size() == 1)
                                                                    {
                                                                        std::wcout << args[0].ToString() << std::endl;
                                                                        return Value();
                                                                    }

                                                                    if (!IS_STR_VALUE(args[0]))
                                                                    {
                                                                        for (const auto &arg : args)
                                                                            std::wcout << arg.ToString() << std::endl;
                                                                        return Value();
                                                                    }

                                                                    std::wstring content = TO_STR_VALUE(args[0]);

                                                                    if (args.size() != 1) // formatting output
                                                                    {
                                                                        size_t pos = (int32_t)content.find(L"{}");
                                                                        size_t argpos = 1;
                                                                        while (pos != std::wstring::npos)
                                                                        {
                                                                            if (argpos < args.size())
                                                                                content.replace(pos, 2, args[argpos++].ToString());
                                                                            else
                                                                                content.replace(pos, 2, L"null");
                                                                            pos = content.find(L"{}");
                                                                        }
                                                                    }

                                                                    size_t pos = content.find(L"\\n");
                                                                    while (pos != std::wstring::npos)
                                                                    {
                                                                        content[pos] = '\n';
                                                                        content.replace(pos + 1, 1, L""); // erase a char
                                                                        pos = content.find(L"\\n");
                                                                    }

                                                                    pos = content.find(L"\\t");
                                                                    while (pos != std::wstring::npos)
                                                                    {
                                                                        content[pos] = '\t';
                                                                        content.replace(pos + 1, 1, L""); // erase a char
                                                                        pos = content.find(L"\\t");
                                                                    }

                                                                    pos = content.find(L"\\r");
                                                                    while (pos != std::wstring::npos)
                                                                    {
                                                                        content[pos] = L'\r';
                                                                        content.replace(pos + 1, 1, L""); // erase a char
                                                                        pos = content.find(L"\\r");
                                                                    }

                                                                    std::wcout << content << std::endl;
                                                                    return Value();
                                                                });

        dsClass->members[L"sizeof"] = new NativeFunctionObject([](const std::vector<Value> &args, const Token* relatedToken) -> Value
                                                               {
                                                                   if (args.empty() || args.size() > 1)
                                                                       Hint::Error(relatedToken, L"[Native function 'sizeof']:Expect a argument.");

                                                                   if (IS_ARRAY_VALUE(args[0]))
                                                                       return Value((int64_t)TO_ARRAY_VALUE(args[0])->elements.size());
                                                                   else if (IS_DICT_VALUE(args[0]))
                                                                       return Value((int64_t)TO_DICT_VALUE(args[0])->elements.size());
                                                                   else if (IS_STR_VALUE(args[0]))
                                                                       return Value((int64_t)TO_STR_VALUE(args[0]).size());
                                                                   else
                                                                       Hint::Error(relatedToken, L"[Native function 'sizeof']:Expect a array,dict ot string argument.");

                                                                   return Value();
                                                               });

        dsClass->members[L"insert"] = new NativeFunctionObject([](const std::vector<Value> &args,const Token* relatedToken) -> Value
                                                               {
                                                                   if (args.empty() || args.size() != 3)
                                                                       Hint::Error(relatedToken, L"[Native function 'insert']:Expect 3 arguments,the arg0 must be array,dict or string object.The arg1 is the index object.The arg2 is the value object.");

                                                                   if (IS_ARRAY_VALUE(args[0]))
                                                                   {
                                                                       ArrayObject *array = TO_ARRAY_VALUE(args[0]);
                                                                       if (!IS_INT_VALUE(args[1]))
                                                                           Hint::Error(relatedToken, L"[Native function 'insert']:Arg1 must be integer type while insert to a array");

                                                                       int64_t iIndex = TO_INT_VALUE(args[1]);

                                                                       if (iIndex < 0 || iIndex >= (int64_t)array->elements.size())
                                                                           Hint::Error(relatedToken, L"[Native function 'insert']:Index out of array's range");

                                                                       array->elements.insert(array->elements.begin() + iIndex, 1, args[2]);
                                                                   }
                                                                   else if (IS_DICT_VALUE(args[0]))
                                                                   {
                                                                       DictObject *dict = TO_DICT_VALUE(args[0]);

                                                                       for (auto [key, value] : dict->elements)
                                                                           if (key == args[1])
                                                                               Hint::Error(relatedToken, L"[Native function 'insert']:Already exist value in the dict object of arg1" + args[1].ToString());

                                                                       dict->elements[args[1]] = args[2];
                                                                   }
                                                                   else if (IS_STR_VALUE(args[0]))
                                                                   {
                                                                       auto &string = TO_STR_VALUE(args[0]);
                                                                       if (!IS_INT_VALUE(args[1]))
                                                                           Hint::Error(relatedToken, L"[Native function 'insert']:Arg1 must be integer type while insert to a array");

                                                                       int64_t iIndex = TO_INT_VALUE(args[1]);

                                                                       if (iIndex < 0 || iIndex >= (int64_t)string.size())
                                                                           Hint::Error(relatedToken, L"[Native function 'insert']:Index out of array's range");

                                                                       string.insert(iIndex, args[2].ToString());
                                                                   }
                                                                   else
                                                                       Hint::Error(relatedToken, L"[Native function 'insert']:Expect a array,dict ot string argument.");

                                                                   return args[0];
                                                               });

        dsClass->members[L"erase"] = new NativeFunctionObject([](const std::vector<Value> &args, const Token* relatedToken) -> Value
                                                              {
                                                                  if (args.empty() || args.size() != 2)
                                                                      Hint::Error(relatedToken, L"[Native function 'erase']:Expect 2 arguments,the arg0 must be array,dict or string object.The arg1 is the corresponding index object.");

                                                                  if (IS_ARRAY_VALUE(args[0]))
                                                                  {
                                                                      ArrayObject *array = TO_ARRAY_VALUE(args[0]);
                                                                      if (!IS_INT_VALUE(args[1]))
                                                                          Hint::Error(relatedToken, L"[Native function 'erase']:Arg1 must be integer type while insert to a array");

                                                                      int64_t iIndex = TO_INT_VALUE(args[1]);

                                                                      if (iIndex < 0 || iIndex >= (int64_t)array->elements.size())
                                                                          Hint::Error(relatedToken, L"[Native function 'erase']:Index out of array's range");

                                                                      array->elements.erase(array->elements.begin() + iIndex);
                                                                  }
                                                                  else if (IS_DICT_VALUE(args[0]))
                                                                  {
                                                                      DictObject *dict = TO_DICT_VALUE(args[0]);

                                                                      bool hasValue = false;

                                                                      for (auto it = dict->elements.begin(); it != dict->elements.end(); ++it)
                                                                          if (it->first == args[1])
                                                                          {
                                                                              dict->elements.erase(it);
                                                                              hasValue = true;
                                                                              break;
                                                                          }

                                                                      if (!hasValue)
                                                                          Hint::Error(relatedToken, L"[Native function 'erase']:No corresponding index in dict.");
                                                                  }
                                                                  else if (IS_STR_VALUE(args[0]))
                                                                  {
                                                                      auto &string = TO_STR_VALUE(args[0]);
                                                                      if (!IS_INT_VALUE(args[1]))
                                                                          Hint::Error(relatedToken, L"[Native function 'erase']:Arg1 must be integer type while insert to a array");

                                                                      int64_t iIndex = TO_INT_VALUE(args[1]);

                                                                      if (iIndex < 0 || iIndex >= (int64_t)string.size())
                                                                          Hint::Error(relatedToken, L"[Native function 'erase']:Index out of array's range");

                                                                      string.erase(string.begin() + iIndex);
                                                                  }
                                                                  else
                                                                      Hint::Error(relatedToken, L"[Native function 'erase']:Expect a array,dict ot string argument.");
                                                                  return args[0];
                                                              });

        memClass->members[L"addressof"] = new NativeFunctionObject([](const std::vector<Value> &args, const Token* relatedToken) -> Value
                                                                   {
                                                                       if (args.empty() || args.size() != 1)
                                                                           Hint::Error(relatedToken, L"[Native function 'addressof']:Expect 1 arguments.");

                                                                       if (!IS_OBJECT_VALUE(args[0]))
                                                                           Hint::Error(relatedToken, L"[Native function 'addressof']:The arg0 is a value,only object has address.");

                                                                       return new StrObject(PointerAddressToString(args[0].object));
                                                                   });

        timeClass->members[L"clock"] = new NativeFunctionObject([](const std::vector<Value> &args, const Token* relatedToken) -> Value
                                                                { return Value((double)clock() / CLOCKS_PER_SEC); });

        mStdLibraries.emplace_back(ioClass);
        mStdLibraries.emplace_back(dsClass);
        mStdLibraries.emplace_back(memClass);
        mStdLibraries.emplace_back(timeClass);

        mStdLibraryMap =
            {
                L"io",
                L"ds",
                L"mem",
                L"time"};
    }
    LibraryManager::~LibraryManager()
    {
    }
}