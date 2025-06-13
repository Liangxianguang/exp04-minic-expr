///
/// @file SimpleRegisterAllocator.cpp
/// @brief 简单或朴素的寄存器分配器 - 增强版实现
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
#include <algorithm>
#include <string>
#include <cctype>
#include "SimpleRegisterAllocator.h"

///
/// @brief Construct a new Simple Register Allocator object
///
SimpleRegisterAllocator::SimpleRegisterAllocator() : currentInstructionIndex(0), globalUsageCounter(0)
{
    // 初始化动态分配相关的数据结构
}

///
/// @brief 分配一个寄存器。如果没有，则选取寄存器中最晚使用的寄存器，同时溢出寄存器到变量中
/// @return int 寄存器编号
/// @param no 指定的寄存器编号
///
int SimpleRegisterAllocator::Allocate(Value * var, int32_t no)
{
    if (var && (var->getLoadRegId() != -1)) {
        // 该变量已经分配了Load寄存器了，不需要再次分配
        return var->getLoadRegId();
    }

    int32_t regno = -1;

    // 尝试指定的寄存器是否可用
    if ((no != -1) && !regBitmap.test(no)) {
        // 可用
        regno = no;
    } else {
        // 查询空闲的寄存器
        for (int k = 0; k < PlatformArm32::maxUsableRegNum; ++k) {
            if (!regBitmap.test(k)) {
                // 找到空闲寄存器
                regno = k;
                break;
            }
        }
    }

    if (regno != -1) {
        // 找到空闲的寄存器
        // 占用
        bitmapSet(regno);
    } else {
        // 没有可用的寄存器分配，需要溢出一个变量的寄存器
        // 溢出的策略：选择最迟加入队列的变量
        Value * oldestVar = regValues.front();

        // 获取Load寄存器编号，设置该变量不再占用Load寄存器
        regno = oldestVar->getLoadRegId();

        // 设置该变量不再占用寄存器
        oldestVar->setLoadRegId(-1);

        // 从队列中删除
        regValues.erase(regValues.begin());
    }

    if (var) {
        // 加入新的变量
        var->setLoadRegId(regno);
        regValues.push_back(var);
    }

    return regno;
}

///
/// @brief 强制占用一个指定的寄存器。如果寄存器被占用，则强制寄存器关联的变量溢出
/// @param no 要分配的寄存器编号
///
void SimpleRegisterAllocator::Allocate(int32_t no)
{
    if (regBitmap.test(no)) {
        // 指定的寄存器已经被占用
        // 释放该寄存器
        free(no);
    }

    // 占用该寄存器
    bitmapSet(no);
}

///
/// @brief 将变量对应的load寄存器标记为空闲状态
/// @param var 变量
///
void SimpleRegisterAllocator::free(Value * var)
{
    if (var && (var->getLoadRegId() != -1)) {
        // 清除该索引的寄存器，变得可使用
        regBitmap.reset(var->getLoadRegId());
        regValues.erase(std::find(regValues.begin(), regValues.end(), var));

        // 从动态分配记录中移除
        std::string varName = var->getName();
        dynamicTempAllocations.erase(varName);
        tempVarPriority.erase(var);

        var->setLoadRegId(-1);
    }
}

///
/// @brief 将寄存器no标记为空闲状态
/// @param no 寄存器编号
///
void SimpleRegisterAllocator::free(int32_t no)
{
    // 无效寄存器，什么都不做，直接返回
    if (no == -1) {
        return;
    }

    // 清除该索引的寄存器，变得可使用
    regBitmap.reset(no);

    // 查找寄存器编号
    auto pIter = std::find_if(regValues.begin(), regValues.end(), [=](auto val) {
        return val->getLoadRegId() == no; // 寄存器编号与 no 匹配
    });

    if (pIter != regValues.end()) {
        // 查找到，则清除，并设置为-1
        Value * var = *pIter;

        // 从动态分配记录中移除
        std::string varName = var->getName();
        dynamicTempAllocations.erase(varName);
        tempVarPriority.erase(var);

        var->setLoadRegId(-1);
        regValues.erase(pIter);
    }
}

///
/// @brief 寄存器被置位，使用过的寄存器被置位
/// @param no
///
void SimpleRegisterAllocator::bitmapSet(int32_t no)
{
    regBitmap.set(no);
    usedBitmap.set(no);
}

