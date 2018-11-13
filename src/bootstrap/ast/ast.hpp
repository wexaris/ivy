#include <utility>

#pragma once
#include "util/span.hpp"
#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////    Node Types    /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/* Base node type.
 * All node types extend this base type.
 */
struct Node {
	Span span;

	explicit Node(Span span) : span(span) {}
	virtual ~Node() = default;

	/* Create the IR in the form of three address code.
	 * Recursively loops through and emits all of the children's IR
	 */
	virtual std::string visit() const = 0;
};

/* Base statement type.
 * All statement types extend this base type.
 */
struct Stmt : public Node {
	explicit Stmt(Span span) : Node(span) {}
	virtual ~Stmt() = default;
};

/* Base expression type.
 * All expression types extend this base type.
 */
struct Expr : public Node {
	explicit Expr(Span span) : Node(span) {}
	virtual ~Expr() = default;
};

/* Base declaration type.
 * All declaration types extend this base type.
 */
struct Decl : public Node {
	Decl(Span span) : Node(span) {}
	virtual ~Decl() = default;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////    Documentation    ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/* A documentation comment. */
struct Doc {
	int bitpos;
	std::string text;

	Doc(int bitpos, std::string text) : bitpos(bitpos), text(std::move(text)) {}

	/*	Returns the length of the comment. */
	inline size_t len() const { return text.length(); }

	/*	Returns the position of the next character after the comment. */
	inline size_t end() const { return bitpos + text.length(); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////    Expressions    ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/*	A placeholder for expressions with errors. */
struct BadExpr : public Expr {
	explicit BadExpr(Span span) : Expr(span) {}
};

/*	A placeholder for expressions with errors. */
struct Ident : public Expr {
	std::string name;

	explicit Ident(std::string name, Span span) : Expr(span), name(name) {}
	// Object
};