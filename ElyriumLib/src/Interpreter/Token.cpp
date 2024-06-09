#include <Interpreter/Token.h>

namespace elyrium {

Token::Token(Type type, std::string_view value, size_type row, size_type col)
	 : m_type(type), m_value(value), m_row(row), m_col(col) {
	if (m_type == Type::none) {
		//assert()

		if (m_value == "true") m_type = Type::keywTrue;
		else if (m_value == "false") m_type = Type::keywFalse;
		else if (m_value == "if") m_type = Type::keywIf;
		else if (m_value == "else") m_type = Type::keywElse;
		else if (m_value == "switch") m_type = Type::keywSwitch;
		else if (m_value == "case") m_type = Type::keywCase;
		else if (m_value == "default") m_type = Type::keywDefault;
		else if (m_value == "for") m_type = Type::keywFor;
		else if (m_value == "while") m_type = Type::keywWhile;
		else if (m_value == "break") m_type = Type::keywBreak;
		else if (m_value == "return") m_type = Type::keywReturn;
		else if (m_value == "returnexpr") m_type = Type::keywReturnexpr;
		else if (m_value == "let") m_type = Type::keywLet;
		else if (m_value == "func") m_type = Type::keywFunc;
		else if (m_value == "namespace") m_type = Type::keywNamespace;
		else if (m_value == "class" || m_value == "struct") m_type = Type::keywStruct;
		else if (m_value == "enum") m_type = Type::keywEnum;
		else if (m_value == "extern") m_type = Type::keywExtern;
		else if (m_value == "local") m_type = Type::keywLocal;
		else if (m_value == "global") m_type = Type::keywGlobal;
		else if (m_value == "const") m_type = Type::keywConst;
		else if (m_value == "constexpr") m_type = Type::keywConstexpr;
		else if (m_value == "this") m_type = Type::keywThis;
		else if (m_value == "nullptr") m_type = Type::keywNullptr;
		else if (m_value == "void") m_type = Type::typeVoid;
		else if (m_value == "bool") m_type = Type::typeBool;
		else if (m_value == "char") m_type = Type::typeChar;
		else if (m_value == "wchar") m_type = Type::typeCharWide;
		else if (m_value == "i8") m_type = Type::typeI8;
		else if (m_value == "i16") m_type = Type::typeI16;
		else if (m_value == "i32") m_type = Type::typeI32;
		else if (m_value == "i64") m_type = Type::typeI64;
		else if (m_value == "u8") m_type = Type::typeU8;
		else if (m_value == "u16") m_type = Type::typeU16;
		else if (m_value == "u32") m_type = Type::typeU32;
		else if (m_value == "u64") m_type = Type::typeU64;
		else if (m_value == "f8") m_type = Type::typeF8;
		else if (m_value == "f16") m_type = Type::typeF16;
		else if (m_value == "f32") m_type = Type::typeF32;
		else if (m_value == "f64") m_type = Type::typeF64;
		else if (m_value == "list") m_type = Type::typeList;
		else if (m_value == "string") m_type = Type::typeString;
		else if (m_value == "generic") m_type = Type::typeGeneric;
		else if (m_value == "__include__") m_type = Type::include;
		else if (m_value == "__entrypoint__") m_type = Type::entrypoint;
		else if (m_value == "__construct__") m_type = Type::construct;
		else if (m_value == "__destruct__") m_type = Type::destruct;
		else if (m_value == "__copy__") m_type = Type::copy;
		else if (m_value == "__move__") m_type = Type::move;
		else if (m_value == "move") m_type = Type::keywMove;
		else m_type = Type::identifier;
	}
}

} // namespace elyrium
