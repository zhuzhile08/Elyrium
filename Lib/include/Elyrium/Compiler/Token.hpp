/*************************
 * @file Token.hpp
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 * 
 * @brief Token to be used for the parser
 * 
 * @date 2024-06-08
 * 
 * @copyright Copyright (c) 2024
 *************************/

#pragma once

#include <Elyrium/Core/Common.hpp>

#include <LSD/String.h>
#include <LSD/StringView.h>

namespace elyrium {

namespace compiler {

class Token {
public:
	using type_tag = uint8;

	enum class Type : type_tag {
		invalid,
		eof,

		// Single character special symbols

		// Seperators
		semicolon = 16,         // ;
		dot,                    // .
		comma,                  // ,
		colon,                  // :

		// Brackets
		parenLeft,              // (
		parenRight,             // )
		braceLeft,              // {
		braceRight,             // }
		bracketLeft,            // [
		bracketRight,           // ]

		// Bitwise operators
		bitXor,                 // ^
		bitNot,                 // ~
		bitOr,                  // |
		bitAnd,                 // &
		deref = bitAnd,		    // &

		// Arithmatic and operators
		add,                    // +
		sub,                    // -
		mul,                    // *
		div,                    // /
		mod,                    // %
		assign,                 // =

		// Logical operators
		greater,                // >
		less,                   // <
		logicNot,               // !


		// Multi character special symbols

		// Logical operators
		equal,             		// ==
		notEqual,          		// !=
		greaterEqual,           // >=
		lessEqual,              // <=
		logicOr,                // ||
		logicAnd,               // &&

		// Arithmatic
		increment,              // ++
		decrement,              // --
		assignAdd,              // +=
		assignSub,              // -=
		assignMul,              // *=
		assignDiv,              // /=
		assignMod,              // %=

		// Bitwise operators
		shiftLeft,              // <<
		shiftRight,             // >>
		assignShiftLeft,        // <<=
		assignShiftRight,       // >>=
		assignBitwXor,          // ^=
		assignBitwNot,          // ~=
		assignBitwOr,           // |=
		assignBitwAnd,          // &=


		// Keywords

		kNull,					// null

		// Boolean
		kTrue,					// true
		kFalse,					// false

		// Move
		kMove,					// move

		// Control flow
		kIf,					// if
		kElse,					// else
		kFor,					// for
		kWhile,					// while
		kDo,					// do
		kBreak,					// break
		kReturn,				// return
		kYield,					// yield

		// Storage specifier
		kLet,					// let
		kConst,					// const
		kGlobal,				// global
		kPtr, 					// ptr

		// Declarations
		kFunc,					// func
		kCoroutine,				// coroutine
		kNamespace,				// namespace
		kClass,					// class
		kEnum,					// enum
		kType,					// type

		// Error handling
		kRaise,					// raise
		kTry,					// try
		kCatch,					// catch

		kThis,					// this

		kImport,				// import

		// Literals
		integral,
		unsignedIntegral,
		floating,
		character,
		string,

		// Identifier
		identifier,

		none = type_tag(-1)
	};

	Token() = default;
	Token(Type type) : m_type(type) { }
	Token(Type type, lsd::StringView data, size_type line, size_type column) : m_type(type), m_data(data), m_line(line), m_column(column) { }

	lsd::String stringify() const;

	[[nodiscard]] Type type() const noexcept {
		return m_type;
	}
	[[nodiscard]] lsd::StringView data() const noexcept {
		return m_data;
	}

private:
	Type m_type { };
	lsd::StringView m_data;

	size_type m_line { };
	size_type m_column { };
};

} // namespace compiler

} // namespace elyrium
