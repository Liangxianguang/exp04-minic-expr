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

    if (_func) {
        printf("\n=== Memory Allocation Analysis for Function %s ===\n", _func->getName().c_str());

        printf("--- Building Function Parameter Mapping ---\n");
        const std::vector<FormalParam *> & params = _func->getParams();
        for (size_t i = 0; i < params.size(); i++) {
            FormalParam * param = params[i];

            // 建立IR名称到FormalParam的映射
            std::string irName = "%t" + std::to_string(i);
            functionParamMap[irName] = param;

            printf("映射: %s -> 参数 %s (regId: %d)\n", irName.c_str(), param->getName().c_str(), param->getRegId());
        }
        printf("✓ Function parameter mapping complete.\n");

        // 1. 检查当前内存分配状态
        printf("--- Checking Current Memory Allocation ---\n");
        _func->printMemoryLayout();

        bool hasConflicts = !_func->validateMemoryAllocation();

        if (hasConflicts) {
            printf("--- DETECTED CONFLICTS: Applying Memory Fix ---\n");
            _func->reallocateMemory();

            // 2. 验证修复结果
            printf("--- Post-Fix Validation ---\n");
            _func->printMemoryLayout();
            _func->validateMemoryAllocation();
        } else {
            printf("✓ No memory conflicts detected.\n");
        }

        printf("=== Memory Allocation Analysis Complete ===\n\n");
    }
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
    // printf("Translating IR operator: %d\n", (int) op);
    //  特别检查指针相关的指令
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

    // **新增：保存函数参数到安全寄存器**
    // 检查函数是否有参数，如果有，将参数寄存器保存到安全位置
    auto & params = func->getParams();
    if (!params.empty()) {
        printf("=== 函数 %s 开始保存参数 (总数: %zu) ===\n", func->getName().c_str(), params.size());

        for (size_t i = 0; i < params.size(); i++) {
            FormalParam * param = params[i];
            if (!param)
                continue;

            if (i < 4) {
                // 前4个参数通过寄存器传递：r0, r1, r2, r3
                int src_reg = param->getRegId(); // 应该是 r0, r1, r2, r3
                int safe_reg = 4 + i;            // 保存到 r4, r5, r6, r7

                printf("  保存寄存器参数 %s: r%d -> r%d\n", param->getName().c_str(), src_reg, safe_reg);
                iloc.inst("mov", PlatformArm32::regName[safe_reg], PlatformArm32::regName[src_reg]);

                // 更新参数的寄存器ID为安全寄存器
                param->setRegId(safe_reg);
                printf("  参数 %s 更新寄存器ID: %d -> %d\n", param->getName().c_str(), src_reg, safe_reg);
            } else {
                // 第5个及以后的参数通过栈传递
                // 这些参数已经在栈上，不需要保存，但需要设置正确的栈偏移
                printf("  栈参数 %s (索引: %zu) - 通过栈传递\n", param->getName().c_str(), i);

                // 栈参数的偏移计算：
                // 栈参数从 fp+8 开始（fp+0是保存的fp, fp+4是返回地址）
                // 第5个参数在 fp+8, 第6个在 fp+12, 以此类推
                int stack_offset = 8 + (i - 4) * 4;

                // 设置参数为栈变量（regId = -1 表示不在寄存器中）
                param->setRegId(-1);

                // 这里可能需要设置栈偏移，但具体实现依赖于内存管理系统
                printf("  栈参数 %s 偏移: fp+%d\n", param->getName().c_str(), stack_offset);
            }
        }
        printf("=== 参数保存完成 ===\n");
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
    if (irStr.find("*") == 0 && irStr.find(" = ") != std::string::npos) {
        printf("  -> Detected pointer store, calling translate_store_ptr\n");
        translate_store_ptr(inst);
        return;
    }

    int32_t arg1_regId = arg1->getRegId();
    int32_t result_regId = result->getRegId();

    if (arg1_regId != -1) {
        iloc.store_var(arg1_regId, result, ARM32_TMP_REG_NO);
    } else if (result_regId != -1) {
        // 统一使用load_var处理，这样能利用常量折叠和参数追踪逻辑
        iloc.load_var(result_regId, arg1);
    } else {
        int32_t temp_regno = simpleRegisterAllocator.Allocate();

        // 统一使用load_var处理，这样能利用常量折叠和参数追踪逻辑
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

        // 统一使用load_var处理，包括常量和变量，这样能利用常量折叠和参数追踪逻辑
        iloc.load_var(load_arg1_reg_no, arg1);
    } else {
        load_arg1_reg_no = arg1_reg_no;
    }

    // 看arg2是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg2_reg_no == -1) {
        // 分配一个寄存器r9
        load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);

        // 统一使用load_var处理，包括常量和变量，这样能利用常量折叠和参数追踪逻辑
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

        // 处理变量操作数
        ConstInt * constVar = dynamic_cast<ConstInt *>(var_val);
        if (constVar != nullptr) {
            // 变量也是常量
            iloc.inst("movw", PlatformArm32::regName[var_reg], "#:lower16:" + std::to_string(constVar->getVal()));
        } else {
            iloc.load_var(var_reg, var_val);
        }

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

    // if (operandNum) {

    //     // 强制占用这几个寄存器参数传递的寄存器
    //     simpleRegisterAllocator.Allocate(0);
    //     simpleRegisterAllocator.Allocate(1);
    //     simpleRegisterAllocator.Allocate(2);
    //     simpleRegisterAllocator.Allocate(3);

    //     // 前四个的后面参数采用栈传递
    //     int esp = 0;
    //     for (int32_t k = 4; k < operandNum; k++) {

    //         auto arg = callInst->getOperand(k);

    //         // 新建一个内存变量，用于栈传值到形参变量中
    //         MemVariable * newVal = func->newMemVariable((Type *) PointerType::get(arg->getType()));
    //         newVal->setMemoryAddr(ARM32_SP_REG_NO, esp);
    //         esp += 4;

    //         Instruction * assignInst = new MoveInstruction(func, newVal, arg);

    //         // 翻译赋值指令
    //         translate_assign(assignInst);

    //         delete assignInst;
    //     }

    //     for (int32_t k = 0; k < operandNum && k < 4; k++) {

    //         auto arg = callInst->getOperand(k);

    //         // 检查实参的类型是否是临时变量。
    //         // 如果是临时变量，该变量可更改为寄存器变量即可，或者设置寄存器号
    //         // 如果不是，则必须开辟一个寄存器变量，然后赋值即可

    //         Instruction * assignInst = new MoveInstruction(func, PlatformArm32::intRegVal[k], arg);

    //         // 翻译赋值指令
    //         translate_assign(assignInst);

    //         delete assignInst;
    //     }
    // }

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

        // 处理前4个参数：需要特殊处理全局变量和全局派生指针
        for (int32_t k = 0; k < operandNum && k < 4; k++) {
            auto arg = callInst->getOperand(k);
            std::string regName = "r" + std::to_string(k);

            printf("  Processing Arg[%d]: '%s' (%s) -> %s\n",
                   k,
                   arg->getName().c_str(),
                   arg->getGlobalSourceInfo().c_str(),
                   regName.c_str());

            // 新增：检查参数是否派生自全局变量
            if (arg->isDerivedFromGlobal()) {
                std::string globalName = arg->getGlobalBaseName();
                int64_t offset = arg->getGlobalOffset();

                printf("    -> GLOBAL DERIVED: %s + %ld -> %s\n", globalName.c_str(), offset, regName.c_str());

                // 直接生成全局地址加载到参数寄存器
                iloc.inst("movw", regName, "#:lower16:" + globalName);
                iloc.inst("movt", regName, "#:upper16:" + globalName);

                if (offset != 0) {
                    iloc.inst("add", regName, regName, "#" + std::to_string(offset));
                }

                printf("    -> Generated global address loading for %s\n", regName.c_str());
            }
            // 新增：检查是否是直接全局变量
            else if (!arg->getName().empty() && arg->getName()[0] == '@') {
                std::string symbolName = arg->getName().substr(1);
                printf("    -> DIRECT GLOBAL: %s -> %s\n", symbolName.c_str(), regName.c_str());

                // 加载全局变量地址到参数寄存器
                iloc.inst("movw", regName, "#:lower16:" + symbolName);
                iloc.inst("movt", regName, "#:upper16:" + symbolName);

                printf("    -> Generated direct global address loading for %s\n", regName.c_str());
            }
            // 新增：检查是否通过旧逻辑检测为全局变量
            else if (isGlobalVariable(arg)) {
                printf("    -> LEGACY GLOBAL DETECTION for arg: %s\n", arg->getName().c_str());

                std::string globalName = getGlobalVariableName(arg);
                if (globalName.empty()) {
                    globalName = "array"; // 默认全局变量名
                }

                // 使用旧逻辑：先加载指针值到寄存器
                iloc.load_var(k, arg);

                printf("    -> Used legacy global loading for %s\n", regName.c_str());
            } else {
                // 原有逻辑：局部变量和常量
                printf("    -> LOCAL/CONSTANT variable -> %s\n", regName.c_str());

                // 检查实参的类型是否是临时变量。
                // 如果是临时变量，该变量可更改为寄存器变量即可，或者设置寄存器号
                // 如果不是，则必须开辟一个寄存器变量，然后赋值即可
                Instruction * assignInst = new MoveInstruction(func, PlatformArm32::intRegVal[k], arg);

                // 翻译赋值指令
                translate_assign(assignInst);

                delete assignInst;

                printf("    -> Used assignment for local variable\n");
            }
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
    Value * result = inst;
    Value * base = inst->getOperand(0);   // 数组基址
    Value * offset = inst->getOperand(1); // 字节偏移量

    printf("=== translate_add_ptr ===\n");
    printf("  base: %s (%s)\n", base->getName().c_str(), base->getGlobalSourceInfo().c_str());
    printf("  offset: %s\n", offset->getName().c_str());

    // 新增：传播全局来源信息
    if (base->isDerivedFromGlobal()) {
        int64_t additionalOffset = 0;

        // 如果偏移是常量，可以静态计算
        if (ConstInt * constOffset = dynamic_cast<ConstInt *>(offset)) {
            additionalOffset = constOffset->getVal();
        }

        // 传播全局来源信息
        base->propagateGlobalSource(result, additionalOffset);
        printf("  -> Propagated global source to result: %s\n", result->getGlobalSourceInfo().c_str());
    }

    // 原有的代码生成逻辑保持不变
    int32_t base_reg = simpleRegisterAllocator.Allocate();
    int32_t offset_reg = simpleRegisterAllocator.Allocate();
    int32_t result_reg = simpleRegisterAllocator.Allocate();

    iloc.load_var(base_reg, base);
    iloc.load_var(offset_reg, offset);

    iloc.inst("add",
              PlatformArm32::regName[result_reg],
              PlatformArm32::regName[base_reg],
              PlatformArm32::regName[offset_reg]);

    iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);

    simpleRegisterAllocator.free(base_reg);
    simpleRegisterAllocator.free(offset_reg);
    simpleRegisterAllocator.free(result_reg);
}

