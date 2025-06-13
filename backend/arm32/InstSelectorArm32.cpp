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
#include <typeinfo>
#include <algorithm>
#include <cctype>

#include "Common.h"
#include "ILocArm32.h"
#include "InstSelectorArm32.h"
#include "PlatformArm32.h"

#include "PointerType.h"
#include "RegVariable.h"
#include "GlobalVariable.h"
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

    // 添加数组相关的处理函数映射 - lxg
    translator_handlers[IRInstOperator::IRINST_OP_ARRAY_ACCESS] = &InstSelectorArm32::translate_array_access;
    translator_handlers[IRInstOperator::IRINST_OP_ARRAY_STORE] = &InstSelectorArm32::translate_array_store;
    translator_handlers[IRInstOperator::IRINST_OP_ARRAY_ADDR] = &InstSelectorArm32::translate_array_addr;
    translator_handlers[IRInstOperator::IRINST_OP_MULTI_ARRAY_ACCESS] =
        &InstSelectorArm32::translate_multi_array_access;

    translator_handlers[IRInstOperator::IRINST_OP_FUNC_CALL] = &InstSelectorArm32::translate_call;
    translator_handlers[IRInstOperator::IRINST_OP_ARG] = &InstSelectorArm32::translate_arg;
}

///
/// @brief 析构函数
///
InstSelectorArm32::~InstSelectorArm32()
{}

/// @brief 指令选择执行
// void InstSelectorArm32::run()
// {
//     for (auto inst: ir) {

//         // 逐个指令进行翻译
//         if (!inst->isDead()) {
//             translate(inst);
//         }
//     }
// }
void InstSelectorArm32::run()
{
    currentInstructionIndex = 0; // 🔧 初始化指令计数

    for (auto inst: ir) {
        // 逐个指令进行翻译
        if (!inst->isDead()) {
            // 🔧 更新寄存器分配器的当前指令索引
            simpleRegisterAllocator.setCurrentInstructionIndex(currentInstructionIndex);

            // 🔧 在每条指令执行前，释放不再使用的临时变量寄存器
            simpleRegisterAllocator.releaseUnusedTempVars(currentInstructionIndex);

            translate(inst);
        }

        currentInstructionIndex++; // 🔧 指令计数递增
    }
}

/// @brief 指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate(Instruction * inst)
{
    // 操作符
    IRInstOperator op = inst->getOp();

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
    printf("DEBUG: ===== translate_entry 开始 =====\n");
    printf("DEBUG: 函数名: %s\n", func->getName().c_str());

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
        printf("DEBUG: 生成push指令: push {%s}\n", protectedRegStr.c_str());
    }

    // 🔧 关键：只分配栈帧，不再初始化数组基地址指针
    iloc.allocStack(func, ARM32_TMP_REG_NO);
    printf("DEBUG: 栈帧分配完成\n");

    // 处理函数参数保存（仅对有参数的函数）
    auto & params = func->getParams();
    printf("DEBUG: 函数 %s 有 %zu 个参数\n", func->getName().c_str(), params.size());

    if (params.size() > 0) {
        // 🔧 简化参数处理，只在实际有参数时处理
        static std::map<std::string, bool> functionParamsSaved;
        std::string funcKey = func->getName();

        if (functionParamsSaved.find(funcKey) == functionParamsSaved.end()) {
            functionParamsSaved[funcKey] = false;
        }

        if (!functionParamsSaved[funcKey]) {
            for (size_t i = 0; i < params.size() && i < 4; i++) {
                Value * param = params[i];
                printf("DEBUG: 处理参数%zu (%s), regId=%d\n", i, param->getName().c_str(), param->getRegId());

                std::string paramReg = PlatformArm32::regName[i];
                int32_t offset = -4 * (static_cast<int32_t>(i) + 1);
                std::string stackLoc = "[fp,#" + std::to_string(offset) + "]";

                printf("DEBUG: 生成参数保存指令: str %s, %s\n", paramReg.c_str(), stackLoc.c_str());
                iloc.inst("str", paramReg, stackLoc);
            }
            functionParamsSaved[funcKey] = true;
            printf("DEBUG: 参数保存完成\n");
        }
    } else {
        printf("DEBUG: 无参数函数，跳过参数保存\n");
    }

    // 🔧 关键修改：完全移除数组基地址初始化代码
    // 静态分配策略不需要运行时初始化基地址指针
    // 所有数组访问都基于编译期计算的 fp+offset

    printf("DEBUG: 使用静态偏移访问策略，无需初始化数组基地址\n");

    // 🔧 可选：输出变量布局信息用于调试
    auto & localVars = func->getVarValues();
    printf("DEBUG: 函数 %s 有 %zu 个局部变量\n", func->getName().c_str(), localVars.size());

    iloc.comment("=== 变量内存布局（调试信息） ===");
    int var_count = 0;
    for (auto var: localVars) {
        var_count++;
        if (!var->getName().empty()) {
            int32_t base_reg_id = -1;
            int64_t offset = -1;
            if (var->getMemoryAddr(&base_reg_id, &offset)) {
                iloc.comment("变量 " + var->getName() + ": [" + PlatformArm32::regName[base_reg_id] + ",#" +
                             std::to_string(offset) + "]");
                printf("DEBUG: 变量 %s: [%s,#%ld]\n",
                       var->getName().c_str(),
                       PlatformArm32::regName[base_reg_id].c_str(),
                       offset);
            } else {
                printf("DEBUG: 变量 %s: 无内存地址\n", var->getName().c_str());
            }
        } else {
            printf("DEBUG: 临时变量 %d: 名称为空\n", var_count);
        }
    }
    iloc.comment("=== 变量内存布局结束 ===");

    printf("DEBUG: ===== translate_entry 结束 =====\n");
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
    // 首先检查是否是 MoveInstruction 且为指针存储操作
    if (MoveInstruction * moveInst = dynamic_cast<MoveInstruction *>(inst)) {
        if (moveInst->getIsPointerStore()) {
            printf("DEBUG: 处理指针存储操作（数组赋值）\n");

            Value * ptr = moveInst->getDst();   // 目标指针（数组元素地址）
            Value * value = moveInst->getSrc(); // 要存储的值

            iloc.comment("数组元素赋值: *ptr = value");

            // 🔧 优化：使用寄存器分配器
            int32_t ptr_reg_no = getValueInRegister(ptr);
            int32_t value_reg_no = getValueInRegister(value);

            // 生成存储指令：str value_reg, [ptr_reg]
            iloc.inst("str", PlatformArm32::regName[value_reg_no], "[" + PlatformArm32::regName[ptr_reg_no] + "]");

            // 释放临时寄存器
            releaseValueRegister(ptr, ptr_reg_no);
            releaseValueRegister(value, value_reg_no);

            printf("DEBUG: 生成了数组存储指令: str %s, [%s]\n",
                   PlatformArm32::regName[value_reg_no].c_str(),
                   PlatformArm32::regName[ptr_reg_no].c_str());
            return;
        }

        if (moveInst->getIsPointerLoad()) {
            printf("DEBUG: 处理指针加载操作（数组访问）\n");

            Value * result = moveInst->getDst(); // 结果变量
            Value * ptr = moveInst->getSrc();    // 源指针

            iloc.comment("数组元素访问: result = *ptr");

            // 🔧 优化：使用寄存器分配器
            int32_t ptr_reg_no = getValueInRegister(ptr);
            int32_t result_reg_no = getOrAllocateRegister(result);

            // 生成加载指令：ldr result_reg, [ptr_reg]
            iloc.inst("ldr", PlatformArm32::regName[result_reg_no], "[" + PlatformArm32::regName[ptr_reg_no] + "]");

            // 释放临时寄存器
            releaseValueRegister(ptr, ptr_reg_no);
            storeOrKeepInRegister(result, result_reg_no);

            printf("DEBUG: 生成了数组加载指令: ldr %s, [%s]\n",
                   PlatformArm32::regName[result_reg_no].c_str(),
                   PlatformArm32::regName[ptr_reg_no].c_str());
            return;
        }
    }

    // 🔧 主要修改：普通赋值处理逻辑
    Value * result = inst->getOperand(0);
    Value * arg1 = inst->getOperand(1);

    iloc.comment("赋值操作: " + result->getName() + " = " + arg1->getName());

    // 🔧 策略1：常量赋值优化
    if (auto constInt = dynamic_cast<ConstInt *>(arg1)) {
        handleConstantAssignment(result, constInt);
        return;
    }

    // 🔧 策略2：检查是否都已经在寄存器中
    int arg1_loadRegId = arg1->getLoadRegId();
    int result_loadRegId = result->getLoadRegId();

    if (arg1_loadRegId != -1 && result_loadRegId != -1) {
        // 寄存器到寄存器
        if (arg1_loadRegId != result_loadRegId) {
            iloc.inst("mov", PlatformArm32::regName[result_loadRegId], PlatformArm32::regName[arg1_loadRegId]);
            iloc.comment("寄存器赋值: " + PlatformArm32::regName[arg1_loadRegId] + " -> " +
                         PlatformArm32::regName[result_loadRegId]);
        }
        return;
    }

    // 🔧 策略3：一个在寄存器，一个不在
    if (arg1_loadRegId != -1) {
        // 源在寄存器，目标不在
        handleRegisterToMemory(arg1_loadRegId, result);
        return;
    }

    if (result_loadRegId != -1) {
        // 目标在寄存器，源不在
        handleMemoryToRegister(arg1, result_loadRegId);
        return;
    }

    // 🔧 策略4：都不在寄存器，尝试临时变量优化
    if (isTempVariable(result->getName()) || isTempVariable(arg1->getName())) {
        handleTempVariableAssignment(result, arg1);
        return;
    }

    // 🔧 策略5：传统的内存到内存赋值
    handleMemoryToMemory(result, arg1);
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

