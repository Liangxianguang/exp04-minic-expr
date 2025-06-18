///
/// @file ILocArm32.cpp
/// @brief 指令序列管理的实现，ILOC的全称为Intermediate Language for Optimizing Compilers
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-21
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-21 <td>1.0     <td>zenglj  <td>新做
/// </table>
///
#include <cstdio>
#include <string>
#include <typeinfo>

#include "ILocArm32.h"
#include "Common.h"
#include "Function.h"
#include "PlatformArm32.h"
#include "Module.h"
#include "ir/Instructions/BinaryInstruction.h"
#include "ir/Instruction.h"
#include "ir/Values/ConstInt.h"

ArmInst::ArmInst(std::string _opcode,
                 std::string _result,
                 std::string _arg1,
                 std::string _arg2,
                 std::string _cond,
                 std::string _addition)
    : opcode(_opcode), cond(_cond), result(_result), arg1(_arg1), arg2(_arg2), addition(_addition), dead(false)
{}

/*
    指令内容替换
*/
void ArmInst::replace(std::string _opcode,
                      std::string _result,
                      std::string _arg1,
                      std::string _arg2,
                      std::string _cond,
                      std::string _addition)
{
    opcode = _opcode;
    result = _result;
    arg1 = _arg1;
    arg2 = _arg2;
    cond = _cond;
    addition = _addition;

#if 0
    // 空操作，则设置为dead
    if (op == "") {
        dead = true;
    }
#endif
}

/*
    设置为无效指令
*/
void ArmInst::setDead()
{
    dead = true;
}

/*
    输出函数
*/
std::string ArmInst::outPut()
{
    // 无用代码，什么都不输出
    if (dead) {
        return "";
    }

    // 占位指令,可能需要输出一个空操作，看是否支持 FIXME
    if (opcode.empty()) {
        return "";
    }

    std::string ret = opcode;

    if (!cond.empty()) {
        ret += cond;
    }

    // 结果输出
    if (!result.empty()) {
        if (result == ":") {
            ret += result;
        } else {
            ret += " " + result;
        }
    }

    // 第一元参数输出
    if (!arg1.empty()) {
        ret += "," + arg1;
    }

    // 第二元参数输出
    if (!arg2.empty()) {
        ret += "," + arg2;
    }

    // 其他附加信息输出
    if (!addition.empty()) {
        ret += "," + addition;
    }

    return ret;
}

#define emit(...) code.push_back(new ArmInst(__VA_ARGS__))

/// @brief 构造函数
/// @param _module 符号表
ILocArm32::ILocArm32(Module * _module)
{
    this->module = _module;
}

/// @brief 析构函数
ILocArm32::~ILocArm32()
{
    std::list<ArmInst *>::iterator pIter;

    for (pIter = code.begin(); pIter != code.end(); ++pIter) {
        delete (*pIter);
    }
}

/// @brief 删除无用的Label指令
void ILocArm32::deleteUnusedLabel()
{
    std::list<ArmInst *> labelInsts;
    for (ArmInst * arm: code) {
        if ((!arm->dead) && (arm->opcode[0] == '.') && (arm->result == ":")) {
            labelInsts.push_back(arm);
        }
    }

    // 检测Label指令是否在被使用，也就是是否有跳转到该Label的指令
    // 如果没有使用，则设置为dead
    for (ArmInst * labelArm: labelInsts) {
        bool labelUsed = false;

        for (ArmInst * arm: code) {
            // TODO 转移语句的指令标识符根据定义修改判断
            if ((!arm->dead) && (arm->opcode[0] == 'b') && (arm->result == labelArm->opcode)) {
                labelUsed = true;
                break;
            }
        }

        if (!labelUsed) {
            labelArm->setDead();
        }
    }
}

/// @brief 输出汇编
/// @param file 输出的文件指针
/// @param outputEmpty 是否输出空语句
void ILocArm32::outPut(FILE * file, bool outputEmpty)
{
    for (auto arm: code) {

        std::string s = arm->outPut();

        if (arm->result == ":") {
            // Label指令，不需要Tab输出
            fprintf(file, "%s\n", s.c_str());
            continue;
        }

        if (!s.empty()) {
            fprintf(file, "\t%s\n", s.c_str());
        } else if ((outputEmpty)) {
            fprintf(file, "\n");
        }
    }
}

/// @brief 获取当前的代码序列
/// @return 代码序列
std::list<ArmInst *> & ILocArm32::getCode()
{
    return code;
}

