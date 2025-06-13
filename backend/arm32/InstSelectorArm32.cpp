///
/// @file InstSelectorArm32.cpp
/// @brief 指令选择器-ARM32的实现
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

#include "Common.h"
#include "ILocArm32.h"
#include "InstSelectorArm32.h"
#include "PlatformArm32.h"
#include "ConstInt.h"

#include "PointerType.h"
#include "RegVariable.h"
#include "Function.h"

#include "LabelInstruction.h"
#include "GotoInstruction.h"
#include "FuncCallInstruction.h"
#include "MoveInstruction.h"

/// @brief 构造函数
/// @param _irCode 指令
/// @param _iloc ILoc
/// @param _func 函数
InstSelectorArm32::InstSelectorArm32(vector<Instruction *> & _irCode,
                                     ILocArm32 & _iloc,
                                     Function * _func,
                                     SimpleRegisterAllocator & allocator)
    : ir(_irCode), iloc(_iloc), func(_func), simpleRegisterAllocator(allocator)
{
    translator_handlers[IRInstOperator::IRINST_OP_ENTRY] = &InstSelectorArm32::translate_entry;
    translator_handlers[IRInstOperator::IRINST_OP_EXIT] = &InstSelectorArm32::translate_exit;

    translator_handlers[IRInstOperator::IRINST_OP_LABEL] = &InstSelectorArm32::translate_label;
    translator_handlers[IRInstOperator::IRINST_OP_GOTO] = &InstSelectorArm32::translate_goto;

    translator_handlers[IRInstOperator::IRINST_OP_ASSIGN] = &InstSelectorArm32::translate_assign;

    translator_handlers[IRInstOperator::IRINST_OP_ADD_I] = &InstSelectorArm32::translate_add_int32;
    translator_handlers[IRInstOperator::IRINST_OP_SUB_I] = &InstSelectorArm32::translate_sub_int32;

    ///在构造函数中添加新的处理函数映射-lxg
    translator_handlers[IRInstOperator::IRINST_OP_MUL_I] = &InstSelectorArm32::translate_mul_int32;
    translator_handlers[IRInstOperator::IRINST_OP_DIV_I] = &InstSelectorArm32::translate_div_int32;
    translator_handlers[IRInstOperator::IRINST_OP_MOD_I] = &InstSelectorArm32::translate_mod_int32;
    translator_handlers[IRInstOperator::IRINST_OP_NEG_I] = &InstSelectorArm32::translate_neg_int32;

    // 添加关系运算符的处理函数映射-lxg
    translator_handlers[IRInstOperator::IRINST_OP_LT_I] = &InstSelectorArm32::translate_lt_int32;
    translator_handlers[IRInstOperator::IRINST_OP_GT_I] = &InstSelectorArm32::translate_gt_int32;
    translator_handlers[IRInstOperator::IRINST_OP_LE_I] = &InstSelectorArm32::translate_le_int32;
    translator_handlers[IRInstOperator::IRINST_OP_GE_I] = &InstSelectorArm32::translate_ge_int32;
    translator_handlers[IRInstOperator::IRINST_OP_EQ_I] = &InstSelectorArm32::translate_eq_int32;
    translator_handlers[IRInstOperator::IRINST_OP_NE_I] = &InstSelectorArm32::translate_ne_int32;

    // 添加数组相关指令的处理-lxg
    translator_handlers[IRInstOperator::IRINST_OP_STORE_PTR] = &InstSelectorArm32::translate_store_ptr;
    translator_handlers[IRInstOperator::IRINST_OP_LOAD_PTR] = &InstSelectorArm32::translate_load_ptr;
    translator_handlers[IRInstOperator::IRINST_OP_ADD_PTR] = &InstSelectorArm32::translate_add_ptr;
    translator_handlers[IRInstOperator::IRINST_OP_ARRAY_ADDR] = &InstSelectorArm32::translate_array_addr;

    translator_handlers[IRInstOperator::IRINST_OP_FUNC_CALL] = &InstSelectorArm32::translate_call;
    translator_handlers[IRInstOperator::IRINST_OP_ARG] = &InstSelectorArm32::translate_arg;
}

///
/// @brief 析构函数
///
InstSelectorArm32::~InstSelectorArm32()
{}

/// @brief 指令选择执行
void InstSelectorArm32::run()
{
    for (auto inst: ir) {

        // 逐个指令进行翻译
        if (!inst->isDead()) {
            translate(inst);
        }
    }
}