void InstSelectorArm32::translate_array_addr(Instruction * inst)
{
    // 对于数组地址计算，也要传播全局来源信息
    Value * result = inst;
    Value * base = inst->getOperand(0);

    printf("=== translate_array_addr ===\n");
    printf("  base: %s (%s)\n", base->getName().c_str(), base->getGlobalSourceInfo().c_str());

    // 传播全局来源信息
    if (base->isDerivedFromGlobal()) {
        base->propagateGlobalSource(result);
        printf("  -> Propagated global source to result: %s\n", result->getGlobalSourceInfo().c_str());
    }

    // 使用通用的二元操作处理
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
    printf("=== Executing translate_store_ptr ===\n");

    // 解析 *ptr = value
    std::string irStr;
    inst->toString(irStr);
    printf("  -> IR instruction: '%s'\n", irStr.c_str());

    // 解析指令：找到指针变量和要存储的值
    size_t eqPos = irStr.find(" = ");
    if (eqPos == std::string::npos) {
        printf("  -> ERROR: Cannot parse store instruction\n");
        return;
    }

    std::string leftPart = irStr.substr(0, eqPos);
    std::string rightPart = irStr.substr(eqPos + 3);

    // 去掉左边的 * 前缀
    if (leftPart[0] == '*') {
        leftPart = leftPart.substr(1);
    }

    Value * ptrVar = nullptr;
    Value * storeValue = nullptr;

    // 修改：使用正确的 API 获取操作数
    for (int i = 0; i < inst->getOperandsNum(); i++) {
        Value * operand = inst->getOperand(i);
        if (operand && operand->getName() == leftPart) {
            ptrVar = operand;
            break;
        }
    }

    // 解析要存储的值
    if (!rightPart.empty() && rightPart.find_first_not_of("0123456789") == std::string::npos) {
        // 是常量
        try {
            int constVal = std::stoi(rightPart);
            storeValue = new ConstInt(constVal);
        } catch (const std::exception & e) {
            printf("  -> ERROR: Cannot parse constant value '%s': %s\n", rightPart.c_str(), e.what());
            return;
        }
    } else {
        // 是变量，需要在操作数中找
        for (int i = 0; i < inst->getOperandsNum(); i++) {
            Value * operand = inst->getOperand(i);
            if (operand && operand->getName() == rightPart) {
                storeValue = operand;
                break;
            }
        }
    }

    if (!ptrVar) {
        printf("  -> ERROR: Cannot find pointer variable\n");
        return;
    }

    printf("  -> ptrVar name: '%s'\n", ptrVar->getName().c_str());
    printf("  -> value name: '%s'\n", storeValue ? storeValue->getName().c_str() : "constant");

    // 其余代码保持不变...
    // 新增：优先检查全局来源
    if (ptrVar->isDerivedFromGlobal()) {
        std::string globalName = ptrVar->getGlobalBaseName();
        int64_t offset = ptrVar->getGlobalOffset();

        printf("  -> Storing to GLOBAL: %s + %ld\n", globalName.c_str(), offset);

        int32_t addr_reg = simpleRegisterAllocator.Allocate();
        int32_t value_reg = simpleRegisterAllocator.Allocate();

        // 加载全局变量基址
        iloc.inst("movw", PlatformArm32::regName[addr_reg], "#:lower16:" + globalName);
        iloc.inst("movt", PlatformArm32::regName[addr_reg], "#:upper16:" + globalName);

        // 加上偏移
        if (offset != 0) {
            iloc.inst("add",
                      PlatformArm32::regName[addr_reg],
                      PlatformArm32::regName[addr_reg],
                      "#" + std::to_string(offset));
        }

        // 加载要存储的值
        if (ConstInt * constVal = dynamic_cast<ConstInt *>(storeValue)) {
            iloc.load_imm(value_reg, constVal->getVal());
        } else {
            iloc.load_var(value_reg, storeValue);
        }

        // 存储到全局地址
        iloc.inst("str", PlatformArm32::regName[value_reg], "[" + PlatformArm32::regName[addr_reg] + "]");

        simpleRegisterAllocator.free(addr_reg);
        simpleRegisterAllocator.free(value_reg);
        printf("=== End translate_store_ptr (GLOBAL) ===\n");
        return;
    }

    // 第二层检查：直接全局变量访问
    if (!ptrVar->getName().empty() && ptrVar->getName()[0] == '@') {
        printf("  -> Detected GLOBAL variable access: %s\n", ptrVar->getName().c_str());

        std::string symbolName = ptrVar->getName().substr(1);

        int32_t ptr_reg = simpleRegisterAllocator.Allocate();
        int32_t value_reg = simpleRegisterAllocator.Allocate();

        iloc.inst("movw", PlatformArm32::regName[ptr_reg], "#:lower16:" + symbolName);
        iloc.inst("movt", PlatformArm32::regName[ptr_reg], "#:upper16:" + symbolName);

        if (ConstInt * constValue = dynamic_cast<ConstInt *>(storeValue)) {
            iloc.load_imm(value_reg, constValue->getVal());
        } else {
            iloc.load_var(value_reg, storeValue);
        }

        iloc.inst("str", PlatformArm32::regName[value_reg], "[" + PlatformArm32::regName[ptr_reg] + "]");

        simpleRegisterAllocator.free(ptr_reg);
        simpleRegisterAllocator.free(value_reg);
        printf("=== End translate_store_ptr (DIRECT GLOBAL) ===\n");
        return;
    }

    // 第三层检查：是否是函数参数
    if (ptrVar->getName().empty() && func) {
        auto & params = func->getParams();
        for (size_t i = 0; i < params.size(); i++) {
            if (params[i] == ptrVar) {
                printf("  -> Storing via PARAMETER[%zu]\n", i);

                int32_t ptr_reg = simpleRegisterAllocator.Allocate();
                int32_t value_reg = simpleRegisterAllocator.Allocate();

                // 从参数位置加载指针
                iloc.inst("ldr", PlatformArm32::regName[ptr_reg], "[fp,#" + std::to_string(8 + i * 4) + "]");

                // 加载要存储的值
                if (ConstInt * constVal = dynamic_cast<ConstInt *>(storeValue)) {
                    iloc.load_imm(value_reg, constVal->getVal());
                } else {
                    iloc.load_var(value_reg, storeValue);
                }

                // 通过指针存储值
                iloc.inst("str", PlatformArm32::regName[value_reg], "[" + PlatformArm32::regName[ptr_reg] + "]");

                simpleRegisterAllocator.free(ptr_reg);
                simpleRegisterAllocator.free(value_reg);
                printf("=== End translate_store_ptr (PARAMETER) ===\n");
                return;
            }
        }
    }

    // 第四层：使用旧的全局变量检测逻辑（兜底）
    if (isGlobalVariable(ptrVar)) {
        printf("  -> Detected via legacy global detection\n");

        int32_t ptr_reg = simpleRegisterAllocator.Allocate();
        int32_t value_reg = simpleRegisterAllocator.Allocate();

        iloc.load_var(ptr_reg, ptrVar);

        if (ConstInt * constValue = dynamic_cast<ConstInt *>(storeValue)) {
            iloc.load_imm(value_reg, constValue->getVal());
        } else {
            iloc.load_var(value_reg, storeValue);
        }

        iloc.inst("str", PlatformArm32::regName[value_reg], "[" + PlatformArm32::regName[ptr_reg] + "]");

        simpleRegisterAllocator.free(ptr_reg);
        simpleRegisterAllocator.free(value_reg);
        printf("=== End translate_store_ptr (LEGACY GLOBAL) ===\n");
        return;
    }

    // 最后：局部变量处理
    printf("  -> Falling back to LOCAL variable processing\n");

    int32_t ptr_reg = simpleRegisterAllocator.Allocate();
    int32_t value_reg = simpleRegisterAllocator.Allocate();

    iloc.load_var(ptr_reg, ptrVar);

    if (ConstInt * constValue = dynamic_cast<ConstInt *>(storeValue)) {
        iloc.inst("movw", PlatformArm32::regName[value_reg], "#" + std::to_string(constValue->getVal()));
    } else {
        iloc.load_var(value_reg, storeValue);
    }

    iloc.inst("str", PlatformArm32::regName[value_reg], "[" + PlatformArm32::regName[ptr_reg] + "]");

    simpleRegisterAllocator.free(ptr_reg);
    simpleRegisterAllocator.free(value_reg);
    printf("=== End translate_store_ptr (LOCAL) ===\n");
}