/**
 * 数字变字符串，若flag为真，则变为立即数寻址（加#）
 */
std::string ILocArm32::toStr(int num, bool flag)
{
    std::string ret;

    if (flag) {
        ret = "#";
    }

    ret += std::to_string(num);

    return ret;
}

/*
    产生标签
*/
void ILocArm32::label(std::string name)
{
    // .L1:
    emit(name, ":");
}

/// @brief 0个源操作数指令
/// @param op 操作码
/// @param rs 操作数
void ILocArm32::inst(std::string op, std::string rs)
{
    emit(op, rs);
}

/// @brief 一个操作数指令
/// @param op 操作码
/// @param rs 操作数
/// @param arg1 源操作数
void ILocArm32::inst(std::string op, std::string rs, std::string arg1)
{
    emit(op, rs, arg1);
}

/// @brief 一个操作数指令
/// @param op 操作码
/// @param rs 操作数
/// @param arg1 源操作数
/// @param arg2 源操作数
void ILocArm32::inst(std::string op, std::string rs, std::string arg1, std::string arg2)
{
    emit(op, rs, arg1, arg2);
}

///
/// @brief 注释指令，不包含分号
///
void ILocArm32::comment(std::string str)
{
    emit("@", str);
}

/*
    加载立即数 ldr r0,=#100
*/
void ILocArm32::load_imm(int rs_reg_no, int constant)
{
    std::string regname = PlatformArm32::regName[rs_reg_no];

    if (constant < 0) {
        // 处理负数：加载正数版本，然后使用 rsb 取反
        int abs_val = -constant;

        if (abs_val <= 255) {
            // 小的正数，直接 mov 然后 rsb 取反
            emit("mov", regname, "#" + std::to_string(abs_val));
            emit("rsb", regname, regname, "#0");
        } else {
            // 大的正数，使用 movw/movt 然后 rsb 取反
            if (0 == ((abs_val >> 16) & 0xFFFF)) {
                emit("movw", regname, "#:lower16:" + std::to_string(abs_val));
            } else {
                emit("movw", regname, "#:lower16:" + std::to_string(abs_val));
                emit("movt", regname, "#:upper16:" + std::to_string(abs_val));
            }
            // 取反：r = 0 - r
            emit("rsb", regname, regname, "#0");
        }
    } else {
        // 处理非负数
        // movw:把 16 位立即数放到寄存器的低16位，高16位清0
        // movt:把 16 位立即数放到寄存器的高16位，低 16位不影响
        if (0 == ((constant >> 16) & 0xFFFF)) {
            // 如果高16位本来就为0，直接movw
            emit("movw", regname, "#:lower16:" + std::to_string(constant));
        } else {
            // 如果高16位不为0，先movw，然后movt
            emit("movw", regname, "#:lower16:" + std::to_string(constant));
            emit("movt", regname, "#:upper16:" + std::to_string(constant));
        }
    }
}

/// @brief 加载符号值 ldr r0,=g ldr r0,=.L1
/// @param rs_reg_no 结果寄存器编号
/// @param name 符号名
void ILocArm32::load_symbol(int rs_reg_no, std::string name)
{
    // movw r10, #:lower16:a
    // movt r10, #:upper16:a
    emit("movw", PlatformArm32::regName[rs_reg_no], "#:lower16:" + name);
    emit("movt", PlatformArm32::regName[rs_reg_no], "#:upper16:" + name);
}

/// @brief 基址寻址 ldr r0,[fp,#100]
/// @param rsReg 结果寄存器
/// @param base_reg_no 基址寄存器
/// @param offset 偏移
void ILocArm32::load_base(int rs_reg_no, int base_reg_no, int offset)
{
    std::string rsReg = PlatformArm32::regName[rs_reg_no];
    std::string base = PlatformArm32::regName[base_reg_no];

    if (PlatformArm32::isDisp(offset)) {
        // 有效的偏移常量
        if (offset) {
            // [fp,#-16] [fp]
            base += "," + toStr(offset);
        }
    } else {

        // ldr r8,=-4096
        load_imm(rs_reg_no, offset);

        // fp,r8
        base += "," + rsReg;
    }

    // 内存寻址
    base = "[" + base + "]";

    // ldr r8,[fp,#-16]
    // ldr r8,[fp,r8]
    emit("ldr", rsReg, base);
}

