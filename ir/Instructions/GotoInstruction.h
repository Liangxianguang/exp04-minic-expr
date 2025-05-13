///
/// @file GotoInstruction.h
/// @brief 无条件跳转指令即goto指令
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-09-29
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#pragma once

#include <string>

#include "Instruction.h"
#include "LabelInstruction.h"
#include "Function.h"

///
/// @brief 无条件跳转指令
///
class GotoInstruction final : public Instruction {

public:
    ///
    /// @brief 无条件跳转指令的构造函数
    /// @param target 跳转目标
    ///
    GotoInstruction(Function * _func, Instruction * _target);

	/// @brief 条件跳转指令的构造函数-lxg
	/// @param _func 所属函数
	/// @param _condition 条件变量
	/// @param _trueTarget 条件为真时的跳转目标
	/// @param _falseTarget 条件为假时的跳转目标
	///
	GotoInstruction(Function * _func, Value * _condition, 
					Instruction * _trueTarget, Instruction * _falseTarget);

	/// @brief 获取假分支目标Label指令
	[[nodiscard]] LabelInstruction * getFalseTarget() const;

    /// @brief 转换成字符串
    void toString(std::string & str) override;

    ///
    /// @brief 获取目标Label指令
    /// @return LabelInstruction*
    ///
    [[nodiscard]] LabelInstruction * getTarget() const;


private:
    ///
    /// @brief 跳转到的目标Label指令
    ///
    LabelInstruction * target;

	/// 加对条件分支的支持-lxg
    /// @brief 条件为假时跳转到的目标Label指令，仅用于条件分支
    ///
    LabelInstruction * falseTarget = nullptr;

    ///
    /// @brief 是否是条件分支
    ///
    bool isConditional = false;
};