/// @brief 指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate(Instruction * inst)
{
    // 操作符
    IRInstOperator op = inst->getOp();

    std::string irStr;
    inst->toString(irStr);
    printf("Translating IR operator: %d\n", (int) op);
    // 特别检查指针相关的指令
    if (irStr.find("*") != std::string::npos) {
        printf("  -> Found pointer operation: %s\n", irStr.c_str());
    }

    map<IRInstOperator, translate_handler>::const_iterator pIter;
    pIter = translator_handlers.find(op);
    if (pIter == translator_handlers.end()) {
        // 没有找到，则说明当前不支持
        printf("Translate: Operator(%d) not support", (int) op);
        return;
    }

    // 开启时输出IR指令作为注释
    if (showLinearIR) {
        outputIRInstruction(inst);
    }

    (this->*(pIter->second))(inst);
}

///
/// @brief 输出IR指令
///
void InstSelectorArm32::outputIRInstruction(Instruction * inst)
{
    std::string irStr;
    inst->toString(irStr);
    if (!irStr.empty()) {
        iloc.comment(irStr);
    }
}

/// @brief NOP翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_nop(Instruction * inst)
{
    (void) inst;
    iloc.nop();
}

/// @brief Label指令指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_label(Instruction * inst)
{
    Instanceof(labelInst, LabelInstruction *, inst);

    iloc.label(labelInst->getName());
}

/// @brief goto指令指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_goto(Instruction * inst)
{
    // Instanceof(gotoInst, GotoInstruction *, inst);

    // // 无条件跳转
    // iloc.jump(gotoInst->getTarget()->getName());
    // 新增有条件跳转-lxg
    Instanceof(gotoInst, GotoInstruction *, inst);
    // 检查是否是条件跳转
    if (gotoInst->getOperandsNum() > 0) {
        // 这是条件跳转
        Value * condition = gotoInst->getOperand(0);
        std::string trueLabel = gotoInst->getTarget()->getName();
        std::string falseLabel = gotoInst->getFalseTarget()->getName();

        // 加载条件到寄存器中
        int condRegNo = simpleRegisterAllocator.Allocate(condition);
        iloc.load_var(condRegNo, condition);

        // 比较与0
        iloc.inst("cmp", PlatformArm32::regName[condRegNo], "#0");

        // 如果不等于0，跳转到trueLabel
        iloc.inst("bne", trueLabel);

        // 否则跳转到falseLabel
        iloc.inst("b", falseLabel);

        // 释放条件寄存器
        simpleRegisterAllocator.free(condition);
    } else {
        // 无条件跳转
        iloc.jump(gotoInst->getTarget()->getName());
    }
}

/// @brief 函数入口指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_entry(Instruction * inst)
{
    // 查看保护的寄存器
    auto & protectedRegNo = func->getProtectedReg();
    auto & protectedRegStr = func->getProtectedRegStr();

    bool first = true;
    for (auto regno: protectedRegNo) {
        if (first) {
            protectedRegStr = PlatformArm32::regName[regno];
            first = false;
        } else {
            protectedRegStr += "," + PlatformArm32::regName[regno];
        }
    }

    if (!protectedRegStr.empty()) {
        iloc.inst("push", "{" + protectedRegStr + "}");
    }

    // 为fun分配栈帧，含局部变量、函数调用值传递的空间等
    iloc.allocStack(func, ARM32_TMP_REG_NO);
}

/// @brief 函数出口指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_exit(Instruction * inst)
{
    if (inst->getOperandsNum()) {
        // 存在返回值
        Value * retVal = inst->getOperand(0);

        // 赋值给寄存器R0
        iloc.load_var(0, retVal);
    }

    // 恢复栈空间
    iloc.inst("mov", "sp", "fp");

    // 保护寄存器的恢复
    auto & protectedRegStr = func->getProtectedRegStr();
    if (!protectedRegStr.empty()) {
        iloc.inst("pop", "{" + protectedRegStr + "}");
    }

    iloc.inst("bx", "lr");
}