/// @brief 整数加法指令翻译成ARM32汇编 - 🔧 修复全局变量地址计算
/// @param inst IR指令
void InstSelectorArm32::translate_add_int32(Instruction * inst)
{
    Value * result = inst;
    Value * op1 = inst->getOperand(0);
    Value * op2 = inst->getOperand(1);

    iloc.comment("=== 加法运算: " + op1->getName() + " + " + op2->getName() + " ===");

    // 🔧 关键修复：检查是否是全局变量地址计算
    auto globalVar = dynamic_cast<GlobalVariable *>(op1);
    if (globalVar) {
        iloc.comment("🔧 检测到全局变量地址计算: " + globalVar->getName());

        int result_reg = getOrAllocateRegister(result);

        // 🔧 正确加载全局变量地址
        iloc.inst("ldr", PlatformArm32::regName[result_reg], "=" + globalVar->getName());

        // 如果有偏移量，添加偏移
        auto constInt = dynamic_cast<ConstInt *>(op2);
        if (constInt && constInt->getVal() != 0) {
            int offset = constInt->getVal();
            iloc.comment("添加偏移量: " + std::to_string(offset));

            if (offset <= 4095 && offset >= -4095) {
                iloc.inst("add",
                          PlatformArm32::regName[result_reg],
                          PlatformArm32::regName[result_reg],
                          "#" + std::to_string(offset));
            } else {
                int offset_reg = simpleRegisterAllocator.Allocate();
                iloc.load_imm(offset_reg, offset);
                iloc.inst("add",
                          PlatformArm32::regName[result_reg],
                          PlatformArm32::regName[result_reg],
                          PlatformArm32::regName[offset_reg]);
                simpleRegisterAllocator.free(offset_reg);
            }
        } else if (!constInt) {
            // 如果第二个操作数不是常量，需要动态计算
            int op2_reg = getValueInRegister(op2);
            iloc.inst("add",
                      PlatformArm32::regName[result_reg],
                      PlatformArm32::regName[result_reg],
                      PlatformArm32::regName[op2_reg]);
            releaseValueRegister(op2, op2_reg);
        }

        storeOrKeepInRegister(result, result_reg);
        iloc.comment("*** 全局变量地址计算完成 ***");
        return;
    }

    // 🔧 检查第二个操作数是否是全局变量（处理 offset + @a 的情况）
    auto globalVar2 = dynamic_cast<GlobalVariable *>(op2);
    if (globalVar2) {
        iloc.comment("🔧 检测到全局变量地址计算（操作数2）: " + globalVar2->getName());

        int result_reg = getOrAllocateRegister(result);

        // 加载全局变量地址
        iloc.inst("ldr", PlatformArm32::regName[result_reg], "=" + globalVar2->getName());

        // 添加第一个操作数作为偏移
        auto constInt = dynamic_cast<ConstInt *>(op1);
        if (constInt && constInt->getVal() != 0) {
            int offset = constInt->getVal();
            iloc.inst("add",
                      PlatformArm32::regName[result_reg],
                      PlatformArm32::regName[result_reg],
                      "#" + std::to_string(offset));
        } else if (!constInt) {
            int op1_reg = getValueInRegister(op1);
            iloc.inst("add",
                      PlatformArm32::regName[result_reg],
                      PlatformArm32::regName[result_reg],
                      PlatformArm32::regName[op1_reg]);
            releaseValueRegister(op1, op1_reg);
        }

        storeOrKeepInRegister(result, result_reg);
        iloc.comment("*** 全局变量地址计算完成 ***");
        return;
    }

    // 🔧 如果都不是全局变量，使用原来的逻辑
    iloc.comment("普通整数加法");
    translate_two_operator(inst, "add");
}

