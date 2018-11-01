#include "token_translate.hpp"

std::string translate::tk_str(Token tk) {
	switch (tk.type()) {
		case (int)TokenType::ID:			return std::string("ID: ") 			+ std::string(tk.lit());
		case (int)TokenType::LF:		    return std::string("LF: ") 			+ std::string(tk.lit());
		case (int)TokenType::LIT_STRING:	return std::string("LIT_STRING: ") 	+ std::string(tk.lit());
		case (int)TokenType::LIT_NUMBER:	return std::string("LIT_NUMBER: ") 	+ std::string(tk.lit());
		case (int)TokenType::LIT_INT:	    return std::string("LIT_INT: ") 	+ std::string(tk.lit());
		case (int)TokenType::LIT_UINT:	    return std::string("LIT_UINT: ") 	+ std::string(tk.lit());
		case (int)TokenType::LIT_FLOAT:	    return std::string("LIT_FLOAT: ") 	+ std::string(tk.lit());
		default: 			                return tk_str(tk.type());
	}
}

std::string translate::tk_str(int type) {
	
	// If the type is 0, return EOF
	if (type == (int)TokenType::END)	return "EOF";

	// If the type is 1-255, return the ASCII character
	if (type > 0 && type <= 255) {
		auto s = std::string(1, (char)type);
		return s;
	}

	// Else, compare the type to the ones in TokeType
	switch (type) {
		// Identifier
		case (int)TokenType::ID:		return "identifier";
		case (int)TokenType::LF:		return "lifetime";
		case (int)TokenType::STATIC_LF:	return "static lifetime";

		// Documentation
		case (int)TokenType::DOC:		return "documentation";

		// UnaryOp
		case (int)TokenType::PLUSPLUS:		return "++";
		case (int)TokenType::MINUSMINUS:	return "--";

		// BinOp
		case (int)TokenType::EQEQ:		return "==";
		case (int)TokenType::SUME:		return "+=";
		case (int)TokenType::SUBE:		return "-=";
		case (int)TokenType::MULE:		return "*=";
		case (int)TokenType::DIVE:		return "/=";
		case (int)TokenType::MODE:		return "%=";
		case (int)TokenType::GE:		return ">=";
		case (int)TokenType::LE:		return "<=";
		case (int)TokenType::ANDE:		return "&=";
		case (int)TokenType::ORE:		return "|=";
		case (int)TokenType::AND:		return "&&";
		case (int)TokenType::OR:		return "||";
		case (int)TokenType::SHL:		return ">>";
		case (int)TokenType::SHR:		return "<<";

		// Miscellaneous
		case (int)TokenType::DOTDOTDOT:		return "...";
		case (int)TokenType::DOTDOT:		return "..";
		case (int)TokenType::SCOPE:			return "::";
		case (int)TokenType::LARROW:		return "<-";
		case (int)TokenType::RARROW:		return "->";
		case (int)TokenType::FATARROW:		return "->";

		// Type
		case (int)TokenType::THING:		return "thing";
		case (int)TokenType::STR:		return "string";
		case (int)TokenType::CHAR:		return "char";
		case (int)TokenType::INT:		return "intr";
		case (int)TokenType::I8:		return "i8";
		case (int)TokenType::I16:		return "i16";
		case (int)TokenType::I32:		return "i32";
		case (int)TokenType::I64:		return "i64";
		case (int)TokenType::UINT:		return "uint";
		case (int)TokenType::U8:		return "u8";
		case (int)TokenType::U16:		return "u16";
		case (int)TokenType::U32:		return "u32";
		case (int)TokenType::U64:		return "u64";
		case (int)TokenType::FLOAT:		return "float";
		case (int)TokenType::F32:		return "f32";
		case (int)TokenType::F64:		return "f64";

		// Literal
		case (int)TokenType::LIT_STRING:	return "string literal";
		case (int)TokenType::LIT_INT:		return "int literal";
		case (int)TokenType::LIT_UINT:		return "uint literal";
		case (int)TokenType::LIT_FLOAT:		return "float literal";

		// Keyword
		case (int)TokenType::PACKAGE:		return "package";
		case (int)TokenType::MOD:			return "mod";
		case (int)TokenType::USE:			return "use";
		case (int)TokenType::IMPORT:		return "import";
		case (int)TokenType::EXPORT:		return "export";
		case (int)TokenType::VAR:			return "var";
		case (int)TokenType::FUN:			return "fun";
		case (int)TokenType::STRUCT:		return "struct";
		case (int)TokenType::TRAIT:			return "trait";
		case (int)TokenType::ENUM:			return "enum";
		case (int)TokenType::UNION:			return "union";
		case (int)TokenType::MACRO:			return "macro";
		case (int)TokenType::IMPL:			return "impl";
		case (int)TokenType::STATIC:		return "static";
		case (int)TokenType::CONST:			return "const";
		case (int)TokenType::TYPE:			return "type";

		case (int)TokenType::LOOP:			return "loop";
		case (int)TokenType::WHILE:			return "while";
		case (int)TokenType::DO:			return "do";
		case (int)TokenType::FOR:			return "for";
		case (int)TokenType::IN:			return "in";
		case (int)TokenType::MATCH:			return "match";
		case (int)TokenType::SWITCH:		return "switch";
		case (int)TokenType::CASE:			return "case";
		case (int)TokenType::WHERE:			return "where";
		case (int)TokenType::RETURN:		return "return";

		case (int)TokenType::PUB:			return "pub";
		case (int)TokenType::PRIV:			return "priv";
		case (int)TokenType::MUT:			return "mut";


		default:			return "<N/A>";
	}
}

bool operator==(int type, const TokenType& other) {
	return type == (int)other;
}
bool operator!=(int type, const TokenType& other) {
	return type != (int)other;
}