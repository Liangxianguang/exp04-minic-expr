///
/// @file Function.cpp
/// @brief 函数实现
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

#include <cstdlib>
#include <string>
#include <map>
#include <algorithm>

#include "IRConstant.h"
#include "Function.h"
#include "Types/PointerType.h" // 包含 ArrayType 定义-lxg

/// @brief 指定函数名字、函数类型的构造函数
/// @param _name 函数名称
/// @param _type 函数类型
/// @param _builtin 是否是内置函数
Function::Function(std::string _name, FunctionType * _type, bool _builtin)
    : GlobalValue(_type, _name), builtIn(_builtin)
{
    returnType = _type->getReturnType();

    // 设置对齐大小
    setAlignment(1);
}

///
/// @brief 析构函数
/// @brief 释放函数占用的内存和IR指令代码
/// @brief 注意：IR指令代码并未释放，需要手动释放
Function::~Function()
{
    Delete();
}

/// @brief 获取函数返回类型
/// @return 返回类型
Type * Function::getReturnType()
{
    return returnType;
}

/// @brief 获取函数的形参列表
/// @return 形参列表
std::vector<FormalParam *> & Function::getParams()
{
    return params;
}

/// @brief 获取函数内的IR指令代码
/// @return IR指令代码
InterCode & Function::getInterCode()
{
    return code;
}

/// @brief 判断该函数是否是内置函数
/// @return true: 内置函数，false：用户自定义
bool Function::isBuiltin()
{
    return builtIn;
}

/// @brief 函数指令信息输出
/// @param str 函数指令
void Function::toString(std::string & str)
{
    if (builtIn) {
        // 内置函数则什么都不输出
        return;
    }

    // 输出函数头
    str = "define " + getReturnType()->toString() + " " + getIRName() + "(";

    bool firstParam = false;
    for (auto & param: params) {

        if (!firstParam) {
            firstParam = true;
        } else {
            str += ", ";
        }

        std::string param_str = param->getType()->toString() + param->getIRName();

        str += param_str;
    }

    str += ")\n";

    str += "{\n";

    // 输出局部变量的名字与IR名字
    for (auto & var: this->varsVector) {
        if (var->getType()->isArrayType()) {
            // 数组类型，使用新的格式：declare i32 %l1[10][10] ;数组a
            auto * arrayType = static_cast<ArrayType *>(var->getType());
            Type * elemType = arrayType->getElementType();
            const std::vector<int> & dimensions = arrayType->getDimensions();

            // 输出基本类型和变量名：declare i32 %l1
            str += "\tdeclare " + elemType->toString() + " " + var->getIRName();

            // 添加数组维度信息：[10][10]
            for (int dim: dimensions) {
                str += "[" + std::to_string(dim) + "]";
            }

            // 添加注释：;数组a
            std::string realName = var->getName();
            if (!realName.empty()) {
                str += " ;数组" + realName;
            }
        } else {
            // 非数组类型使用原有格式
            str += "\tdeclare " + var->getType()->toString() + " " + var->getIRName();

            std::string realName = var->getName();
            if (!realName.empty()) {
                str += " ; " + std::to_string(var->getScopeLevel()) + ":" + realName;
            }
        }
        str += "\n";
    }

    // 输出临时变量的declare形式
    // 遍历所有的线性IR指令，文本输出
    for (auto & inst: code.getInsts()) {

        if (inst->hasResultValue()) {

            // 局部变量和临时变量需要输出declare语句
            str += "\tdeclare " + inst->getType()->toString() + " " + inst->getIRName() + "\n";
        }
    }

    // 遍历所有的线性IR指令，文本输出
    for (auto & inst: code.getInsts()) {

        std::string instStr;
        inst->toString(instStr);

        if (!instStr.empty()) {

            // Label指令不加Tab键
            if (inst->getOp() == IRInstOperator::IRINST_OP_LABEL) {
                str += instStr + "\n";
            } else {
                str += "\t" + instStr + "\n";
            }
        }
    }

    // 输出函数尾部
    str += "}\n";
}

/// @brief 设置函数出口指令
/// @param inst 出口Label指令
void Function::setExitLabel(Instruction * inst)
{
    exitLabel = inst;
}

/// @brief 获取函数出口指令
/// @return 出口Label指令
Instruction * Function::getExitLabel()
{
    return exitLabel;
}

/// @brief 设置函数返回值变量
/// @param val 返回值变量，要求必须是局部变量，不能是临时变量
void Function::setReturnValue(LocalVariable * val)
{
    returnValue = val;
}

/// @brief 获取函数返回值变量
/// @return 返回值变量
LocalVariable * Function::getReturnValue()
{
    return returnValue;
}

