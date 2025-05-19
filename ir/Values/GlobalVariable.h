///
/// @file GlobalVariable.h
/// @brief 全局变量描述类
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

#include "GlobalValue.h"
#include "IRConstant.h"
#include "../Types/PointerType.h" // 添加此行以引入 ArrayType 类的定义-lxg

///
/// @brief 全局变量，寻址时通过符号名或变量名来寻址
///
class GlobalVariable : public GlobalValue {

public:
    ///
    /// @brief 构建全局变量，默认对齐为4字节
    /// @param _type 类型
    /// @param _name 名字
    ///
    explicit GlobalVariable(Type * _type, std::string _name) : GlobalValue(_type, _name)
    {
        // 设置对齐大小
        setAlignment(4);
    }

    ///
    /// @brief  检查是否是函数
    /// @return true 是函数
    /// @return false 不是函数
    ///
    [[nodiscard]] bool isGlobalVarible() const override
    {
        return true;
    }

    ///
    /// @brief 是否属于BSS段的变量，即未初始化过的变量，或者初值都为0的变量
    /// @return true
    /// @return false
    ///
    [[nodiscard]] bool isInBSSSection() const
    {
        return this->inBSSSection;
    }

    ///
    /// @brief 取得变量所在的作用域层级
    /// @return int32_t 层级
    ///
    int32_t getScopeLevel() override
    {
        return 0;
    }

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    int32_t getLoadRegId() override
    {
        return this->loadRegNo;
    }

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    void setLoadRegId(int32_t regId) override
    {
        this->loadRegNo = regId;
    }

    ///
    /// @brief Declare指令IR显示
    /// @param str
    ///
    // void toDeclareString(std::string & str)
    // {
    //     str = "declare " + getType()->toString() + " " + getIRName();
    // }
	//修改 ir/Values/GlobalVariable.h 中的 toDeclareString 方法，使其支持数组类型-lxg
	void toDeclareString(std::string & str)
	{
		if (getType()->isArrayType()) {
			// 数组类型，使用新的格式：declare i32 @a[10][10] ;全局数组a
			auto* arrayType = static_cast<ArrayType*>(getType());
			Type* elemType = arrayType->getElementType();
			const std::vector<int>& dimensions = arrayType->getDimensions();
			
			// 检查是否为有效数组 - 数组不应该有空维度，并且维度大小都应该大于0
			bool isValidArray = !dimensions.empty();
			for (int dim : dimensions) {
				if (dim <= 0) {
					isValidArray = false;
					break;
				}
			}
			
			if (isValidArray) {
				// 输出基本类型和变量名：declare i32 @a
				str = "declare " + elemType->toString() + " " + getIRName();
				
				// 添加数组维度信息：[10][10]
				for (int dim : dimensions) {
					str += "[" + std::to_string(dim) + "]";
				}
				
				// 添加注释：;全局数组a
				std::string realName = getName();
				if (!realName.empty()) {
					str += " ;全局数组" + realName;
				}
			} else {
				// 无效的数组，按照普通变量处理
				str = "declare " + getType()->toString() + " " + getIRName();
			}
		} else {
			// 非数组类型使用原有格式
			str = "declare " + getType()->toString() + " " + getIRName();
		}
	}

private:
    ///
    /// @brief 变量加载到寄存器中时对应的寄存器编号
    ///
    int32_t loadRegNo = -1;

    ///
    /// @brief 默认全局变量在BSS段，没有初始化，或者即使初始化过，但都值都为0
    ///
    bool inBSSSection = true;
};
