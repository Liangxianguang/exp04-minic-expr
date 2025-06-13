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

#include "ILocArm32.h"
#include "Common.h"
#include "Function.h"
#include "PlatformArm32.h"
#include "Module.h"

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
    // movw:把 16 位立即数放到寄存器的低16位，高16位清0
    // movt:把 16 位立即数放到寄存器的高16位，低 16位不影响
    if (0 == ((constant >> 16) & 0xFFFF)) {
        // 如果高16位本来就为0，直接movw
        emit("movw", PlatformArm32::regName[rs_reg_no], "#:lower16:" + std::to_string(constant));
    } else {
        // 如果高16位不为0，先movw，然后movt
        emit("movw", PlatformArm32::regName[rs_reg_no], "#:lower16:" + std::to_string(constant));
        emit("movt", PlatformArm32::regName[rs_reg_no], "#:upper16:" + std::to_string(constant));
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
    if (Instanceof(constVal, ConstInt *, src_var)) {
        // 整型常量
        load_imm(rs_reg_no, constVal->getVal());
    } else if (src_var->getLoadRegId() != -1) {
        // 🔧 修复：优先检查 LoadRegId（用于临时变量）
        int32_t src_regId = src_var->getLoadRegId();
        if (src_regId != rs_reg_no) {
            emit("mov", PlatformArm32::regName[rs_reg_no], PlatformArm32::regName[src_regId]);
        }
    } else if (src_var->getRegId() != -1) {
        // 源操作数为寄存器变量
        int32_t src_regId = src_var->getRegId();
        if (src_regId != rs_reg_no) {
            emit("mov", PlatformArm32::regName[rs_reg_no], PlatformArm32::regName[src_regId]);
        }
    } else if (Instanceof(globalVar, GlobalVariable *, src_var)) {
        // 全局变量
        load_symbol(rs_reg_no, globalVar->getName());

        if (globalVar->getType()->isArrayType()) {
            comment("全局数组：使用地址");
        } else {
            comment("全局变量：加载值");
            emit("ldr", PlatformArm32::regName[rs_reg_no], "[" + PlatformArm32::regName[rs_reg_no] + "]");
        }
    } else {
        // 🔧 修复：检查是否有有效的内存地址
        int32_t var_baseRegId = -1;
        int64_t var_offset = -1;

        bool result = src_var->getMemoryAddr(&var_baseRegId, &var_offset);
        if (result && var_baseRegId != -1 && var_offset != -1) {
            // 有效的栈+偏移寻址
            load_base(rs_reg_no, var_baseRegId, var_offset);
        } else {
            // 🔧 临时变量处理：没有有效地址
            comment("警告: 变量 " + src_var->getName() + " 无有效地址，设为0");
            load_imm(rs_reg_no, 0);
        }
    }
}

/// @brief 加载变量地址到寄存器
/// @param rs_reg_no
/// @param var
void ILocArm32::lea_var(int rs_reg_no, Value * var)
{
    // 被加载的变量肯定不是常量！
    // 被加载的变量肯定不是寄存器变量！

    // 检查是否为全局变量-lxg
    if (Instanceof(globalVar, GlobalVariable *, var)) {
        // 全局变量：直接加载符号地址，不要加载值！
        comment("加载全局变量地址: " + globalVar->getName());
        load_symbol(rs_reg_no, globalVar->getName());
        // 关键：不要添加ldr指令！我们要的是地址，不是值
        return;
    }

    // 局部变量处理
    // 栈帧偏移
    int32_t var_baseRegId = -1;
    int64_t var_offset = -1;

    bool result = var->getMemoryAddr(&var_baseRegId, &var_offset);
    if (!result) {
        minic_log(LOG_ERROR, "BUG");
    }

    comment("加载局部变量地址");

    // lea r8, [fp,#-16]
    leaStack(rs_reg_no, var_baseRegId, var_offset);
}