/// @brief 获取最大栈帧深度
/// @return 栈帧深度
int Function::getMaxDep()
{
    return maxDepth;
}

/// @brief 设置最大栈帧深度
/// @param dep 栈帧深度
void Function::setMaxDep(int dep)
{
    maxDepth = dep;

    // 设置函数栈帧被重定位标记，用于生成不同的栈帧保护代码
    relocated = true;
}

/// @brief 获取本函数需要保护的寄存器
/// @return 要保护的寄存器
std::vector<int32_t> & Function::getProtectedReg()
{
    return protectedRegs;
}

/// @brief 获取本函数需要保护的寄存器字符串
/// @return 要保护的寄存器
std::string & Function::getProtectedRegStr()
{
    return protectedRegStr;
}

/// @brief 获取函数调用参数个数的最大值
/// @return 函数调用参数个数的最大值
int Function::getMaxFuncCallArgCnt()
{
    return maxFuncCallArgCnt;
}

/// @brief 设置函数调用参数个数的最大值
/// @param count 函数调用参数个数的最大值
void Function::setMaxFuncCallArgCnt(int count)
{
    maxFuncCallArgCnt = count;
}

/// @brief 函数内是否存在函数调用
/// @return 是否存在函调用
bool Function::getExistFuncCall()
{
    return funcCallExist;
}

/// @brief 设置函数是否存在函数调用
/// @param exist true: 存在 false: 不存在
void Function::setExistFuncCall(bool exist)
{
    funcCallExist = exist;
}

/// @brief 新建变量型Value。先检查是否存在，不存在则创建，否则失败
/// @param name 变量ID
/// @param type 变量类型
/// @param scope_level 局部变量的作用域层级
LocalVariable * Function::newLocalVarValue(Type * type, std::string name, int32_t scope_level)
{
    // 创建变量并加入符号表
    LocalVariable * varValue = new LocalVariable(type, name, scope_level);

    // varsVector表中可能存在变量重名的信息
    varsVector.push_back(varValue);

    return varValue;
}

/// @brief 新建一个内存型的Value，并加入到符号表，用于后续释放空间
/// \param type 变量类型
/// \return 临时变量Value
MemVariable * Function::newMemVariable(Type * type)
{
    // 肯定唯一存在，直接插入即可
    MemVariable * memValue = new MemVariable(type);

    memVector.push_back(memValue);

    return memValue;
}

/// @brief 清理函数内申请的资源
void Function::Delete()
{
    // 清理IR指令
    code.Delete();

    // 清理Value
    for (auto & var: varsVector) {
        delete var;
    }

    varsVector.clear();
}

///
/// @brief 函数内的Value重命名
///
void Function::renameIR()
{
    // 内置函数忽略
    if (isBuiltin()) {
        return;
    }

    int32_t nameIndex = 0;

    // 形式参数重命名
    for (auto & param: this->params) {
        param->setIRName(IR_TEMP_VARNAME_PREFIX + std::to_string(nameIndex++));
    }

    // 局部变量重命名
    for (auto & var: this->varsVector) {

        var->setIRName(IR_LOCAL_VARNAME_PREFIX + std::to_string(nameIndex++));
    }

    // 遍历所有的指令进行命名
    for (auto inst: this->getInterCode().getInsts()) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_LABEL) {
            inst->setIRName(IR_LABEL_PREFIX + std::to_string(nameIndex++));
        } else if (inst->hasResultValue()) {
            inst->setIRName(IR_TEMP_VARNAME_PREFIX + std::to_string(nameIndex++));
        }
    }
}

///
/// @brief 获取统计的ARG指令的个数
/// @return int32_t 个数
///
int32_t Function::getRealArgcount()
{
    return this->realArgCount;
}

///
/// @brief 用于统计ARG指令个数的自增函数，个数加1
///
void Function::realArgCountInc()
{
    this->realArgCount++;
}

///
/// @brief 用于统计ARG指令个数的清零
///
void Function::realArgCountReset()
{
    this->realArgCount = 0;
}

/// @brief 计算变量的实际大小（字节）
/// @param type 变量类型
/// @return 变量占用的总字节数
int32_t Function::calculateVariableSize(Type * type)
{
    if (!type) {
        return 4; // 默认4字节
    }

    if (type->isArrayType()) {
        // 使用ArrayType的getTotalSize()方法
        ArrayType * arrayType = dynamic_cast<ArrayType *>(type);
        if (arrayType) {
            int32_t totalSize = arrayType->getTotalSize();
            printf("Array size calculation: %d bytes\n", totalSize);
            return totalSize;
        }
        return 32; // 备用默认值
    } else if (type->isPointerType()) {
        return 4; // 指针在ARM32中是4字节
    } else {
        return 4; // 普通类型默认4字节
    }
}