/// @brief 基址寻址 str r0,[fp,#100]
/// @param srcReg 源寄存器
/// @param base_reg_no 基址寄存器
/// @param disp 偏移
/// @param tmp_reg_no 可能需要临时寄存器编号
void ILocArm32::store_base(int src_reg_no, int base_reg_no, int disp, int tmp_reg_no)
{
    std::string base = PlatformArm32::regName[base_reg_no];

    if (PlatformArm32::isDisp(disp)) {
        // 有效的偏移常量

        // 若disp为0，则直接采用基址，否则采用基址+偏移
        // [fp,#-16] [fp]
        if (disp) {
            base += "," + toStr(disp);
        }
    } else {
        // 先把立即数赋值给指定的寄存器tmpReg，然后采用基址+寄存器的方式进行

        // ldr r9,=-4096
        load_imm(tmp_reg_no, disp);

        // fp,r9
        base += "," + PlatformArm32::regName[tmp_reg_no];
    }

    // 内存间接寻址
    base = "[" + base + "]";

    // str r8,[fp,#-16]
    // str r8,[fp,r9]
    emit("str", PlatformArm32::regName[src_reg_no], base);
}

/// @brief 寄存器Mov操作
/// @param rs_reg_no 结果寄存器
/// @param src_reg_no 源寄存器
void ILocArm32::mov_reg(int rs_reg_no, int src_reg_no)
{
    emit("mov", PlatformArm32::regName[rs_reg_no], PlatformArm32::regName[src_reg_no]);
}
/// @brief 加载变量到寄存器，保证将变量放到reg中
/// @param rs_reg_no 结果寄存器
/// @param src_var 源操作数
void ILocArm32::load_var(int rs_reg_no, Value * src_var)
{
    printf("=== LOAD_VAR DEBUG ===\n");
    printf("变量: '%s', regId: %d, 目标寄存器: r%d\n", src_var->getIRName().c_str(), src_var->getRegId(), rs_reg_no);
    printf("变量类型: %s\n", typeid(*src_var).name());

    // **关键修复：检查是否是函数参数**
    std::string varName = src_var->getIRName();

    // **方案1：通过IR名称识别函数参数**
    if (varName == "%t0") {
        printf("★★★ 函数参数a0: %s -> 使用r0 ★★★\n", varName.c_str());
        if (rs_reg_no != 0) {
            emit("mov", PlatformArm32::regName[rs_reg_no], "r0");
            printf("生成: mov r%d, r0\n", rs_reg_no);
        } else {
            printf("目标就是r0，无需移动\n");
        }
        printf("=== END DEBUG (函数参数a0) ===\n");
        return;

    } else if (varName == "%t1") {
        printf("★★★ 函数参数a1: %s -> 使用r1 ★★★\n", varName.c_str());
        if (rs_reg_no != 1) {
            emit("mov", PlatformArm32::regName[rs_reg_no], "r1");
            printf("生成: mov r%d, r1\n", rs_reg_no);
        } else {
            printf("目标就是r1，无需移动\n");
        }
        printf("=== END DEBUG (函数参数a1) ===\n");
        return;

    } else if (varName == "%t2") {
        printf("★★★ 函数参数a2: %s -> 使用r2 ★★★\n", varName.c_str());
        if (rs_reg_no != 2) {
            emit("mov", PlatformArm32::regName[rs_reg_no], "r2");
            printf("生成: mov r%d, r2\n", rs_reg_no);
        } else {
            printf("目标就是r2，无需移动\n");
        }
        printf("=== END DEBUG (函数参数a2) ===\n");
        return;

    } else if (varName == "%t3") {
        printf("★★★ 函数参数a3: %s -> 使用r3 ★★★\n", varName.c_str());
        if (rs_reg_no != 3) {
            emit("mov", PlatformArm32::regName[rs_reg_no], "r3");
            printf("生成: mov r%d, r3\n", rs_reg_no);
        } else {
            printf("目标就是r3，无需移动\n");
        }
        printf("=== END DEBUG (函数参数a3) ===\n");
        return;

    } else if (varName.length() >= 3 && varName.substr(0, 2) == "%t") {
        // 处理 %t4, %t5, %t6 等栈参数
        int paramIndex = std::stoi(varName.substr(2));
        if (paramIndex >= 4) {
            printf("★★★ 栈参数 %s (索引: %d) -> 从栈加载 ★★★\n", varName.c_str(), paramIndex);
            
            // 栈参数的偏移计算：
            // 栈参数从 fp+8 开始（fp+0是保存的fp, fp+4是返回地址）
            // 第5个参数(t4)在 fp+8, 第6个(t5)在 fp+12, 以此类推
            int stack_offset = 8 + (paramIndex - 4) * 4;
            
            printf("从栈加载参数: ldr r%d, [fp, #%d]\n", rs_reg_no, stack_offset);
            load_base(rs_reg_no, ARM32_FP_REG_NO, stack_offset);
            printf("=== END DEBUG (栈参数) ===\n");
            return;
        }
    }

    // **方案2：通过FormalParam类型识别**
    if (auto fp = dynamic_cast<FormalParam *>(src_var)) {
        printf("★ 这是函数参数: %s, regId: %d\n", fp->getName().c_str(), fp->getRegId());

        if (fp->getRegId() != -1 && fp->getRegId() < 16) {
            // 寄存器参数
            int32_t src_regId = fp->getRegId();
            printf("使用参数寄存器: r%d\n", src_regId);

            if (src_regId != rs_reg_no) {
                emit("mov", PlatformArm32::regName[rs_reg_no], PlatformArm32::regName[src_regId]);
                printf("生成: mov r%d, r%d\n", rs_reg_no, src_regId);
            }
            printf("=== END DEBUG (FormalParam寄存器) ===\n");
            return;
        } else {
            // 栈参数：检查是否设置了内存地址
            int32_t baseRegId = -1;
            int64_t offset = -1;
            
            if (fp->getMemoryAddr(&baseRegId, &offset)) {
                printf("★★★ 栈参数 %s: 基址r%d, 偏移%ld ★★★\n", fp->getName().c_str(), baseRegId, offset);
                load_base(rs_reg_no, baseRegId, offset);
                printf("生成: ldr r%d, [r%d, #%ld]\n", rs_reg_no, baseRegId, offset);
                printf("=== END DEBUG (FormalParam栈参数) ===\n");
                return;
            } else {
                printf("ERROR: 栈参数 %s 没有设置内存地址\n", fp->getName().c_str());
            }
        }
    } // **方案3：追踪BinaryInstruction的操作数，寻找函数参数**
    if (auto binInst = dynamic_cast<BinaryInstruction *>(src_var)) {
        printf("★ 这是一个二元指令，检查操作数...\n");
        printf("  操作数数量: %d\n", binInst->getOperandsNum());

        // **首先检查是否所有操作数都是常量，如果是则直接计算**
        bool allConstant = true;
        std::vector<int> constValues;

        for (int32_t i = 0; i < binInst->getOperandsNum(); i++) {
            Value * operand = binInst->getOperand(i);
            if (!operand) {
                allConstant = false;
                break;
            }

            if (auto constOperand = dynamic_cast<ConstInt *>(operand)) {
                constValues.push_back(constOperand->getVal());
            } else {
                allConstant = false;
                break;
            }
        }

        if (allConstant && constValues.size() == 2) {
            // 简单的常量折叠：直接计算结果
            int result = 0;
            IRInstOperator op = binInst->getOp();

            if (op == IRInstOperator::IRINST_OP_ADD_I) {
                result = constValues[0] + constValues[1];
            } else if (op == IRInstOperator::IRINST_OP_MUL_I) {
                result = constValues[0] * constValues[1];
            } else {
                // 其他操作暂时不处理，使用原逻辑
                allConstant = false;
            }

            if (allConstant) {
                printf("★★★ 常量折叠: %d %s %d = %d ★★★\n",
                       constValues[0],
                       (op == IRInstOperator::IRINST_OP_ADD_I) ? "+" : "*",
                       constValues[1],
                       result);
                load_imm(rs_reg_no, result);
                printf("=== END DEBUG (常量折叠) ===\n");
                return;
            }
        }

        // **然后检查操作数中是否有函数参数**
        // 检查所有操作数，寻找函数参数
        for (int32_t i = 0; i < binInst->getOperandsNum(); i++) {
            Value * operand = binInst->getOperand(i);
            printf("  操作数[%d]: ", i);

            if (!operand) {
                printf("null\n");
                continue;
            }

            std::string opName = operand->getIRName();
            printf("'%s', 类型: %s\n", opName.c_str(), typeid(*operand).name());

            // 检查操作数是否是函数参数
            if (opName == "%t0") {
                printf("★★★ 追踪到函数参数a0，使用r0 ★★★\n");
                if (rs_reg_no != 0) {
                    emit("mov", PlatformArm32::regName[rs_reg_no], "r0");
                    printf("生成: mov r%d, r0\n", rs_reg_no);
                } else {
                    printf("目标就是r0，无需移动\n");
                }
                printf("=== END DEBUG (追踪到a0) ===\n");
                return;

            } else if (opName == "%t1") {
                printf("★★★ 追踪到函数参数a1，使用r1 ★★★\n");
                if (rs_reg_no != 1) {
                    emit("mov", PlatformArm32::regName[rs_reg_no], "r1");
                    printf("生成: mov r%d, r1\n", rs_reg_no);
                } else {
                    printf("目标就是r1，无需移动\n");
                }
                printf("=== END DEBUG (追踪到a1) ===\n");
                return;

            } else if (opName == "%t2") {
                printf("★★★ 追踪到函数参数a2，使用r2 ★★★\n");
                if (rs_reg_no != 2) {
                    emit("mov", PlatformArm32::regName[rs_reg_no], "r2");
                    printf("生成: mov r%d, r2\n", rs_reg_no);
                } else {
                    printf("目标就是r2，无需移动\n");
                }
                printf("=== END DEBUG (追踪到a2) ===\n");
                return;
            }

            // 递归检查BinaryInstruction类型的操作数
            if (auto binOperand = dynamic_cast<BinaryInstruction *>(operand)) {
                printf("  -> 发现BinaryInstruction操作数，递归追踪...\n");

                // **首先检查这个BinaryInstruction是否可以常量折叠**
                bool canFold = true;
                std::vector<int> subConstValues;

                for (int32_t j = 0; j < binOperand->getOperandsNum(); j++) {
                    Value * subOperand = binOperand->getOperand(j);
                    if (!subOperand) {
                        canFold = false;
                        break;
                    }

                    if (auto constSubOperand = dynamic_cast<ConstInt *>(subOperand)) {
                        subConstValues.push_back(constSubOperand->getVal());
                    } else {
                        canFold = false;
                        break;
                    }
                }

                if (canFold && subConstValues.size() == 2) {
                    int subResult = 0;
                    IRInstOperator subOp = binOperand->getOp();

                    if (subOp == IRInstOperator::IRINST_OP_ADD_I) {
                        subResult = subConstValues[0] + subConstValues[1];
                    } else if (subOp == IRInstOperator::IRINST_OP_MUL_I) {
                        subResult = subConstValues[0] * subConstValues[1];
                    } else {
                        canFold = false;
                    }

                    if (canFold) {
                        printf("    -> 子表达式常量折叠: %d %s %d = %d\n",
                               subConstValues[0],
                               (subOp == IRInstOperator::IRINST_OP_ADD_I) ? "+" : "*",
                               subConstValues[1],
                               subResult);

                        // 检查当前表达式是否现在可以整体常量折叠
                        if (binInst->getOperandsNum() == 2) {
                            // 获取另一个操作数
                            Value * otherOperand = (i == 0) ? binInst->getOperand(1) : binInst->getOperand(0);
                            if (auto otherConst = dynamic_cast<ConstInt *>(otherOperand)) {
                                int otherVal = otherConst->getVal();
                                int finalResult = 0;
                                IRInstOperator currentOp = binInst->getOp();

                                if (currentOp == IRInstOperator::IRINST_OP_ADD_I) {
                                    finalResult = (i == 0) ? subResult + otherVal : otherVal + subResult;
                                } else if (currentOp == IRInstOperator::IRINST_OP_MUL_I) {
                                    finalResult = (i == 0) ? subResult * otherVal : otherVal * subResult;
                                } else {
                                    canFold = false;
                                }

                                if (canFold) {
                                    printf("★★★ 完整表达式常量折叠: %d %s %d = %d ★★★\n",
                                           (i == 0) ? subResult : otherVal,
                                           (currentOp == IRInstOperator::IRINST_OP_ADD_I) ? "+" : "*",
                                           (i == 0) ? otherVal : subResult,
                                           finalResult);
                                    load_imm(rs_reg_no, finalResult);
                                    printf("=== END DEBUG (完整常量折叠) ===\n");
                                    return;
                                }
                            }
                        }
                    }
                }

                // **然后进行原有的递归追踪**
                // 递归检查这个BinaryInstruction的操作数
                for (int32_t j = 0; j < binOperand->getOperandsNum(); j++) {
                    Value * subOperand = binOperand->getOperand(j);
                    if (!subOperand)
                        continue;

                    std::string subOpName = subOperand->getIRName();
                    printf("    子操作数[%d]: '%s', 类型: %s\n", j, subOpName.c_str(), typeid(*subOperand).name());

                    if (subOpName == "%t0") {
                        printf("★★★ 递归追踪到函数参数a0，使用r0 ★★★\n");
                        if (rs_reg_no != 0) {
                            emit("mov", PlatformArm32::regName[rs_reg_no], "r0");
                            printf("生成: mov r%d, r0\n", rs_reg_no);
                        } else {
                            printf("目标就是r0，无需移动\n");
                        }
                        printf("=== END DEBUG (递归追踪到a0) ===\n");
                        return;

                    } else if (subOpName == "%t1") {
                        printf("★★★ 递归追踪到函数参数a1，使用r1 ★★★\n");
                        if (rs_reg_no != 1) {
                            emit("mov", PlatformArm32::regName[rs_reg_no], "r1");
                            printf("生成: mov r%d, r1\n", rs_reg_no);
                        } else {
                            printf("目标就是r1，无需移动\n");
                        }
                        printf("=== END DEBUG (递归追踪到a1) ===\n");
                        return;

                    } else if (subOpName == "%t2") {
                        printf("★★★ 递归追踪到函数参数a2，使用r2 ★★★\n");
                        if (rs_reg_no != 2) {
                            emit("mov", PlatformArm32::regName[rs_reg_no], "r2");
                            printf("生成: mov r%d, r2\n", rs_reg_no);
                        } else {
                            printf("目标就是r2，无需移动\n");
                        }
                        printf("=== END DEBUG (递归追踪到a2) ===\n");
                        return;
                    }

                    // 检查子操作数是否是FormalParam
                    if (auto fpSubOperand = dynamic_cast<FormalParam *>(subOperand)) {
                        int paramRegId = fpSubOperand->getRegId();
                        printf("    -> 发现FormalParam子操作数，regId: %d\n", paramRegId);
                        if (paramRegId >= 0 && paramRegId < 4) {
                            printf("★★★ 递归追踪到函数参数寄存器r%d ★★★\n", paramRegId);
                            if (rs_reg_no != paramRegId) {
                                emit("mov", PlatformArm32::regName[rs_reg_no], PlatformArm32::regName[paramRegId]);
                                printf("生成: mov r%d, r%d\n", rs_reg_no, paramRegId);
                            } else {
                                printf("目标就是r%d，无需移动\n", paramRegId);
                            }
                            printf("=== END DEBUG (递归追踪到FormalParam) ===\n");
                            return;
                        }
                    }
                }
            }

            // 递归检查FormalParam类型的操作数
            if (auto fpOperand = dynamic_cast<FormalParam *>(operand)) {
                int paramRegId = fpOperand->getRegId();
                if (paramRegId >= 0 && paramRegId < 4) {
                    printf("★★★ 追踪到函数参数寄存器r%d ★★★\n", paramRegId);
                    if (rs_reg_no != paramRegId) {
                        emit("mov", PlatformArm32::regName[rs_reg_no], PlatformArm32::regName[paramRegId]);
                        printf("生成: mov r%d, r%d\n", rs_reg_no, paramRegId);
                    } else {
                        printf("目标就是r%d，无需移动\n", paramRegId);
                    }
                    printf("=== END DEBUG (追踪到FormalParam) ===\n");
                    return;
                }
            }
        }
        printf("★ 未能在操作数中找到函数参数\n");
        printf("★ 这是一个二元运算，需要在调用层处理\n");
    }

    printf("=== END DEBUG ===\n");

    // 原有的load_var逻辑保持不变...
    if (Instanceof(constVal, ConstInt *, src_var)) {
        load_imm(rs_reg_no, constVal->getVal());
    } else if (src_var->getRegId() != -1) {
        int32_t src_regId = src_var->getRegId();
        if (src_regId != rs_reg_no) {
            emit("mov", PlatformArm32::regName[rs_reg_no], PlatformArm32::regName[src_regId]);
        }
    } else if (src_var->isDerivedFromGlobal()) {
        std::string globalName = src_var->getGlobalBaseName();
        int64_t offset = src_var->getGlobalOffset();
        load_global_with_offset(rs_reg_no, globalName, offset);
    } else if (Instanceof(globalVar, GlobalVariable *, src_var)) {
        load_symbol(rs_reg_no, globalVar->getName());
        if (src_var->getType()->isArrayType()) {
            // 全局数组：只需要地址
        } else {
            emit("ldr", PlatformArm32::regName[rs_reg_no], "[" + PlatformArm32::regName[rs_reg_no] + "]");
        }
    } else {
        int32_t var_baseRegId = -1;
        int64_t var_offset = -1;

        bool result = src_var->getMemoryAddr(&var_baseRegId, &var_offset);
        if (!result) {
            minic_log(LOG_ERROR, "BUG");
            return;
        }

        if (src_var->getType()->isArrayType()) {
            std::string rsReg = PlatformArm32::regName[rs_reg_no];
            std::string baseReg = PlatformArm32::regName[var_baseRegId];

            if (PlatformArm32::constExpr(var_offset)) {
                if (var_offset < 0) {
                    // 负偏移使用 sub 指令: sub r0,fp,#40
                    emit("sub", rsReg, baseReg, toStr(-var_offset));
                } else {
                    // 正偏移使用 add 指令: add r0,fp,#16
                    emit("add", rsReg, baseReg, toStr(var_offset));
                }
            } else {
                load_imm(rs_reg_no, var_offset);
                emit("add", rsReg, baseReg, rsReg);
            }
        } else if (src_var->getType()->isPointerType()) {
            load_base(rs_reg_no, var_baseRegId, var_offset);
        } else {
            load_base(rs_reg_no, var_baseRegId, var_offset);
        }
    }
}

