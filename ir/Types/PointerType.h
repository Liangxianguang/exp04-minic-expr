///
/// @file PointerType.h
/// @brief 指针类型描述类
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

#include "Type.h"
#include "StorageSet.h"
#include <vector>

///
/// @brief 指针类型
///
class PointerType : public Type {

    ///
    /// @brief Hash用结构体，提供Hash函数
    ///
    struct PointerTypeHasher final {

        /// @brief Given a PointerType, returns a hash value for the type
        ///        that is suitable for use in a std::unordered_map.
        ///
        /// @param[in] type The PointerType to hash
        ///
        /// @returns A hash value for the type
        size_t operator()(const PointerType & type) const noexcept
        {
            return std::hash<const Type *>{}(type.getPointeeType());
        }
    };

    ///
    /// @brief 判断两者相等的结构体，提供等于函数
    ///
    struct PointerTypeEqual final {
        /// @brief Checks if two PointerType objects are equal.
        ///
        /// @param[in] lhs  The first PointerType to compare
        /// @param[in] rhs  The second PointerType to compare
        ///
        /// @returns true if the two PointerTypes are equal, false otherwise.
        ///
        size_t operator()(const PointerType & lhs, const PointerType & rhs) const noexcept
        {
            return lhs.getPointeeType() == rhs.getPointeeType();
        }
    };

public:
    /// @brief PointerType的构造函数
    /// @param[in] pointeeType 指针所指向的类型
    ///
    /// 该构造函数将Type的ID设置为PointerTypeID，并且
    /// 保存pointeeType的指针到pointeeType成员变量
    explicit PointerType(const Type * pointeeType) : Type(PointerTyID)
    {
        this->pointeeType = pointeeType;

        if (pointeeType->isPointerType()) {

            Instanceof(pType, const PointerType *, pointeeType);
            this->rootType = pType->getRootType();
            this->depth = pType->getDepth() + 1;
        } else {
            this->rootType = pointeeType;
            this->depth = 1;
        }
    }

    ///
    /// @brief 返回根类型，也就是连续解引用后的类型
    /// @return const Type*
    ///
    [[nodiscard]] const Type * getRootType() const
    {
        return rootType;
    }

    ///
    /// @brief 返回指针所指向的类型，只解引用一次
    /// @return const Type*
    ///
    [[nodiscard]] const Type * getPointeeType() const
    {
        return pointeeType;
    }

    ///
    /// @brief 返回指针的深度，也就是可连续解引用的次数
    /// @return int32_t
    ///
    [[nodiscard]] int32_t getDepth() const
    {
        return depth;
    }

    ///
    /// @brief 获取指针类型
    /// @param pointee
    /// @return const PointerType*
    ///
    static const PointerType * get(Type * pointee)
    {
        static StorageSet<PointerType, PointerTypeHasher, PointerTypeEqual> storageSet;
        return storageSet.get(pointee);
    }

    ///
    /// @brief 获取类型的IR标识符
    /// @return std::string IR标识符void
    ///
    [[nodiscard]] std::string toString() const override
    {
        return pointeeType->toString() + "*";
    }

private:
    ///
    /// @brief 指针直接指向的类型，在指针操作中只解引用一次
    /// 例如：指针类型[3 x i32]***的指向类型为[3 x i32]**，只取掉最后一个*
    ///
    const Type * pointeeType = nullptr;

    ///
    /// @brief 所有指针操作都解引用后指向的元素类型，不再是指针类型
    /// 例如：指针类型[3 x i32]***的根类型为[3 x i32]
    ///
    const Type * rootType = nullptr;

    ///
    /// @brief *连续的个数，指针的深度
    /// 例如：指针类型[3 x i32]***的深度为3
    ///
    int32_t depth = 1;
};

//添加 ArrayType 类的定义和实现-lxg
///
/// @brief 数组类型
///
class ArrayType : public Type {
private:
    /// @brief 元素类型
    Type* elementType;
    
    /// @brief 数组维度数量
    int dimension;
    
    /// @brief 每个维度的大小
    std::vector<int> dimensions;
    
public:
    /// @brief 构造函数
    /// @param elemType 元素类型
    /// @param dims 各维度大小
    ArrayType(Type* elemType, const std::vector<int>& dims)
        : Type(ArrayTyID), elementType(elemType), dimensions(dims)
    {
        dimension = dimensions.size();
    }
    
    /// @brief 获取元素类型
    /// @return 元素类型
    Type* getElementType() const { return elementType; }
    
    /// @brief 获取维度数量
    /// @return 维度数量
    int getDimension() const { return dimension; }
    
    /// @brief 获取各维度大小
    /// @return 各维度大小数组
    const std::vector<int>& getDimensions() const { return dimensions; }
    
    /// @brief 获取单个元素大小（字节）
    /// @return 单个元素大小
    int getElementSize() const { return elementType->getSize(); }
    
    /// @brief 获取整个数组占用空间大小（字节）
    /// @return 整个数组大小
    int getTotalSize() const {
        int total = getElementSize();
        for (int dim : dimensions) {
            total *= dim;
        }
        return total;
    }
    
    /// @brief 获取类型大小
    /// @return 整个数组大小
    int32_t getSize() const override { return getTotalSize(); }
    
    /// @brief 获取类型字符串表示
    /// @return 类型字符串
    std::string toString() const override {
        std::string result = elementType->toString() + "[";
        for (size_t i = 0; i < dimensions.size(); i++) {
            if (i > 0) {
                result += "][";
            }
            result += std::to_string(dimensions[i]);
        }
        result += "]";
        return result;
    }
    
    /// @brief 创建数组类型的静态方法
    /// @param elemType 元素类型
    /// @param dims 维度大小列表
    /// @return 数组类型指针
    static ArrayType* get(Type* elemType, const std::vector<int>& dims) {
        return new ArrayType(elemType, dims);
    }
};