/// @brief 重新分配所有变量的内存地址（修复地址冲突）
void Function::reallocateMemory()
{
    if (memoryFixed) {
        return;
    }

    printf("=== Starting Memory Reallocation for Function %s ===\n", getName().c_str());

    const int32_t framePointerReg = 11;

    // 第一步：计算所有变量的总空间
    int32_t totalArraySize = 0;
    int32_t totalVarSize = 0;
    int32_t arrayCount = 0;

    printf("--- Phase 1: Calculating Space Requirements ---\n");
    for (auto & var: varsVector) {
        int32_t varSize = calculateVariableSize(var->getType());
        if (var->getType()->isArrayType()) {
            totalArraySize += varSize;
            arrayCount++;
            printf("Array %s: %d bytes\n", var->getName().c_str(), varSize);
        } else {
            totalVarSize += varSize;
        }
    }

    for (auto & memVar: memVector) {
        totalVarSize += calculateVariableSize(memVar->getType());
    }

    printf("Total space needed: arrays=%d bytes (%d arrays), variables=%d bytes\n",
           totalArraySize,
           arrayCount,
           totalVarSize);

    // 第二步：从fp-4开始，往负方向分配
    int32_t currentOffset = -4;

    printf("--- Phase 2: Allocating Arrays (corrected layout) ---\n");

    // 先分配所有数组
    for (size_t i = 0; i < varsVector.size(); i++) {
        auto & var = varsVector[i];

        if (var->getType()->isArrayType()) {
            int32_t arraySize = calculateVariableSize(var->getType());

            // 关键修正：数组基地址应该指向数组的最低地址
            // 先移动currentOffset到数组的最低位置
            currentOffset -= arraySize;

            // 数组基地址 = 最低地址 + (arraySize - 4)，使得：
            // b[0] = base + 0 指向最高地址
            // b[n-1] = base + (n-1)*4 指向最低地址
            int32_t arrayBaseOffset = currentOffset + arraySize - 4;

            printf("Allocating Array[%zu] %s: size=%d, base at fp%+d",
                   i,
                   var->getName().c_str(),
                   arraySize,
                   arrayBaseOffset);

            var->setMemoryAddr(framePointerReg, arrayBaseOffset);

            // 验证数组访问范围
            printf(" (elements: fp%+d to fp%+d)", arrayBaseOffset, currentOffset);

            // 验证访问安全性
            if (currentOffset < 0) {
                printf(" ✓ SAFE\n");
            } else {
                printf(" ⚠️  WARNING: Array extends to positive offsets!\n");
            }

            // 留4字节间隙
            currentOffset -= 4;
        }
    }

    printf("--- Phase 3: Allocating Non-Array Variables ---\n");

    // 分配非数组变量
    for (size_t i = 0; i < varsVector.size(); i++) {
        auto & var = varsVector[i];

        if (!var->getType()->isArrayType()) {
            int32_t varSize = calculateVariableSize(var->getType());

            printf("Allocating LocalVar[%zu] %s: size=%d, at fp%+d\n",
                   i,
                   var->getName().c_str(),
                   varSize,
                   currentOffset);

            var->setMemoryAddr(framePointerReg, currentOffset);
            currentOffset -= varSize;
        }
    }

    printf("--- Phase 4: Allocating MemVariables ---\n");

    // 分配MemVariable
    for (size_t i = 0; i < memVector.size(); i++) {
        auto & memVar = memVector[i];

        int32_t varSize = calculateVariableSize(memVar->getType());

        printf("Allocating MemVar[%zu] %s: size=%d, at fp%+d\n", i, memVar->getName().c_str(), varSize, currentOffset);

        memVar->setMemoryAddr(framePointerReg, currentOffset);
        currentOffset -= varSize;
    }

    // 第五步：计算栈帧大小
    int32_t totalStackSize = -(currentOffset + 4);

    // 8字节对齐
    totalStackSize = (totalStackSize + 7) & ~7;

    int32_t oldMaxDepth = getMaxDep();
    setMaxDep(totalStackSize);

    printf("--- Final Memory Layout Summary ---\n");
    printf("Stack frame size updated: %d -> %d bytes (8-byte aligned)\n", oldMaxDepth, totalStackSize);
    printf("Memory layout:\n");
    printf("  Allocation range: fp-4 to fp%+d\n", currentOffset);
    printf("  Total usage: %d bytes\n", totalStackSize);
    printf("  Arrays: %d bytes (%d arrays)\n", totalArraySize, arrayCount);
    printf("  Variables: %d bytes\n", totalVarSize);

    memoryFixed = true;
    printf("=== Memory Reallocation Complete ===\n");
}