void InstSelectorArm32::translate_load_ptr(Instruction * inst)
{
    printf("Executing translate_load_ptr\n");

    // 对于 result = *ptr 这种ASSIGN指令
    Value * result = inst->getOperand(0); // 结果变量
    Value * ptrVar = inst->getOperand(1); // 指针变量

    printf("  -> result name: '%s'\n", result->getName().c_str());
    printf("  -> ptrVar name: '%s'\n", ptrVar->getName().c_str());

    // 新增：优先检查全局来源追踪
    if (ptrVar->isDerivedFromGlobal()) {
        std::string globalName = ptrVar->getGlobalBaseName();
        int64_t offset = ptrVar->getGlobalOffset();

        printf("  -> Using GLOBAL source: %s + %ld\n", globalName.c_str(), offset);

        int32_t addr_reg = simpleRegisterAllocator.Allocate();
        int32_t result_reg = simpleRegisterAllocator.Allocate();

        // 加载全局变量基址
        iloc.inst("movw", PlatformArm32::regName[addr_reg], "#:lower16:" + globalName);
        iloc.inst("movt", PlatformArm32::regName[addr_reg], "#:upper16:" + globalName);

        // 如果有偏移，加上偏移
        if (offset != 0) {
            iloc.inst("add",
                      PlatformArm32::regName[addr_reg],
                      PlatformArm32::regName[addr_reg],
                      "#" + std::to_string(offset));
        }

        // 从全局地址读取值
        iloc.inst("ldr", PlatformArm32::regName[result_reg], "[" + PlatformArm32::regName[addr_reg] + "]");

        // 存储结果
        iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);

        simpleRegisterAllocator.free(addr_reg);
        simpleRegisterAllocator.free(result_reg);
        printf("=== End translate_load_ptr (GLOBAL) ===\n");
        return;
    }

    // 第二层检查：直接全局变量访问
    if (!ptrVar->getName().empty() && ptrVar->getName()[0] == '@') {
        printf("  -> Loading from DIRECT GLOBAL variable: %s\n", ptrVar->getName().c_str());

        std::string symbolName = ptrVar->getName().substr(1);

        int32_t ptr_reg = simpleRegisterAllocator.Allocate();
        int32_t result_reg = simpleRegisterAllocator.Allocate();

        iloc.inst("movw", PlatformArm32::regName[ptr_reg], "#:lower16:" + symbolName);
        iloc.inst("movt", PlatformArm32::regName[ptr_reg], "#:upper16:" + symbolName);

        iloc.inst("ldr", PlatformArm32::regName[result_reg], "[" + PlatformArm32::regName[ptr_reg] + "]");

        iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);

        simpleRegisterAllocator.free(ptr_reg);
        simpleRegisterAllocator.free(result_reg);
        printf("=== End translate_load_ptr (DIRECT GLOBAL) ===\n");
        return;
    }

    // 第三层检查：检查是否是函数参数（参数也可能是全局数组的切片）
    if (ptrVar->getName().empty() && func) {
        // 检查是否在函数参数列表中
        auto & params = func->getParams();
        for (size_t i = 0; i < params.size(); i++) {
            if (params[i] == ptrVar) {
                printf("  -> Using PARAMETER[%zu] (likely global array slice)\n", i);

                int32_t ptr_reg = simpleRegisterAllocator.Allocate();
                int32_t result_reg = simpleRegisterAllocator.Allocate();

                // 从参数位置加载指针
                iloc.inst("ldr", PlatformArm32::regName[ptr_reg], "[fp,#" + std::to_string(8 + i * 4) + "]");

                // 从指针指向的地址读取值
                iloc.inst("ldr", PlatformArm32::regName[result_reg], "[" + PlatformArm32::regName[ptr_reg] + "]");

                iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);

                simpleRegisterAllocator.free(ptr_reg);
                simpleRegisterAllocator.free(result_reg);
                printf("=== End translate_load_ptr (PARAMETER) ===\n");
                return;
            }
        }
    }

    // 第四层：使用旧的全局变量检测逻辑（兜底）
    if (isGlobalVariable(ptrVar)) {
        printf("  -> Detected via legacy global detection\n");

        std::string globalName = getGlobalVariableName(ptrVar);
        if (globalName.empty()) {
            globalName = "array"; // 默认
        }

        int32_t ptr_reg = simpleRegisterAllocator.Allocate();
        int32_t result_reg = simpleRegisterAllocator.Allocate();

        iloc.load_var(ptr_reg, ptrVar);
        iloc.inst("ldr", PlatformArm32::regName[result_reg], "[" + PlatformArm32::regName[ptr_reg] + "]");
        iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);

        simpleRegisterAllocator.free(ptr_reg);
        simpleRegisterAllocator.free(result_reg);
        printf("=== End translate_load_ptr (LEGACY GLOBAL) ===\n");
        return;
    }

    // 最后：局部变量处理
    printf("  -> Falling back to LOCAL variable processing\n");

    int32_t ptr_reg = simpleRegisterAllocator.Allocate();
    int32_t result_reg = simpleRegisterAllocator.Allocate();

    iloc.load_var(ptr_reg, ptrVar);
    iloc.inst("ldr", PlatformArm32::regName[result_reg], "[" + PlatformArm32::regName[ptr_reg] + "]");
    iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);

    simpleRegisterAllocator.free(ptr_reg);
    simpleRegisterAllocator.free(result_reg);
    printf("=== End translate_load_ptr (LOCAL) ===\n");
}

