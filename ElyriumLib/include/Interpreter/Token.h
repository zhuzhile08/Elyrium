/*************************
 * @file Token.h
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 * 
 * @brief Token to be used for the parser
 * 
 * @date 2024-06-08
 * @copyright Copyright (c) 2024
 *************************/

#pragma once

#include <Core/Common.h>

#include <LSD/UnorderedSparseMap.h>

#include <fmt/core.h>

#include <string_view>

namespace elyrium {

class Token {
public:
	enum class Type {
		none, // for type lookup

		// single character special symbols

		// seperators
		semicolon,              // ;
		eol = semicolon,
		dot,                    // .
		comma,                  // ,
		colon,                  // :

		// brackets
		parenLeft,              // (
		parenRight,             // )
		braceLeft,              // {
		braceRight,             // }
		bracketLeft,            // [
		bracketRight,           // ]

		// bitwise operators
		bitwXor,                // ^
		bitwNot,                // ~
		bitwOr,                 // |
		bitwAnd,                // &

		// arithmatic and operators
		asterisk,               // *, asterisk and not multiply because its function can't be immediatly determined
		add,                    // +
		sub,                    // -
		div,                    // /
		mod,                    // %

		// logical operators
		equal,                  // =
		greater,                // >
		less,                   // <
		logicNot,               // !


		// multi character special symbols
		
		// seperators
		colonColon,             // ::

		// logical operators
		equalEqual,             // ==
		equalLogicNot,          // !=
		equalGreater,           // >=
		equalLess,              // <=
		logicOr,                // ||
		logicAnd,               // &&

		// arithmatic
		increment,              // ++
		decrement,              // --
		equalAdd,               // +=
		equalSub,               // -=
		equalMul,               // *=
		equalDiv,               // /=
		equalMod,               // %=

		// bitwise operators
		shiftLeft,              // <<
		shiftRight,             // >>
		equalBitwXor,           // ^=
		equalBitwNot,           // ~=
		equalBitwOr,            // |=
		equalBitwAnd,           // &=

		// comments
		commentLine,            // //
		commentBlockLeft,       // /*
		commentBlockRight,      // */

		// special type signifiers
		rvalue,                 // **


		// literals
		identifier,
		number,
		character,
		string,


		// keywords

		// boolean
		keywTrue,
		keywFalse,

		// control flow
		keywIf,
		keywElse,
		keywSwitch,
		keywCase,
		keywDefault,
		keywFor,
		keywWhile,
		keywBreak,
		keywReturn,
		keywReturnexpr,

		// declarations
		keywLet,
		keywFunc,
		keywNamespace,
		keywStruct,
		keywEnum,

		// storage specifiers
		keywExtern, // not exactly a storage specifier
		keywLocal,
		keywGlobal,
		keywConst,
		keywConstexpr,

		// this
		keywThis,

		// nullptr
		keywNullptr,

		// types
		typeVoid,
		typeBool,
		typeChar,
		typeCharWide,
		typeI8,
		typeI16,
		typeI32,
		typeI64,
		typeU8,
		typeU16,
		typeU32,
		typeU64,
		typeF8,
		typeF16,
		typeF32,
		typeF64,
		typeList,
		typeString,
		typeGeneric,

		// special functions
		include,
		entrypoint,
		construct,
		destruct,
		copy,
		move,
		keywMove, // move keyword, not function
	};

	Token() = default;
	Token(Type type, std::string_view value, size_type row, size_type col);

	std::string stringify() const {
		return fmt::format("[{} \"{}\" {}, {}]", static_cast<int>(m_type), m_value, m_col, m_row);
	}

private:
	Type m_type;
	std::string_view m_value;

	size_type m_row;
	size_type m_col;

	friend class Lexer;
};

} // namespace elyrium