/// @brief 赋值指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_assign(Instruction * inst)
{
    Value * result = inst->getOperand(0);
    Value * arg1 = inst->getOperand(1);

    // 简化的赋值逻辑，移除复杂的指针解引用处理
    // 指针解引用应该通过专门的 LOAD_PTR 指令处理
    // 获取指令的字符串表示进行调试
    std::string irStr;
    inst->toString(irStr);
    printf("ASSIGN: %s\n", irStr.c_str());
    // 检查是否是指针解引用：%l10 = *%l9
    if (irStr.find(" = *") != std::string::npos) {
        printf("  -> Detected pointer dereference, calling translate_load_ptr\n");
        translate_load_ptr(inst);
        return;
    }

    // 检查是否是指针存储：*%l9 = 1
    if (irStr.find("*") == 0 || (irStr.find(" *") != std::string::npos && irStr.find(" = ") != std::string::npos)) {
        printf("  -> Detected pointer store, calling translate_store_ptr\n");
        translate_store_ptr(inst);
        return;
    }

    int32_t arg1_regId = arg1->getRegId();
    int32_t result_regId = result->getRegId();

    if (arg1_regId != -1) {
        iloc.store_var(arg1_regId, result, ARM32_TMP_REG_NO);
    } else if (result_regId != -1) {
        iloc.load_var(result_regId, arg1);
    } else {
        int32_t temp_regno = simpleRegisterAllocator.Allocate();
        iloc.load_var(temp_regno, arg1);
        iloc.store_var(temp_regno, result, ARM32_TMP_REG_NO);
        simpleRegisterAllocator.free(temp_regno);
    }
}

/// @brief 二元操作指令翻译成ARM32汇编
/// @param inst IR指令
/// @param operator_name 操作码
/// @param rs_reg_no 结果寄存器号
/// @param op1_reg_no 源操作数1寄存器号
/// @param op2_reg_no 源操作数2寄存器号
void InstSelectorArm32::translate_two_operator(Instruction * inst, string operator_name)
{
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = inst->getRegId();
    int32_t load_result_reg_no, load_arg1_reg_no, load_arg2_reg_no;

    // 看arg1是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg1_reg_no == -1) {

        // 分配一个寄存器r8
        load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);

        // arg1 -> r8，这里可能由于偏移不满足指令的要求，需要额外分配寄存器
        iloc.load_var(load_arg1_reg_no, arg1);
    } else {
        load_arg1_reg_no = arg1_reg_no;
    }

    // 看arg2是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg2_reg_no == -1) {

        // 分配一个寄存器r9
        load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);

        // arg2 -> r9
        iloc.load_var(load_arg2_reg_no, arg2);
    } else {
        load_arg2_reg_no = arg2_reg_no;
    }

    // 看结果变量是否是寄存器，若不是则需要分配一个新的寄存器来保存运算的结果
    if (result_reg_no == -1) {
        // 分配一个寄存器r10，用于暂存结果
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
    } else {
        load_result_reg_no = result_reg_no;
    }

    // r8 + r9 -> r10
    iloc.inst(operator_name,
              PlatformArm32::regName[load_result_reg_no],
              PlatformArm32::regName[load_arg1_reg_no],
              PlatformArm32::regName[load_arg2_reg_no]);

    // 结果不是寄存器，则需要把rs_reg_name保存到结果变量中
    if (result_reg_no == -1) {

        // 这里使用预留的临时寄存器，因为立即数可能过大，必须借助寄存器才可操作。

        // r10 -> result
        iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(arg1);
    simpleRegisterAllocator.free(arg2);
    simpleRegisterAllocator.free(result);
}

/// @brief 整数加法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_add_int32(Instruction * inst)
{
    translate_two_operator(inst, "add");
}

/// @brief 整数减法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_sub_int32(Instruction * inst)
{
    translate_two_operator(inst, "sub");
}

// 修改现有的translate_mul_int32函数
void InstSelectorArm32::translate_mul_int32(Instruction * inst)
{
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    // 检查是否是常数乘法，可以用移位优化（针对数组索引计算）
    ConstInt * const_val = nullptr;
    Value * var_val = nullptr;

    if (dynamic_cast<ConstInt *>(arg1)) {
        const_val = dynamic_cast<ConstInt *>(arg1);
        var_val = arg2;
    } else if (dynamic_cast<ConstInt *>(arg2)) {
        const_val = dynamic_cast<ConstInt *>(arg2);
        var_val = arg1;
    }

    if (const_val && isPowerOfTwo(const_val->getVal())) {
        // 使用移位指令优化
        int shift_amount = 0;
        int value = const_val->getVal();
        while (value > 1) {
            value >>= 1;
            shift_amount++;
        }

        int32_t var_reg = simpleRegisterAllocator.Allocate();
        int32_t result_reg = simpleRegisterAllocator.Allocate();

        iloc.load_var(var_reg, var_val);

        if (shift_amount == 0) {
            // 乘以1，直接移动
            iloc.mov_reg(result_reg, var_reg);
        } else {
            // 左移优化
            iloc.inst("lsl",
                      PlatformArm32::regName[result_reg],
                      PlatformArm32::regName[var_reg],
                      "#" + std::to_string(shift_amount));
        }

        iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);

        simpleRegisterAllocator.free(var_reg);
        simpleRegisterAllocator.free(result_reg);
    } else {
        // 使用通用的乘法处理
        translate_two_operator(inst, "mul");
    }
}