/// @brief 检查是否为全局变量
bool InstSelectorArm32::isGlobalVariable(Value * var)
{
    if (!var) {
        printf("    -> var is null\n");
        return false;
    }

    printf("    -> Checking variable: '%s'\n", var->getName().c_str());

    // 1. 直接全局变量（以 @ 开头）
    if (!var->getName().empty() && var->getName()[0] == '@') {
        printf("    -> Direct global variable: %s\n", var->getName().c_str());
        return true;
    }

    // 2. 对于匿名变量，检查是否来自全局数组计算
    if (var->getName().empty()) {
        printf("    -> Anonymous variable, checking if derived from global\n");
        return isPointerDerivedFromGlobalAnonymous(var);
    }

    // 3. 对于命名变量，检查是否从全局变量派生
    return isPointerDerivedFromGlobal(var);
}
/// @brief 检查指针是否从全局变量派生
bool InstSelectorArm32::isPointerDerivedFromGlobal(Value * var)
{
    if (!var || var->getName().empty()) {
        return false;
    }

    printf("    -> Checking if %s is derived from global\n", var->getName().c_str());

    // 遍历当前函数的所有指令，查找该变量的定义
    for (auto inst: ir) {
        // 情况1: 赋值指令 %l1 = %t24
        if (inst->getOp() == IRInstOperator::IRINST_OP_ASSIGN && inst->hasResultValue() && inst == var) {

            Value * source = inst->getOperand(0);
            printf("      -> Found assignment: %s = %s\n",
                   var->getName().c_str(),
                   source ? source->getName().c_str() : "null");

            // 递归检查源变量（但要避免无限递归）
            if (source && source != var) {
                if (!source->getName().empty() && source->getName()[0] == '@') {
                    printf("      -> Source is direct global\n");
                    return true;
                }
                // 对于其他情况，暂时不递归以避免复杂性
            }
        }

        // 情况2: ADD_I 指令 %t24 = add @array,%t23
        if (inst->getOp() == IRInstOperator::IRINST_OP_ADD_I && inst->hasResultValue() && inst == var) {

            printf("      -> Found ADD_I instruction for %s\n", var->getName().c_str());

            // 检查操作数中是否包含全局变量
            for (int i = 0; i < inst->getOperandsNum(); i++) {
                Value * operand = inst->getOperand(i);
                if (operand && !operand->getName().empty() && operand->getName()[0] == '@') {
                    printf("      -> ADD_I operand %d is global: %s\n", i, operand->getName().c_str());
                    return true;
                }
            }
        }

        // 情况3: ADD_PTR 指令
        if (inst->getOp() == IRInstOperator::IRINST_OP_ADD_PTR && inst->hasResultValue() && inst == var) {

            printf("      -> Found ADD_PTR instruction for %s\n", var->getName().c_str());

            // 检查操作数中是否包含全局变量
            for (int i = 0; i < inst->getOperandsNum(); i++) {
                Value * operand = inst->getOperand(i);
                if (operand && !operand->getName().empty() && operand->getName()[0] == '@') {
                    printf("      -> ADD_PTR operand %d is global: %s\n", i, operand->getName().c_str());
                    return true;
                }
            }
        }

        // 情况4: ARRAY_ADDR 指令
        if (inst->getOp() == IRInstOperator::IRINST_OP_ARRAY_ADDR && inst->hasResultValue() && inst == var) {

            printf("      -> Found ARRAY_ADDR instruction for %s\n", var->getName().c_str());

            // 检查数组基址是否为全局变量
            if (inst->getOperandsNum() > 0) {
                Value * baseArray = inst->getOperand(0);
                if (baseArray && !baseArray->getName().empty() && baseArray->getName()[0] == '@') {
                    printf("      -> ARRAY_ADDR base is global: %s\n", baseArray->getName().c_str());
                    return true;
                }
            }
        }
    }

    return false;
}

