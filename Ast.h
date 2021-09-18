#pragma once
#include <string>

#include <unordered_map>
#include <vector>
#include <memory>
namespace lwScript
{
	enum class AstType
	{
		//expr
		NUM,
		STR,
		NIL,
		BOOL,
		IDENTIFIER,
		GROUP,
		FUNCTION,
		STRUCT,
		ARRAY,
		PREFIX,
		INFIX,
		INDEX,
		REF,
		FUNCTION_CALL,
		STRUCT_CALL,
		//stmt
		LET,
		EXPR,
		RETURN,
		IF,
		SCOPE,
		WHILE,
		ASTSTMTS,
	};

	struct AstNode
	{
		AstNode() {}
		virtual ~AstNode() {}

		virtual std::string Stringify() = 0;
		virtual AstType Type() = 0;
	};

	struct Expr
	{
		Expr() {}
		virtual ~Expr() {}

		virtual std::string Stringify() = 0;
		virtual AstType Type() = 0;
	};

	struct NumExpr : public Expr
	{
		NumExpr() : value(0.0) {}
		NumExpr(double value) : value(value) {}
		~NumExpr() {}

		std::string Stringify() override { return std::to_string(value); }
		AstType Type() override { return AstType::NUM; }

		double value;
	};

	struct StrExpr : public Expr
	{
		StrExpr() {}
		StrExpr(std::string_view str) : value(str) {}

		std::string Stringify() override { return value; }
		AstType Type() override { return AstType::STR; }

		std::string value;
	};

	struct NilExpr : public Expr
	{
		NilExpr() {}
		~NilExpr() {}

		std::string Stringify() override { return "nil"; }
		AstType Type() override { return AstType::NIL; }
	};

	struct BoolExpr : public Expr
	{
		BoolExpr():value(false) {}
		BoolExpr(bool value):value(value) {}
		~BoolExpr() {}

		std::string Stringify() override { return value ? "true" : "false"; }
		AstType Type() override { return AstType::BOOL; }
		bool value;
	};

	struct IdentifierExpr : public Expr
	{
		IdentifierExpr() {}
		IdentifierExpr(std::string_view literal) : literal(literal) {}
		~IdentifierExpr() {}

		std::string Stringify() override { return literal; }
		AstType Type() override { return AstType::IDENTIFIER; }

		std::string literal;
	};

	struct ArrayExpr : public Expr
	{
		ArrayExpr() {}
		ArrayExpr(std::vector<Expr *> elements) : elements(elements) {}
		~ArrayExpr()
		{
			std::vector<Expr *>().swap(elements);
		}

		std::string Stringify() override
		{
			std::string result = "[";

			if (!elements.empty())
			{
				for (auto e : elements)
					result += e->Stringify() + ",";
				result = result.substr(0, result.size() - 1);
			}
			result += "]";
			return result;
		}
		AstType Type() override { return AstType::ARRAY; }

		std::vector<Expr *> elements;
	};

	struct GroupExpr : public Expr
	{
		GroupExpr() {}
		GroupExpr(Expr *expr) : expr(expr) {}
		~GroupExpr() {}

		std::string Stringify() override { return "(" + expr->Stringify() + ")"; }
		AstType Type() override { return AstType::GROUP; }

		Expr *expr;
	};

	struct PrefixExpr : public Expr
	{
		PrefixExpr() : right(nullptr) {}
		PrefixExpr(std::string_view op, Expr *right) : op(op), right(right) {}
		~PrefixExpr()
		{
			delete right;
			right = nullptr;
		}

		std::string Stringify() override { return op + right->Stringify(); }
		AstType Type() override { return AstType::PREFIX; }

		std::string op;
		Expr *right;
	};

	struct InfixExpr : public Expr
	{
		InfixExpr() : left(nullptr), right(nullptr) {}
		InfixExpr(std::string_view op, Expr *left, Expr *right) : op(op), left(left), right(right) {}
		~InfixExpr()
		{
			delete left;
			left = nullptr;

			delete right;
			right = nullptr;
		}

