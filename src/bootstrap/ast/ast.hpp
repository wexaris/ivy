#pragma once
#include "source/span.hpp"
#include "visitor.hpp"
#include <vector>
#include <string>

namespace ast { 
	struct Ident;
	struct Stmt;
	struct Expr;
	struct Type;
	struct GenericParam;
	struct Param;
}

/* A single attribute.
 * Has an int for the type and a Span. */
struct Attr {
	TokenType ty;
	Span sp;
};
/* A vector of attributes. */
struct Attributes : std::vector<Attr> {
	
	bool contains(TokenType ty) {
		for (auto attr : *this)
			if (attr.ty == ty)
				return true;
		return false;
	}

	inline bool is_empty() { return this->size() == 0; }
};

/* A vector of identifier nodes. */
using Path = std::vector<std::unique_ptr<ast::Ident>>;

/* A vector of Type nodes. */
using TypeVec = std::vector<std::unique_ptr<ast::Type>>;
/* A vector of statement nodes. */
using StmtVec = std::vector<std::unique_ptr<ast::Stmt>>;
/* A vector of expression nodes. */
using ExprVec = std::vector<std::unique_ptr<ast::Expr>>;
/* A vector of generic parameter nodes. */
using GenericParamVec = std::vector<std::unique_ptr<ast::GenericParam>>;
/* A vector of parameter nodes. */
using ParamVec = std::vector<std::unique_ptr<ast::Param>>;

struct FunBlock {
	Span sp;
	StmtVec stmts;
	FunBlock(StmtVec& stmts, Span& span) :
		sp(std::move(span)),
		stmts(std::move(stmts))
	{}
};

/* Contains constructs related to abstract syntax tree generation.
 * Includes all of the AST node classes, as well as the 'NodeType' enumerator. */
namespace ast {

	enum class NodeType {

		// decl
		BadDecl,
		DeclTransUnit,
		DeclModule,
		DeclModuleImport,
		DeclPackageImport,
		DeclVar,
		DeclType,
		DeclUse,
		DeclFun,
		DeclStruct,

		// stmt
		BadStmt,

		// expr
		BadExpr,
		ExprSum,
		ExprSub,
		ExprMul,
		ExprDiv,
		ExprExp,
		ExprBool,
		ExprString,
		ExprChar,
		ExprInteger,
		ExprFloat,
		ExprIdent,
		ExprTuple,

		// type
		BadType,
		TypeUnknown,
		TypeBool,
		TypeString,
		TypeChar,
		TypeInteger,
		TypeFloat,
		TypeTuple,

		// other
		Lifetime,
		Ident,
		Path,
		GenericType,
		GenericLifetime,
		Param
	};

	/* Documentation info. */
	struct Doc { // TODO:  Currently not being collected
		std::string_view text;
		explicit Doc(std::string_view text) : text(text) {}
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////    Base Classes    ////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	/* Base node class. */
	struct Node {
		NodeType type;
		Span span;

		Node(NodeType type, Span&& span) : type(type), span(std::move(span)) {}
		virtual ~Node() = default;

		/* Create the IR in the form of three address code.
		 * Recursively loops through and emits all of the children's IR */
		virtual std::string accept(Visitor& visitor) const = 0;
	};

	/* Base statement node. */
	struct Stmt : public Node {
		Stmt(NodeType type, Span&& span) : Node(type, std::move(span)) {}
		virtual ~Stmt() = default;
		virtual std::string accept(Visitor& visitor) const override = 0;
	};

	/* Base declaration node. */
	struct Decl : public Stmt {
		Decl(NodeType type, Span&& span) : Stmt(type, std::move(span)) {}
		virtual ~Decl() = default;
		virtual std::string accept(Visitor& visitor) const override = 0;
	};

	/* Base expression node. */
	struct Expr : public Stmt {
		Expr(NodeType type, Span&& span) : Stmt(type, std::move(span)) {}
		virtual ~Expr() = default;
		virtual std::string accept(Visitor&) const override = 0;
	};

	/* Base type node. */
	struct Type : public Node {
		Type(NodeType type, Span&& span) : Node(type, std::move(span)) {}
		virtual ~Type() = default;
		virtual std::string accept(Visitor&) const override = 0;
	};


	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////    Other Classes    ///////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	/* A lifetime node. */
	struct Lifetime : public Node {
		std::string_view name;