// 添加辅助函数
bool InstSelectorArm32::isPowerOfTwo(int value)
{
    return value > 0 && (value & (value - 1)) == 0;
}

/// @brief 函数调用指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_call(Instruction * inst)
{
    FuncCallInstruction * callInst = dynamic_cast<FuncCallInstruction *>(inst);

    int32_t operandNum = callInst->getOperandsNum();

    if (operandNum != realArgCount) {

        // 两者不一致 也可能没有ARG指令，正常
        if (realArgCount != 0) {

            minic_log(LOG_ERROR, "ARG指令的个数与调用函数个数不一致");
        }
    }

    if (operandNum) {

        // 强制占用这几个寄存器参数传递的寄存器
        simpleRegisterAllocator.Allocate(0);
        simpleRegisterAllocator.Allocate(1);
        simpleRegisterAllocator.Allocate(2);
        simpleRegisterAllocator.Allocate(3);

        // 前四个的后面参数采用栈传递
        int esp = 0;
        for (int32_t k = 4; k < operandNum; k++) {

            auto arg = callInst->getOperand(k);

            // 新建一个内存变量，用于栈传值到形参变量中
            MemVariable * newVal = func->newMemVariable((Type *) PointerType::get(arg->getType()));
            newVal->setMemoryAddr(ARM32_SP_REG_NO, esp);
            esp += 4;

            Instruction * assignInst = new MoveInstruction(func, newVal, arg);

            // 翻译赋值指令
            translate_assign(assignInst);

            delete assignInst;
        }

        for (int32_t k = 0; k < operandNum && k < 4; k++) {

            auto arg = callInst->getOperand(k);

            // 检查实参的类型是否是临时变量。
            // 如果是临时变量，该变量可更改为寄存器变量即可，或者设置寄存器号
            // 如果不是，则必须开辟一个寄存器变量，然后赋值即可

            Instruction * assignInst = new MoveInstruction(func, PlatformArm32::intRegVal[k], arg);

            // 翻译赋值指令
            translate_assign(assignInst);

            delete assignInst;
        }
    }

    iloc.call_fun(callInst->getName());

    if (operandNum) {
        simpleRegisterAllocator.free(0);
        simpleRegisterAllocator.free(1);
        simpleRegisterAllocator.free(2);
        simpleRegisterAllocator.free(3);
    }

    // 赋值指令
    if (callInst->hasResultValue()) {

        // 新建一个赋值操作
        Instruction * assignInst = new MoveInstruction(func, callInst, PlatformArm32::intRegVal[0]);

        // 翻译赋值指令
        translate_assign(assignInst);

        delete assignInst;
    }

    // 函数调用后清零，使得下次可正常统计
    realArgCount = 0;
}

void InstSelectorArm32::translate_add_ptr(Instruction * inst)
{
    // 指针/数组地址计算：base_addr + offset
    Value * result = inst;
    Value * base = inst->getOperand(0);   // 数组基址
    Value * offset = inst->getOperand(1); // 字节偏移量

    int32_t base_reg = simpleRegisterAllocator.Allocate();
    int32_t offset_reg = simpleRegisterAllocator.Allocate();
    int32_t result_reg = simpleRegisterAllocator.Allocate();

    // 加载数组基址 - 注意这里应该是地址，不是值
    if (base->getType()->isArrayType()) {
        // 对于数组类型，load_var应该返回数组首地址
        iloc.load_var(base_reg, base);
    } else {
        // 对于指针类型，load_var返回指针值
        iloc.load_var(base_reg, base);
    }

    // 加载偏移量
    iloc.load_var(offset_reg, offset);

    // 计算最终地址：base + offset
    iloc.inst("add",
              PlatformArm32::regName[result_reg],
              PlatformArm32::regName[base_reg],
              PlatformArm32::regName[offset_reg]);

    // 存储结果地址
    iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);

    simpleRegisterAllocator.free(base_reg);
    simpleRegisterAllocator.free(offset_reg);
    simpleRegisterAllocator.free(result_reg);
}

void InstSelectorArm32::translate_array_addr(Instruction * inst)
{
    // 数组地址计算，通常是基址 + 偏移
    translate_two_operator(inst, "add");
}