		std::string Stringify() override { return left->Stringify() + op + right->Stringify(); }
		AstType Type() override { return AstType::INFIX; }

		std::string op;
		Expr *left;
		Expr *right;
	};


	struct IndexExpr : public Expr
	{
		IndexExpr() {}
		IndexExpr(Expr *array, Expr *index) : array(array), index(index) {}
		~IndexExpr()
		{
			delete array;
			array = nullptr;
			delete index;
			index = nullptr;
		}
		std::string Stringify() override { return array->Stringify() + "[" + index->Stringify() + "]"; }

		AstType Type() override { return AstType::INDEX; }

		Expr *array;
		Expr *index;
	};

	struct RefExpr :public Expr
	{
		RefExpr() {}
		RefExpr(Expr* expr) : expr(expr) {}
		~RefExpr()
		{
			delete expr;
			expr = nullptr;
		}
		std::string Stringify() override { return "ref "+expr->Stringify(); }

		AstType Type() override { return AstType::REF; }

		Expr* expr;
	};

	struct FunctionCallExpr : public Expr
	{
		FunctionCallExpr() {}
		FunctionCallExpr(IdentifierExpr*function, std::vector<Expr *> arguments) : function(function), arguments(arguments) {}
		~FunctionCallExpr() {}

		std::string Stringify() override
		{
			std::string result = function->Stringify() + "(";

			if (!arguments.empty())
			{
				for (const auto &arg : arguments)
					result += arg->Stringify() + ",";
				result = result.substr(0, result.size() - 1);
			}
			result += ")";
			return result;
		}
		AstType Type() override { return AstType::FUNCTION_CALL; }

		IdentifierExpr *function;
		std::vector<Expr *> arguments;
	};

	struct StructCallExpr :public Expr
	{
		StructCallExpr() {}
		StructCallExpr(IdentifierExpr* structure, Expr* member) : structure(structure), member(member) {}
		~StructCallExpr() {}

		std::string Stringify() override
		{
			return structure->Stringify() + "."+member->Stringify();

		}
		AstType Type() override { return AstType::STRUCT_CALL; }

		IdentifierExpr* structure;
		Expr* member;
	};


	struct Stmt : public AstNode
	{
		Stmt() {}
		virtual ~Stmt() {}

		virtual std::string Stringify() = 0;
		virtual AstType Type() = 0;
	};

	struct ExprStmt : public Stmt
	{
		ExprStmt() : expr(nullptr) {}
		ExprStmt(Expr *expr) : expr(expr) {}
		~ExprStmt()
		{
			delete expr;
			expr = nullptr;
		}

		std::string Stringify() override { return expr->Stringify() + ";"; }
		AstType Type() override { return AstType::EXPR; }

		Expr *expr;
	};

	struct LetStmt : public Stmt
	{
		LetStmt() {}
		LetStmt(std::unordered_map<IdentifierExpr *, Expr *> variables) : variables(variables) {}
		~LetStmt() { std::unordered_map<IdentifierExpr *, Expr *>().swap(variables); }

		std::string Stringify() override
		{
			std::string result = "let ";
			if (!variables.empty())
			{
				for (const auto &[key, value] : variables)
					result += key->Stringify() + "=" + value->Stringify() + ",";
				result = result.substr(0, result.size() - 1);
			}
			result += ";";
			return result;
		}
		AstType Type() override { return AstType::LET; }

		std::unordered_map<IdentifierExpr *, Expr *> variables;
	};

	struct ReturnStmt : public Stmt
	{
		ReturnStmt() : expr(nullptr) {}
		ReturnStmt(Expr *expr) : expr(expr) {}
		~ReturnStmt()
		{
			delete expr;
			expr = nullptr;
		}

		std::string Stringify() override { return "return " + expr->Stringify() + ";"; }
		AstType Type() override { return AstType::RETURN; }

		Expr *expr;
	};

