///
/// @file InstSelectorArm32.h
/// @brief 指令选择器-ARM32
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
#pragma once

#include <map>
#include <vector>

#include "Function.h"
#include "ILocArm32.h"
#include "Instruction.h"
#include "PlatformArm32.h"
#include "SimpleRegisterAllocator.h"
#include "RegVariable.h"

using namespace std;

/// @brief 指令选择器-ARM32
class InstSelectorArm32 {

    /// @brief 所有的IR指令
    std::vector<Instruction *> & ir;

    /// @brief 指令变换
    ILocArm32 & iloc;

    /// @brief 要处理的函数
    Function * func;

protected:
    /// @brief 指令翻译成ARM32汇编
    /// @param inst IR指令
    void translate(Instruction * inst);

    /// @brief NOP翻译成ARM32汇编
    /// @param inst IR指令
    void translate_nop(Instruction * inst);

    /// @brief 函数入口指令翻译成ARM32汇编
    /// @param inst IR指令
    void translate_entry(Instruction * inst);

    /// @brief 函数出口指令翻译成ARM32汇编
    /// @param inst IR指令
    void translate_exit(Instruction * inst);

    /// @brief 赋值指令翻译成ARM32汇编
    /// @param inst IR指令
    void translate_assign(Instruction * inst);

    /// @brief Label指令指令翻译成ARM32汇编
    /// @param inst IR指令
    void translate_label(Instruction * inst);

    /// @brief goto指令指令翻译成ARM32汇编
    /// @param inst IR指令
    void translate_goto(Instruction * inst);

    /// @brief 整数加法指令翻译成ARM32汇编
    /// @param inst IR指令
    void translate_add_int32(Instruction * inst);

    /// @brief 整数减法指令翻译成ARM32汇编
    /// @param inst IR指令
    void translate_sub_int32(Instruction * inst);

	///添加乘法、除法和求余操作函数的声明-lxg
	/// @brief 整数乘法指令翻译成ARM32汇编
	/// @param inst IR指令
	void translate_mul_int32(Instruction * inst)
	{
		translate_two_operator(inst, "mul");
	}

	/// @brief 整数除法指令翻译成ARM32汇编
	/// @param inst IR指令
	void translate_div_int32(Instruction * inst)
	{
		translate_two_operator(inst, "sdiv");
	}

	/// @brief 整数求余指令翻译成ARM32汇编
	/// @param inst IR指令
	void translate_mod_int32(Instruction * inst)
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
			// arg1 -> r8
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

		// 计算商
		iloc.inst("sdiv", 
				PlatformArm32::regName[load_result_reg_no],
				PlatformArm32::regName[load_arg1_reg_no],
				PlatformArm32::regName[load_arg2_reg_no]);

		// 计算商与除数的乘积
		iloc.inst("mul",
				PlatformArm32::regName[load_result_reg_no],
				PlatformArm32::regName[load_result_reg_no],
				PlatformArm32::regName[load_arg2_reg_no]);

		// 计算余数：被除数 - (商 * 除数)
		iloc.inst("sub",
				PlatformArm32::regName[load_result_reg_no],
				PlatformArm32::regName[load_arg1_reg_no],
				PlatformArm32::regName[load_result_reg_no]);

		// 结果不是寄存器，则需要把结果保存到结果变量中
		if (result_reg_no == -1) {
			// r10 -> result
			iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
		}

