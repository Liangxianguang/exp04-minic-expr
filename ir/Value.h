///
/// @file Value.h
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
#pragma once

#include <cstdint>
#include <string>

#include "Use.h"
#include "Type.h"

///
/// @brief 值类，每个值都要有一个类型，全局变量和局部变量可以有名字，
/// 但通过运算得到的指令类值没有名字，只有在需要输出时给定名字即可
///
/// Value表示所有可计算的值的基类，例如常量、指令、参数等。
/// 每个Value都有一个类型(Type)和一个名字(Name)。Value是IR中所有可计算实体的抽象。
///
class Value {

protected:
    /// @brief 变量名，函数名等原始的名字，可能为空串
    std::string name;

    ///
    /// @brief IR名字，用于文本IR的输出
    ///
    std::string IRName;

    /// @brief 类型
    Type * type;

    ///
    /// @brief define-use链，这个定值被使用的所有边，即所有的User
    ///
    std::vector<Use *> uses;

    /// @brief 全局变量基址名称（去掉@前缀），如"array"
    std::string globalBaseName;

    /// @brief 相对于全局变量基址的偏移量（字节）
    int64_t globalOffset;

    /// @brief 是否有全局来源
    bool hasGlobalSource;

public:
    /// @brief 构造函数
    /// @param _type
    explicit Value(Type * _type);

    /// @brief 析构函数
    virtual ~Value();

    /// @brief 获取名字
    /// @return 变量名
    [[nodiscard]] virtual std::string getName() const;

    ///
    /// @brief 设置名字
    /// @param _name 名字
    ///
    void setName(std::string _name);

    /// @brief 获取名字
    /// @return 变量名
    [[nodiscard]] virtual std::string getIRName() const;

    ///
    /// @brief 设置名字
    /// @param _name 名字
    ///
    void setIRName(std::string _name);

    /// @brief 获取类型
    /// @return 变量名
    virtual Type * getType();

    ///
    /// @brief 增加一条边，增加Value被使用次数
    /// @param use
    ///
    void addUse(Use * use);

    ///
    /// @brief 消除一条边，减少Value被使用次数
    /// @param use
    ///
    void removeUse(Use * use);

    ///
    /// @brief 取得变量所在的作用域层级
    /// @return int32_t 层级
    ///
    virtual int32_t getScopeLevel();

    ///
    /// @brief 获得分配的寄存器编号或ID
    /// @return int32_t 寄存器编号
    ///
    virtual int32_t getRegId();

    ///
    /// @brief @brief 如是内存变量型Value，则获取基址寄存器和偏移
    /// @param regId 寄存器编号
    /// @param offset 相对偏移
    /// @return true 是内存型变量
    /// @return false 不是内存型变量
    ///
    virtual bool getMemoryAddr(int32_t * regId = nullptr, int64_t * offset = nullptr);

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    virtual int32_t getLoadRegId();

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    virtual void setLoadRegId(int32_t regId);

    /// @brief 通过名称前缀判断是否是全局变量-lxg
    bool isGlobalVariable() const
    {
        const std::string & name = getName();
        return !name.empty() && name[0] == '@';
    }

    /// @brief 通过名称前缀判断是否是局部变量
    bool isLocalVariable() const
    {
        const std::string & name = getName();
        return !name.empty() && name[0] == '%';
    }

    /// @brief 设置全局来源信息
    /// @param baseName 全局变量名（不含@前缀）
    /// @param offset 相对偏移量（字节）
    void setGlobalSource(const std::string & baseName, int64_t offset = 0);

    /// @brief 清除全局来源信息
    void clearGlobalSource();

    /// @brief 获取全局变量基址名称
    /// @return 全局变量名（不含@前缀）
    std::string getGlobalBaseName() const;

    /// @brief 获取相对于全局变量的偏移量
    /// @return 偏移量（字节）
    int64_t getGlobalOffset() const;

    /// @brief 判断是否派生自全局变量
    /// @return true 如果派生自全局变量
    bool isDerivedFromGlobal() const;

    /// @brief 传播全局来源信息给另一个Value
    /// @param target 目标Value
    /// @param additionalOffset 额外偏移量
    void propagateGlobalSource(Value * target, int64_t additionalOffset = 0) const;

    /// @brief 调试输出全局来源信息
    /// @return 全局来源信息字符串
    std::string getGlobalSourceInfo() const;
};