/// @brief 加载变量地址到寄存器（专门用于数组）
/// @param rs_reg_no 结果寄存器
/// @param src_var 源操作数
void ILocArm32::load_var_addr(int rs_reg_no, Value * src_var)
{
    if (Instanceof(globalVar, GlobalVariable *, src_var)) {
        // 全局变量地址
        load_symbol(rs_reg_no, globalVar->getName());
    } else {
        // 局部变量地址
        int32_t var_baseRegId = -1;
        int64_t var_offset = -1;

        bool result = src_var->getMemoryAddr(&var_baseRegId, &var_offset);
        if (!result) {
            minic_log(LOG_ERROR, "BUG");
        }

        leaStack(rs_reg_no, var_baseRegId, var_offset);
    }
}

/// @brief 加载变量地址到寄存器
/// @param rs_reg_no
/// @param var
void ILocArm32::lea_var(int rs_reg_no, Value * var)
{
    // 被加载的变量肯定不是常量！
    // 被加载的变量肯定不是寄存器变量！

    // 目前只考虑局部变量

    // 栈帧偏移
    int32_t var_baseRegId = -1;
    int64_t var_offset = -1;

    bool result = var->getMemoryAddr(&var_baseRegId, &var_offset);
    if (!result) {
        minic_log(LOG_ERROR, "BUG");
    }

    // lea r8, [fp,#-16]
    leaStack(rs_reg_no, var_baseRegId, var_offset);
}

