///
/// @file BinaryInstruction.cpp
/// @brief 二元操作指令
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
#include "BinaryInstruction.h"

/// @brief 构造函数
/// @param _op 操作符
/// @param _result 结果操作数
/// @param _srcVal1 源操作数1
/// @param _srcVal2 源操作数2
BinaryInstruction::BinaryInstruction(Function * _func,
                                     IRInstOperator _op,
                                     Value * _srcVal1,
                                     Value * _srcVal2,
                                     Type * _type)
    : Instruction(_func, _op, _type)
{
    // addOperand(_srcVal1);
    // addOperand(_srcVal2);
	// 对于一元操作符，不添加第二个操作数-lxg
	addOperand(_srcVal1);
	if (_srcVal2 != nullptr || _op != IRInstOperator::IRINST_OP_NEG_I) {
		addOperand(_srcVal2);
	}
}

/// @brief 转换成字符串
/// @param str 转换后的字符串
void BinaryInstruction::toString(std::string & str)
{

    Value *src1 = getOperand(0), *src2 = getOperand(1);

    switch (op) {
        case IRInstOperator::IRINST_OP_ADD_I:

            // 加法指令，二元运算
            str = getIRName() + " = add " + src1->getIRName() + "," + src2->getIRName();
            break;
        case IRInstOperator::IRINST_OP_SUB_I:

            // 减法指令，二元运算
            str = getIRName() + " = sub " + src1->getIRName() + "," + src2->getIRName();
            break;
		/// 在 toString 函数中添加对乘法、除法和求余操作的处理-lxg
		 case IRInstOperator::IRINST_OP_MUL_I:
            // 乘法指令，二元运算
            str = getIRName() + " = mul " + src1->getIRName() + "," + src2->getIRName();
            break;
		case IRInstOperator::IRINST_OP_DIV_I:
			str = getIRName() + " = div " + src1->getIRName() + "," + src2->getIRName();
			break;
		case IRInstOperator::IRINST_OP_MOD_I:
			str = getIRName() + " = mod " + src1->getIRName() + "," + src2->getIRName();
			break;
		case IRInstOperator::IRINST_OP_NEG_I:
			// 一元负号指令
			str = getIRName() + " = neg " + src1->getIRName();
			break;
		/// 添加关系运算符的处理-lxg
		case IRInstOperator::IRINST_OP_LT_I:
			str = getIRName() + " = icmp lt " + src1->getIRName() + "," + src2->getIRName();
			break;
		case IRInstOperator::IRINST_OP_GT_I:
			str = getIRName() + " = icmp gt " + src1->getIRName() + "," + src2->getIRName();
			break;
		case IRInstOperator::IRINST_OP_LE_I:
			str = getIRName() + " = icmp le " + src1->getIRName() + "," + src2->getIRName();
			break;
		case IRInstOperator::IRINST_OP_GE_I:
			str = getIRName() + " = icmp ge " + src1->getIRName() + "," + src2->getIRName();
			break;
		case IRInstOperator::IRINST_OP_EQ_I:
			str = getIRName() + " = icmp eq " + src1->getIRName() + "," + src2->getIRName();
			break;
		case IRInstOperator::IRINST_OP_NE_I:
			str = getIRName() + " = icmp ne " + src1->getIRName() + "," + src2->getIRName();
			break;
        default:
            // 未知指令
            Instruction::toString(str);
            break;
    }
}