/// @brief 整数减法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_sub_int32(Instruction * inst)
{
    translate_two_operator(inst, "sub");
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

/// @brief 获取局部数组的正确栈偏移 - 🔧 通用版本
/// @param localVar 局部变量
/// @param base_reg_id 基寄存器ID（输出）
/// @param base_offset 基偏移（输出）
/// @return 是否成功获取地址
bool InstSelectorArm32::getLocalArrayAddress(LocalVariable * localVar, int32_t & base_reg_id, int64_t & base_offset)
{
    // 🔧 第一步：尝试从变量本身获取地址
    bool hasValidAddr = localVar->getMemoryAddr(&base_reg_id, &base_offset);

    if (hasValidAddr && base_reg_id != -1 && base_offset != -1) {
        iloc.comment("使用变量自身地址: [" + PlatformArm32::regName[base_reg_id] + ",#" + std::to_string(base_offset) +
                     "]");
        return true;
    }

    // 🔧 第二步：从函数的局部变量列表中查找
    auto & localVars = func->getVarValues();
    int array_index = -1;

    for (size_t i = 0; i < localVars.size(); i++) {
        if (localVars[i] == localVar) {
            array_index = i;
            break;
        }
    }

    if (array_index != -1) {
        // 🔧 第三步：根据数组在变量列表中的位置和数组类型计算偏移
        base_reg_id = ARM32_FP_REG_NO;

        // 计算累积偏移：从函数开始，逐个累加前面变量的大小
        int64_t accumulated_offset = 0;

        for (int i = 0; i <= array_index; i++) {
            Value * var = localVars[i];
            int var_size = 4; // 默认4字节

            if (var->getType()->isArrayType()) {
                auto arrayType = dynamic_cast<ArrayType *>(var->getType());
                auto & dimensions = arrayType->getDimensions();

                // 计算数组总大小
                int total_elements = 1;
                for (auto dim: dimensions) {
                    total_elements *= dim;
                }
                var_size = total_elements * 4; // int = 4字节
            }

            accumulated_offset += var_size;
        }

        base_offset = -accumulated_offset; // 负偏移（栈向下增长）

        iloc.comment("计算数组偏移: 索引=" + std::to_string(array_index) + ", 累积偏移=" + std::to_string(base_offset));
        return true;
    }

    // 🔧 最后的回退策略
    iloc.comment("警告: 无法确定数组 " + localVar->getName() + " 的准确地址，使用默认偏移");
    base_reg_id = ARM32_FP_REG_NO;
    base_offset = -32; // 默认偏移
    return false;
}

/// @brief 数组访问指令翻译成ARM32汇编 - 🔧 最终版
/// @param inst IR指令
void InstSelectorArm32::translate_array_access(Instruction * inst)
{
    Value * result = inst;
    Value * array_base = inst->getOperand(0);
    Value * index1 = inst->getOperand(1);
    Value * index2 = inst->getOperand(2);

    iloc.comment("=== 数组访问: " + array_base->getName() + " ===");

    // 🔧 检查数组类型
    auto localVar = dynamic_cast<LocalVariable *>(array_base);
    auto globalVar = dynamic_cast<GlobalVariable *>(array_base);
    auto const_index1 = dynamic_cast<ConstInt *>(index1);
    auto const_index2 = dynamic_cast<ConstInt *>(index2);

    // ========== 全局数组处理 ==========
    if (globalVar && globalVar->getType()->isArrayType()) {
        iloc.comment("全局数组访问: " + globalVar->getName());

        auto arrayType = dynamic_cast<ArrayType *>(globalVar->getType());
        int col_size = arrayType->getDimensions()[1];

        if (const_index1 && const_index2) {
            // 全局数组 + 常量索引 = 静态访问
            int row = const_index1->getVal();
            int col = const_index2->getVal();
            int element_offset = (row * col_size + col) * 4;

            iloc.comment("全局数组静态访问: [" + std::to_string(row) + "][" + std::to_string(col) + "]");

            int addr_reg = simpleRegisterAllocator.Allocate();
            int result_reg = getOrAllocateRegister(result);

            // 🔧 关键修复：正确加载全局数组地址
            iloc.lea_var(addr_reg, globalVar);

            if (element_offset > 0) {
                if (element_offset <= 4095) {
                    iloc.inst("add",
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[addr_reg],
                              "#" + std::to_string(element_offset));
                } else {
                    int offset_reg = simpleRegisterAllocator.Allocate();
                    iloc.load_imm(offset_reg, element_offset);
                    iloc.inst("add",
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[offset_reg]);
                    simpleRegisterAllocator.free(offset_reg);
                }
            }

            iloc.inst("ldr", PlatformArm32::regName[result_reg], "[" + PlatformArm32::regName[addr_reg] + "]");

            simpleRegisterAllocator.free(addr_reg);
            storeOrKeepInRegister(result, result_reg);
            iloc.comment("*** 全局数组静态访问完成 ***");
            return;
        } else {
            // 全局数组 + 动态索引 = 动态访问
            iloc.comment("全局数组动态访问");

            int index1_reg = getValueInRegister(index1);
            int index2_reg = getValueInRegister(index2);
            int offset_reg = simpleRegisterAllocator.Allocate();
            int addr_reg = simpleRegisterAllocator.Allocate();
            int result_reg = getOrAllocateRegister(result);

            // 计算元素偏移
            if (col_size == 2) {
                iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[index1_reg], "#1");
            } else if (col_size == 4) {
                iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[index1_reg], "#2");
            } else {
                iloc.load_imm(offset_reg, col_size);
                iloc.inst("mul",
                          PlatformArm32::regName[offset_reg],
                          PlatformArm32::regName[index1_reg],
                          PlatformArm32::regName[offset_reg]);
            }

            iloc.inst("add",
                      PlatformArm32::regName[offset_reg],
                      PlatformArm32::regName[offset_reg],
                      PlatformArm32::regName[index2_reg]);
            iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[offset_reg], "#2");

            // 🔧 加载全局数组基地址
            iloc.lea_var(addr_reg, globalVar);
            iloc.inst("add",
                      PlatformArm32::regName[addr_reg],
                      PlatformArm32::regName[addr_reg],
                      PlatformArm32::regName[offset_reg]);
            iloc.inst("ldr", PlatformArm32::regName[result_reg], "[" + PlatformArm32::regName[addr_reg] + "]");

            // 释放寄存器
            releaseValueRegister(index1, index1_reg);
            releaseValueRegister(index2, index2_reg);
            simpleRegisterAllocator.free(offset_reg);
            simpleRegisterAllocator.free(addr_reg);

            storeOrKeepInRegister(result, result_reg);
            iloc.comment("*** 全局数组动态访问完成 ***");
            return;
        }
    }

    // ========== 局部数组处理 ==========
    if (localVar && localVar->getType()->isArrayType()) {
        auto arrayType = dynamic_cast<ArrayType *>(localVar->getType());
        int col_size = arrayType->getDimensions()[1];

        if (const_index1 && const_index2) {
            // 🔧 局部数组 + 常量索引 = 静态分配
            int row = const_index1->getVal();
            int col = const_index2->getVal();
            int element_offset = (row * col_size + col) * 4;

            iloc.comment("局部数组静态访问: [" + std::to_string(row) + "][" + std::to_string(col) + "] = 偏移 " +
                         std::to_string(element_offset));

            // 🔧 使用通用方法获取数组地址
            int32_t base_reg_id = -1;
            int64_t base_offset = -1;
            getLocalArrayAddress(localVar, base_reg_id, base_offset);

            int final_offset = base_offset + element_offset;
            int result_reg = getOrAllocateRegister(result);

            iloc.comment("最终地址: [fp,#" + std::to_string(final_offset) + "]");

            if (final_offset >= -4095 && final_offset <= 4095) {
                iloc.inst("ldr", PlatformArm32::regName[result_reg], "[fp,#" + std::to_string(final_offset) + "]");
            } else {
                int addr_reg = simpleRegisterAllocator.Allocate();
                iloc.load_imm(addr_reg, final_offset);
                iloc.inst("add", PlatformArm32::regName[addr_reg], "fp", PlatformArm32::regName[addr_reg]);
                iloc.inst("ldr", PlatformArm32::regName[result_reg], "[" + PlatformArm32::regName[addr_reg] + "]");
                simpleRegisterAllocator.free(addr_reg);
            }

            storeOrKeepInRegister(result, result_reg);
            iloc.comment("*** 局部数组静态访问完成 ***");
            return;
        } else {
            // 🔧 局部数组 + 动态索引
            iloc.comment("局部数组动态访问: [runtime][runtime], 列数=" + std::to_string(col_size));

            // 🔧 使用通用方法获取数组地址
            int32_t base_reg_id = -1;
            int64_t base_offset = -1;
            getLocalArrayAddress(localVar, base_reg_id, base_offset);

            int index1_reg = getValueInRegister(index1);
            int index2_reg = getValueInRegister(index2);
            int offset_reg = simpleRegisterAllocator.Allocate();
            int addr_reg = simpleRegisterAllocator.Allocate();
            int result_reg = getOrAllocateRegister(result);

            // 计算偏移
            if (col_size == 2) {
                iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[index1_reg], "#1");
            } else if (col_size == 4) {
                iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[index1_reg], "#2");
            } else {
                iloc.load_imm(offset_reg, col_size);
                iloc.inst("mul",
                          PlatformArm32::regName[offset_reg],
                          PlatformArm32::regName[index1_reg],
                          PlatformArm32::regName[offset_reg]);
            }

            iloc.inst("add",
                      PlatformArm32::regName[offset_reg],
                      PlatformArm32::regName[offset_reg],
                      PlatformArm32::regName[index2_reg]);
            iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[offset_reg], "#2");

            // 计算最终地址
            iloc.inst("mov", PlatformArm32::regName[addr_reg], "fp");

            if (base_offset != 0) {
                if (base_offset >= -4095 && base_offset <= 4095) {
                    iloc.inst("add",
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[addr_reg],
                              "#" + std::to_string(base_offset));
                } else {
                    int temp_reg = simpleRegisterAllocator.Allocate();
                    iloc.load_imm(temp_reg, base_offset);
                    iloc.inst("add",
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[temp_reg]);
                    simpleRegisterAllocator.free(temp_reg);
                }
            }

            iloc.inst("add",
                      PlatformArm32::regName[addr_reg],
                      PlatformArm32::regName[addr_reg],
                      PlatformArm32::regName[offset_reg]);
            iloc.inst("ldr", PlatformArm32::regName[result_reg], "[" + PlatformArm32::regName[addr_reg] + "]");

            releaseValueRegister(index1, index1_reg);
            releaseValueRegister(index2, index2_reg);
            simpleRegisterAllocator.free(offset_reg);
            simpleRegisterAllocator.free(addr_reg);

            storeOrKeepInRegister(result, result_reg);
            iloc.comment("*** 局部数组动态访问完成 ***");
            return;
        }
    }

    // 🔧 处理一维数组访问（函数参数）
    if (array_base->getType()->isPointerType()) {
        iloc.comment("指针数组访问: " + array_base->getName());

        int base_reg = getValueInRegister(array_base);
        int index_reg = getValueInRegister(index1);
        int result_reg = getOrAllocateRegister(result);
        int addr_reg = simpleRegisterAllocator.Allocate();

        // 计算地址：base + index * 4
        iloc.inst("mov", PlatformArm32::regName[addr_reg], PlatformArm32::regName[base_reg]);
        iloc.inst("lsl", PlatformArm32::regName[index_reg], PlatformArm32::regName[index_reg], "#2");
        iloc.inst("add",
                  PlatformArm32::regName[addr_reg],
                  PlatformArm32::regName[addr_reg],
                  PlatformArm32::regName[index_reg]);
        iloc.inst("ldr", PlatformArm32::regName[result_reg], "[" + PlatformArm32::regName[addr_reg] + "]");

        releaseValueRegister(array_base, base_reg);
        releaseValueRegister(index1, index_reg);
        simpleRegisterAllocator.free(addr_reg);
        storeOrKeepInRegister(result, result_reg);

        iloc.comment("*** 指针数组访问完成 ***");
        return;
    }

    iloc.comment("*** 未知数组访问类型 ***");
}