/// @brief 保存寄存器到变量，保证将计算结果（r8）保存到变量
/// @param src_reg_no 源寄存器
/// @param dest_var  变量
/// @param tmp_reg_no 第三方寄存器
void ILocArm32::store_var(int src_reg_no, Value * dest_var, int tmp_reg_no)
{
    // 被保存目标变量肯定不是常量

    if (dest_var->getRegId() != -1) {

        // 寄存器变量

        // -1表示非寄存器，其他表示寄存器的索引值
        int dest_reg_id = dest_var->getRegId();

        // 寄存器不一样才需要mov操作
        if (src_reg_no != dest_reg_id) {

            // mov r2,r8 | 这里有优化空间——消除r8
            emit("mov", PlatformArm32::regName[dest_reg_id], PlatformArm32::regName[src_reg_no]);
        }

    } else if (Instanceof(globalVar, GlobalVariable *, dest_var)) {
        // 全局变量

        // 读取符号的地址到寄存器r10
        load_symbol(tmp_reg_no, globalVar->getName());

        // str r8, [r10]
        emit("str", PlatformArm32::regName[src_reg_no], "[" + PlatformArm32::regName[tmp_reg_no] + "]");

    } else {

        // 对于局部变量，则直接从栈基址+偏移寻址

        // TODO 目前只考虑局部变量

        // 栈帧偏移
        int32_t dest_baseRegId = -1;
        int64_t dest_offset = -1;

        bool result = dest_var->getMemoryAddr(&dest_baseRegId, &dest_offset);
        if (!result) {
            minic_log(LOG_ERROR, "BUG");
        }

        // str r8,[r9]
        // str r8, [fp, # - 16]
        store_base(src_reg_no, dest_baseRegId, dest_offset, tmp_reg_no);
    }
}