///
/// @brief 动态为临时变量分配寄存器（按需分配）
/// @param tempVar 临时变量
/// @param instructionIndex 当前指令索引
/// @return 分配的寄存器号，-1表示需要溢出到栈
///
int SimpleRegisterAllocator::dynamicAllocateTemp(Value * tempVar, int instructionIndex)
{
    if (!tempVar)
        return -1;

    // 检查是否已经分配了寄存器
    if (tempVar->getLoadRegId() != -1) {
        return tempVar->getLoadRegId();
    }

    std::string varName = tempVar->getName();

    // 🔧 策略1：优先分配caller-saved寄存器(r0-r3)给短生命周期临时变量
    if (isTempVariable(tempVar) && (varName.empty() || varName[0] == 't')) {
        // 先尝试r0-r3（caller-saved，不需要保护）
        for (int k = 0; k <= 3; ++k) {
            if (!regBitmap.test(k)) {
                bitmapSet(k);
                tempVar->setLoadRegId(k);
                regValues.push_back(tempVar);

                // 记录动态分配
                if (!varName.empty()) {
                    dynamicTempAllocations[varName] = k;
                }
                tempVarPriority[tempVar] = 1; // 最高优先级

                return k;
            }
        }

        // 如果r0-r3都被占用，尝试r4-r7
        for (int k = 4; k <= 7; ++k) {
            if (!regBitmap.test(k)) {
                bitmapSet(k);
                tempVar->setLoadRegId(k);
                regValues.push_back(tempVar);

                if (!varName.empty()) {
                    dynamicTempAllocations[varName] = k;
                }
                tempVarPriority[tempVar] = 2;

                return k;
            }
        }
    }

    // 🔧 策略2：智能驱逐 - 优先驱逐生命周期已结束的变量
    if (instructionIndex != -1) {
        releaseUnusedTempVars(instructionIndex);

        // 重新尝试分配
        for (int k = 0; k <= 7; ++k) {
            if (!regBitmap.test(k)) {
                bitmapSet(k);
                tempVar->setLoadRegId(k);
                regValues.push_back(tempVar);

                if (isTempVariable(tempVar) && !varName.empty()) {
                    dynamicTempAllocations[varName] = k;
                    tempVarPriority[tempVar] = 3;
                }

                return k;
            }
        }
    }

    // 🔧 策略3：传统的溢出策略
    return evictForTemp(tempVar, 3);
}

///
/// @brief 分析变量生命周期
/// @param instructions 指令列表
///
void SimpleRegisterAllocator::analyzeVariableLifetime(const std::vector<Instruction *> & instructions)
{
    variableLifetime.clear();

    // 遍历所有指令，记录变量的首次定义和最后使用
    for (size_t i = 0; i < instructions.size(); ++i) {
        Instruction * inst = instructions[i];

        // 记录结果变量的定义
        if (inst->hasResultValue()) {
            Value * result = inst;
            if (variableLifetime.find(result) == variableLifetime.end()) {
                variableLifetime[result] = std::make_pair(i, i);
            }
        }

        // 记录操作数的使用
        for (int j = 0; j < inst->getOperandsNum(); ++j) { // 🔧 修复：getOperandNum -> getOperandsNum
            Value * operand = inst->getOperand(j);
            if (operand) {
                auto it = variableLifetime.find(operand);
                if (it != variableLifetime.end()) {
                    it->second.second = i; // 更新最后使用位置
                } else {
                    // 可能是函数参数或全局变量，从头开始
                    variableLifetime[operand] = std::make_pair(0, i);
                }
            }
        }
    }
}

///
/// @brief 检查变量是否还会被使用
/// @param var 变量
/// @param currentIndex 当前指令索引
/// @return true表示还会被使用
///
bool SimpleRegisterAllocator::willBeUsedLater(Value * var, int currentIndex)
{
    auto it = variableLifetime.find(var);
    if (it != variableLifetime.end()) {
        return currentIndex < it->second.second;
    }
    return false; // 未知变量，保守处理
}

///
/// @brief 释放不再使用的临时变量寄存器
/// @param currentIndex 当前指令索引
/// @return 释放的寄存器数量
///
int SimpleRegisterAllocator::releaseUnusedTempVars(int currentIndex)
{
    int freedCount = 0;
    std::vector<Value *> toRelease;

    // 查找可以释放的变量
    for (auto var: regValues) {
        if (isTempVariable(var) && !willBeUsedLater(var, currentIndex)) {
            toRelease.push_back(var);
        }
    }

    // 释放这些变量的寄存器
    for (auto var: toRelease) {
        free(var);
        freedCount++;
    }

    return freedCount;
}