/// @brief 检查匿名指针是否从全局变量派生
bool InstSelectorArm32::isPointerDerivedFromGlobalAnonymous(Value * var)
{
    if (!var) {
        return false;
    }

    printf("      -> Checking anonymous pointer for global derivation\n");

    // 遍历所有指令，查找定义该匿名变量的指令
    for (auto inst: ir) {
        if (inst->hasResultValue() && inst == var) {
            printf("      -> Found defining instruction: op=%d\n", (int) inst->getOp());

            // ADD_I 指令：检查是否是全局数组地址计算
            if (inst->getOp() == IRInstOperator::IRINST_OP_ADD_I) {
                printf("      -> ADD_I instruction found\n");
                for (int i = 0; i < inst->getOperandsNum(); i++) {
                    Value * operand = inst->getOperand(i);
                    if (operand) {
                        printf("      -> Operand %d: '%s'\n", i, operand->getName().c_str());
                        if (!operand->getName().empty() && operand->getName()[0] == '@') {
                            printf("      -> Found global array operand: %s\n", operand->getName().c_str());
                            return true;
                        }
                    }
                }
            }

            // ADD_PTR 指令
            if (inst->getOp() == IRInstOperator::IRINST_OP_ADD_PTR) {
                printf("      -> ADD_PTR instruction found\n");
                for (int i = 0; i < inst->getOperandsNum(); i++) {
                    Value * operand = inst->getOperand(i);
                    if (operand) {
                        printf("      -> Operand %d: '%s'\n", i, operand->getName().c_str());
                        if (!operand->getName().empty() && operand->getName()[0] == '@') {
                            printf("      -> Found global array operand: %s\n", operand->getName().c_str());
                            return true;
                        }
                    }
                }
            }

            // ARRAY_ADDR 指令
            if (inst->getOp() == IRInstOperator::IRINST_OP_ARRAY_ADDR) {
                printf("      -> ARRAY_ADDR instruction found\n");
                if (inst->getOperandsNum() > 0) {
                    Value * baseArray = inst->getOperand(0);
                    if (baseArray) {
                        printf("      -> Base array: '%s'\n", baseArray->getName().c_str());
                        if (!baseArray->getName().empty() && baseArray->getName()[0] == '@') {
                            printf("      -> Found global array base: %s\n", baseArray->getName().c_str());
                            return true;
                        }
                    }
                }
            }

            // ASSIGN 指令：检查源操作数
            if (inst->getOp() == IRInstOperator::IRINST_OP_ASSIGN) {
                printf("      -> ASSIGN instruction found\n");
                Value * source = inst->getOperand(0);
                if (source) {
                    printf("      -> Source: '%s'\n", source->getName().c_str());
                    // 递归检查源变量
                    return isGlobalVariable(source);
                }
            }
        }
    }

    printf("      -> No global derivation found for anonymous variable\n");
    return false;
}

