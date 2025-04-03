/*************************
 * @file Opcodes.hpp
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 * 
 * @brief Virtual machine opcodes
 * 
 * @date 2025-03-26
 * @copyright Copyright (c) 2025
 *************************/

#pragma once

namespace elyrium {

enum class Opcode {
	nop = 0,

	load = 16,
	store,
	swap,

	add = 32,
	subtract,
	multiply,
	divivde,
	modulo,
	negate,
	positive,

	adds,
	subtracts,
	multiplys,
	divivdes,
	modulos,

	bitShiftLeft = 46,
	bitShiftRight,

	bitNot = 48,
	bitAnd,
	bitOr,
	bitXOr,
	compare,

	jump = 64,
	ret,
	syscall,

	jumpIfEqual = 72,
	jumpIfNotEqual,
	jumpIfLarger,
	jumpIfSmaller,
	jumpIfCarrySet,
	jumpIfCarryClear,
	jumpIfOverflowSet,
	jumpIfOverflowClear,
	jumpIfZero,
	jumpIfPositive,
	jumpIfNegative,
	jumpIfInf,
	jumpIfNan,

	clearCarry = 96,
	setCarry,
	clearOverflow,
	setOverflow,
	clearFlags,
	setFlags
};

} // namespace elyrium
