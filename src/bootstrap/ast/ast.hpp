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
	struct UnaryOp;
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

/* An identifier-expression pair. */
struct IDExprPair {
	std::unique_ptr<ast::Ident> name;
	std::unique_ptr<ast::Expr> value;

	IDExprPair() = default;
	IDExprPair(ast::Ident* name, ast::Expr* val) : name(name), value(val) {}
};

enum class Mutability {
	Mutable,
	Immutable,
};


/* A vector of identifier nodes. */
using Path = std::vector<std::unique_ptr<ast::Ident>>;

/* A vector of identifier nodes. */
using StructFieldVec = std::vector<IDExprPair>;

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
/* A vector of parameter nodes. */
using UnaryOpVec = std::vector<std::unique_ptr<ast::UnaryOp>>;

struct FunBlock {
	Span sp;
	StmtVec stmts;
	bool defined;

	FunBlock() = default;

	FunBlock(StmtVec& stmts, Span& span) :
		sp(std::move(span)),
		stmts(std::move(stmts)),
		defined(true)
	{}
};

/* Contains constructs related to abstract syntax tree generation.
 * Includes all of the AST node classes, as well as the 'NodeType' enumerator. */
namespace ast {

	enum class NodeType {

		// decl
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
		StmtReturn,
		StmtBreak,
		StmtContinue,

		// expr
		ExprAssign,
		ExprEq,
		ExprNotEq,
		ExprLesser,
		ExprLesserEq,
		ExprGreater,
		ExprGreaterEq,
		ExprSum,
		ExprSumEq,
		ExprSub,
		ExprSubEq,
		ExprMul,
		ExprMulEq,
		ExprDiv,
		ExprDivEq,
		ExprMod,
		ExprModEq,
		ExprExp,
		ExprAnd,
		ExprOr,
		ExprMemAcc,
		ValueBool,
		ValueString,
		ValueChar,
		ValueInt,
		ValueFloat,
		ValueVoid,
		ValuePath,
		ValueFunCall,
		ValueMacroInvoc,
		ValueStruct,
		ValueArray,
		ValueTuple,

		// type
		TypeInfer,
		TypeThing,
		TypeBool,
		TypeStr,
		TypeChar,
		TypeISize,
		TypeI8,
		TypeI16,
		TypeI32,
		TypeI64,
		TypeI128,
		TypeUSize,
		TypeU8,
		TypeU16,
		TypeU32,
		TypeU64,
		TypeU128,
		TypeFSize,
		TypeF32,
		TypeF64,
		TypeVoid,
		TypePath,
		TypeTuple,
		TypeRef,
		TypePtr,
		TypeSlice,
		TypeArray,
		TypeSelf,
		
		// unary op
		UopNeg,
		UopNot,
		UopAddr,
		UopDeref,

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

	/* Base declaration statement node. */
	struct Decl : public Stmt {
		Decl(NodeType type, Span&& span) : Stmt(type, std::move(span)) {}
		virtual ~Decl() = default;
		virtual std::string accept(Visitor& visitor) const override = 0;
	};

	/* Base expression statement node. */
	struct Expr : public Stmt {
		Expr(NodeType type, Span&& span) : Stmt(type, std::move(span)) {}
		virtual ~Expr() = default;
		virtual std::string accept(Visitor&) const override = 0;
	};

	/* Base expression value node. */
	struct Value : public Expr {
		UnaryOpVec uops;
		Value(NodeType type, Span&& span) : Expr(type, std::move(span)) {}
		virtual ~Value() = default;
		inline void add_uop(UnaryOp* uop) { uops.push_back(std::unique_ptr<UnaryOp>(uop)); }
		virtual std::string accept(Visitor&) const override = 0;
	};

	/* Base type node. */
	struct Type : public Node {
		Type(NodeType type, Span&& span) : Node(type, std::move(span)) {}
		virtual ~Type() = default;
		virtual std::string accept(Visitor&) const override = 0;
	};

	/* Base primitive type node. */
	struct TypePrimitive : public Type {
		TypePrimitive(NodeType type, Span&& span) : Type(type, std::move(span)) {}
		virtual ~TypePrimitive() = default;
		virtual std::string accept(Visitor&) const override = 0;
	};