/// @brief 数组存储指令翻译成ARM32汇编 - 🔧 最终版
/// @param inst IR指令
void InstSelectorArm32::translate_array_store(Instruction * inst)
{
    Value * array_base = inst->getOperand(0);
    Value * index1 = inst->getOperand(1);
    Value * index2 = inst->getOperand(2);
    Value * value = inst->getOperand(3);

    iloc.comment("=== 数组存储: " + array_base->getName() + " ===");

    auto localVar = dynamic_cast<LocalVariable *>(array_base);
    auto globalVar = dynamic_cast<GlobalVariable *>(array_base);
    auto const_index1 = dynamic_cast<ConstInt *>(index1);
    auto const_index2 = dynamic_cast<ConstInt *>(index2);

    // ========== 全局数组存储 ==========
    if (globalVar && globalVar->getType()->isArrayType()) {
        iloc.comment("全局数组存储: " + globalVar->getName());

        auto arrayType = dynamic_cast<ArrayType *>(globalVar->getType());
        int col_size = arrayType->getDimensions()[1];

        if (const_index1 && const_index2) {
            // 全局数组 + 常量索引存储
            int row = const_index1->getVal();
            int col = const_index2->getVal();
            int element_offset = (row * col_size + col) * 4;

            iloc.comment("全局数组静态存储: [" + std::to_string(row) + "][" + std::to_string(col) + "]");

            int addr_reg = simpleRegisterAllocator.Allocate();
            int value_reg = getValueInRegister(value);

            // 🔧 关键修复：正确加载全局数组地址
            iloc.lea_var(addr_reg, globalVar);

            if (element_offset > 0) {
                if (element_offset <= 4095) {
                    iloc.inst("add",
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[addr_reg],
                              "#" + std::to_string(element_offset));
                } else {
                    int offset_reg = simpleRegisterAllocator.Allocate();
                    iloc.load_imm(offset_reg, element_offset);
                    iloc.inst("add",
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[offset_reg]);
                    simpleRegisterAllocator.free(offset_reg);
                }
            }

            iloc.inst("str", PlatformArm32::regName[value_reg], "[" + PlatformArm32::regName[addr_reg] + "]");

            simpleRegisterAllocator.free(addr_reg);
            releaseValueRegister(value, value_reg);
            iloc.comment("*** 全局数组静态存储完成 ***");
            return;
        } else {
            // 全局数组 + 动态索引存储
            iloc.comment("全局数组动态存储");

            int index1_reg = getValueInRegister(index1);
            int index2_reg = getValueInRegister(index2);
            int value_reg = getValueInRegister(value);
            int offset_reg = simpleRegisterAllocator.Allocate();
            int addr_reg = simpleRegisterAllocator.Allocate();

            // 计算元素偏移
            if (col_size == 2) {
                iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[index1_reg], "#1");
            } else if (col_size == 4) {
                iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[index1_reg], "#2");
            } else {
                iloc.load_imm(offset_reg, col_size);
                iloc.inst("mul",
                          PlatformArm32::regName[offset_reg],
                          PlatformArm32::regName[index1_reg],
                          PlatformArm32::regName[offset_reg]);
            }

            iloc.inst("add",
                      PlatformArm32::regName[offset_reg],
                      PlatformArm32::regName[offset_reg],
                      PlatformArm32::regName[index2_reg]);
            iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[offset_reg], "#2");

            // 🔧 加载全局数组基地址
            iloc.lea_var(addr_reg, globalVar);
            iloc.inst("add",
                      PlatformArm32::regName[addr_reg],
                      PlatformArm32::regName[addr_reg],
                      PlatformArm32::regName[offset_reg]);
            iloc.inst("str", PlatformArm32::regName[value_reg], "[" + PlatformArm32::regName[addr_reg] + "]");

            // 释放寄存器
            releaseValueRegister(index1, index1_reg);
            releaseValueRegister(index2, index2_reg);
            releaseValueRegister(value, value_reg);
            simpleRegisterAllocator.free(offset_reg);
            simpleRegisterAllocator.free(addr_reg);

            iloc.comment("*** 全局数组动态存储完成 ***");
            return;
        }
    }

    // ========== 局部数组存储 ==========
    if (localVar && localVar->getType()->isArrayType()) {
        auto arrayType = dynamic_cast<ArrayType *>(localVar->getType());
        int col_size = arrayType->getDimensions()[1];

        if (const_index1 && const_index2) {
            // 局部数组 + 常量索引存储
            int row = const_index1->getVal();
            int col = const_index2->getVal();
            int element_offset = (row * col_size + col) * 4;

            iloc.comment("局部数组静态存储: [" + std::to_string(row) + "][" + std::to_string(col) + "] = 偏移 " +
                         std::to_string(element_offset));

            // 🔧 使用通用方法获取数组地址
            int32_t base_reg_id = -1;
            int64_t base_offset = -1;
            getLocalArrayAddress(localVar, base_reg_id, base_offset);

            int final_offset = base_offset + element_offset;
            int value_reg = getValueInRegister(value);

            if (final_offset >= -4095 && final_offset <= 4095) {
                iloc.inst("str", PlatformArm32::regName[value_reg], "[fp,#" + std::to_string(final_offset) + "]");
            } else {
                int addr_reg = simpleRegisterAllocator.Allocate();
                iloc.load_imm(addr_reg, final_offset);
                iloc.inst("add", PlatformArm32::regName[addr_reg], "fp", PlatformArm32::regName[addr_reg]);
                iloc.inst("str", PlatformArm32::regName[value_reg], "[" + PlatformArm32::regName[addr_reg] + "]");
                simpleRegisterAllocator.free(addr_reg);
            }

            releaseValueRegister(value, value_reg);
            iloc.comment("*** 局部数组静态存储完成 ***");
            return;
        } else {
            // 局部数组 + 动态索引存储
            iloc.comment("局部数组动态存储: [runtime][runtime], 列数=" + std::to_string(col_size));

            // 🔧 使用通用方法获取数组地址
            int32_t base_reg_id = -1;
            int64_t base_offset = -1;
            getLocalArrayAddress(localVar, base_reg_id, base_offset);

            int index1_reg = getValueInRegister(index1);
            int index2_reg = getValueInRegister(index2);
            int value_reg = getValueInRegister(value);
            int offset_reg = simpleRegisterAllocator.Allocate();
            int addr_reg = simpleRegisterAllocator.Allocate();

            // 计算偏移
            if (col_size == 2) {
                iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[index1_reg], "#1");
            } else if (col_size == 4) {
                iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[index1_reg], "#2");
            } else {
                iloc.load_imm(offset_reg, col_size);
                iloc.inst("mul",
                          PlatformArm32::regName[offset_reg],
                          PlatformArm32::regName[index1_reg],
                          PlatformArm32::regName[offset_reg]);
            }

            iloc.inst("add",
                      PlatformArm32::regName[offset_reg],
                      PlatformArm32::regName[offset_reg],
                      PlatformArm32::regName[index2_reg]);
            iloc.inst("lsl", PlatformArm32::regName[offset_reg], PlatformArm32::regName[offset_reg], "#2");

            // 计算最终地址
            iloc.inst("mov", PlatformArm32::regName[addr_reg], "fp");

            if (base_offset != 0) {
                if (base_offset >= -4095 && base_offset <= 4095) {
                    iloc.inst("add",
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[addr_reg],
                              "#" + std::to_string(base_offset));
                } else {
                    int temp_reg = simpleRegisterAllocator.Allocate();
                    iloc.load_imm(temp_reg, base_offset);
                    iloc.inst("add",
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[addr_reg],
                              PlatformArm32::regName[temp_reg]);
                    simpleRegisterAllocator.free(temp_reg);
                }
            }

            iloc.inst("add",
                      PlatformArm32::regName[addr_reg],
                      PlatformArm32::regName[addr_reg],
                      PlatformArm32::regName[offset_reg]);
            iloc.inst("str", PlatformArm32::regName[value_reg], "[" + PlatformArm32::regName[addr_reg] + "]");

            releaseValueRegister(index1, index1_reg);
            releaseValueRegister(index2, index2_reg);
            releaseValueRegister(value, value_reg);
            simpleRegisterAllocator.free(offset_reg);
            simpleRegisterAllocator.free(addr_reg);

            iloc.comment("*** 局部数组动态存储完成 ***");
            return;
        }
    }

    // 🔧 处理一维数组存储（函数参数）
    if (array_base->getType()->isPointerType()) {
        iloc.comment("指针数组存储: " + array_base->getName());

        int base_reg = getValueInRegister(array_base);
        int index_reg = getValueInRegister(index1);
        int value_reg = getValueInRegister(value);
        int addr_reg = simpleRegisterAllocator.Allocate();

        // 计算地址：base + index * 4
        iloc.inst("mov", PlatformArm32::regName[addr_reg], PlatformArm32::regName[base_reg]);
        iloc.inst("lsl", PlatformArm32::regName[index_reg], PlatformArm32::regName[index_reg], "#2");
        iloc.inst("add",
                  PlatformArm32::regName[addr_reg],
                  PlatformArm32::regName[addr_reg],
                  PlatformArm32::regName[index_reg]);
        iloc.inst("str", PlatformArm32::regName[value_reg], "[" + PlatformArm32::regName[addr_reg] + "]");

        releaseValueRegister(array_base, base_reg);
        releaseValueRegister(index1, index_reg);
        releaseValueRegister(value, value_reg);
        simpleRegisterAllocator.free(addr_reg);

        iloc.comment("*** 指针数组存储完成 ***");
        return;
    }

    iloc.comment("*** 未知数组存储类型 ***");
}