		// 释放寄存器
		simpleRegisterAllocator.free(arg1);
		simpleRegisterAllocator.free(arg2);
		simpleRegisterAllocator.free(result);
	}

	/// @brief 整数负号指令翻译成ARM32汇编
	/// @param inst IR指令
	void translate_neg_int32(Instruction * inst)
	{
		Value * result = inst;
		Value * arg1 = inst->getOperand(0);
		
		int32_t arg1_reg_no = arg1->getRegId();
		int32_t result_reg_no = inst->getRegId();
		int32_t load_result_reg_no, load_arg1_reg_no;
		
		// 看arg1是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
		if (arg1_reg_no == -1) {
			// 分配一个寄存器r8
			load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);
			// arg1 -> r8
			iloc.load_var(load_arg1_reg_no, arg1);
		} else {
			load_arg1_reg_no = arg1_reg_no;
		}
		
		// 看结果变量是否是寄存器，若不是则需要分配一个新的寄存器来保存运算的结果
		if (result_reg_no == -1) {
			// 分配一个寄存器，用于暂存结果
			load_result_reg_no = simpleRegisterAllocator.Allocate(result);
		} else {
			load_result_reg_no = result_reg_no;
		}
		
		// 使用rsb指令计算负值 (rsb rd, rn, #0 相当于 rd = 0 - rn)
		iloc.inst("rsb", 
				PlatformArm32::regName[load_result_reg_no],
				PlatformArm32::regName[load_arg1_reg_no],
				"#0");
		
		// 结果不是寄存器，则需要把结果保存到结果变量中
		if (result_reg_no == -1) {
			// 保存结果到内存
			iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
		}
		
		// 释放寄存器
		simpleRegisterAllocator.free(arg1);
		simpleRegisterAllocator.free(result);
	}

	//添加关系运算符的处理函数实现-lxg
	/// @brief 整数关系运算指令翻译成ARM32汇编(统一处理函数)
	/// @param inst IR指令
	/// @param condition ARM的条件码(eq,ne,lt,gt,le,ge)
	void translate_cmp_int32(Instruction * inst, const string& condition)
	{
		Value * result = inst;
		Value * arg1 = inst->getOperand(0);
		Value * arg2 = inst->getOperand(1);

		int32_t arg1_reg_no = arg1->getRegId();
		int32_t arg2_reg_no = arg2->getRegId();
		int32_t result_reg_no = inst->getRegId();
		int32_t load_arg1_reg_no, load_arg2_reg_no, load_result_reg_no;

		// 加载第一个操作数
		if (arg1_reg_no == -1) {
			load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);
			iloc.load_var(load_arg1_reg_no, arg1);
		} else {
			load_arg1_reg_no = arg1_reg_no;
		}

		// 加载第二个操作数
		if (arg2_reg_no == -1) {
			load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);
			iloc.load_var(load_arg2_reg_no, arg2);
		} else {
			load_arg2_reg_no = arg2_reg_no;
		}

		// 为结果分配寄存器
		if (result_reg_no == -1) {
			load_result_reg_no = simpleRegisterAllocator.Allocate(result);
		} else {
			load_result_reg_no = result_reg_no;
		}

		// 比较两个操作数（cmp只接受两个参数）
		iloc.inst("cmp", 
				PlatformArm32::regName[load_arg1_reg_no],
				PlatformArm32::regName[load_arg2_reg_no]);
		
		// 根据条件设置结果为0或1
		// 使用mov{条件}指令，条件满足时设为1，否则设为0
		iloc.inst("mov", PlatformArm32::regName[load_result_reg_no], "#0");  // 默认为0
		iloc.inst("mov" + condition, PlatformArm32::regName[load_result_reg_no], "#1");  // 条件满足时为1

		// 保存结果
		if (result_reg_no == -1) {
			iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
		}

		// 释放寄存器
		simpleRegisterAllocator.free(arg1);
		simpleRegisterAllocator.free(arg2);
		simpleRegisterAllocator.free(result);
	}

	/// @brief 整数小于指令翻译成ARM32汇编
	/// @param inst IR指令
	void translate_lt_int32(Instruction * inst)
	{
		translate_cmp_int32(inst, "lt");
	}

	/// @brief 整数大于指令翻译成ARM32汇编
	/// @param inst IR指令
	void translate_gt_int32(Instruction * inst)
	{
		translate_cmp_int32(inst, "gt");
	}

	/// @brief 整数小于等于指令翻译成ARM32汇编
	/// @param inst IR指令
	void translate_le_int32(Instruction * inst)
	{
		translate_cmp_int32(inst, "le");
	}

	/// @brief 整数大于等于指令翻译成ARM32汇编
	/// @param inst IR指令
	void translate_ge_int32(Instruction * inst)
	{
		translate_cmp_int32(inst, "ge");
	}

	/// @brief 整数等于指令翻译成ARM32汇编
	/// @param inst IR指令
	void translate_eq_int32(Instruction * inst)
	{
		translate_cmp_int32(inst, "eq");
	}

	/// @brief 整数不等于指令翻译成ARM32汇编
	/// @param inst IR指令
	void translate_ne_int32(Instruction * inst)
	{
		translate_cmp_int32(inst, "ne");
	}

	
    /// @brief 二元操作指令翻译成ARM32汇编
    /// @param inst IR指令
    /// @param operator_name 操作码
    void translate_two_operator(Instruction * inst, string operator_name);

    /// @brief 函数调用指令翻译成ARM32汇编
    /// @param inst IR指令
    void translate_call(Instruction * inst);

    ///
    /// @brief 实参指令翻译成ARM32汇编
    /// @param inst
    ///
    void translate_arg(Instruction * inst);

    ///
    /// @brief 输出IR指令
    ///
    void outputIRInstruction(Instruction * inst);

    /// @brief IR翻译动作函数原型
    typedef void (InstSelectorArm32::*translate_handler)(Instruction *);

    /// @brief IR动作处理函数清单
    map<IRInstOperator, translate_handler> translator_handlers;

    ///
    /// @brief 简单的朴素寄存器分配方法
    ///
    SimpleRegisterAllocator & simpleRegisterAllocator;

    ///
    /// @brief 函数实参累计
    ///
    int32_t argCount = 0;

    /// @brief 累计的实参个数
    int32_t realArgCount = 0;

    ///
    /// @brief 显示IR指令内容
    ///
    bool showLinearIR = false;

public:
    /// @brief 构造函数
    /// @param _irCode IR指令
    /// @param _func 函数
    /// @param _iloc 后端指令
    InstSelectorArm32(std::vector<Instruction *> & _irCode,
                      ILocArm32 & _iloc,
                      Function * _func,
                      SimpleRegisterAllocator & allocator);

    ///
    /// @brief 析构函数
    ///
    ~InstSelectorArm32();

    ///
    /// @brief 设置是否输出线性IR的内容
    /// @param show true显示，false显示
    ///
    void setShowLinearIR(bool show)
    {
        showLinearIR = show;
    }

    /// @brief 指令选择
    void run();
};