	struct UnaryOp : public Node {
		UnaryOp(NodeType type, Span&& span) : Node(type, std::move(span)) {}
		virtual ~UnaryOp() = default;
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

	/* A generic parameter node. */
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

	/* A type and lifetime node.
	 * Used in variable declaration. */
	struct TypeWithLifetime : public Node {
		std::unique_ptr<Type> type;
		std::unique_ptr<Lifetime> lf;

		TypeWithLifetime(Type* type, Lifetime* lf, Span& span) : Node(NodeType::Param, std::move(span)),
			type(type),
			lf(lf)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};


	///////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////    Declarations    ////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

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
	 * Has a Path* type,
	 * Has an Expr* expr,  */
	struct DeclVar : public Decl {
		std::unique_ptr<Ident> name;
		std::unique_ptr<Lifetime> lf;
		std::unique_ptr<Type> type;
		std::unique_ptr<Expr> expr;

		DeclVar(Ident* name, Lifetime* lf, Type* type, Expr* expr, Span& span) : Decl(NodeType::DeclVar, std::move(span)),
			name(name),
			lf(lf),
			type(type),
			expr(expr)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A type declaration node.
	 * Has an Ident* name,
	 * Has a Path* type, */
	struct DeclType : public Decl {
		std::unique_ptr<Ident> name;
		std::unique_ptr<Type> type;

		DeclType(Ident* name, Type* type, Span& span) : Decl(NodeType::DeclType, std::move(span)),
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

	/* A return statement node. */
	struct StmtReturn : public Stmt {
		std::unique_ptr<Expr> item;