///
/// @brief 实参指令翻译成ARM32汇编
/// @param inst
///
void InstSelectorArm32::translate_arg(Instruction * inst)
{
    // 翻译之前必须确保源操作数要么是寄存器，要么是内存，否则出错。
    Value * src = inst->getOperand(0);

    // 当前统计的ARG指令个数
    int32_t regId = src->getRegId();

    if (realArgCount < 4) {
        // 前四个参数
        if (regId != -1) {
            if (regId != realArgCount) {
                // 肯定寄存器分配有误
                minic_log(LOG_ERROR, "第%d个ARG指令对象寄存器分配有误: %d", argCount + 1, regId);
            }
        } else {
            minic_log(LOG_ERROR, "第%d个ARG指令对象不是寄存器", argCount + 1);
        }
    } else {
        // 必须是内存分配，若不是则出错
        int32_t baseRegId;
        bool result = src->getMemoryAddr(&baseRegId);
        if ((!result) || (baseRegId != ARM32_SP_REG_NO)) {

            minic_log(LOG_ERROR, "第%d个ARG指令对象不是SP寄存器寻址", argCount + 1);
        }
    }

    realArgCount++;
}

void InstSelectorArm32::translate_store_ptr(Instruction * inst)
{
    printf("Executing translate_store_ptr\n");

    // 对于 *%l9 = 1 这种ASSIGN指令
    Value * ptrDest = inst->getOperand(0); // 这个可能是 "*%l9" 的表示
    Value * value = inst->getOperand(1);   // 要存储的值

    // 需要从指针表达式中提取实际的指针变量
    std::string ptrName = ptrDest->getName();
    printf("  -> Pointer destination name: %s\n", ptrName.c_str());

    // 如果名称以*开头，我们需要找到实际的指针变量
    Value * actualPtrVar = nullptr;
    if (ptrName.find("*") == 0) {
        // 去掉*前缀
        std::string realPtrName = ptrName.substr(1);
        printf("  -> Looking for actual pointer variable: %s\n", realPtrName.c_str());

        // 从函数的变量列表中查找实际的指针变量
        for (auto var: func->getVarValues()) {
            if (var->getName() == realPtrName) {
                actualPtrVar = var;
                break;
            }
        }

        if (!actualPtrVar) {
            printf("ERROR: Cannot find pointer variable %s\n", realPtrName.c_str());
            return;
        }
    } else {
        // 直接使用目标操作数
        actualPtrVar = ptrDest;
    }

    int32_t ptr_reg = simpleRegisterAllocator.Allocate();
    int32_t value_reg = simpleRegisterAllocator.Allocate();

    // 加载指针地址
    iloc.load_var(ptr_reg, actualPtrVar);

    // 加载要存储的值
    iloc.load_var(value_reg, value);

    // 存储到指针指向的地址
    iloc.inst("str", PlatformArm32::regName[value_reg], "[" + PlatformArm32::regName[ptr_reg] + "]");

    simpleRegisterAllocator.free(ptr_reg);
    simpleRegisterAllocator.free(value_reg);
}

/// @brief 加载指针指向的值到寄存器
void InstSelectorArm32::translate_load_ptr(Instruction * inst)
{
    printf("Executing translate_load_ptr\n");

    // 对于 %l10 = *%l9 这种ASSIGN指令
    Value * result = inst->getOperand(0); // %l10
    Value * ptrSrc = inst->getOperand(1); // 这个可能是 "*%l9" 的表示

    // 需要从指针表达式中提取实际的指针变量
    // 检查操作数的名称
    std::string ptrName = ptrSrc->getName();
    printf("  -> Pointer source name: %s\n", ptrName.c_str());

    // 如果名称以*开头，我们需要找到实际的指针变量
    Value * actualPtrVar = nullptr;
    if (ptrName.find("*") == 0) {
        // 去掉*前缀
        std::string realPtrName = ptrName.substr(1);
        printf("  -> Looking for actual pointer variable: %s\n", realPtrName.c_str());

        // 从函数的变量列表中查找实际的指针变量
        for (auto var: func->getVarValues()) {
            if (var->getName() == realPtrName) {
                actualPtrVar = var;
                break;
            }
        }

        if (!actualPtrVar) {
            printf("ERROR: Cannot find pointer variable %s\n", realPtrName.c_str());
            return;
        }
    } else {
        // 直接使用源操作数
        actualPtrVar = ptrSrc;
    }

    int32_t ptr_reg = simpleRegisterAllocator.Allocate();
    int32_t result_reg = simpleRegisterAllocator.Allocate();

    // 加载指针值（地址）
    iloc.load_var(ptr_reg, actualPtrVar);

    // 从指针地址加载数据：ldr result_reg, [ptr_reg]
    iloc.inst("ldr", PlatformArm32::regName[result_reg], "[" + PlatformArm32::regName[ptr_reg] + "]");

    // 存储到结果变量
    iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);

    simpleRegisterAllocator.free(ptr_reg);
    simpleRegisterAllocator.free(result_reg);
}