///
/// @file Value.cpp
/// @brief 值操作类型，所有的变量、函数、常量都是Value
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

#include <algorithm>

#include "Value.h"
#include "Use.h"

/// @brief 构造函数
/// @param _type
Value::Value(Type * _type) : type(_type), globalOffset(0), hasGlobalSource(false)
{
    // 不需要增加代码
}

/// @brief 析构函数
Value::~Value()
{
    // 如有资源清理，请这里追加代码
}

/// @brief 获取名字
/// @return 变量名
std::string Value::getName() const
{
    return name;
}

///
/// @brief 设置名字
/// @param _name 名字
///
void Value::setName(std::string _name)
{
    this->name = _name;
}

/// @brief 获取名字
/// @return 变量名
std::string Value::getIRName() const
{
    return IRName;
}

///
/// @brief 设置名字
/// @param _name 名字
///
void Value::setIRName(std::string _name)
{
    this->IRName = _name;
}

/// @brief 获取类型
/// @return 变量名
Type * Value::getType()
{
    return type;
}

///
/// @brief 增加一条边，增加Value被使用次数
/// @param use
///
void Value::addUse(Use * use)
{
    uses.push_back(use);
}

///
/// @brief 消除一条边，减少Value被使用次数
/// @param use
///
void Value::removeUse(Use * use)
{
    auto pIter = std::find(uses.begin(), uses.end(), use);
    if (pIter != uses.end()) {
        uses.erase(pIter);
    }
}

///
/// @brief 取得变量所在的作用域层级
/// @return int32_t 层级
///
int32_t Value::getScopeLevel()
{
    return -1;
}

///
/// @brief 获得分配的寄存器编号或ID
/// @return int32_t 寄存器编号 -1代表无效的寄存器编号
///
int32_t Value::getRegId()
{
    return -1;
}

///
/// @brief @brief 如是内存变量型Value，则获取基址寄存器和偏移
/// @param regId 寄存器编号
/// @param offset 相对偏移
/// @return true 是内存型变量
/// @return false 不是内存型变量
///
bool Value::getMemoryAddr(int32_t * regId, int64_t * offset)
{
    (void) regId;
    (void) offset;
    return false;
}

///
/// @brief 对该Value进行Load用的寄存器编号
/// @return int32_t 寄存器编号
///
int32_t Value::getLoadRegId()
{
    return -1;
}

///
/// @brief 对该Value进行Load用的寄存器编号
/// @return int32_t 寄存器编号
///
void Value::setLoadRegId(int32_t regId)
{
    (void) regId;
}

/// @brief 设置全局来源信息
/// @param baseName 全局变量名（不含@前缀）
/// @param offset 相对偏移量（字节）
void Value::setGlobalSource(const std::string & baseName, int64_t offset)
{
    globalBaseName = baseName;
    globalOffset = offset;
    hasGlobalSource = true;

    // 调试输出
    printf("Setting global source for Value '%s': base=%s, offset=%ld\n", getName().c_str(), baseName.c_str(), offset);
}

/// @brief 清除全局来源信息
void Value::clearGlobalSource()
{
    globalBaseName.clear();
    globalOffset = 0;
    hasGlobalSource = false;
}

/// @brief 获取全局变量基址名称
/// @return 全局变量名（不含@前缀）
std::string Value::getGlobalBaseName() const
{
    return globalBaseName;
}

/// @brief 获取相对于全局变量的偏移量
/// @return 偏移量（字节）
int64_t Value::getGlobalOffset() const
{
    return globalOffset;
}

/// @brief 判断是否派生自全局变量
/// @return true 如果派生自全局变量
bool Value::isDerivedFromGlobal() const
{
    return hasGlobalSource && !globalBaseName.empty();
}

/// @brief 传播全局来源信息给另一个Value
/// @param target 目标Value
/// @param additionalOffset 额外偏移量
void Value::propagateGlobalSource(Value * target, int64_t additionalOffset) const
{
    if (target && isDerivedFromGlobal()) {
        target->setGlobalSource(globalBaseName, globalOffset + additionalOffset);

        // 调试输出
        printf("Propagating global source from '%s' to '%s': base=%s, offset=%ld\n",
               getName().c_str(),
               target->getName().c_str(),
               globalBaseName.c_str(),
               globalOffset + additionalOffset);
    }
}

/// @brief 调试输出全局来源信息
/// @return 全局来源信息字符串
std::string Value::getGlobalSourceInfo() const
{
    if (isDerivedFromGlobal()) {
        return "GLOBAL[" + globalBaseName + "+" + std::to_string(globalOffset) + "]";
    } else {
        return "LOCAL";
    }
}