		StmtReturn(Expr* item, Span& span) : Stmt(NodeType::StmtReturn, std::move(span)),
			item(item)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A break statement node. */
	struct StmtBreak : public Stmt {
		explicit StmtBreak(Span& span) : Stmt(NodeType::StmtBreak, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A continue statement node. */
	struct StmtContinue : public Stmt {
		explicit StmtContinue(Span& span) : Stmt(NodeType::StmtContinue, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};


	///////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////    Expressions    ////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	/* An assignment statement node. */
	struct ExprAssign : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprAssign(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprAssign, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An assignment expression node. */
	struct ExprEq : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprEq(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprEq, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An inequality expression node. */
	struct ExprNotEq : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprNotEq(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprNotEq, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An inequality expression node. */
	struct ExprSumEq : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprSumEq(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprSumEq, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An inequality expression node. */
	struct ExprSubEq : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprSubEq(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprSubEq, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An inequality expression node. */
	struct ExprMulEq : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprMulEq(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprMulEq, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An inequality expression node. */
	struct ExprDivEq : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprDivEq(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprDivEq, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An inequality expression node. */
	struct ExprModEq : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprModEq(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprModEq, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An inequality expression node. */
	struct ExprLesserEq : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprLesserEq(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprLesserEq, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An inequality expression node. */
	struct ExprGreaterEq : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprGreaterEq(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprGreaterEq, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A sum expression node. */
	struct ExprLesser : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprLesser(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprLesser, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A sum expression node. */
	struct ExprGreater : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprGreater(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprGreater, std::move(span)),
			left(lhs),
			right(rhs)
		{}
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

	/* A division expression node. */
	struct ExprMod : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprMod(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprMod, std::move(span)),
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

	/* An inequality expression node. */
	struct ExprAnd : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprAnd(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprAnd, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An inequality expression node. */
	struct ExprOr : public Expr {
		std::unique_ptr<Expr> left;
		std::unique_ptr<Expr> right;

		ExprOr(Expr* lhs, Expr* rhs, Span& span) : Expr(NodeType::ExprOr, std::move(span)),
			left(lhs),
			right(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A member access expression node. */
	struct ExprMemAcc : public Expr {
		std::unique_ptr<Expr> lhs;
		std::unique_ptr<Ident> rhs;

		ExprMemAcc(Expr* lhs, Ident* rhs, Span& span) : Expr(NodeType::ExprMemAcc, std::move(span)),
			lhs(lhs),
			rhs(rhs)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};
	
	/* A boolean value node. */
	struct ValueBool : public Value {
		bool value;

		ValueBool(bool val, Span& span) : Value(NodeType::ValueBool, std::move(span)),
			value(val)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A string value node. */
	struct ValueString : public Value {
		std::string_view value;

		ValueString(std::string_view val, Span& span) : Value(NodeType::ValueString, std::move(span)),
			value(val)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A character value node. */
	struct ValueChar : public Value {
		int64_t value;

		ValueChar(int64_t val, Span& span) : Value(NodeType::ValueChar, std::move(span)),
			value(val)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A integer value node. */
	struct ValueInt : public Value {
		int64_t value;

		ValueInt(size_t val, Span& span) : Value(NodeType::ValueInt, std::move(span)),
			value(val)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A floating point value node. */
	struct ValueFloat : public Value {
		double value;

		ValueFloat(double val, Span& span) : Value(NodeType::ValueFloat, std::move(span)),
			value(val)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A void value node. */
	struct ValueVoid : public Value {
		explicit ValueVoid(Span& span) : Value(NodeType::ValueVoid, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A value at some path. */
	struct ValuePath : public Value {
		std::unique_ptr<Path> path;

		ValuePath(Path* path, Span& span) : Value(NodeType::ValuePath, std::move(span)),
			path(path)
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A value returned by some function call. */
	struct ValueFunCall : public Value {
		std::unique_ptr<Path> name;
		ExprVec args;

		ValueFunCall(Path* name, ExprVec& args, Span& span) : Value(NodeType::ValueFunCall, std::move(span)),
			name(name),
			args(std::move(args))
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An struct creation value node. */
	struct ValueStruct : public Value {
		std::unique_ptr<Path> name;
		StructFieldVec fields;

		ValueStruct(Path* name, StructFieldVec& fields, Span& span) : Value(NodeType::ValueStruct, std::move(span)),
			name(name),
			fields(std::move(fields))
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An array value node. */
	struct ValueArray : public Value {
		ExprVec items;

		ValueArray(ExprVec& items, Span& span) : Value(NodeType::ValueArray, std::move(span)),
			items(std::move(items))
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A value returned by some macro invocation. */
	struct ValueMacroInvoc : public Value {
		std::unique_ptr<Path> name;
		ExprVec args;

		ValueMacroInvoc(Path* name, ExprVec& args, Span& span) : Value(NodeType::ValueMacroInvoc, std::move(span)),
			name(name),
			args(std::move(args))
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A tuple value node.
	 * Contains a vector of sub-expressions. */
	struct ValueTuple : public Value {
		ExprVec items;

		ValueTuple(ExprVec& items, Span& span) : Value(NodeType::ValueTuple, std::move(span)),
			items(std::move(items))
		{}
		std::string accept(Visitor&) const override { return std::string(); }
	};


	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////    Types    ///////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	/* A primitive 'thing' type node. */
	struct TypeThing : public TypePrimitive {
		TypeThing(Span& span) : TypePrimitive(NodeType::TypeThing, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive isize type node. */
	struct TypeStr : public TypePrimitive {
		TypeStr(Span& span) : TypePrimitive(NodeType::TypeStr, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive isize type node. */
	struct TypeChar : public TypePrimitive {
		TypeChar(Span& span) : TypePrimitive(NodeType::TypeChar, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive isize type node. */
	struct TypeISize : public TypePrimitive {
		TypeISize(Span& span) : TypePrimitive(NodeType::TypeISize, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive i8 type node. */
	struct TypeI8 : public TypePrimitive {
		TypeI8(Span& span) : TypePrimitive(NodeType::TypeI8, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive i16 type node. */
	struct TypeI16 : public TypePrimitive {
		TypeI16(Span& span) : TypePrimitive(NodeType::TypeI16, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive i32 type node. */
	struct TypeI32 : public TypePrimitive {
		TypeI32(Span& span) : TypePrimitive(NodeType::TypeI32, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive i64 type node. */
	struct TypeI64 : public TypePrimitive {
		TypeI64(Span& span) : TypePrimitive(NodeType::TypeI64, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive usize type node. */
	struct TypeUSize : public TypePrimitive {
		TypeUSize(Span& span) : TypePrimitive(NodeType::TypeUSize, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive u8 type node. */
	struct TypeU8 : public TypePrimitive {
		TypeU8(Span& span) : TypePrimitive(NodeType::TypeU8, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive u16 type node. */
	struct TypeU16 : public TypePrimitive {
		TypeU16(Span& span) : TypePrimitive(NodeType::TypeU16, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive u32 type node. */
	struct TypeU32 : public TypePrimitive {
		TypeU32(Span& span) : TypePrimitive(NodeType::TypeU32, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive u64 type node. */
	struct TypeU64 : public TypePrimitive {
		TypeU64(Span& span) : TypePrimitive(NodeType::TypeU64, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive fsize type node. */
	struct TypeFSize : public TypePrimitive {
		TypeFSize(Span& span) : TypePrimitive(NodeType::TypeFSize, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive f32 type node. */
	struct TypeF32 : public TypePrimitive {
		TypeF32(Span& span) : TypePrimitive(NodeType::TypeF32, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A primitive f64 type node. */
	struct TypeF64 : public TypePrimitive {
		TypeF64(Span& span) : TypePrimitive(NodeType::TypeF64, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An primitive 'void' type node. */
	struct TypeVoid : public TypePrimitive {
		explicit TypeVoid(Span& span) : TypePrimitive(NodeType::TypeVoid, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An omitted type, that still needs to be inferred. */
	struct TypeInfer : public Type {
		explicit TypeInfer(Span& span) : Type(NodeType::TypeInfer, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A path to a non-primitive type node. */
	struct TypePath : public Type {
		std::unique_ptr<Path> path;
		GenericParamVec generics;

		TypePath(Path* path, GenericParamVec& generics, Span& span) : Type(NodeType::TypePath, std::move(span)),
			path(path),
			generics(std::move(generics))
		{}

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

	/* A reference type node to another type. */
	struct TypeRef : public Type {
		std::unique_ptr<Type> type;
		Mutability mut;

		TypeRef(Type* type, Mutability mut, Span& span) : Type(NodeType::TypeRef, std::move(span)),
			type(type),
			mut(mut)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A pointer type node to another type. */
	struct TypePtr : public Type {
		std::unique_ptr<Type> type;
		Mutability mut;

		TypePtr(Type* type, Mutability mut, Span& span) : Type(NodeType::TypePtr, std::move(span)),
			type(type),
			mut(mut)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* An unknown-size array slice type node. */
	struct TypeSlice : public Type {
		std::unique_ptr<Type> type;

		TypeSlice(Type* type, Span& span) : Type(NodeType::TypeSlice, std::move(span)),
			type(type)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A fixed size array type node. */
	struct TypeArray : public Type {
		std::unique_ptr<Type> type;
		std::unique_ptr<Expr> len;

		TypeArray(Type* type, Expr* len, Span& span) : Type(NodeType::TypeArray, std::move(span)),
			type(type),
			len(len)
		{}

		std::string accept(Visitor&) const override { return std::string(); }
	};

	/* A 'self' type node. */ // FIXME:  'self' should be implemented differently
	struct TypeSelf : public Type {
		explicit TypeSelf(Span& span) : Type(NodeType::TypeSelf, std::move(span)) {}
		std::string accept(Visitor&) const override { return std::string(); }
	};


	///////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////    Unary Op    //////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
                                                
	struct UopNeg : public UnaryOp {
		explicit UopNeg(Span& span) : UnaryOp(NodeType::UopNeg, std::move(span)) {}
		virtual std::string accept(Visitor&) const override { return std::string(); }
	};

	struct UopNot : public UnaryOp {
		explicit UopNot(Span& span) : UnaryOp(NodeType::UopNot, std::move(span)) {}
		virtual std::string accept(Visitor&) const override { return std::string(); }
	};

	struct UopAddr : public UnaryOp {
		explicit UopAddr(Span& span) : UnaryOp(NodeType::UopAddr, std::move(span)) {}
		virtual std::string accept(Visitor&) const override { return std::string(); }
	};

	struct UopDeref : public UnaryOp {
		explicit UopDeref(Span& span) : UnaryOp(NodeType::UopDeref, std::move(span)) {}
		virtual std::string accept(Visitor&) const override { return std::string(); }
	};
}

using ASTRoot = ast::DeclTransUnit;