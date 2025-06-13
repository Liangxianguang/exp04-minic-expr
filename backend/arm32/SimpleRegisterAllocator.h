///
/// @file SimpleRegisterAllocator.h
/// @brief 简单或朴素的寄存器分配器 - 增强版：支持动态分配和生命周期管理
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
#pragma once

#include <vector>
#include <map>
#include <string>
#include <algorithm>

#include "BitMap.h"
#include "Value.h"
#include "PlatformArm32.h"
#include "Instruction.h"

class SimpleRegisterAllocator {

public:
    ///
    /// @brief Construct a new Simple Register Allocator object
    ///
    SimpleRegisterAllocator();

    ///
    /// @brief 尝试按指定的寄存器编号进行分配，若能分配，则直接分配，否则从小达到的次序分配一个寄存器。
    /// 如果没有，则选取寄存器中最晚使用的寄存器，同时溢出寄存器到变量中
    /// @param var 分配寄存器的变量
    /// @param no 指定的寄存器编号
    /// @return int 寄存器编号
    ///
    int Allocate(Value * var = nullptr, int32_t no = -1);

    ///
    /// @brief 强制占用一个指定的寄存器。如果寄存器被占用，则强制寄存器关联的变量溢出
    /// @param no 要分配的寄存器编号
    ///
    void Allocate(int32_t no);

    ///
    /// @brief 将变量对应的load寄存器标记为空闲状态
    /// @param var 变量
    ///
    void free(Value * var);

    ///
    /// @brief 将寄存器no标记为空闲状态
    /// @param no 寄存器编号
    ///
    void free(int32_t);

    /// @brief 检查变量是否为临时变量
    /// @param var 变量
    /// @return true表示是临时变量
    bool isTempVariable(Value * var);

    // 🔧 新增：动态分配相关方法
    /// @brief 动态为临时变量分配寄存器（按需分配）
    /// @param tempVar 临时变量
    /// @param instructionIndex 当前指令索引
    /// @return 分配的寄存器号，-1表示需要溢出到栈
    int dynamicAllocateTemp(Value * tempVar, int instructionIndex = -1);
    
    /// @brief 检查变量是否还会被使用
    /// @param var 变量
    /// @param currentIndex 当前指令索引
    /// @return true表示还会被使用
    bool willBeUsedLater(Value * var, int currentIndex);
    
    /// @brief 分析变量生命周期
    /// @param instructions 指令列表
    void analyzeVariableLifetime(const std::vector<Instruction *> & instructions);
    
    /// @brief 释放不再使用的临时变量寄存器
    /// @param currentIndex 当前指令索引
    /// @return 释放的寄存器数量
    int releaseUnusedTempVars(int currentIndex);
    
    /// @brief 设置当前指令索引
    void setCurrentInstructionIndex(int index) { currentInstructionIndex = index; }

    /// @brief 获取可用寄存器数量
    /// @return 可用寄存器数量
    int getAvailableRegCount();

protected:
    ///
    /// @brief 寄存器被置位，使用过的寄存器被置位
    /// @param no
    ///
    void bitmapSet(int32_t no);

    /// @brief 为临时变量执行智能驱逐
    /// @param newTempVar 新的临时变量
    /// @param priority 新变量的优先级
    /// @return 驱逐后可用的寄存器号
    int evictForTemp(Value * newTempVar, int priority = 3);

    /// @brief 智能释放算法：基于生命周期和优先级
    /// @param urgency 紧急程度
    /// @return 释放的寄存器数量
    int smartFreeByLifetime(int urgency = 1);

protected:
    ///
    /// @brief 寄存器位图：1已被占用，0未被使用
    ///
    BitMap<PlatformArm32::maxUsableRegNum> regBitmap;

    ///
    /// @brief 寄存器被那个Value占用。按照时间次序加入
    ///
    std::vector<Value *> regValues;

    ///
    /// @brief 使用过的所有寄存器编号
    ///
    BitMap<PlatformArm32::maxUsableRegNum> usedBitmap;

    // 🔧 动态分配相关数据结构
    /// @brief 临时变量动态分配映射
    std::map<std::string, int> dynamicTempAllocations;
    
    /// @brief 变量生命周期跟踪 <起始指令索引, 结束指令索引>
    std::map<Value *, std::pair<int, int>> variableLifetime;
    
    /// @brief 当前指令计数器
    int currentInstructionIndex;
    
    /// @brief 临时变量优先级记录（1=最高，5=最低）
    std::map<Value *, int> tempVarPriority;
    
    /// @brief 全局使用计数器
    int globalUsageCounter;
};