	struct IfStmt : public Stmt
	{
		IfStmt() : condition(nullptr), thenBranch(nullptr), elseBranch(nullptr) {}
		IfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch)
			: condition(condition),
			  thenBranch(thenBranch),
			  elseBranch(elseBranch)
		{
		}
		~IfStmt()
		{
			delete condition;
			condition = nullptr;
			delete thenBranch;
			thenBranch = nullptr;
			delete elseBranch;
			elseBranch = nullptr;
		}

		std::string Stringify() override
		{
			std::string result;
			result = "if(" + condition->Stringify() + ")" + thenBranch->Stringify();
			if (elseBranch != nullptr)
				result +="else"+ elseBranch->Stringify();
			return result;
		}
		AstType Type() override { return AstType::IF; }

		Expr *condition;
		Stmt *thenBranch;
		Stmt *elseBranch;
	};

	struct ScopeStmt : public Stmt
	{
		ScopeStmt() {}
		ScopeStmt(std::vector<Stmt *> stmts) : stmts(stmts) {}
		~ScopeStmt() { std::vector<Stmt *>().swap(stmts); }

		std::string Stringify() override
		{
			std::string result = "{";
			for (const auto &stmt : stmts)
				result += stmt->Stringify();
			result += "}";
			return result;
		}

		AstType Type() override { return AstType::SCOPE; }
		std::vector<Stmt *> stmts;
	};

	struct FunctionExpr : public Expr
	{
		FunctionExpr() : body(nullptr) {}
		FunctionExpr(std::vector<IdentifierExpr *> parameters, ScopeStmt *body) : parameters(parameters), body(body) {}
		~FunctionExpr()
		{
			std::vector<IdentifierExpr *>().swap(parameters);
			delete body;
			body = nullptr;
		}

		std::string Stringify() override
		{
			std::string result = "fn(";
			if (!parameters.empty())
			{
				for (auto param : parameters)
					result += param->Stringify() + ",";
				result = result.substr(0, result.size() - 1);
			}
			result += ")";
			result += body->Stringify();
			return result;
		}
		AstType Type() override { return AstType::FUNCTION; }

		std::vector<IdentifierExpr *> parameters;
		ScopeStmt *body;
	};

	struct StructExpr : public Expr
	{
		StructExpr() {}
		StructExpr(std::vector<LetStmt*> letStmts) : letStmts(letStmts) {}
		~StructExpr()
		{
			std::vector<LetStmt*>().swap(letStmts);
		}

		std::string Stringify() override
		{
			std::string result = "struct {";
			if (!letStmts.empty())
			{
				for (auto letStmt : letStmts)
					result += letStmt->Stringify() + "\n";
				result = result.substr(0, result.size() - 1);
			}
			result += "}";
			return result;
		}
		AstType Type() override { return AstType::STRUCT; }

		std::vector<LetStmt*> letStmts;
	};

	struct WhileStmt : public Stmt
	{
		WhileStmt() : condition(nullptr), body(nullptr) {}
		WhileStmt(Expr *condition, Stmt *body)
			: condition(condition),
			  body(body)
		{
		}
		~WhileStmt()
		{
			delete condition;
			condition = nullptr;
			delete body;
			body = nullptr;
		}

		std::string Stringify() override
		{
			return "while(" + condition->Stringify() + ")" + body->Stringify();
		}
		AstType Type() override { return AstType::WHILE; }

		Expr *condition;
		Stmt *body;
	};

	struct AstStmts : public Stmt
	{
		AstStmts() {}
		AstStmts(std::vector<Stmt *> stmts) : stmts(stmts) {}
		~AstStmts() { std::vector<Stmt *>().swap(stmts); }

		std::string Stringify() override
		{
			std::string result;
			for (const auto &stmt : stmts)
				result += stmt->Stringify();
			return result;
		}
		AstType Type() override { return AstType::ASTSTMTS; }

		std::vector<Stmt *> stmts;
	};
}