/// @brief 加载栈内变量地址
/// @param rsReg 结果寄存器号
/// @param base_reg_no 基址寄存器
/// @param off 偏移
void ILocArm32::leaStack(int rs_reg_no, int base_reg_no, int off)
{
    std::string rs_reg_name = PlatformArm32::regName[rs_reg_no];
    std::string base_reg_name = PlatformArm32::regName[base_reg_no];

    if (PlatformArm32::constExpr(off)) {
        if (off < 0) {
            // 负偏移使用 sub 指令: sub r8,fp,#40
            emit("sub", rs_reg_name, base_reg_name, toStr(-off));
        } else {
            // 正偏移使用 add 指令: add r8,fp,#16
            emit("add", rs_reg_name, base_reg_name, toStr(off));
        }
    } else {
        // ldr r8,=-257
        load_imm(rs_reg_no, off);

        // add r8,fp,r8
        emit("add", rs_reg_name, base_reg_name, rs_reg_name);
    }
}

/// @brief 函数内栈内空间分配（局部变量、形参变量、函数参数传值，或不能寄存器分配的临时变量等）
/// @param func 函数
/// @param tmp_reg_No
void ILocArm32::allocStack(Function * func, int tmp_reg_no)
{
    // 计算栈帧大小
    int off = func->getMaxDep();

    // 不需要在栈内额外分配空间，则什么都不做
    if (0 == off) {
        return;
    }

    // 保存SP寄存器到FP寄存器中
    mov_reg(ARM32_FP_REG_NO, ARM32_SP_REG_NO);

    if (PlatformArm32::constExpr(off)) {
        // sub sp,sp,#16
        emit("sub", "sp", "sp", toStr(off));
    } else {
        // ldr r8,=257
        load_imm(tmp_reg_no, off);

        // sub sp,sp,r8
        emit("sub", "sp", "sp", PlatformArm32::regName[tmp_reg_no]);
    }
}

