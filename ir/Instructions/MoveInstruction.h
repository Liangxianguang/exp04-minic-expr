///
/// @file MoveInstruction.h
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
#pragma once

#include <string>

#include "Value.h"
#include "Instruction.h"

class Function;

///
/// @brief 复制指令
///
class MoveInstruction : public Instruction {


// 在 MoveInstruction.h 中添加
private:
    bool isPointerStore = false;  // 是否是通过指针存储值
    bool isPointerLoad = false;   // 是否是通过指针加载值
    bool isArrayToPointer = false; // 是否是数组到指针的转换

public:
    ///
    /// @brief 构造函数
    /// @param _func 所属的函数
    /// @param result 结构操作数
    /// @param srcVal1 源操作数
    ///
    MoveInstruction(Function * _func, Value * result, Value * srcVal1);

    /// @brief 转换成字符串-lxg
    void toString(std::string & str) override;

    // 设置为指针存储操作
    void setIsPointerStore(bool isStore) {
        isPointerStore = isStore;
    }
    
    // 获取是否为指针存储操作
    bool getIsPointerStore() const {
        return isPointerStore;
    }
    
    // 设置为指针加载操作
    void setIsPointerLoad(bool isLoad) {
        isPointerLoad = isLoad;
    }
    
    // 获取是否为指针加载操作
    bool getIsPointerLoad() const {
        return isPointerLoad;
    }

    // 设置为数组到指针转换操作
    void setIsArrayToPointer(bool isConvert)
    {
        isArrayToPointer = isConvert;
    }

    // 获取是否为数组到指针转换操作
    bool getIsArrayToPointer() const
    {
        return isArrayToPointer;
    }
};