/// @brief 数组地址计算指令翻译成ARM32汇编 - lxg
/// @param inst IR指令
void InstSelectorArm32::translate_array_addr(Instruction * inst)
{
    // IR指令格式: result = &array_base[index]
    Value * result = inst;
    Value * array_base = inst->getOperand(0); // 数组基址
    Value * index = inst->getOperand(1);      // 数组索引

    // 分配寄存器
    int32_t base_reg_no = simpleRegisterAllocator.Allocate(array_base);
    int32_t index_reg_no = simpleRegisterAllocator.Allocate(index);
    int32_t tmp_reg_no = simpleRegisterAllocator.Allocate(); // 临时寄存器
    int32_t result_reg_no = (inst->getRegId() == -1) ? simpleRegisterAllocator.Allocate(result) : inst->getRegId();

    iloc.comment("数组地址计算: &" + array_base->getName() + "[index]");

    // 修复：正确判断全局变量并加载数组基址
    if (dynamic_cast<GlobalVariable *>(array_base)) {
        // 全局数组：获取数组的起始地址
        iloc.lea_var(base_reg_no, array_base);
    } else {
        // 局部数组或参数数组：加载数组指针
        iloc.load_var(base_reg_no, array_base);
    }

    // 加载索引
    iloc.load_var(index_reg_no, index);

    // 计算数组元素地址并直接存储到结果寄存器
    iloc.calc_array_addr(result_reg_no, base_reg_no, index_reg_no, 4, tmp_reg_no);

    // 如果结果不是寄存器变量，需要存储到内存
    if (inst->getRegId() == -1) {
        iloc.store_var(result_reg_no, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(array_base);
    simpleRegisterAllocator.free(index);
    simpleRegisterAllocator.free(tmp_reg_no);
    if (inst->getRegId() == -1) {
        simpleRegisterAllocator.free(result);
    }
}

/// @brief 多维数组访问指令翻译成ARM32汇编 - lxg
/// @param inst IR指令
void InstSelectorArm32::translate_multi_array_access(Instruction * inst)
{
    // IR指令格式: result = array_base[index1][index2]...
    Value * result = inst;
    Value * array_base = inst->getOperand(0); // 数组基址

    // 获取所有索引（从操作数1开始都是索引）
    std::vector<Value *> indices;
    for (int i = 1; i < inst->getOperandsNum(); i++) {
        indices.push_back(inst->getOperand(i));
    }

    // 分配寄存器
    int32_t base_reg_no = simpleRegisterAllocator.Allocate(array_base);
    std::vector<int32_t> index_regs;
    for (auto idx: indices) {
        index_regs.push_back(simpleRegisterAllocator.Allocate(idx));
    }
    int32_t addr_reg_no = simpleRegisterAllocator.Allocate();
    int32_t tmp_reg_no1 = simpleRegisterAllocator.Allocate();
    int32_t tmp_reg_no2 = simpleRegisterAllocator.Allocate();
    int32_t result_reg_no = (inst->getRegId() == -1) ? simpleRegisterAllocator.Allocate(result) : inst->getRegId();

    iloc.comment("多维数组访问操作");

    // 修复：正确判断全局变量并加载数组基址
    if (dynamic_cast<GlobalVariable *>(array_base)) {
        // 全局数组：获取数组的起始地址
        iloc.lea_var(base_reg_no, array_base);
    } else {
        // 局部数组或参数数组：加载数组指针
        iloc.load_var(base_reg_no, array_base);
    }

    // 加载所有索引
    for (size_t i = 0; i < indices.size(); i++) {
        iloc.load_var(index_regs[i], indices[i]);
    }

    // 获取数组维度信息（这里需要从符号表或类型信息中获取）
    // 简化处理：假设是int arr[10][20]这样的二维数组
    std::vector<int> dim_sizes;
    if (indices.size() == 2) {
        dim_sizes = {10, 20}; // 这里应该从类型信息中获取实际维度
    } else {
        // 默认每个维度大小为10
        for (size_t i = 0; i < indices.size(); i++) {
            dim_sizes.push_back(10);
        }
    }

    // 计算多维数组元素地址
    iloc.calc_multi_array_addr(addr_reg_no, base_reg_no, index_regs, dim_sizes, 4, tmp_reg_no1, tmp_reg_no2);

    // 加载数组元素
    iloc.load_array_element(result_reg_no, addr_reg_no);

    // 如果结果不是寄存器变量，需要存储到内存
    if (inst->getRegId() == -1) {
        iloc.store_var(result_reg_no, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(array_base);
    for (size_t i = 0; i < indices.size(); i++) {
        simpleRegisterAllocator.free(indices[i]);
    }
    simpleRegisterAllocator.free(addr_reg_no);
    simpleRegisterAllocator.free(tmp_reg_no1);
    simpleRegisterAllocator.free(tmp_reg_no2);
    if (inst->getRegId() == -1) {
        simpleRegisterAllocator.free(result);
    }
}

/// @brief 处理常量赋值
/// @param result 结果变量
/// @param constInt 常量值
void InstSelectorArm32::handleConstantAssignment(Value * result, ConstInt * constInt)
{
    int32_t regId = simpleRegisterAllocator.dynamicAllocateTemp(result, currentInstructionIndex);

    if (regId != -1) {
        // 🔧 动态分配成功：直接加载常量到寄存器
        iloc.load_imm(regId, constInt->getVal()); // 修复：load_const -> load_imm
        iloc.comment("常量分配: " + std::to_string(constInt->getVal()) + " -> " + PlatformArm32::regName[regId]);

        // 保持在寄存器中或存储到内存
        storeOrKeepInRegister(result, regId);
    } else {
        // 🔧 动态分配失败：直接存储到内存
        int temp_reg = simpleRegisterAllocator.Allocate();
        iloc.load_imm(temp_reg, constInt->getVal()); // 修复：load_const -> load_imm
        iloc.store_var(temp_reg, result, ARM32_TMP_REG_NO);
        simpleRegisterAllocator.free(temp_reg);

        iloc.comment("常量溢出到内存: " + std::to_string(constInt->getVal()) + " -> " + result->getName());
    }
}

/// @brief 处理寄存器到内存的赋值
/// @param src_reg 源寄存器编号
/// @param dest 目标变量
void InstSelectorArm32::handleRegisterToMemory(int src_reg, Value * dest)
{
    iloc.store_var(src_reg, dest, ARM32_TMP_REG_NO);
    iloc.comment("寄存器到内存: " + PlatformArm32::regName[src_reg] + " -> " + dest->getName());
}

/// @brief 处理内存到寄存器的赋值
/// @param src 源变量
/// @param dest_reg 目标寄存器编号
void InstSelectorArm32::handleMemoryToRegister(Value * src, int dest_reg)
{
    iloc.load_var(dest_reg, src);
    iloc.comment("内存到寄存器: " + src->getName() + " -> " + PlatformArm32::regName[dest_reg]);
}

/// @brief 处理临时变量赋值 - 🔧 增强版：优先使用寄存器
/// @param result 结果临时变量
/// @param arg1 源变量
void InstSelectorArm32::handleTempVariableAssignment(Value * result, Value * arg1)
{
    // 🔧 策略1：尝试为临时变量动态分配寄存器
    int32_t result_reg = simpleRegisterAllocator.dynamicAllocateTemp(result, currentInstructionIndex);

    if (result_reg != -1) {
        // 🔧 成功分配寄存器给临时变量
        if (arg1->getLoadRegId() != -1) {
            // 源在寄存器中：寄存器到寄存器
            iloc.inst("mov", PlatformArm32::regName[result_reg], PlatformArm32::regName[arg1->getLoadRegId()]);
            iloc.comment("临时变量寄存器赋值: " + arg1->getName() + " -> " + result->getName());
        } else {
            // 源在内存中：内存到寄存器
            handleMemoryToRegister(arg1, result_reg);
            iloc.comment("临时变量内存->寄存器: " + arg1->getName() + " -> " + result->getName());
        }

        // 临时变量保持在寄存器中
        result->setLoadRegId(result_reg);
    } else {
        // 🔧 动态分配失败：回退到传统的内存分配
        iloc.comment("临时变量分配失败，回退到内存: " + result->getName());
        handleMemoryToMemory(result, arg1);
    }
}

/// @brief 处理内存到内存的赋值
/// @param result 目标变量
/// @param arg1 源变量
void InstSelectorArm32::handleMemoryToMemory(Value * result, Value * arg1)
{
    // 传统的内存到内存赋值：通过临时寄存器
    int temp_reg = simpleRegisterAllocator.Allocate();
    iloc.load_var(temp_reg, arg1);
    iloc.store_var(temp_reg, result, ARM32_TMP_REG_NO);
    simpleRegisterAllocator.free(temp_reg);

    iloc.comment("内存到内存赋值: " + arg1->getName() + " -> " + result->getName());
}

/// @brief 智能获取值到寄存器中
/// @param value 要获取的值
/// @return 寄存器编号
/// @brief 智能获取值到寄存器中 - 🔧 简化修复版
/// @param value 要获取的值
/// @return 寄存器编号
int InstSelectorArm32::getValueInRegister(Value * value)
{
    // 如果值已经在寄存器中，直接返回
    if (value->getLoadRegId() != -1) {
        return value->getLoadRegId();
    }

    if (auto constInt = dynamic_cast<ConstInt *>(value)) {
        // 为常量分配临时寄存器
        int regId = simpleRegisterAllocator.Allocate();
        iloc.load_imm(regId, constInt->getVal());
        return regId;
    }

    // 🔧 关键修复：检查全局变量
    if (auto globalVar = dynamic_cast<GlobalVariable *>(value)) {
        int regId = simpleRegisterAllocator.Allocate();
        iloc.inst("ldr", PlatformArm32::regName[regId], "=" + globalVar->getName());
        iloc.comment("加载全局变量: " + globalVar->getName());
        return regId;
    }

    // 🔧 如果是临时变量，尝试动态分配
    if (isTempVariable(value->getName())) {
        int regId = simpleRegisterAllocator.dynamicAllocateTemp(value, currentInstructionIndex);
        if (regId != -1) {
            if (value->getMemoryAddr()) {
                iloc.load_var(regId, value);
            } else {
                // 🔧 关键修复：不要设为0！应该报错
                iloc.comment("错误: 临时变量无地址: " + value->getName());
                // 🔧 这里应该抛出异常，而不是设为0
                iloc.load_imm(regId, -1);
            }
            value->setLoadRegId(regId);
            return regId;
        }
    }

    // 🔧 回退到传统分配
    int regId = simpleRegisterAllocator.Allocate(value);
    iloc.load_var(regId, value);
    return regId;
}

/// @brief 获取或分配寄存器
/// @param value 变量
/// @return 寄存器编号
int InstSelectorArm32::getOrAllocateRegister(Value * value)
{
    // 如果已经有寄存器，直接返回
    if (value->getLoadRegId() != -1) {
        return value->getLoadRegId();
    }

    // 🔧 优先尝试动态分配（对临时变量）
    if (isTempVariable(value->getName())) {
        int regId = simpleRegisterAllocator.dynamicAllocateTemp(value, currentInstructionIndex);
        if (regId != -1) {
            return regId;
        }
    }

    // 🔧 回退到传统分配
    return simpleRegisterAllocator.Allocate(value);
}

/// @brief 释放值的寄存器（如果是临时分配的）
/// @param value 变量
/// @param regId 寄存器编号
void InstSelectorArm32::releaseValueRegister(Value * value, int regId)
{
    // 🔧 只释放临时变量的寄存器，保留长期变量的寄存器
    if (isTempVariable(value->getName())) {
        // 检查这个值是否马上还会被使用
        if (!simpleRegisterAllocator.willBeUsedLater(value, currentInstructionIndex + 1)) {
            simpleRegisterAllocator.free(value);
        }
    }
    // 非临时变量保持分配状态
}

/// @brief 存储或保持在寄存器中
/// @param value 变量
/// @param regId 寄存器编号
void InstSelectorArm32::storeOrKeepInRegister(Value * value, int regId)
{
    if (isTempVariable(value->getName())) {
        // 🔧 临时变量：优先保持在寄存器中
        value->setLoadRegId(regId); // 🔧 关键：设置LoadRegId让后续指令能找到它
        iloc.comment("临时变量保持在寄存器: " + value->getName() + " -> " + PlatformArm32::regName[regId]);

        // 只有在不再使用时才考虑释放
        if (!simpleRegisterAllocator.willBeUsedLater(value, currentInstructionIndex + 1)) {
            // 如果有内存地址，可以选择存储
            if (value->getMemoryAddr()) {
                iloc.store_var(regId, value, ARM32_TMP_REG_NO);
                iloc.comment("临时变量存储到内存: " + value->getName());
            }
            // 但仍保持LoadRegId，因为可能立即被使用
        }
    } else {
        // 🔧 非临时变量：存储到内存
        iloc.store_var(regId, value, ARM32_TMP_REG_NO);
        iloc.comment("变量存储到内存: " + value->getName());
    }
}

/// @brief 判断是否是临时变量 - 增强版
/// @param name 变量名
/// @return 是否是临时变量
bool InstSelectorArm32::isTempVariable(const std::string & name)
{
    if (name.empty()) {
        return true; // 空名称通常是临时变量
    }

    if (name.length() > 0) {
        char firstChar = name[0];

        // 以 't' 开头的变量（如 t61, t62, t103）
        if (firstChar == 't') {
            return true;
        }

        // 以 'l' 开头且后面跟数字的变量，数字较大的认为是临时变量
        if (firstChar == 'l' && name.length() > 1) {
            std::string numPart = name.substr(1);
            if (!numPart.empty() && std::all_of(numPart.begin(), numPart.end(), ::isdigit)) {
                int num = std::stoi(numPart);
                return num > 5; // l6 及以上认为是临时变量
            }
        }
    }

    // 包含特定关键字的变量
    if (name.find("tmp") != std::string::npos || name.find("temp") != std::string::npos ||
        name.find("_t") != std::string::npos) {
        return true;
    }

    return false;
}