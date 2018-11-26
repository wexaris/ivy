#pragma once

/* An enumeration of all the types of tokens.
 * Single character tokens are omitted since they can be specified with a character.
 * For this reason the enumeration starts at 256. */
enum class TokenType {

	END = 0,

	// Identifier
	ID = 256,
	LF,				// 'a

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
	ORE,			// |=
	ANDE,			// &=
	OR,				// ||
	SHL,			// <<
	// These should probably be built in the parser,
	// since they can be meant to be single tokens.
	//	&&type
	//	<type<type>>
	//AND,			// &&
	//SHR,			// >>

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
	LIT_CHAR,
	LIT_NUMBER,		// For lexer. Actual type is decided when parsing.
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
	PACKAGE,
	MOD,
	USE,
	IMPORT,
	EXPORT,
	VAR,
	FUN,
	STRUCT,
	TRAIT,
	ENUM,
	UNION,
	MACRO,
	IMPL,
	STATIC,
	CONST,
	TYPE,
	
	LOOP,
	WHILE,
	DO,
	FOR,
	IN,
	MATCH,
	SWITCH,
	CASE,
	WHERE,
	RETURN,

	PUB,
	PRIV,
	MUT,

	UNKNOWN
};

bool operator==(int type, const TokenType& other);
bool operator!=(int type, const TokenType& other);