///
/// @brief 智能驱逐策略：为临时变量腾出寄存器
/// @param newTempVar 新的临时变量
/// @param priority 新变量的优先级
/// @return 驱逐后可用的寄存器号
///
int SimpleRegisterAllocator::evictForTemp(Value * newTempVar, int priority)
{
    Value * victimVar = nullptr;
    int victimReg = -1;
    int maxPriority = 0; // 数字越大优先级越低

    // 🔧 驱逐策略：优先驱逐非临时变量，其次驱逐低优先级临时变量
    for (auto var: regValues) {
        int varPriority = 10; // 非临时变量默认最低优先级

        if (isTempVariable(var)) {
            auto it = tempVarPriority.find(var);
            if (it != tempVarPriority.end()) {
                varPriority = it->second;
            } else {
                varPriority = 5; // 默认中等优先级
            }
        }

        // 找优先级更低的变量进行驱逐
        if (varPriority > priority && varPriority > maxPriority) {
            victimVar = var;
            victimReg = var->getLoadRegId();
            maxPriority = varPriority;
        }
    }

    if (victimVar && victimReg != -1) {
        // 执行驱逐
        free(victimVar);
        return victimReg;
    }

    return -1; // 驱逐失败
}

///
/// @brief 检查是否有足够的寄存器用于临时变量
/// @return 可用寄存器数量
///
int SimpleRegisterAllocator::getAvailableRegCount()
{
    int count = 0;

    // 统计所有可用寄存器数量
    for (int k = 0; k < PlatformArm32::maxUsableRegNum; ++k) {
        if (!regBitmap.test(k)) {
            count++;
        }
    }

    return count;
}

///
/// @brief 智能释放算法：基于生命周期和优先级
/// @param urgency 紧急程度
/// @return 释放的寄存器数量
///
int SimpleRegisterAllocator::smartFreeByLifetime(int urgency)
{
    int freedCount = 0;
    std::vector<Value *> candidatesForFree;

    // 🔧 收集可以释放的临时变量
    for (auto var: regValues) {
        if (isTempVariable(var)) {
            auto it = tempVarPriority.find(var);
            int varPriority = (it != tempVarPriority.end()) ? it->second : 5;

            // 根据紧急程度决定释放条件
            if (urgency >= varPriority) {
                candidatesForFree.push_back(var);
            }
        }
    }

    // 按优先级排序，优先释放低优先级的
    std::sort(candidatesForFree.begin(), candidatesForFree.end(), [this](Value * a, Value * b) {
        auto it_a = tempVarPriority.find(a);
        auto it_b = tempVarPriority.find(b);
        int prio_a = (it_a != tempVarPriority.end()) ? it_a->second : 5;
        int prio_b = (it_b != tempVarPriority.end()) ? it_b->second : 5;
        return prio_a > prio_b;
    });

    // 释放寄存器
    for (auto var: candidatesForFree) {
        free(var);
        freedCount++;

        if (freedCount >= urgency)
            break; // 限制释放数量
    }

    return freedCount;
}

///
/// @brief 检查变量是否为临时变量
/// @param var 变量
/// @return true表示是临时变量
///
bool SimpleRegisterAllocator::isTempVariable(Value * var)
{
    if (!var)
        return false;

    std::string name = var->getName();

    // 🔧 临时变量判断条件：
    // 1. 名称为空
    // 2. 以 't' 开头（如 t1, t2, t103）
    // 3. 以 'l' 开头且数字较大（如 l6, l7, l56）
    // 4. 包含 "tmp" 字符串

    if (name.empty()) {
        return true;
    }

    if (name.length() > 0 && name[0] == 't') {
        return true;
    }

    if (name.length() > 1 && name[0] == 'l') {
        // 提取数字部分
        std::string numPart = name.substr(1);
        if (!numPart.empty() && std::all_of(numPart.begin(), numPart.end(), ::isdigit)) {
            int num = std::stoi(numPart);
            return num > 5; // l6 及以上认为是临时变量
        }
    }

    if (name.find("tmp") != std::string::npos) {
        return true;
    }

    return false;
}