///
/// @file MoveInstruction.cpp
/// @brief Move指令，也就是DragonIR的Asssign指令
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

#include "MoveInstruction.h"

///
/// @brief 构造函数
/// @param _func 所属的函数
/// @param result 结构操作数
/// @param srcVal1 源操作数
///
MoveInstruction::MoveInstruction(Function * _func, Value * _result, Value * _srcVal1)
    : Instruction(_func, IRInstOperator::IRINST_OP_ASSIGN, VoidType::getType())
{
    addOperand(_result);
    addOperand(_srcVal1);
}

/// @brief 转换成字符串显示
/// @param str 转换后的字符串
// void MoveInstruction::toString(std::string & str)
// {

//     Value *dstVal = getOperand(0), *srcVal = getOperand(1);

//     str = dstVal->getIRName() + " = " + srcVal->getIRName();
// }
//修改 MoveInstruction::toString 方法以支持指针存储操作的打印
void MoveInstruction::toString(std::string & str)
{
    Value *dstVal = getOperand(0), *srcVal = getOperand(1);

    if (isPointerStore) {
        // 指针存储格式：*%t8 = %l4
        str = "*" + dstVal->getIRName() + " = " + srcVal->getIRName();
    } else if (isPointerLoad) {
        // 指针加载格式：%l4 = *%t8
        str = dstVal->getIRName() + " = *" + srcVal->getIRName();
    } else {
        // 普通赋值格式：%l0 = %t52
        str = dstVal->getIRName() + " = " + srcVal->getIRName();
    }
}