/// @brief 保存寄存器到变量，保证将计算结果保存到变量
/// @param src_reg_no 源寄存器
/// @param dest_var  变量
/// @param tmp_reg_no 第三方寄存器
void ILocArm32::store_var(int src_reg_no, Value * dest_var, int tmp_reg_no)
{
    // 🔧 修复：优先检查 LoadRegId（用于临时变量）
    if (dest_var->getLoadRegId() != -1) {
        int dest_reg_id = dest_var->getLoadRegId();
        if (src_reg_no != dest_reg_id) {
            emit("mov", PlatformArm32::regName[dest_reg_id], PlatformArm32::regName[src_reg_no]);
        }
    } else if (dest_var->getRegId() != -1) {
        // 寄存器变量
        int dest_reg_id = dest_var->getRegId();
        if (src_reg_no != dest_reg_id) {
            emit("mov", PlatformArm32::regName[dest_reg_id], PlatformArm32::regName[src_reg_no]);
        }
    } else if (Instanceof(globalVar, GlobalVariable *, dest_var)) {
        // 全局变量
        load_symbol(tmp_reg_no, globalVar->getName());
        emit("str", PlatformArm32::regName[src_reg_no], "[" + PlatformArm32::regName[tmp_reg_no] + "]");
    } else {
        // 🔧 修复：检查是否有有效的内存地址
        int32_t dest_baseRegId = -1;
        int64_t dest_offset = -1;

        bool result = dest_var->getMemoryAddr(&dest_baseRegId, &dest_offset);
        if (result && dest_baseRegId != -1 && dest_offset != -1) {
            // 有效的栈+偏移寻址
            store_base(src_reg_no, dest_baseRegId, dest_offset, tmp_reg_no);
        } else {
            // 🔧 临时变量处理：没有有效地址，尝试分配寄存器
            comment("临时变量 " + dest_var->getName() + " 无内存地址，尝试寄存器存储");

            // 这里需要访问寄存器分配器，暂时跳过存储
            comment("跳过临时变量 " + dest_var->getName() + " 的存储");
        }
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

    if (PlatformArm32::constExpr(off))
        // add r8,fp,#-16
        emit("add", rs_reg_name, base_reg_name, toStr(off));
    else {
        // ldr r8,=-257
        load_imm(rs_reg_no, off);

        // add r8,fp,r8
        emit("add", rs_reg_name, base_reg_name, rs_reg_name);
    }
}

/// @brief 函数内栈内空间分配（局部变量、形参变量、函数参数传值，或不能寄存器分配的临时变量等）
/// @param func 函数
/// @param tmp_reg_no 临时寄存器号
void ILocArm32::allocStack(Function * func, int tmp_reg_no)
{
    // 🔧 简化栈分配策略：只计算实际变量需要的空间
    int actual_stack_size = 0;

    auto & localVars = func->getVarValues();
    comment("=== 栈空间分配开始 ===");
    comment("函数: " + func->getName() + ", 变量总数: " + std::to_string(localVars.size()));

    // 🔧 关键修改：只为持久变量分配栈空间
    for (auto var: localVars) {
        // 🔧 跳过临时变量 - 它们应该尽量保持在寄存器中
        if (var->getName().empty() || var->getName().find("tmp") != std::string::npos ||
            var->getName().find("t") == 0) {
            comment("跳过临时变量: " + var->getName());
            continue;
        }

        if (var->getType()->isArrayType()) {
            int array_size = var->getType()->getSize();
            actual_stack_size += array_size;
            comment("数组 " + var->getName() + ": " + std::to_string(array_size) + " 字节");
        } else {
            actual_stack_size += 4;
            comment("局部变量 " + var->getName() + ": 4 字节");
        }
    }

    // 🔧 为少量必要的临时变量预留空间（如寄存器不够用时的溢出）
    int temp_spill_space = 32; // 预留8个临时变量的空间
    actual_stack_size += temp_spill_space;
    comment("临时变量溢出空间: " + std::to_string(temp_spill_space) + " 字节");

    // 16字节对齐
    actual_stack_size = (actual_stack_size + 15) & ~15;
    comment("栈帧大小: " + std::to_string(actual_stack_size) + " 字节");

    if (actual_stack_size == 0) {
        comment("无需分配栈空间");
        return;
    }

    // 保存FP并分配栈空间
    mov_reg(ARM32_FP_REG_NO, ARM32_SP_REG_NO);

    if (PlatformArm32::constExpr(actual_stack_size)) {
        inst("sub", "sp", "sp", toStr(actual_stack_size));
    } else {
        load_imm(tmp_reg_no, actual_stack_size);
        inst("sub", "sp", "sp", PlatformArm32::regName[tmp_reg_no]);
    }

    comment("栈帧分配完成 - 静态偏移访问策略");
    comment("=== 栈空间分配结束 ===");
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

/// @brief 数组元素地址计算 - 一维数组-lxg
/// @param rs_reg_no 结果寄存器号
/// @param base_reg_no 基址寄存器号
/// @param index_reg_no 索引寄存器号
/// @param element_size 元素大小(字节)
/// @param tmp_reg_no 临时寄存器号
void ILocArm32::calc_array_addr(int rs_reg_no, int base_reg_no, int index_reg_no, int element_size, int tmp_reg_no)
{
    std::string rs_reg = PlatformArm32::regName[rs_reg_no];
    std::string base_reg = PlatformArm32::regName[base_reg_no];
    std::string index_reg = PlatformArm32::regName[index_reg_no];
    std::string tmp_reg = PlatformArm32::regName[tmp_reg_no];

    comment("计算数组元素地址: base + index * " + std::to_string(element_size));

    // 根据元素大小选择不同的地址计算方式
    if (element_size == 1) {
        // 字节数组：addr = base + index
        emit("add", rs_reg, base_reg, index_reg);
    } else if (element_size == 4) {
        // 4字节数组(int)：addr = base + index * 4
        // 使用左移2位来实现乘以4的优化: index << 2
        emit("add", rs_reg, base_reg, index_reg + ", lsl #2");
    } else if (element_size == 8) {
        // 8字节数组：addr = base + index * 8
        // 使用左移3位来实现乘以8的优化
        emit("add", rs_reg, base_reg, index_reg + ", lsl #3");
    } else {
        // 其他大小：先计算index * element_size，再加上base
        load_imm(tmp_reg_no, element_size);
        emit("mul", tmp_reg, index_reg, tmp_reg);
        emit("add", rs_reg, base_reg, tmp_reg);
    }
}

/// @brief 多维数组元素地址计算
/// @param rs_reg_no 结果寄存器号
/// @param base_reg_no 基址寄存器号
/// @param indices_regs 各维度索引寄存器号向量
/// @param dim_sizes 各维度大小向量
/// @param element_size 元素大小(字节)
/// @param tmp_reg_no1 临时寄存器1号
/// @param tmp_reg_no2 临时寄存器2号
void ILocArm32::calc_multi_array_addr(int rs_reg_no,
                                      int base_reg_no,
                                      const std::vector<int> & indices_regs,
                                      const std::vector<int> & dim_sizes,
                                      int element_size,
                                      int tmp_reg_no1,
                                      int tmp_reg_no2)
{
    std::string rs_reg = PlatformArm32::regName[rs_reg_no];
    std::string base_reg = PlatformArm32::regName[base_reg_no];
    std::string tmp_reg1 = PlatformArm32::regName[tmp_reg_no1];
    std::string tmp_reg2 = PlatformArm32::regName[tmp_reg_no2];

    comment("计算多维数组元素地址");

    // 计算多维数组的线性偏移
    // offset = index[0] * dim[1] * dim[2] * ... + index[1] * dim[2] * ... + ...

    // 初始化结果为第一个索引
    mov_reg(rs_reg_no, indices_regs[0]);

    // 计算每一维的贡献
    for (size_t i = 0; i < indices_regs.size() - 1; i++) {
        // 计算当前维的乘数：dim[i+1] * dim[i+2] * ...
        int multiplier = 1;
        for (size_t j = i + 1; j < dim_sizes.size(); j++) {
            multiplier *= dim_sizes[j];
        }

        // rs_reg = rs_reg * multiplier
        load_imm(tmp_reg_no1, multiplier);
        emit("mul", rs_reg, rs_reg, tmp_reg1);

        // 如果不是最后一维，加上下一维的索引
        if (i + 1 < indices_regs.size()) {
            std::string next_index_reg = PlatformArm32::regName[indices_regs[i + 1]];
            emit("add", rs_reg, rs_reg, next_index_reg);
        }
    }

    // 乘以元素大小
    if (element_size == 4) {
        // 优化：左移2位
        emit("lsl", rs_reg, rs_reg, "#2");
    } else if (element_size == 8) {
        // 优化：左移3位
        emit("lsl", rs_reg, rs_reg, "#3");
    } else if (element_size != 1) {
        load_imm(tmp_reg_no1, element_size);
        emit("mul", rs_reg, rs_reg, tmp_reg1);
    }

    // 加上基址
    emit("add", rs_reg, base_reg, rs_reg);
}

/// @brief 加载数组元素到寄存器 - 通过地址寄存器
/// @param rs_reg_no 结果寄存器号
/// @param addr_reg_no 地址寄存器号
void ILocArm32::load_array_element(int rs_reg_no, int addr_reg_no)
{
    std::string rs_reg = PlatformArm32::regName[rs_reg_no];
    std::string addr_reg = PlatformArm32::regName[addr_reg_no];

    comment("加载数组元素到寄存器");
    // ldr rs_reg, [addr_reg]
    emit("ldr", rs_reg, "[" + addr_reg + "]");
}

/// @brief 存储寄存器到数组元素 - 通过地址寄存器
/// @param src_reg_no 源寄存器号
/// @param addr_reg_no 地址寄存器号
void ILocArm32::store_array_element(int src_reg_no, int addr_reg_no)
{
    std::string src_reg = PlatformArm32::regName[src_reg_no];
    std::string addr_reg = PlatformArm32::regName[addr_reg_no];

    comment("存储寄存器值到数组元素");
    // str src_reg, [addr_reg]
    emit("str", src_reg, "[" + addr_reg + "]");
}

/// @brief 立即数左移操作 - 用于地址计算优化
/// @param rs_reg_no 结果寄存器号
/// @param src_reg_no 源寄存器号
/// @param shift_bits 左移位数
void ILocArm32::lsl_imm(int rs_reg_no, int src_reg_no, int shift_bits)
{
    std::string rs_reg = PlatformArm32::regName[rs_reg_no];
    std::string src_reg = PlatformArm32::regName[src_reg_no];

    // lsl rs_reg, src_reg, #shift_bits
    emit("lsl", rs_reg, src_reg, "#" + std::to_string(shift_bits));
}

/// @brief 静态数组访问 - 编译期已知偏移
void ILocArm32::load_array_static(int rs_reg_no, int base_reg_no, int static_offset)
{
    comment("静态数组访问: [" + PlatformArm32::regName[base_reg_no] + ",#" + std::to_string(static_offset) + "]");

    if (static_offset >= -4095 && static_offset <= 4095) {
        // ARM直接支持的偏移范围
        inst("ldr",
             PlatformArm32::regName[rs_reg_no],
             "[" + PlatformArm32::regName[base_reg_no] + ",#" + std::to_string(static_offset) + "]");
    } else {
        // 偏移超出范围，分步计算
        load_imm(rs_reg_no, static_offset);
        inst("add",
             PlatformArm32::regName[rs_reg_no],
             PlatformArm32::regName[base_reg_no],
             PlatformArm32::regName[rs_reg_no]);
        inst("ldr", PlatformArm32::regName[rs_reg_no], "[" + PlatformArm32::regName[rs_reg_no] + "]");
    }
}

/// @brief 静态数组存储 - 编译期已知偏移
void ILocArm32::store_array_static(int src_reg_no, int base_reg_no, int static_offset, int tmp_reg_no)
{
    comment("静态数组存储: [" + PlatformArm32::regName[base_reg_no] + ",#" + std::to_string(static_offset) + "]");

    if (static_offset >= -4095 && static_offset <= 4095) {
        // ARM直接支持的偏移范围
        inst("str",
             PlatformArm32::regName[src_reg_no],
             "[" + PlatformArm32::regName[base_reg_no] + ",#" + std::to_string(static_offset) + "]");
    } else {
        // 偏移超出范围，使用临时寄存器
        load_imm(tmp_reg_no, static_offset);
        inst("add",
             PlatformArm32::regName[tmp_reg_no],
             PlatformArm32::regName[base_reg_no],
             PlatformArm32::regName[tmp_reg_no]);
        inst("str", PlatformArm32::regName[src_reg_no], "[" + PlatformArm32::regName[tmp_reg_no] + "]");
    }
}

/// @brief 动态数组访问 - 运行时计算偏移
void ILocArm32::load_array_dynamic(int rs_reg_no, int base_reg_no, int offset_reg_no)
{
    comment("动态数组访问: [" + PlatformArm32::regName[base_reg_no] + "+" + PlatformArm32::regName[offset_reg_no] +
            "]");

    inst("add",
         PlatformArm32::regName[rs_reg_no],
         PlatformArm32::regName[base_reg_no],
         PlatformArm32::regName[offset_reg_no]);
    inst("ldr", PlatformArm32::regName[rs_reg_no], "[" + PlatformArm32::regName[rs_reg_no] + "]");
}

/// @brief 动态数组存储 - 运行时计算偏移
void ILocArm32::store_array_dynamic(int src_reg_no, int base_reg_no, int offset_reg_no)
{
    comment("动态数组存储: [" + PlatformArm32::regName[base_reg_no] + "+" + PlatformArm32::regName[offset_reg_no] +
            "]");

    inst("add",
         PlatformArm32::regName[offset_reg_no],
         PlatformArm32::regName[base_reg_no],
         PlatformArm32::regName[offset_reg_no]);
    inst("str", PlatformArm32::regName[src_reg_no], "[" + PlatformArm32::regName[offset_reg_no] + "]");
}