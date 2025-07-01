/*************************
 * @file Token.hpp
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 * 
 * @brief Token to be used for the parser
 * 
 * @date 2024-06-08
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
	using type_tag = size_type;

	enum class Type : type_tag {
		none,
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
		bitXOr,                 // ^
		bitNot,                 // ~
		bitOr,                  // |
		bitAnd,                 // &
		deref = bitAnd,		    // &

		// Arithmatic and operators
		add,                    // +
		sub,                    // -
		mul,                    // *
		pointer = mul,
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
		spaceship,				// <=>
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
		assignBitXOr,           // ^=
		assignBitNot,           // ~=
		assignBitOr,            // |=
		assignBitAnd,           // &=


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
		kDo,					// do
		kBreak,					// break
		kContinue,				// continue
		kReturn,				// return
		kYield,					// yield

		// Storage specifier
		kLet,					// let

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

		// Attribute
		attribute,
	};

	Token() = default;
	Token(Type type) : m_type(type) { }
	Token(Type type, lsd::StringView data, size_type line, size_type column) : m_type(type), m_data(data), m_line(line), m_column(column) { }

	lsd::String stringify() const;
	lsd::String lineSource(std::size_t& additionalSpaces) const noexcept;

	[[nodiscard]] Type type() const noexcept {
		return m_type;
	}
	[[nodiscard]] lsd::StringView data() const noexcept {
		return m_data;
	}
	[[nodiscard]] size_type line() const noexcept {
		return m_line;
	}
	[[nodiscard]] size_type column() const noexcept {
		return m_column;
	}

private:
	Type m_type { };
	lsd::StringView m_data;

	size_type m_line { };
	size_type m_column { };
};

} // namespace compiler

} // namespace elyrium