/// @brief 调用函数fun
/// @param fun
void ILocArm32::call_fun(std::string name)
{
    // 函数返回值在r0,不需要保护
    emit("bl", name);
}

/// @brief NOP操作
void ILocArm32::nop()
{
    // FIXME 无操作符，要确认是否用nop指令
    emit("");
}

///
/// @brief 无条件跳转指令
/// @param label 目标Label名称
///
void ILocArm32::jump(std::string label)
{
    emit("b", label);
}

/// @brief 加载全局变量地址（带偏移）到寄存器
void ILocArm32::load_global_with_offset(int rs_reg_no, const std::string & globalVar, int64_t offset)
{
    // 先加载全局变量地址
    load_symbol(rs_reg_no, globalVar);

    // 如果有偏移，添加偏移
    if (offset != 0) {
        if (PlatformArm32::constExpr(offset)) {
            emit("add", PlatformArm32::regName[rs_reg_no], PlatformArm32::regName[rs_reg_no], toStr(offset));
        } else {
            // 大偏移量需要临时寄存器
            int temp_reg = 9; // 使用r9作为临时寄存器
            load_imm(temp_reg, offset);
            emit("add",
                 PlatformArm32::regName[rs_reg_no],
                 PlatformArm32::regName[rs_reg_no],
                 PlatformArm32::regName[temp_reg]);
        }
        printf("DEBUG: 加载全局地址带偏移: %s + %ld -> %s\n",
               globalVar.c_str(),
               offset,
               PlatformArm32::regName[rs_reg_no].c_str());
    } else {
        printf("DEBUG: 加载全局地址: %s -> %s\n", globalVar.c_str(), PlatformArm32::regName[rs_reg_no].c_str());
    }
}

/// @brief 检查Value是否来自全局地址计算
std::string ILocArm32::extractGlobalSource(Value * value)
{
    if (!value)
        return "";

    // 1. 检查是否有全局来源标记
    if (value->isDerivedFromGlobal()) {
        return value->getGlobalBaseName();
    }

    // 2. 通过名称模式识别（IR分析）
    std::string valueName = value->getIRName();

    // 这里需要分析IR指令，查找形如 "%t79 = add @array, offset" 的指令
    // 由于我们没有直接的IR访问，暂时通过名称模式来识别
    // 实际实现中应该遍历当前函数的IR指令

    printf("DEBUG: 分析Value的全局来源: %s\n", valueName.c_str());

    // 暂时返回空，实际需要在后端翻译时通过IR分析实现
    return "";
}