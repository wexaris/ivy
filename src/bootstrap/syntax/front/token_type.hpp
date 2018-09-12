#pragma once

/*	An enumeration of all the types of tokens.
	Single character tokens are omitted since they can be specified with a character.
	For this reason the enumeration starts at 256. */
enum TokenType {

	END = 0,

	// Identifier
	ID = 256,
	NAME,

	// Documentation
	DOC,

	// UnaryOp
	PLUSPLUS,		// ++
	MINUSMINUS,		// --

	// BinOp
	EQEQ,			// ==
	NE,				// !=
	SUME,			// +=
	SUBE,			// -=
	MULE,			// *=
	DIVE,			// /=
	MODE,			// %=
	CARE,			// ^=
	GE,				// >=
	LE,				// <=
	ANDE,			// &=
	ORE,			// |=
	AND,			// &&
	OR,				// ||
	SHL,			// >>
	SHR,			// <<

	// Miscellaneous
	DOTDOTDOT,		// ...
	DOTDOT,			// ..
	SCOPE,			// ::
	LARROW,			// <-
	RARROW,			// ->
	DARROW,			// <->
	FATARROW,		// =>

	// Literal
	LIT_STRING,
	LIT_INT,
	LIT_UINT,
	LIT_FLOAT,

	// Type
	THING,
	STR,
	CHAR,
	INT,
	I8,
	I16,
	I32,
	I64,
	UINT,
	U8,
	U16,
	U32,
	U64,
	FLOAT,
	F32,
	F64,

	// Keyword
	PACK,
	MODULE,
	IMPORT,
	EXPORT,
	CLASS,
	STRUCT,
	ENUM,
	UNION,
	FUN,
	MACRO,
	RETURN,
	VAR,
	
	LOOP,
	WHILE,
	DO,
	FOR,
	IN,

	MATCH,
	SWITCH,
	CASE,
	WHERE,

	PUB,
	PRIV,

	MUT,
	CONST,
	STATIC,
	FINAL,
};