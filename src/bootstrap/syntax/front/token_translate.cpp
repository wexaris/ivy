#include "token_translate.hpp"

std::string translate::tk_str(Token tk) {
	if (tk.type() == ID) 			return std::string("ID: ") 			+ tk.val().str;
	if (tk.type() == NAME) 			return std::string("NAME: ") 		+ tk.val().str;
	if (tk.type() == LIT_STRING) 	return std::string("LIT_STRING: ") 	+ tk.val().str;
	if (tk.type() == LIT_INT) 		return std::string("LIT_INT: ") 	+ std::to_string(tk.val().sint);
	if (tk.type() == LIT_UINT) 		return std::string("LIT_UINT: ") 	+ std::to_string(tk.val().uint);
	if (tk.type() == LIT_FLOAT) 	return std::string("LIT_FLOAT: ") 	+ std::to_string(tk.val().real);
	return tk_str(tk.type());
}

std::string translate::tk_str(int type) {
	
	// If the type is 0, return EOF
	if (type == 0)	return "EOF";

	// If the type is 1-255, return the ASCII character
	if (type > 0 && type <= 255)
		return std::string(1, (char)type);

	// Else, compare the type to the ones in TokeType
	switch (type) {
		// Identifier
		case ID:		return "ID";
		case NAME:		return "NAME";

		// Documentation
		case DOC:		return "DOC";

		// UnaryOp
		case PLUSPLUS:		return "++";
		case MINUSMINUS:	return "--";

		// BinOp
		case EQEQ:		return "==";
		case SUME:		return "+=";
		case SUBE:		return "-=";
		case MULE:		return "*=";
		case DIVE:		return "/=";
		case MODE:		return "%=";
		case GE:		return ">=";
		case LE:		return "<=";
		case ANDE:		return "&=";
		case ORE:		return "|=";
		case AND:		return "&&";
		case OR:		return "||";
		case SHL:		return ">>";
		case SHR:		return "<<";

		// Miscellaneous
		case DOTDOTDOT:		return "...";
		case DOTDOT:		return "..";
		case SCOPE:			return "::";
		case LARROW:		return "<-";
		case RARROW:		return "->";
		case FATARROW:		return "->";

		// Type
		case THING:		return "THING";
		case STR:		return "STR";
		case CHAR:		return "CHAR";
		case INT:		return "INT";
		case I8:		return "I8";
		case I16:		return "I16";
		case I32:		return "I32";
		case I64:		return "I64";
		case UINT:		return "UINT";
		case U8:		return "U8";
		case U16:		return "U16";
		case U32:		return "U32";
		case U64:		return "U64";
		case FLOAT:		return "FLOAT";
		case F32:		return "F32";
		case F64:		return "F64";

		// Literal
		case LIT_STRING:	return "LIT_STRING";
		case LIT_INT:		return "LIT_INT";
		case LIT_UINT:		return "LIT_UINT";
		case LIT_FLOAT:		return "LIT_FLOAT";

		// Keyword
		case PACK:			return "PACK";
		case MODULE:		return "MODULE";
		case IMPORT:		return "IMPORT";
		case EXPORT:		return "EXPORT";
		case CLASS:			return "CLASS";
		case STRUCT:		return "STRUCT";
		case ENUM:			return "ENUM";
		case UNION:			return "UNION";
		case FUN:			return "FUN";
		case MACRO:			return "MACRO";
		case RETURN:		return "RETURN";
		case VAR:			return "VAR";
		case LOOP:			return "LOOP";
		case WHILE:			return "WHILE";
		case DO:			return "DO";
		case FOR:			return "FOR";
		case IN:			return "IN";
		case MATCH:			return "MATCH";
		case SWITCH:		return "SWITCH";
		case CASE:			return "CASE";
		case WHERE:			return "WHERE";
		case PUB:			return "PUB";
		case PRIV:			return "PRIV";
		case MUT:			return "MUT";
		case CONST:			return "CONST";
		case STATIC:		return "STATIC";
		case FINAL:			return "FINAL";

		default:		return "<N/A>";
	}
}