		Lifetime(std::string_view name, Span& span) : Node(NodeType::Lifetime, std::move(span)),
			name(name)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An identifier node. */
	struct Ident : public Node {
		std::string_view name;

		Ident(std::string_view name, Span& span) : Node(NodeType::Ident, std::move(span)),
			name(name)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An path node.
	 * Contains a vector of identfiers corresponding to scopes. */
	struct Path : public Node {
		::Path sub_paths;

		Path(::Path& sub_paths, Span& span) : Node(NodeType::Path, std::move(span)),
			sub_paths(std::move(sub_paths))
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A generic parameter  node. */
	struct GenericParam : public Node {
		GenericParam(NodeType type, Span&& span) : Node(type, std::move(span)) {}
		virtual ~GenericParam() = default;
		virtual std::string accept(Visitor&) const override = 0;
	};

	/* A generic type parameter node. */
	struct GenericType : public GenericParam {
		std::unique_ptr<Type> type;

		GenericType(Type* ty, Span& span) : GenericParam(NodeType::GenericType, std::move(span)),
			type(ty)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A generic lifetime parameter node. */
	struct GenericLifetime : public GenericParam {
		std::unique_ptr<Lifetime> lf;

		GenericLifetime(Lifetime* lf, Span& span) : GenericParam(NodeType::GenericLifetime, std::move(span)),
			lf(lf)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A parameter node. */
	struct Param : public Node {
		std::unique_ptr<Ident> name;
		std::unique_ptr<Type> type;

		Param(Ident* name, Type* type, Span& span) : Node(NodeType::Param, std::move(span)),
			name(name),
			type(type)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};