/// @brief 验证内存分配是否有冲突
bool Function::validateMemoryAllocation()
{
    printf("=== Validating Memory Allocation ===\n");

    // 使用map来收集所有的地址分配信息
    std::map<int64_t, std::vector<std::string>> offsetMap;

    // 收集所有LocalVariable的地址
    for (auto & var: varsVector) {
        int32_t baseReg;
        int64_t offset;
        if (var->getMemoryAddr(&baseReg, &offset)) {
            std::string varInfo =
                var->getName() + " (LocalVar, " + (var->getType()->isPointerType() ? "pointer)" : "normal)");
            offsetMap[offset].push_back(varInfo);
        }
    }

    // 收集所有MemVariable的地址
    for (auto & memVar: memVector) {
        int32_t baseReg;
        int64_t offset;
        if (memVar->getMemoryAddr(&baseReg, &offset)) {
            std::string varInfo =
                memVar->getName() + " (MemVar, " + (memVar->getType()->isPointerType() ? "pointer)" : "normal)");
            offsetMap[offset].push_back(varInfo);
        }
    }

    // 检查冲突
    bool hasConflict = false;
    int conflictCount = 0;

    for (auto & pair: offsetMap) {
        if (pair.second.size() > 1) {
            printf("CONFLICT #%d at offset %ld:\n", ++conflictCount, pair.first);
            for (auto & varInfo: pair.second) {
                printf("  - %s\n", varInfo.c_str());
            }
            hasConflict = true;
        }
    }

    if (!hasConflict) {
        printf("✓ No memory allocation conflicts detected.\n");
    } else {
        printf("✗ Found %d memory allocation conflicts.\n", conflictCount);
    }

    printf("=== Validation Complete ===\n");
    return !hasConflict;
}

/// @brief 打印详细的内存布局信息（调试用）
void Function::printMemoryLayout()
{
    printf("=== Memory Layout for Function %s ===\n", getName().c_str());

    // 收集所有变量的地址信息
    std::vector<std::tuple<int64_t, std::string, std::string, std::string, int32_t>> layout;

    // 收集LocalVariable信息
    for (auto & var: varsVector) {
        int32_t baseReg;
        int64_t offset;
        if (var->getMemoryAddr(&baseReg, &offset)) {
            std::string typeStr;
            int32_t size;

            if (var->getType()->isArrayType()) {
                ArrayType * arrayType = dynamic_cast<ArrayType *>(var->getType());
                if (arrayType) {
                    auto & dims = arrayType->getDimensions();
                    typeStr = "array[";
                    for (size_t i = 0; i < dims.size(); i++) {
                        if (i > 0)
                            typeStr += "][";
                        typeStr += std::to_string(dims[i]);
                    }
                    typeStr += "]";
                    size = arrayType->getTotalSize();
                } else {
                    typeStr = "array";
                    size = 32;
                }
            } else if (var->getType()->isPointerType()) {
                typeStr = "pointer";
                size = 4;
            } else {
                typeStr = "normal";
                size = 4;
            }

            layout.push_back(std::make_tuple(offset, var->getName(), "LocalVar", typeStr, size));
        }
    }

    // 收集MemVariable信息
    for (auto & memVar: memVector) {
        int32_t baseReg;
        int64_t offset;
        if (memVar->getMemoryAddr(&baseReg, &offset)) {
            std::string typeStr = memVar->getType()->isPointerType() ? "pointer" : "normal";
            int32_t size = calculateVariableSize(memVar->getType());
            layout.push_back(std::make_tuple(offset, memVar->getName(), "MemVar", typeStr, size));
        }
    }

    // 按地址排序（从高地址到低地址）
    std::sort(layout.begin(), layout.end(), [](const auto & a, const auto & b) {
        return std::get<0>(a) > std::get<0>(b);
    });

    // 打印布局表
    printf("Stack layout (high to low address):\n");
    printf("  Address    | Variable   | Category | Type            | Size\n");
    printf("  -----------|------------|----------|-----------------|------\n");

    for (auto & item: layout) {
        printf("  fp%+ld | %-10s | %-8s | %-15s | %d\n",
               std::get<0>(item),         // offset
               std::get<1>(item).c_str(), // name
               std::get<2>(item).c_str(), // category
               std::get<3>(item).c_str(), // type
               std::get<4>(item));        // size
    }

    printf("Total variables: LocalVar=%zu, MemVar=%zu\n", varsVector.size(), memVector.size());
    printf("Current stack frame size: %d bytes\n", getMaxDep());
    printf("=== End Memory Layout ===\n");
}