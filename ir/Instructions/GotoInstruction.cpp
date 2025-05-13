///
/// @file GotoInstruction.cpp
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

#include "VoidType.h"

#include "GotoInstruction.h"

///
/// @brief 无条件跳转指令的构造函数
/// @param target 跳转目标
///
GotoInstruction::GotoInstruction(Function * _func, Instruction * _target)
    : Instruction(_func, IRInstOperator::IRINST_OP_GOTO, VoidType::getType())
{
    // // 真假目标一样，则无条件跳转
    // target = static_cast<LabelInstruction *>(_target);
	// 无条件跳转-lxg
    target = static_cast<LabelInstruction *>(_target);
    isConditional = false;
}

///
/// @brief 条件跳转指令的构造函数-lxg
/// @param _func 所属函数
/// @param _condition 条件变量
/// @param _trueTarget 条件为真时的跳转目标
/// @param _falseTarget 条件为假时的跳转目标
///
GotoInstruction::GotoInstruction(Function * _func, Value * _condition, 
                                Instruction * _trueTarget, Instruction * _falseTarget)
    : Instruction(_func, IRInstOperator::IRINST_OP_GOTO, VoidType::getType())
{
    // 条件分支
    addOperand(_condition); // 添加条件作为操作数
    target = static_cast<LabelInstruction *>(_trueTarget);
    falseTarget = static_cast<LabelInstruction *>(_falseTarget);
    isConditional = true;
}

/// @brief 转换成IR指令文本
void GotoInstruction::toString(std::string & str)
{
    // str = "br label " + target->getIRName();
	if (isConditional) {
        // 条件分支
        str = "bc " + getOperand(0)->getIRName() + ", label " + 
              target->getIRName() + ", label " + falseTarget->getIRName();
    } else {
        // 无条件跳转
        str = "br label " + target->getIRName();
    }
}

///
/// @brief 获取目标Label指令
/// @return LabelInstruction* label指令
///
LabelInstruction * GotoInstruction::getTarget() const
{
    return target;
}

///
/// @brief 获取假分支目标Label指令-lxg
/// @return LabelInstruction* label指令
///
LabelInstruction * GotoInstruction::getFalseTarget() const
{
    return falseTarget;
}