	///////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////    Declarations    ////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	/* A placeholder node for declarations with errors. */
	struct BadDecl : public Decl {
		explicit BadDecl(Span& span) : Decl(NodeType::BadDecl, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A translation unit declaration node. */
	struct DeclTransUnit : public Decl {
		std::vector<std::unique_ptr<Decl>> declarations;

		explicit DeclTransUnit(const TranslationUnit* tu) : Decl(NodeType::DeclTransUnit, Span(*tu, tu->start_pos(), tu->end_pos())) {}

		Decl* add_decl(Decl* sub) {
			if (!sub) return nullptr;
			declarations.push_back(std::unique_ptr<Decl>(sub));
			return declarations.back().get();
		}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A module declaration node. */
	struct DeclModule : public Decl {
		std::unique_ptr<Path> path;
		std::vector<std::unique_ptr<Decl>> declarations;

		DeclModule(Path* path, std::vector<std::unique_ptr<Decl>>& decls, Span& span) : Decl(NodeType::DeclModule, std::move(span)),
			path(path),
			declarations(std::move(decls))
		{}

		Decl* add_decl(Decl* sub) {
			if (!sub)
				return nullptr;
			
			declarations.push_back(std::unique_ptr<Decl>(sub));
			return declarations.back().get();
		}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* TODO:  A module import declaration node. */
	struct DeclModuleImport : public Decl {
		std::unique_ptr<Path> path;
		DeclModuleImport(Path* path, Span& span) : Decl(NodeType::DeclModuleImport, std::move(span)),
			path(path)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* TODO:  A package import declaration node. */
	struct DeclPackageImport : public Decl {
		std::unique_ptr<Path> path;
		DeclPackageImport(Path* path, Span& span) : Decl(NodeType::DeclPackageImport, std::move(span)),
			path(path)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A variable declaration node.
	 * Has an Ident* name,
	 * Has a Path* type,  FIXME:  add 'Type' container
	 * Has an Expr* expr,  */
	struct DeclVar : public Decl {
		std::unique_ptr<Ident> name;
		std::unique_ptr<Path> type;
		std::unique_ptr<Expr> expr;

		DeclVar(Ident* name, Path* type, Expr* expr, Span& span) : Decl(NodeType::DeclVar, std::move(span)),
			name(name),
			type(type),
			expr(expr)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A type declaration node.
	 * Has an Ident* name,
	 * Has a Path* type,  FIXME:  add 'Type' container */
	struct DeclType : public Decl {
		std::unique_ptr<Ident> name;
		std::unique_ptr<Path> type;

		DeclType(Ident* name, Path* type, Span& span) : Decl(NodeType::DeclType, std::move(span)),
			name(name),
			type(type)	
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A 'use' scope declaration node. */
	struct DeclUse : public Decl {
		std::unique_ptr<Path> path;

		DeclUse(Path* path, Span& span) : Decl(NodeType::DeclUse, std::move(span)),
			path(path)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A function declaration node. */
	struct DeclFun : public Decl {
		std::unique_ptr<Ident> name;
		GenericParamVec generic_params;
		ParamVec params;
		std::unique_ptr<Type> ret_type;
		FunBlock block;

		DeclFun(Ident* name, GenericParamVec& generic_params, ParamVec& params, Type* ret, FunBlock& block, Span& span)
			: Decl(NodeType::DeclFun, std::move(span)),
			name(name),
			generic_params(std::move(generic_params)),
			params(std::move(params)),
			ret_type(ret),
			block(std::move(block))
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};


	///////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////    Statements    /////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	/* A placeholder node for statements with errors. */
	struct BadStmt : public Stmt {
		explicit BadStmt(Span& span) : Stmt(NodeType::BadStmt, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};


	///////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////    Expressions    ////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	/* A placeholder node for expressions with errors. */
	struct BadExpr : public Expr {
		explicit BadExpr(Span& span) : Expr(NodeType::BadExpr, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A sum expression node. */
	struct ExprSum : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprSum(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprSum, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A subtraction expression node. */
	struct ExprSub : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprSub(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprSub, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A multiplication expression node. */
	struct ExprMul : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprMul(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprMul, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A division expression node. */
	struct ExprDiv : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprDiv(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprDiv, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An exponent expression node. */
	struct ExprExp : public Expr {
		std::unique_ptr<Expr> base;
		std::unique_ptr<Expr> exp;

		ExprExp(Expr* base, Expr* exp, Span& span) : Expr(NodeType::ExprExp, std::move(span)),
			base(base),
			exp(exp)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};
	
	/* An expression node with a boolean value. */
	struct ExprBool : public Expr {
		bool value;

		ExprBool(bool val, Span& span) : Expr(NodeType::ExprBool, std::move(span)),
			value(val)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An expression node with a string value. */
	struct ExprString : public Expr {
		std::string_view value;

		ExprString(std::string_view val, Span& span) : Expr(NodeType::ExprString, std::move(span)),
			value(val)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An expression node with a character value. */
	struct ExprChar : public Expr {
		int value;

		ExprChar(int val, Span& span) : Expr(NodeType::ExprChar, std::move(span)),
			value(val)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An expression node with an integer value. */
	struct ExprInteger : public Expr {
		size_t value;

		ExprInteger(size_t val, Span& span) : Expr(NodeType::ExprInteger, std::move(span)),
			value(val)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An expression node with a floating point value. */
	struct ExprFloat : public Expr {
		double value;

		ExprFloat(double val, Span& span) : Expr(NodeType::ExprFloat, std::move(span)),
			value(val)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A tuple expression node.
	 * Contains a vector of sub-expressions. */
	struct ExprTuple : public Expr {
		ExprVec items;

		ExprTuple(ExprVec& items, Span& span) : Expr(NodeType::ExprTuple, std::move(span)),
			items(std::move(items))
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};


	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////    Types    ///////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	/* A placeholder node for types with errors. */
	struct BadType : public Type {
		explicit BadType(Span& span) : Type(NodeType::BadType, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An empty 'void' type node. */
	struct TypeVoid : public Type {
		explicit TypeVoid(Span& span) : Type(NodeType::TypeTuple, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A 'self' type node. */
	struct TypeSelf : public Type {
		explicit TypeSelf(Span& span) : Type(NodeType::TypeTuple, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A tuple type node.
	 * Cotntains a vector of sub-types. */
	struct TypeTuple : public Type {
		TypeVec items;

		TypeTuple(TypeVec& items, Span& span) : Type(NodeType::TypeTuple, std::move(span)),
			items(std::move(items))
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};
}

using ASTRoot = ast::DeclTransUnit;