/// @brief 获取全局变量名称（去掉@前缀）
std::string InstSelectorArm32::getGlobalVariableName(Value * var)
{
    if (!var) {
        return "";
    }

    printf("    -> Getting global name for: '%s'\n", var->getName().c_str());

    // 1. 直接全局变量
    if (!var->getName().empty() && var->getName()[0] == '@') {
        std::string name = var->getName().substr(1);
        printf("    -> Direct global name: %s\n", name.c_str());
        return name;
    }

    // 2. 追溯到原始全局变量
    Value * globalBase = findOriginalGlobalVariable(var);
    if (globalBase && !globalBase->getName().empty() && globalBase->getName()[0] == '@') {
        std::string name = globalBase->getName().substr(1);
        printf("    -> Traced global name: %s\n", name.c_str());
        return name;
    }

    printf("    -> No global name found\n");
    return "";
}

/// @brief 查找变量的原始全局变量
Value * InstSelectorArm32::findOriginalGlobalVariable(Value * var)
{
    if (!var) {
        return nullptr;
    }

    printf("      -> Tracing original global for: '%s'\n", var->getName().c_str());

    // 如果本身就是全局变量，直接返回
    if (!var->getName().empty() && var->getName()[0] == '@') {
        printf("      -> Already global: %s\n", var->getName().c_str());
        return var;
    }

    // 查找定义该变量的指令
    for (auto inst: ir) {
        if (inst->hasResultValue() && inst == var) {
            printf("      -> Found defining instruction: op=%d\n", (int) inst->getOp());

            // 赋值指令：继续追溯源变量
            if (inst->getOp() == IRInstOperator::IRINST_OP_ASSIGN) {
                Value * source = inst->getOperand(0);
                if (source) {
                    printf("      -> Tracing ASSIGN source: '%s'\n", source->getName().c_str());
                    return findOriginalGlobalVariable(source);
                }
            }

            // ADD_I 指令：查找全局变量操作数
            if (inst->getOp() == IRInstOperator::IRINST_OP_ADD_I) {
                for (int i = 0; i < inst->getOperandsNum(); i++) {
                    Value * operand = inst->getOperand(i);
                    if (operand && !operand->getName().empty() && operand->getName()[0] == '@') {
                        printf("      -> Found global in ADD_I: %s\n", operand->getName().c_str());
                        return operand;
                    }
                }
            }

            // ADD_PTR 指令：查找全局变量操作数
            if (inst->getOp() == IRInstOperator::IRINST_OP_ADD_PTR) {
                for (int i = 0; i < inst->getOperandsNum(); i++) {
                    Value * operand = inst->getOperand(i);
                    if (operand && !operand->getName().empty() && operand->getName()[0] == '@') {
                        printf("      -> Found global in ADD_PTR: %s\n", operand->getName().c_str());
                        return operand;
                    }
                }
            }

            // ARRAY_ADDR 指令：查找数组基址
            if (inst->getOp() == IRInstOperator::IRINST_OP_ARRAY_ADDR) {
                if (inst->getOperandsNum() > 0) {
                    Value * baseArray = inst->getOperand(0);
                    if (baseArray && !baseArray->getName().empty() && baseArray->getName()[0] == '@') {
                        printf("      -> Found global in ARRAY_ADDR: %s\n", baseArray->getName().c_str());
                        return baseArray;
                    }
                }
            }
        }
    }

    printf("      -> No original global found\n");
    return nullptr;
}

/// @brief 根据IR名称查找函数参数
FormalParam * InstSelectorArm32::getFormalParamByIRName(const std::string & irName)
{
    auto it = functionParamMap.find(irName);
    if (it != functionParamMap.end()) {
        printf("找到函数参数映射: %s -> %s\n", irName.c_str(), it->second->getName().c_str());
        return it->second;
    }
    return nullptr;
}