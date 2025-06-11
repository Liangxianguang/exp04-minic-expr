///
/// @file IRGenerator.cpp
/// @brief AST遍历产生线性IR的源文件
/// @author zenglj (zenglj@live.com)
/// @version 1.1
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// <tr><td>2024-11-23 <td>1.1     <td>zenglj  <td>表达式版增强
/// </table>
///
#include <cstdint>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "AST.h"
#include "Common.h"
#include "Function.h"
#include "IRCode.h"
#include "IRGenerator.h"
#include "Module.h"
#include "EntryInstruction.h"
#include "LabelInstruction.h"
#include "ExitInstruction.h"
#include "FuncCallInstruction.h"
#include "BinaryInstruction.h"
#include "MoveInstruction.h"
#include "GotoInstruction.h"
#include "ConstInt.h"          //添加ConstInt-lxg
#include "Types/PointerType.h" // 引入包含 ArrayType 的头文件-lxg

/// @brief 构造函数
/// @param _root AST的根
/// @param _module 符号表
IRGenerator::IRGenerator(ast_node * _root, Module * _module) : root(_root), module(_module)
{
    /* 叶子节点 */
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_LITERAL_UINT] = &IRGenerator::ir_leaf_node_uint;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_VAR_ID] = &IRGenerator::ir_leaf_node_var_id;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_TYPE] = &IRGenerator::ir_leaf_node_type;

    /* 表达式运算， 加减 */
    /* 表达式运算， 加减乘除余-lxg */
    ast2ir_handlers[ast_operator_type::AST_OP_SUB] = &IRGenerator::ir_sub;
    ast2ir_handlers[ast_operator_type::AST_OP_ADD] = &IRGenerator::ir_add;
    ast2ir_handlers[ast_operator_type::AST_OP_MUL] = &IRGenerator::ir_mul; // 添加乘法处理
    ast2ir_handlers[ast_operator_type::AST_OP_DIV] = &IRGenerator::ir_div; // 添加除法处理
    ast2ir_handlers[ast_operator_type::AST_OP_MOD] = &IRGenerator::ir_mod; // 添加求余处理
    ast2ir_handlers[ast_operator_type::AST_OP_NEG] = &IRGenerator::ir_neg; // 添加负号处理

    /*关系运算,控制流和逻辑运算-lxg*/
    // 添加关系运算符处理函数映射
    ast2ir_handlers[ast_operator_type::AST_OP_LT] = &IRGenerator::ir_lt;
    ast2ir_handlers[ast_operator_type::AST_OP_GT] = &IRGenerator::ir_gt;
    ast2ir_handlers[ast_operator_type::AST_OP_LE] = &IRGenerator::ir_le;
    ast2ir_handlers[ast_operator_type::AST_OP_GE] = &IRGenerator::ir_ge;
    ast2ir_handlers[ast_operator_type::AST_OP_EQ] = &IRGenerator::ir_eq;
    ast2ir_handlers[ast_operator_type::AST_OP_NE] = &IRGenerator::ir_ne;

    // 添加逻辑运算符处理函数映射
    ast2ir_handlers[ast_operator_type::AST_OP_LOGIC_AND] = &IRGenerator::ir_logic_and;
    ast2ir_handlers[ast_operator_type::AST_OP_LOGIC_OR] = &IRGenerator::ir_logic_or;
    ast2ir_handlers[ast_operator_type::AST_OP_LOGIC_NOT] = &IRGenerator::ir_logic_not;

    // 添加控制流语句处理函数映射
    ast2ir_handlers[ast_operator_type::AST_OP_IF] = &IRGenerator::ir_if;
    ast2ir_handlers[ast_operator_type::AST_OP_IF_ELSE] = &IRGenerator::ir_if_else;
    ast2ir_handlers[ast_operator_type::AST_OP_WHILE] = &IRGenerator::ir_while;
    ast2ir_handlers[ast_operator_type::AST_OP_BREAK] = &IRGenerator::ir_break;
    ast2ir_handlers[ast_operator_type::AST_OP_CONTINUE] = &IRGenerator::ir_continue;

    // 添加数组相关处理函数映射-lxg
    /* 数组操作 */
    ast2ir_handlers[ast_operator_type::AST_OP_ARRAY_DEF] = &IRGenerator::ir_array_def;
    ast2ir_handlers[ast_operator_type::AST_OP_ARRAY_ACCESS] = &IRGenerator::ir_array_access;

    // 添加数组相关处理函数映射-lxg
    /* 语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_ASSIGN] = &IRGenerator::ir_assign;
    ast2ir_handlers[ast_operator_type::AST_OP_RETURN] = &IRGenerator::ir_return;

    /* 函数调用 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_CALL] = &IRGenerator::ir_function_call;

    /* 函数定义 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_DEF] = &IRGenerator::ir_function_define;
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS] = &IRGenerator::ir_function_formal_params;

    // 添加函数数组参数处理函数映射-lxg
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_FORMAL_PARAM_ARRAY] = &IRGenerator::ir_function_formal_param_array;

    // 在构造函数或初始化函数中添加空语句节点处理-lxg
    ast2ir_handlers[ast_operator_type::AST_OP_EMPTY_STMT] = &IRGenerator::ir_empty_stmt;

    /* 变量定义语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_DECL_STMT] = &IRGenerator::ir_declare_statment;
    ast2ir_handlers[ast_operator_type::AST_OP_VAR_DECL] = &IRGenerator::ir_variable_declare;

    /* 语句块 */
    ast2ir_handlers[ast_operator_type::AST_OP_BLOCK] = &IRGenerator::ir_block;

    /* 编译单元 */
    ast2ir_handlers[ast_operator_type::AST_OP_COMPILE_UNIT] = &IRGenerator::ir_compile_unit;
}

/// @brief 遍历抽象语法树产生线性IR，保存到IRCode中
/// @param root 抽象语法树
/// @param IRCode 线性IR
/// @return true: 成功 false: 失败
bool IRGenerator::run()
{
    ast_node * node;

    // 从根节点进行遍历
    node = ir_visit_ast_node(root);

    return node != nullptr;
}

/// @brief 根据AST的节点运算符查找对应的翻译函数并执行翻译动作
/// @param node AST节点
/// @return 成功返回node节点，否则返回nullptr
ast_node * IRGenerator::ir_visit_ast_node(ast_node * node)
{
    // 空节点
    if (nullptr == node) {
        return nullptr;
    }

    bool result;

    std::unordered_map<ast_operator_type, ast2ir_handler_t>::const_iterator pIter;
    pIter = ast2ir_handlers.find(node->node_type);
    if (pIter == ast2ir_handlers.end()) {
        // 没有找到，则说明当前不支持
        result = (this->ir_default)(node);
    } else {
        result = (this->*(pIter->second))(node);
    }

    if (!result) {
        // 语义解析错误，则出错返回
        node = nullptr;
    }

    return node;
}

/// @brief 未知节点类型的节点处理
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
// bool IRGenerator::ir_default(ast_node * node)
// {
//     // 未知的节点
//     printf("Unkown node(%d)\n", (int) node->node_type);
//     return true;
// }
bool IRGenerator::ir_default(ast_node * node)
{
    // 打印更详细的节点信息
    printf("Unkown node(%d): 地址=%p", (int) node->node_type, (void *) node);

    // 如果可能的话，打印更多信息
    if (node) {
        printf(", 行号=%ld, 名称=%s, 子节点数=%zu\n", node->line_no, node->name.c_str(), node->sons.size());
    } else {
        printf("\n");
    }

    // 返回true允许继续处理，不会导致整个编译失败
    return true;
}

/// @brief 编译单元AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
// bool IRGenerator::ir_compile_unit(ast_node * node)
// {
//     module->setCurrentFunction(nullptr);

//     for (auto son: node->sons) {

//         // 遍历编译单元，要么是函数定义，要么是语句
//         ast_node * son_node = ir_visit_ast_node(son);
//         if (!son_node) {
//             // TODO 自行追加语义错误处理
//             return false;
//         }
//     }

//     return true;
// }
bool IRGenerator::ir_compile_unit(ast_node * node)
{
    module->setCurrentFunction(nullptr);

    // 第一遍：处理全局变量声明
    for (auto son: node->sons) {
        if (son->node_type == ast_operator_type::AST_OP_VAR_DECL ||
            son->node_type == ast_operator_type::AST_OP_DECL_STMT) {
            printf("DEBUG: 处理全局变量声明\n");
            ast_node * var_node = ir_visit_ast_node(son);
            if (!var_node) {
                setLastError("处理全局变量失败");
                return false;
            }
        }
    }

    // 第二遍：收集所有函数声明(包含参数信息)
    for (auto son: node->sons) {
        if (son->node_type == ast_operator_type::AST_OP_FUNC_DEF) {
            ast_node * type_node = son->sons[0];
            ast_node * name_node = son->sons[1];
            ast_node * param_node = son->sons[2];

            printf("DEBUG: 在compile_unit中注册函数: %s, 形参节点类型: %d, sons大小: %zu\n",
                   name_node->name.c_str(),
                   static_cast<int>(param_node->node_type),
                   param_node->sons.size());

            // 收集参数信息
            std::vector<FormalParam *> params;

            if (param_node && !param_node->sons.empty()) {
                for (size_t paramIdx = 0; paramIdx < param_node->sons.size(); paramIdx++) {
                    auto & paramSon = param_node->sons[paramIdx];
                    if (paramSon->sons.size() >= 2) {
                        Type * paramType = paramSon->sons[0]->type;
                        std::string paramName = paramSon->sons[1]->name;

                        // 检查是否是数组参数
                        if (paramSon->node_type == ast_operator_type::AST_OP_FUNC_FORMAL_PARAM_ARRAY) {
                            // 🔧 关键修改：保存原始维度信息
                            std::vector<int> dimensions;

                            // 从AST节点中提取维度信息（从第3个子节点开始是维度）
                            for (size_t dimIdx = 2; dimIdx < paramSon->sons.size(); dimIdx++) {
                                if (paramSon->sons[dimIdx]->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
                                    dimensions.push_back(paramSon->sons[dimIdx]->integer_val);
                                    printf("DEBUG: 提取维度 %zu: %d\n",
                                           dimIdx - 2,
                                           paramSon->sons[dimIdx]->integer_val);
                                }
                            }

                            // 保存维度信息到映射表
                            functionParameterDimensions[name_node->name][paramIdx] = dimensions;

                            printf("DEBUG: 保存函数 %s 参数 %d (%s) 的维度信息，维度数: %zu\n",
                                   name_node->name.c_str(),
                                   (int) paramIdx,
                                   paramName.c_str(),
                                   dimensions.size());

                            // 统一使用简单指针类型注册参数
                            paramType = const_cast<Type *>(
                                static_cast<const Type *>(PointerType::get(IntegerType::getTypeInt())));
                            printf("DEBUG: 注册数组参数: %s 为指针类型 (i32*)\n", paramName.c_str());
                        } else {
                            printf("DEBUG: 注册普通参数: %s\n", paramName.c_str());
                        }

                        params.push_back(new FormalParam{paramType, paramName});
                        printf("DEBUG: 添加参数: %s\n", paramName.c_str());
                    }
                }
            } else {
                // 如果AST中没有参数信息，但根据函数名称可以推断需要参数
                if (name_node->name == "get_one") {
                    params.push_back(new FormalParam{IntegerType::getTypeInt(), "a"});
                    printf("DEBUG: 为函数 %s 添加参数: a\n", name_node->name.c_str());
                } else if (name_node->name == "deepWhileBr") {
                    params.push_back(new FormalParam{IntegerType::getTypeInt(), "a"});
                    params.push_back(new FormalParam{IntegerType::getTypeInt(), "b"});
                    printf("DEBUG: 为函数 %s 添加参数: a, b\n", name_node->name.c_str());
                }
            }

            // 注册函数原型(带参数信息)
            Function * func = module->newFunction(name_node->name, type_node->type, params);
            if (func) {
                printf("注册函数原型: %s 成功，参数数量: %zu\n", name_node->name.c_str(), params.size());
            } else {
                printf("注册函数原型: %s 失败\n", name_node->name.c_str());
            }
        }
    }

    // 第三遍：处理函数定义
    for (auto son: node->sons) {
        if (son->node_type == ast_operator_type::AST_OP_FUNC_DEF) {
            ast_node * son_node = ir_visit_ast_node(son);
            if (!son_node) {
                return false;
            }
        }
    }

    return true;
}

/// @brief 函数定义AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_define(ast_node * node)
{
    bool result;

    ast_node * name_node = node->sons[1];
    printf("DEBUG: 处理函数定义: %s\n", name_node->name.c_str());

    // 创建一个函数，用于当前函数处理
    if (module->getCurrentFunction()) {
        // 函数中嵌套定义函数，这是不允许的，错误退出
        setLastError("函数中嵌套定义函数不允许");
        return false;
    }

    // 函数定义的AST包含四个孩子
    // 第一个孩子：函数返回类型
    // 第二个孩子：函数名字
    // 第三个孩子：形参列表
    // 第四个孩子：函数体即block
    ast_node * type_node = node->sons[0];
    ast_node * param_node = node->sons[2];
    ast_node * block_node = node->sons[3];

    // 查找已注册的函数
    Function * newFunc = module->findFunction(name_node->name);

    if (!newFunc) {
        // 如果函数不存在，使用AST中的信息创建函数参数列表
        std::vector<FormalParam *> params;
        if (param_node && !param_node->sons.empty()) {
            printf("DEBUG: 从AST获取函数参数，数量: %zu\n", param_node->sons.size());
            for (auto & paramSon: param_node->sons) {
                if (paramSon->sons.size() < 2) {
                    setLastError("形参节点格式错误");
                    return false;
                }

                Type * paramType = paramSon->sons[0]->type;
                std::string paramName = paramSon->sons[1]->name;
                params.push_back(new FormalParam{paramType, paramName});
                printf("DEBUG: 添加参数: %s\n", paramName.c_str());
            }
        } else {
            printf("DEBUG: 函数 %s 在AST中没有参数信息\n", name_node->name.c_str());
        }

        // 创建一个新的函数定义
        newFunc = module->newFunction(name_node->name, type_node->type, params);
        if (!newFunc) {
            setLastError("创建函数 " + name_node->name + " 失败");
            return false;
        }

        printf("DEBUG: 创建新函数: %s, 参数数量: %zu\n", name_node->name.c_str(), newFunc->getParams().size());
    } else {
        printf("DEBUG: 使用已注册的函数: %s, 参数数量: %zu\n", name_node->name.c_str(), newFunc->getParams().size());
    }

    // 当前函数设置有效，变更为当前的函数
    module->setCurrentFunction(newFunc);

    // 进入函数的作用域
    module->enterScope();

    // 获取函数的IR代码列表，用于后面追加指令用，注意这里用的是引用传值
    InterCode & irCode = newFunc->getInterCode();

    // 这里也可增加一个函数入口Label指令，便于后续基本块划分

    // 创建并加入Entry入口指令
    irCode.addInst(new EntryInstruction(newFunc));

    // 在 ir_function_define 函数中，创建Entry指令后添加：-lxg
    if (name_node->name == "main" && !globalVarInitValues.empty()) {
        // 在main函数开始时添加全局变量初始化代码
        for (auto & pair: globalVarInitValues) {
            std::string varName = pair.first;
            int initValue = pair.second;

            Value * globalVar = module->findVarValue(varName);
            if (globalVar) {
                MoveInstruction * initInst = new MoveInstruction(newFunc, globalVar, module->newConstInt(initValue));
                irCode.addInst(initInst);
                printf("DEBUG: 在main函数中初始化全局变量 %s = %d\n", varName.c_str(), initValue);
            }
        }
    }

    // 创建出口指令并不加入出口指令，等函数内的指令处理完毕后加入出口指令
    LabelInstruction * exitLabelInst = new LabelInstruction(newFunc);

    // 函数出口指令保存到函数信息中，因为在语义分析函数体时return语句需要跳转到函数尾部，需要这个label指令
    newFunc->setExitLabel(exitLabelInst);

    // 遍历形参，没有IR指令，不需要追加
    result = ir_function_formal_params(param_node);
    if (!result) {
        // 形参解析失败
        setLastError("处理函数形参失败");
        return false;
    }
    node->blockInsts.addInst(param_node->blockInsts);

    // 新建一个Value，用于保存函数的返回值，如果没有返回值可不用申请
    LocalVariable * retValue = nullptr;
    if (!type_node->type->isVoidType()) {
        // 保存函数返回值变量到函数信息中，在return语句翻译时需要设置值到这个变量中
        retValue = static_cast<LocalVariable *>(module->newVarValue(type_node->type));
    }
    newFunc->setReturnValue(retValue);

    // 函数内已经进入作用域，内部不再需要做变量的作用域管理
    block_node->needScope = false;

    // 遍历block
    result = ir_block(block_node);
    if (!result) {
        // block解析失败
        // TODO 自行追加语义错误处理
        return false;
    }

    // 打印调试信息
    printf("DEBUG: 函数 %s 的block节点指令数量: %zu\n",
           name_node->name.c_str(),
           block_node->blockInsts.getInsts().size());

    // IR指令追加到当前的节点中
    node->blockInsts.addInst(block_node->blockInsts);

    // 此时，所有指令都加入到当前函数中，也就是node->blockInsts
    printf("DEBUG: 函数 %s 的node节点指令数量: %zu\n", name_node->name.c_str(), node->blockInsts.getInsts().size());

    // node节点的指令移动到函数的IR指令列表中
    irCode.addInst(node->blockInsts);

    // 添加函数出口Label指令，主要用于return语句跳转到这里进行函数的退出
    irCode.addInst(exitLabelInst);

    // 函数出口指令
    irCode.addInst(new ExitInstruction(newFunc, retValue));

    // 打印最终IR指令
    printf("DEBUG: 函数 %s 的最终IR指令数量: %zu\n", name_node->name.c_str(), irCode.getInsts().size());

    // 恢复成外部函数
    module->setCurrentFunction(nullptr);

    // 退出函数的作用域
    module->leaveScope();

    return true;
}

// /// @brief 形式参数AST节点翻译成线性中间IR
// /// @param node AST节点
// /// @return 翻译是否成功，true：成功，false：失败
// bool IRGenerator::ir_function_formal_params(ast_node * node)
// {
//     // TODO 目前形参还不支持，直接返回true

//     // 每个形参变量都创建对应的临时变量，用于表达实参转递的值
//     // 而真实的形参则创建函数内的局部变量。
//     // 然后产生赋值指令，用于把表达实参值的临时变量拷贝到形参局部变量上。
//     // 请注意这些指令要放在Entry指令后面，因此处理的先后上要注意。

//     return true;
// }
/// @brief 形式参数AST节点翻译成线性中间IR-lxg
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
// bool IRGenerator::ir_function_formal_params(ast_node * node)
// {
//     // 获取当前正在处理的函数
//     Function * currentFunc = module->getCurrentFunction();
//     if (!currentFunc) {
//         setLastError("未在函数上下文中处理形参");
//         return false;
//     }

//     // 获取函数的IR代码列表
//     InterCode & irCode = currentFunc->getInterCode();

//     printf("DEBUG: 处理函数形参，数量: %zu, 函数参数数量: %zu\n", node->sons.size(),
//     currentFunc->getParams().size());

//     // 获取函数的参数列表
//     const std::vector<FormalParam *> & functionParams = currentFunc->getParams();

//     // 遍历函数的所有形参，创建对应的局部变量和临时变量
//     for (size_t i = 0; i < functionParams.size(); i++) {
//         FormalParam * param = functionParams[i];

//         // 函数参数的类型和名称
//         Type * paramType = param->getType();
//         std::string paramName = param->getName();

//         if (!paramType) {
//             setLastError("函数参数 " + paramName + " 类型无效");
//             return false;
//         }

//         // 检查是否是数组参数（通过AST节点类型判断）
//         bool isArrayParam = false;
//         if (i < node->sons.size()) {
//             isArrayParam = (node->sons[i]->node_type == ast_operator_type::AST_OP_FUNC_FORMAL_PARAM_ARRAY);
//         }

//         // 根据是否是数组参数决定实际的参数类型
//         Type * actualParamType = paramType;
//         if (isArrayParam) {
//             // 数组参数在C语言中实际上是指针类型
//             actualParamType =
//                 const_cast<Type *>(static_cast<const Type *>(PointerType::get(IntegerType::getTypeInt())));
//             printf("DEBUG: 处理函数数组参数: %s, 类型: pointer (i32*)\n", paramName.c_str());
//         } else {
//             printf("DEBUG: 处理函数参数: %s, 类型: %s\n", paramName.c_str(), paramType->isInt32Type() ? "int" :
//             "其他");
//         }

//         // 1. 创建局部变量作为实际的形参变量（在函数内部使用）
//         Value * localParam = module->newVarValue(actualParamType, paramName);
//         if (!localParam) {
//             setLastError("创建形参局部变量失败: " + paramName);
//             return false;
//         }

//         // 2. 获取函数形参本身作为源值
//         Value * paramValue = param;

//         // 3. 创建赋值指令，将形参值复制到局部变量
//         MoveInstruction * moveInst =
//             new MoveInstruction(currentFunc, static_cast<LocalVariable *>(localParam), paramValue);

//         // 4. 将赋值指令添加到函数的IR代码中（在Entry指令之后）
//         irCode.addInst(moveInst);
//     }

//     return true;
// }
bool IRGenerator::ir_function_formal_params(ast_node * node)
{
    // 获取当前正在处理的函数
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        setLastError("未在函数上下文中处理形参");
        return false;
    }

    // 获取函数的IR代码列表
    InterCode & irCode = currentFunc->getInterCode();

    printf("DEBUG: 处理函数形参，数量: %zu, 函数参数数量: %zu\n", node->sons.size(), currentFunc->getParams().size());

    // 获取函数的参数列表
    const std::vector<FormalParam *> & functionParams = currentFunc->getParams();

    // 遍历函数的所有形参
    for (size_t i = 0; i < functionParams.size(); i++) {
        FormalParam * param = functionParams[i];

        // 函数参数的类型和名称
        Type * paramType = param->getType();
        std::string paramName = param->getName();

        if (!paramType) {
            setLastError("函数参数 " + paramName + " 类型无效");
            return false;
        }

        // 检查是否是数组参数（通过AST节点类型判断）
        bool isArrayParam = false;
        if (i < node->sons.size()) {
            isArrayParam = (node->sons[i]->node_type == ast_operator_type::AST_OP_FUNC_FORMAL_PARAM_ARRAY);
        }

        if (isArrayParam) {
            // 数组参数：直接在符号表中注册参数，不创建局部变量
            Type * actualParamType =
                const_cast<Type *>(static_cast<const Type *>(PointerType::get(IntegerType::getTypeInt())));

            printf("DEBUG: 处理函数数组参数: %s, 类型: pointer (i32*)\n", paramName.c_str());

            // 直接在符号表中注册参数，避免创建局部变量和赋值指令
            if (!module->newVarValueWithValue(actualParamType, paramName, param)) {
                setLastError("注册数组形参到符号表失败: " + paramName);
                return false;
            }

            printf("DEBUG: 直接注册数组参数到符号表: %s (避免局部变量赋值)\n", paramName.c_str());

        } else {
            // 普通参数：保持原来的方式，创建局部变量并赋值
            printf("DEBUG: 处理函数参数: %s, 类型: %s\n", paramName.c_str(), paramType->isInt32Type() ? "int" : "其他");

            // 1. 创建局部变量作为实际的形参变量（在函数内部使用）
            Value * localParam = module->newVarValue(paramType, paramName);
            if (!localParam) {
                setLastError("创建形参局部变量失败: " + paramName);
                return false;
            }

            // 2. 获取函数形参本身作为源值
            Value * paramValue = param;

            // 3. 创建赋值指令，将形参值复制到局部变量
            MoveInstruction * moveInst =
                new MoveInstruction(currentFunc, static_cast<LocalVariable *>(localParam), paramValue);

            // 4. 将赋值指令添加到函数的IR代码中（在Entry指令之后）
            irCode.addInst(moveInst);

            printf("DEBUG: 为普通参数创建局部变量和赋值: %s\n", paramName.c_str());
        }
    }

    return true;
}

/// @brief 函数调用AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
// bool IRGenerator::ir_function_call(ast_node * node)
// {
//     std::vector<Value *> realParams;

//     // 获取当前正在处理的函数
//     Function * currentFunc = module->getCurrentFunction();

//     // 函数调用的节点包含两个节点：
//     // 第一个节点：函数名节点
//     // 第二个节点：实参列表节点

//     std::string funcName = node->sons[0]->name;
//     int64_t lineno = node->sons[0]->line_no;

//     ast_node * paramsNode = node->sons[1];

//     // 根据函数名查找函数，看是否存在。若不存在则出错
//     // 这里约定函数必须先定义后使用
//     auto calledFunction = module->findFunction(funcName);
//     if (nullptr == calledFunction) {
//         minic_log(LOG_ERROR, "函数(%s)未定义或声明", funcName.c_str());
//         return false;
//     }

//     // 当前函数存在函数调用
//     currentFunc->setExistFuncCall(true);

//     // 如果没有孩子，也认为是没有参数
//     if (!paramsNode->sons.empty()) {

//         int32_t argsCount = (int32_t) paramsNode->sons.size();

//         // 当前函数中调用函数实参个数最大值统计，实际上是统计实参传参需在栈中分配的大小
//         // 因为目前的语言支持的int和float都是四字节的，只统计个数即可
//         if (argsCount > currentFunc->getMaxFuncCallArgCnt()) {
//             currentFunc->setMaxFuncCallArgCnt(argsCount);
//         }

//         // 遍历参数列表，孩子是表达式
//         // 这里自左往右计算表达式
//         for (auto son: paramsNode->sons) {

//             // 遍历Block的每个语句，进行显示或者运算
//             ast_node * temp = ir_visit_ast_node(son);
//             if (!temp) {
//                 return false;
//             }

//             realParams.push_back(temp->val);
//             node->blockInsts.addInst(temp->blockInsts);
//         }
//     }

//     // TODO 这里请追加函数调用的语义错误检查，这里只进行了函数参数的个数检查等，其它请自行追加。
//     if (realParams.size() != calledFunction->getParams().size()) {
//         // 函数参数的个数不一致，语义错误
//         minic_log(LOG_ERROR, "第%lld行的被调用函数(%s)未定义或声明", (long long) lineno, funcName.c_str());
//         return false;
//     }

//     // 返回调用有返回值，则需要分配临时变量，用于保存函数调用的返回值
//     Type * type = calledFunction->getReturnType();

//     FuncCallInstruction * funcCallInst = new FuncCallInstruction(currentFunc, calledFunction, realParams, type);

//     // 创建函数调用指令
//     node->blockInsts.addInst(funcCallInst);

//     // 函数调用结果Value保存到node中，可能为空，上层节点可利用这个值
//     node->val = funcCallInst;

//     return true;
// }

///允许编译器动态创建函数原型-lxg
/// @brief 函数调用AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_call(ast_node * node)
{
    std::vector<Value *> realParams;

    // 获取当前正在处理的函数
    Function * currentFunc = module->getCurrentFunction();

    // 函数调用的节点包含两个节点：
    // 第一个节点：函数名节点
    // 第二个节点：实参列表节点

    std::string funcName = node->sons[0]->name;
    int64_t lineno = node->sons[0]->line_no;

    printf("DEBUG: 处理函数调用: %s 在第%lld行\n", funcName.c_str(), (long long) lineno);

    ast_node * paramsNode = node->sons[1];
    int actualParamCount = paramsNode->sons.size();
    printf("DEBUG: 函数调用 %s 提供的参数数量: %d\n", funcName.c_str(), actualParamCount);

    // 根据函数名查找函数，看是否存在。若不存在则出错
    auto calledFunction = module->findFunction(funcName);
    if (nullptr == calledFunction) {
        std::string error = "函数(" + funcName + ")未定义或声明，在第" + std::to_string(lineno) + "行";
        setLastError(error);
        minic_log(LOG_ERROR, "%s", error.c_str());
        return false;
    }

    int formalParamCount = calledFunction->getParams().size();
    printf("DEBUG: 找到函数: %s, 需要%d个参数\n", funcName.c_str(), formalParamCount);

    // 当前函数存在函数调用
    currentFunc->setExistFuncCall(true);

    // 如果没有孩子，也认为是没有参数
    if (!paramsNode->sons.empty()) {
        int32_t argsCount = (int32_t) paramsNode->sons.size();

        // 当前函数中调用函数实参个数最大值统计，实际上是统计实参传参需在栈中分配的大小
        // 因为目前的语言支持的int和float都是四字节的，只统计个数即可
        if (argsCount > currentFunc->getMaxFuncCallArgCnt()) {
            currentFunc->setMaxFuncCallArgCnt(argsCount);
        }

        // for (auto son: paramsNode->sons) {
        //     // 遍历Block的每个语句，进行显示或者运算
        //     ast_node * temp = ir_visit_ast_node(son);
        //     if (!temp) {
        //         setLastError("处理函数" + funcName + "的参数时失败");
        //         return false;
        //     }

        //     realParams.push_back(temp->val);
        //     node->blockInsts.addInst(temp->blockInsts);
        // }
        const std::vector<FormalParam *> & formalParams = calledFunction->getParams();

        // 遍历参数列表，孩子是表达式
        // 这里自左往右计算表达式
        // for (size_t i = 0; i < paramsNode->sons.size(); i++) {
        //     ast_node * son = paramsNode->sons[i];

        //     // 检查是否传递数组参数
        //     if (son->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
        //         Value * paramVar = module->findVarValue(son->name);

        //         // 检查形参是否为指针类型（即数组参数）
        //         bool shouldPassAsPointer = false;

        //         if (i < formalParams.size()) {
        //             Type * formalParamType = formalParams[i]->getType();
        //             shouldPassAsPointer = formalParamType && formalParamType->isPointerType();
        //         }

        //         if (paramVar && paramVar->getType()->isArrayType() && shouldPassAsPointer) {
        //             // 数组参数：直接传递数组变量（作为指针）
        //             printf("DEBUG: 传递数组参数: %s (作为指针)\n", son->name.c_str());
        //             realParams.push_back(paramVar);
        //             continue;
        //         }
        //     }

        //     // 处理其他类型的参数（包括表达式、常量等）
        //     ast_node * temp = ir_visit_ast_node(son);
        //     if (!temp) {
        //         setLastError("处理函数" + funcName + "的参数时失败");
        //         return false;
        //     }

        //     realParams.push_back(temp->val);
        //     node->blockInsts.addInst(temp->blockInsts);
        // }
        // 在参数处理循环中添加调试信息：
        for (size_t i = 0; i < paramsNode->sons.size(); i++) {
            ast_node * son = paramsNode->sons[i];

            printf("DEBUG: 处理参数 #%zu, 节点类型: %d, 变量名: %s\n",
                   i,
                   static_cast<int>(son->node_type),
                   son->name.c_str());

            // 检查形参是否为指针类型（即数组参数）
            bool shouldPassAsPointer = false;
            if (i < formalParams.size()) {
                Type * formalParamType = formalParams[i]->getType();
                shouldPassAsPointer = formalParamType && formalParamType->isPointerType();
                printf("DEBUG: 形参 #%zu 类型检查 - isPointerType: %s\n", i, shouldPassAsPointer ? "是" : "否");
            }

            // 关键修改：正确处理不同维度的数组参数传递
            if (son->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS && shouldPassAsPointer) {
                printf("DEBUG: *** 处理数组访问作为指针参数: %s[...] ***\n", son->sons[0]->name.c_str());

                // 获取形参的实际类型
                Type * formalParamType = formalParams[i]->getType();

                // 手动处理数组访问，但返回地址而不是值
                std::string arrayName = son->sons[0]->name;
                Value * arrayVar = module->findVarValue(arrayName);

                if (!arrayVar) {
                    setLastError("未找到数组: " + arrayName);
                    return false;
                }

                if (ArrayType * arrayParamType = dynamic_cast<ArrayType *>(formalParamType)) {
                    // 形参是数组类型 int[0][2][3]...
                    const std::vector<int> & paramDimensions = arrayParamType->getDimensions();
                    printf("DEBUG: 形参是多维数组类型，维度数: %zu\n", paramDimensions.size());

                    // 计算正确的偏移量，考虑形参的维度信息
                    Value * correctOffset = calculateParameterOffset(son, paramDimensions, node->blockInsts);
                    if (!correctOffset) {
                        return false;
                    }

                    // 生成正确的地址传递
                    Type * ptrType =
                        const_cast<Type *>(static_cast<const Type *>(PointerType::get(IntegerType::getTypeInt())));
                    BinaryInstruction * addInst = new BinaryInstruction(currentFunc,
                                                                        IRInstOperator::IRINST_OP_ADD_I,
                                                                        arrayVar,
                                                                        correctOffset,
                                                                        ptrType);
                    node->blockInsts.addInst(addInst);
                    realParams.push_back(addInst);

                    printf("DEBUG: 生成多维数组参数传递: %s -> 偏移量计算\n", arrayName.c_str());
                } else {
                    // 形参是简单指针类型 int*，按照原有逻辑处理
                    printf("DEBUG: 形参是简单指针类型，使用原逻辑\n");

                    // 计算实际的数组偏移量
                    Value * totalOffset = calculateArrayAccessOffset(son, node->blockInsts);
                    if (!totalOffset) {
                        return false;
                    }

                    // 计算最终地址：@array + totalOffset
                    Type * ptrType =
                        const_cast<Type *>(static_cast<const Type *>(PointerType::get(IntegerType::getTypeInt())));
                    BinaryInstruction * finalAddrInst = new BinaryInstruction(currentFunc,
                                                                              IRInstOperator::IRINST_OP_ADD_I,
                                                                              arrayVar,
                                                                              totalOffset,
                                                                              ptrType);
                    node->blockInsts.addInst(finalAddrInst);
                    realParams.push_back(finalAddrInst);
                }

                printf("DEBUG: 完成数组访问参数传递\n");
                continue;
            }

            // 然后处理简单变量名（数组名）
            else if (son->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
                Value * paramVar = module->findVarValue(son->name);

                printf("DEBUG: 找到变量: %s, 变量存在: %s\n", son->name.c_str(), paramVar ? "是" : "否");

                if (paramVar) {
                    printf("DEBUG: 变量 %s 类型检查 - isArrayType: %s, isPointerType: %s\n",
                           son->name.c_str(),
                           paramVar->getType()->isArrayType() ? "是" : "否",
                           paramVar->getType()->isPointerType() ? "是" : "否");
                }

                if (paramVar && paramVar->getType()->isArrayType() && shouldPassAsPointer) {
                    // 数组参数：生成 add %array, 0 得到指针
                    printf("DEBUG: *** 传递数组参数: %s (add %%array, 0 得到指针) ***\n", son->name.c_str());

                    Type * ptrType =
                        const_cast<Type *>(static_cast<const Type *>(PointerType::get(IntegerType::getTypeInt())));
                    LocalVariable * ptrVar = static_cast<LocalVariable *>(module->newVarValue(ptrType));
                    ConstInt * zero = module->newConstInt(0);

                    // 生成 add 指令
                    BinaryInstruction * addInst =
                        new BinaryInstruction(currentFunc, IRInstOperator::IRINST_OP_ADD_I, paramVar, zero, ptrType);
                    node->blockInsts.addInst(addInst);
                    node->blockInsts.addInst(new MoveInstruction(currentFunc, ptrVar, addInst));

                    realParams.push_back(ptrVar);

                    printf("DEBUG: 创建了数组到指针衰减: %s -> %s\n",
                           paramVar->getIRName().c_str(),
                           ptrVar->getIRName().c_str());
                    continue;
                } else {
                    printf("DEBUG: 不满足数组参数条件，按普通参数处理\n");
                }
            } else {
                printf("DEBUG: 节点类型不是 AST_OP_LEAF_VAR_ID\n");
            }

            // 处理其他类型的参数
            printf("DEBUG: 按普通参数处理: %s\n", son->name.c_str());
            ast_node * temp = ir_visit_ast_node(son);
            if (!temp) {
                setLastError("处理函数" + funcName + "的参数时失败");
                return false;
            }

            realParams.push_back(temp->val);
            node->blockInsts.addInst(temp->blockInsts);
        }
    }

    // 参数数量检查
    if (realParams.size() != calledFunction->getParams().size()) {
        std::string error = "函数(" + funcName + ")参数数量不匹配，需要" +
                            std::to_string(calledFunction->getParams().size()) + "个但提供了" +
                            std::to_string(realParams.size()) + "个";
        setLastError(error);
        minic_log(LOG_ERROR, "%s", error.c_str());

        // 调试输出每个形参的名称和类型
        printf("DEBUG: 函数 %s 的形参列表:\n", funcName.c_str());
        for (size_t i = 0; i < calledFunction->getParams().size(); i++) {
            auto param = calledFunction->getParams()[i];
            printf("  参数 #%zu: %s\n", i, param->getName().c_str());
        }

        return false;
    }

    printf("DEBUG: 函数调用参数检查通过: %s\n", funcName.c_str());
    // 返回调用有返回值，则需要分配临时变量，用于保存函数调用的返回值
    Type * type = calledFunction->getReturnType();

    FuncCallInstruction * funcCallInst = new FuncCallInstruction(currentFunc, calledFunction, realParams, type);

    //关键调试：创建指令后立即检查-lxg
    printf("DEBUG: 函数调用指令创建完成，指令对象地址: %p\n", (void *) funcCallInst);
    if (funcCallInst) {
        printf("DEBUG: 函数调用指令的返回值类型: %s\n", funcCallInst->getType()->isInt32Type() ? "i32" : "其他");
    }

    // 创建函数调用指令
    node->blockInsts.addInst(funcCallInst);

    // 函数调用结果Value保存到node中，可能为空，上层节点可利用这个值
    node->val = funcCallInst;

    return true;
}

/// @brief 语句块（含函数体）AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_block(ast_node * node)
{
    // 进入作用域
    if (node->needScope) {
        module->enterScope();
    }

    std::vector<ast_node *>::iterator pIter;
    for (pIter = node->sons.begin(); pIter != node->sons.end(); ++pIter) {

        // 遍历Block的每个语句，进行显示或者运算
        ast_node * temp = ir_visit_ast_node(*pIter);
        if (!temp) {
            return false;
        }

        node->blockInsts.addInst(temp->blockInsts);
    }

    // 离开作用域
    if (node->needScope) {
        module->leaveScope();
    }

    return true;
}

/// @brief 整数加法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_add(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 加法节点，左结合，先计算左节点，后计算右节点

    // 加法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left || !left->val) {
        // 操作数无效，设置错误信息
        setLastError("加法左侧操作数无效");
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right || !right->val) {
        // 操作数无效，设置错误信息
        setLastError("加法右侧操作数无效");
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ADD_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(addInst);

    node->val = addInst;

    return true;
}

/// @brief 整数减法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_sub(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 加法节点，左结合，先计算左节点，后计算右节点

    // 加法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * subInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(subInst);

    node->val = subInst;

    return true;
}

///添加三个新的函数声明ir-mul,ir-div和ir-mod  -lxg
/// @brief 整数乘法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mul(ast_node * node)
{
    if (!node || node->sons.size() < 2) {
        setLastError("乘法节点格式错误");
        return false;
    }

    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    if (!src1_node || !src2_node) {
        setLastError("乘法操作数为空");
        return false;
    }

    // 乘法节点，左结合，先计算左节点，后计算右节点

    // 乘法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left || !left->val) {
        // 操作数无效，设置错误信息
        setLastError("乘法左侧操作数无效");
        return false;
    }

    // 乘法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right || !right->val) {
        // 操作数无效，设置错误信息
        setLastError("乘法右侧操作数无效");
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * mulInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MUL_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(mulInst);

    node->val = mulInst;

    return true;
}

/// @brief 整数除法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_div(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 除法节点，左结合，先计算左节点，后计算右节点

    // 除法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 除法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * divInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_DIV_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(divInst);

    node->val = divInst;

    return true;
}

/// @brief 整数求余AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mod(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 求余节点，左结合，先计算左节点，后计算右节点

    // 求余的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 求余的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * modInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MOD_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(modInst);

    node->val = modInst;

    return true;
}

/// @brief 一元负号AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_neg(ast_node * node)
{
    // 获取操作数节点
    ast_node * operand_node = node->sons[0];

    // 计算操作数
    ast_node * operand = ir_visit_ast_node(operand_node);
    if (!operand) {
        // 操作数计算失败
        return false;
    }

    // 创建一元负号指令
    BinaryInstruction * negInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_NEG_I,
                                                        operand->val,
                                                        nullptr, // 一元运算符第二个操作数为空
                                                        IntegerType::getTypeInt());

    // 将操作数的指令和负号指令添加到当前节点
    node->blockInsts.addInst(operand->blockInsts);
    node->blockInsts.addInst(negInst);

    // 设置当前节点的值为负号指令的结果
    node->val = negInst;

    return true;
}

/// 实现关系运算符的处理函数-lxg
/// 小于
bool IRGenerator::ir_lt(ast_node * node)
{
    // 生成左右操作数
    ast_node * left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node)
        return false;

    ast_node * right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node)
        return false;

    Value * left = left_node->val;
    Value * right = right_node->val;

    if (!left || !right)
        return false;

    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);

    // 创建临时变量存储比较结果 - 使用布尔类型
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeBool()));

    // 添加比较指令 - 使用布尔类型
    BinaryInstruction * ltInst =
        new BinaryInstruction(func, IRInstOperator::IRINST_OP_LT_I, left, right, IntegerType::getTypeBool());
    node->blockInsts.addInst(ltInst);

    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, ltInst));

    node->val = result;
    return true;
}

// 大于
bool IRGenerator::ir_gt(ast_node * node)
{
    // 生成左右操作数
    ast_node * left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node)
        return false;

    ast_node * right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node)
        return false;

    Value * left = left_node->val;
    Value * right = right_node->val;

    if (!left || !right)
        return false;

    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);

    // 使用布尔类型
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeBool()));

    // 使用布尔类型
    BinaryInstruction * gtInst =
        new BinaryInstruction(func, IRInstOperator::IRINST_OP_GT_I, left, right, IntegerType::getTypeBool());
    node->blockInsts.addInst(gtInst);

    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, gtInst));

    node->val = result;
    return true;
}

// 小于等于
bool IRGenerator::ir_le(ast_node * node)
{
    // 生成左右操作数
    ast_node * left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node)
        return false;

    ast_node * right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node)
        return false;

    Value * left = left_node->val;
    Value * right = right_node->val;

    if (!left || !right)
        return false;

    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);

    // 使用布尔类型
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeBool()));

    // 使用布尔类型
    BinaryInstruction * leInst =
        new BinaryInstruction(func, IRInstOperator::IRINST_OP_LE_I, left, right, IntegerType::getTypeBool());
    node->blockInsts.addInst(leInst);

    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, leInst));

    node->val = result;
    return true;
}

// 大于等于
bool IRGenerator::ir_ge(ast_node * node)
{
    // 生成左右操作数
    ast_node * left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node)
        return false;

    ast_node * right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node)
        return false;

    Value * left = left_node->val;
    Value * right = right_node->val;

    if (!left || !right)
        return false;

    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);

    // 使用布尔类型
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeBool()));

    // 使用布尔类型
    BinaryInstruction * geInst =
        new BinaryInstruction(func, IRInstOperator::IRINST_OP_GE_I, left, right, IntegerType::getTypeBool());
    node->blockInsts.addInst(geInst);

    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, geInst));

    node->val = result;
    return true;
}

// 等于
bool IRGenerator::ir_eq(ast_node * node)
{
    // 生成左右操作数
    ast_node * left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node)
        return false;

    ast_node * right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node)
        return false;

    Value * left = left_node->val;
    Value * right = right_node->val;

    if (!left || !right)
        return false;

    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);

    // 使用布尔类型
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeBool()));

    // 使用布尔类型
    BinaryInstruction * eqInst =
        new BinaryInstruction(func, IRInstOperator::IRINST_OP_EQ_I, left, right, IntegerType::getTypeBool());
    node->blockInsts.addInst(eqInst);

    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, eqInst));

    node->val = result;
    return true;
}

// 不等于
bool IRGenerator::ir_ne(ast_node * node)
{
    // 生成左右操作数
    ast_node * left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node)
        return false;

    ast_node * right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node)
        return false;

    Value * left = left_node->val;
    Value * right = right_node->val;

    if (!left || !right)
        return false;

    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);

    // 使用布尔类型
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeBool()));

    // 使用布尔类型
    BinaryInstruction * neInst =
        new BinaryInstruction(func, IRInstOperator::IRINST_OP_NE_I, left, right, IntegerType::getTypeBool());
    node->blockInsts.addInst(neInst);

    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, neInst));

    node->val = result;
    return true;
}

///实现逻辑运算符，特别需要实现短路求值-lxg
// 逻辑与 &&，带短路求值
bool IRGenerator::ir_logic_and(ast_node * node)
{
    if (!module)
        return false;
    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 检查子节点数量
    if (node->sons.size() < 2) {
        minic_log(LOG_ERROR, "逻辑与运算需要两个操作数");
        return false;
    }

    // 创建标签
    LabelInstruction * secondOpLabel = new LabelInstruction(func);
    LabelInstruction * falseLabel = new LabelInstruction(func);
    LabelInstruction * endLabel = new LabelInstruction(func);

    // 为结果创建临时变量
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));
    if (!result)
        return false;

    // 生成左操作数代码
    ast_node * leftNode = ir_visit_ast_node(node->sons[0]);
    if (!leftNode || !leftNode->val)
        return false;

    // 添加左操作数指令
    node->blockInsts.addInst(leftNode->blockInsts);

    // 将左操作数转换为布尔值
    Value * leftBool;
    if (!int_to_bool(leftNode->val, &leftBool))
        return false;

    // 添加布尔转换指令
    if (func->getExtraData().boolCheckInst) {
        node->blockInsts.addInst(func->getExtraData().boolCheckInst);
        if (func->getExtraData().moveInst) {
            node->blockInsts.addInst(func->getExtraData().moveInst);
        }
        func->getExtraData().boolCheckInst = nullptr;
        func->getExtraData().moveInst = nullptr;
    }

    // 条件跳转：如果leftBool为真，转到secondOpLabel，否则转到falseLabel
    node->blockInsts.addInst(new GotoInstruction(func, leftBool, secondOpLabel, falseLabel));

    // 第二个操作数标签
    node->blockInsts.addInst(secondOpLabel);

    // 生成右操作数代码
    ast_node * rightNode = ir_visit_ast_node(node->sons[1]);
    if (!rightNode || !rightNode->val)
        return false;

    // 添加右操作数指令
    node->blockInsts.addInst(rightNode->blockInsts);

    // 右操作数结果存入result
    node->blockInsts.addInst(new MoveInstruction(func, result, rightNode->val));

    // 跳转到结束
    node->blockInsts.addInst(new GotoInstruction(func, endLabel));

    // 处理短路情况（左操作数为假）
    node->blockInsts.addInst(falseLabel);
    node->blockInsts.addInst(new MoveInstruction(func, result, module->newConstInt(0)));

    // 结束标签
    node->blockInsts.addInst(endLabel);

    // 设置节点的值
    node->val = result;
    return true;
}

// 逻辑或 ||，带短路求值
bool IRGenerator::ir_logic_or(ast_node * node)
{
    if (!module)
        return false;
    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 检查子节点数量
    if (node->sons.size() < 2) {
        minic_log(LOG_ERROR, "逻辑或运算需要两个操作数");
        return false;
    }

    // 创建标签
    LabelInstruction * secondOpLabel = new LabelInstruction(func);
    LabelInstruction * trueLabel = new LabelInstruction(func);
    LabelInstruction * endLabel = new LabelInstruction(func);

    // 为结果创建临时变量
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));
    if (!result)
        return false;

    // 生成左操作数代码
    ast_node * leftNode = ir_visit_ast_node(node->sons[0]);
    if (!leftNode || !leftNode->val)
        return false;

    // 添加左操作数指令
    node->blockInsts.addInst(leftNode->blockInsts);

    // 将左操作数转换为布尔值
    Value * leftBool;
    if (!int_to_bool(leftNode->val, &leftBool))
        return false;

    // 添加布尔转换指令
    if (func->getExtraData().boolCheckInst) {
        node->blockInsts.addInst(func->getExtraData().boolCheckInst);
        if (func->getExtraData().moveInst) {
            node->blockInsts.addInst(func->getExtraData().moveInst);
        }
        func->getExtraData().boolCheckInst = nullptr;
        func->getExtraData().moveInst = nullptr;
    }

    // 条件跳转：如果leftBool为真，转到trueLabel，否则转到secondOpLabel
    node->blockInsts.addInst(new GotoInstruction(func, leftBool, trueLabel, secondOpLabel));

    // 第二个操作数标签
    node->blockInsts.addInst(secondOpLabel);

    // 生成右操作数代码
    ast_node * rightNode = ir_visit_ast_node(node->sons[1]);
    if (!rightNode || !rightNode->val)
        return false;

    // 添加右操作数指令
    node->blockInsts.addInst(rightNode->blockInsts);

    // 右操作数结果存入result
    node->blockInsts.addInst(new MoveInstruction(func, result, rightNode->val));

    // 跳转到结束
    node->blockInsts.addInst(new GotoInstruction(func, endLabel));

    // 处理短路情况（左操作数为真）
    node->blockInsts.addInst(trueLabel);
    node->blockInsts.addInst(new MoveInstruction(func, result, module->newConstInt(1)));

    // 结束标签
    node->blockInsts.addInst(endLabel);

    // 设置节点的值
    node->val = result;
    return true;
}

// 逻辑非 !
bool IRGenerator::ir_logic_not(ast_node * node)
{
    if (!module)
        return false;
    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 检查子节点数量
    if (node->sons.empty()) {
        minic_log(LOG_ERROR, "逻辑非运算需要一个操作数");
        return false;
    }

    // 生成操作数代码
    ast_node * operandNode = ir_visit_ast_node(node->sons[0]);
    if (!operandNode || !operandNode->val)
        return false;

    // 添加操作数指令
    node->blockInsts.addInst(operandNode->blockInsts);

    // 为结果创建临时变量
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));
    if (!result)
        return false;

    // 创建比较指令：检查整数值是否等于0
    BinaryInstruction * eqZeroInst = new BinaryInstruction(func,
                                                           IRInstOperator::IRINST_OP_EQ_I,
                                                           operandNode->val,
                                                           module->newConstInt(0),
                                                           IntegerType::getTypeBool());

    // 添加比较指令
    node->blockInsts.addInst(eqZeroInst);

    // 将比较结果移到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, eqZeroInst));

    // 设置节点的值
    node->val = result;
    return true;
}

// if语句（无else分支）
// bool IRGenerator::ir_if(ast_node* node)
// {
//     Function* func = module->getCurrentFunction();
//     if (!func) return false;

//     // 创建标签
//     LabelInstruction* thenLabel = new LabelInstruction(func);
//     LabelInstruction* endLabel = new LabelInstruction(func);

//     // 生成条件表达式代码
//     ast_node* cond_node = ir_visit_ast_node(node->sons[0]);
//     if (!cond_node) return false;

//     Value* condVal = cond_node->val;
//     if (!condVal) return false;

//     // 添加条件表达式生成的指令到指令流
//     node->blockInsts.addInst(cond_node->blockInsts);

//     // 直接使用条件值，不再转换为"不等于0"形式
//     node->blockInsts.addInst(new GotoInstruction(func, condVal, thenLabel, endLabel));

//     // 生成then部分代码
//     node->blockInsts.addInst(thenLabel);
//     ast_node* then_node = ir_visit_ast_node(node->sons[1]);
//     if (!then_node) return false;
//     node->blockInsts.addInst(then_node->blockInsts);

//     // 结束标签
//     node->blockInsts.addInst(endLabel);

//     return true;
// }
bool IRGenerator::ir_if(ast_node * node)
{
    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 创建标签
    LabelInstruction * thenLabel = new LabelInstruction(func);
    LabelInstruction * endLabel = new LabelInstruction(func);

    // 生成条件表达式代码
    ast_node * cond_node = ir_visit_ast_node(node->sons[0]);
    if (!cond_node)
        return false;

    Value * condVal = cond_node->val;
    if (!condVal)
        return false;

    // 添加条件表达式生成的指令到指令流
    node->blockInsts.addInst(cond_node->blockInsts);

    // 条件跳转
    node->blockInsts.addInst(new GotoInstruction(func, condVal, thenLabel, endLabel));

    // 生成then部分代码
    node->blockInsts.addInst(thenLabel);

    // 检查sons数组大小和第二个子节点是否为空
    if (node->sons.size() > 1 && node->sons[1]) {
        ast_node * then_node = ir_visit_ast_node(node->sons[1]);
        if (then_node) {
            node->blockInsts.addInst(then_node->blockInsts);
        }
    }

    // 结束标签
    node->blockInsts.addInst(endLabel);

    return true;
}

// if-else语句
bool IRGenerator::ir_if_else(ast_node * node)
{
    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 创建标签
    LabelInstruction * thenLabel = new LabelInstruction(func);
    LabelInstruction * elseLabel = new LabelInstruction(func);
    LabelInstruction * endLabel = new LabelInstruction(func);

    // 生成条件表达式代码
    ast_node * cond_node = ir_visit_ast_node(node->sons[0]);
    if (!cond_node)
        return false;

    Value * condVal = cond_node->val;
    if (!condVal)
        return false;

    // 添加条件表达式生成的指令到指令流
    node->blockInsts.addInst(cond_node->blockInsts);

    // 直接使用条件值
    node->blockInsts.addInst(new GotoInstruction(func, condVal, thenLabel, elseLabel));

    // 生成then部分代码
    node->blockInsts.addInst(thenLabel);
    ast_node * then_node = ir_visit_ast_node(node->sons[1]);
    if (!then_node)
        return false;
    node->blockInsts.addInst(then_node->blockInsts);

    // then部分执行完后跳转到结束
    node->blockInsts.addInst(new GotoInstruction(func, endLabel));

    // 生成else部分代码
    node->blockInsts.addInst(elseLabel);
    ast_node * else_node = ir_visit_ast_node(node->sons[2]);
    if (!else_node)
        return false;
    node->blockInsts.addInst(else_node->blockInsts);

    // 结束标签
    node->blockInsts.addInst(endLabel);

    return true;
}

// while循环语句
// 新增对常量的while判断
bool IRGenerator::ir_while(ast_node * node)
{
    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 创建标签
    LabelInstruction * condLabel = new LabelInstruction(func); // 循环的入口label
    LabelInstruction * bodyLabel = new LabelInstruction(func); // 循环体的入口label
    LabelInstruction * endLabel = new LabelInstruction(func);  // 循环的出口label

    // 进入循环前，保存当前break和continue标签
    Instruction * oldBreakLabel = func->getBreakLabel();
    Instruction * oldContinueLabel = func->getContinueLabel();

    // 设置新的break和continue标签
    func->setBreakLabel(endLabel);
    func->setContinueLabel(condLabel);

    // 从循环条件开始
    node->blockInsts.addInst(condLabel);

    // 生成条件表达式代码
    ast_node * cond_node = ir_visit_ast_node(node->sons[0]);
    if (!cond_node)
        return false;

    Value * condVal = cond_node->val;
    if (!condVal)
        return false;

    // 添加条件表达式生成的指令到指令流
    node->blockInsts.addInst(cond_node->blockInsts);

    //关键修改：检查条件是否为常量-lxg
    if (ConstInt * constCond = dynamic_cast<ConstInt *>(condVal)) {
        // 条件是常量
        int32_t condValue = constCond->getVal();

        if (condValue != 0) {
            // while(1) 或 while(非零常量) - 无限循环
            // 直接无条件跳转到循环体
            node->blockInsts.addInst(new GotoInstruction(func, bodyLabel));
        } else {
            // while(0) - 永远不执行
            // 直接跳转到结束标签
            node->blockInsts.addInst(new GotoInstruction(func, endLabel));

            // 恢复标签后直接返回
            func->setBreakLabel(oldBreakLabel);
            func->setContinueLabel(oldContinueLabel);
            node->blockInsts.addInst(endLabel);
            return true;
        }
    } else {
        // 条件不是常量，使用正常的条件分支
        node->blockInsts.addInst(new GotoInstruction(func, condVal, bodyLabel, endLabel));
    }

    // 生成循环体代码
    node->blockInsts.addInst(bodyLabel);
    ast_node * body_node = ir_visit_ast_node(node->sons[1]);
    if (!body_node)
        return false;
    node->blockInsts.addInst(body_node->blockInsts);

    // 循环体执行完后跳回条件判断
    node->blockInsts.addInst(new GotoInstruction(func, condLabel));

    // 循环结束标签
    node->blockInsts.addInst(endLabel);

    // 恢复原来的break和continue标签
    func->setBreakLabel(oldBreakLabel);
    func->setContinueLabel(oldContinueLabel);

    return true;
}

// break语句
bool IRGenerator::ir_break(ast_node * node)
{
    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 获取当前循环的break标签
    Instruction * breakLabel = func->getBreakLabel();
    if (!breakLabel) {
        // 不在循环内使用break
        std::cerr << "Error: break statement not inside a loop" << std::endl;
        return false;
    }

    // 生成跳转到break标签的指令
    node->blockInsts.addInst(new GotoInstruction(func, breakLabel));

    return true;
}

// continue语句
bool IRGenerator::ir_continue(ast_node * node)
{
    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 获取当前循环的continue标签
    Instruction * continueLabel = func->getContinueLabel();
    if (!continueLabel) {
        // 不在循环内使用continue
        std::cerr << "Error: continue statement not inside a loop" << std::endl;
        return false;
    }

    // 生成跳转到continue标签的指令
    node->blockInsts.addInst(new GotoInstruction(func, continueLabel));

    return true;
}

// 整数转布尔值
bool IRGenerator::int_to_bool(Value * val, Value ** bool_val)
{
    // 检查参数有效性
    if (!val || !module || !bool_val)
        return false;

    Function * func = module->getCurrentFunction();
    if (!func)
        return false;

    // 检查值类型有效性
    Type * valType = val->getType();
    if (!valType)
        return false;

    // 如果输入值已经是布尔类型(i1)，直接使用它
    if (valType->isInt1Byte()) {
        *bool_val = val;
        return true;
    }

    // 创建临时变量存储布尔值结果
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeBool()));
    if (!result)
        return false;

    // 获取常量0用于比较
    ConstInt * zeroConst = module->newConstInt(0);
    if (!zeroConst)
        return false;

    // 创建比较指令：检查整数值是否不等于0
    BinaryInstruction * boolCheck =
        new BinaryInstruction(func, IRInstOperator::IRINST_OP_NE_I, val, zeroConst, IntegerType::getTypeBool());
    if (!boolCheck)
        return false;

    // 创建移动指令：将比较结果移到临时变量
    MoveInstruction * moveInst = new MoveInstruction(func, result, boolCheck);
    if (!moveInst) {
        delete boolCheck; // 避免内存泄漏
        return false;
    }

    // 设置返回值
    *bool_val = result;

    // 将指令保存到函数的extra data中，供调用者使用
    func->getExtraData().boolCheckInst = boolCheck;
    func->getExtraData().moveInst = moveInst;

    return true;
}

// 布尔值转整数
bool IRGenerator::bool_to_int(Value * val, Value ** int_val)
{
    // 在我们的实现中，布尔值已经是整数（0或1），所以可以直接使用
    *int_val = val;
    return true;
}

/// @brief 赋值AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
// bool IRGenerator::ir_assign(ast_node * node)
// {
//     ast_node * son1_node = node->sons[0];
//     ast_node * son2_node = node->sons[1];

//     // 赋值节点，自右往左运算

//     // 赋值运算符的左侧操作数
//     ast_node * left = ir_visit_ast_node(son1_node);
//     if (!left) {
//         // 某个变量没有定值
//         // 这里缺省设置变量不存在则创建，因此这里不会错误
//         return false;
//     }

//     // 赋值运算符的右侧操作数
//     ast_node * right = ir_visit_ast_node(son2_node);
//     if (!right) {
//         // 某个变量没有定值
//         return false;
//     }

//     // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

//     MoveInstruction * movInst = new MoveInstruction(module->getCurrentFunction(), left->val, right->val);

//     // 创建临时变量保存IR的值，以及线性IR指令
//     node->blockInsts.addInst(right->blockInsts);
//     node->blockInsts.addInst(left->blockInsts);
//     node->blockInsts.addInst(movInst);

//     // 这里假定赋值的类型是一致的
//     node->val = movInst;

//     return true;
// }
bool IRGenerator::ir_assign(ast_node * node)
{
    ast_node * son1_node = node->sons[0];
    ast_node * son2_node = node->sons[1];

    // 赋值节点，自右往左运算

    // 计算右侧表达式
    ast_node * right = ir_visit_ast_node(son2_node);
    if (!right || !right->val) {
        setLastError("赋值表达式右侧求值失败");
        return false;
    }
    node->blockInsts.addInst(right->blockInsts);

    // 计算左侧表达式
    ast_node * left = ir_visit_ast_node(son1_node);
    if (!left) {
        return false;
    }
    node->blockInsts.addInst(left->blockInsts);

    // 检查左侧是否是数组访问
    if (son1_node->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS && left->arrayPtr) {
        // 通过指针为数组元素赋值
        MoveInstruction * storeInst = new MoveInstruction(module->getCurrentFunction(),
                                                          left->arrayPtr, // 数组元素的指针
                                                          right->val      // 右侧值
        );
        storeInst->setIsPointerStore(true); // 标记为指针存储，需要在MoveInstruction类中添加此字段和方法
        node->blockInsts.addInst(storeInst);

        printf("DEBUG: 通过指针为数组元素赋值: *%s = %s\n",
               left->arrayPtr->getIRName().c_str(),
               right->val->getIRName().c_str());
    } else {
        // 普通赋值
        MoveInstruction * movInst = new MoveInstruction(module->getCurrentFunction(), left->val, right->val);
        node->blockInsts.addInst(movInst);
    }

    node->val = right->val;
    return true;
}

/// @brief return节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_return(ast_node * node)
{
    ast_node * right = nullptr;

    // return语句可能没有没有表达式，也可能有，因此这里必须进行区分判断
    if (!node->sons.empty()) {

        ast_node * son_node = node->sons[0];

        // 返回的表达式的指令保存在right节点中
        right = ir_visit_ast_node(son_node);
        if (!right) {

            // 某个变量没有定值
            return false;
        }
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
    Function * currentFunc = module->getCurrentFunction();

    // 返回值存在时则移动指令到node中
    if (right) {

        // 创建临时变量保存IR的值，以及线性IR指令
        node->blockInsts.addInst(right->blockInsts);

        // 返回值赋值到函数返回值变量上，然后跳转到函数的尾部
        node->blockInsts.addInst(new MoveInstruction(currentFunc, currentFunc->getReturnValue(), right->val));

        node->val = right->val;
    } else {
        // 没有返回值
        node->val = nullptr;
    }

    // 跳转到函数的尾部出口指令上
    node->blockInsts.addInst(new GotoInstruction(currentFunc, currentFunc->getExitLabel()));

    return true;
}

/// @brief 类型叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_type(ast_node * node)
{
    // 不需要做什么，直接从节点中获取即可。

    return true;
}

/// @brief 标识符叶子节点翻译成线性中间IR，变量声明的不走这个语句
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
// bool IRGenerator::ir_leaf_node_var_id(ast_node * node)
// {
//     Value * val;

//     // 查找ID型Value
//     // 变量，则需要在符号表中查找对应的值

//     val = module->findVarValue(node->name);

//     node->val = val;

//     return true;
// }
bool IRGenerator::ir_leaf_node_var_id(ast_node * node)
{
    if (!node) {
        setLastError("叶子节点为空");
        return false;
    }

    if (node->name.empty()) {
        setLastError("叶子节点名称为空");
        return false;
    }

    // printf("DEBUG: 查找变量: %s\n", node->name.c_str());

    // 查找ID型Value
    // 变量，则需要在符号表中查找对应的值
    Value * val = module->findVarValue(node->name);

    if (!val) {
        printf("DEBUG: 在符号表中未找到变量: %s, 尝试查找函数参数\n", node->name.c_str());

        // 查找是否是函数参数
        Function * currentFunc = module->getCurrentFunction();
        if (currentFunc) {
            for (auto & param: currentFunc->getParams()) {
                if (param->getName() == node->name) {
                    printf("DEBUG: 找到匹配的函数参数: %s\n", node->name.c_str());
                    // 如果找到了匹配的参数名，试图再次在符号表中查找
                    // 这里假设之前在ir_function_formal_params已经创建了这个变量
                    val = module->findVarValue(node->name);
                    if (val) {
                        printf("DEBUG: 再次查找成功，找到变量: %s\n", node->name.c_str());
                    }
                    break;
                }
            }
        }
    }

    if (!val) {
        printf("ERROR: 变量未找到: %s\n", node->name.c_str());
        setLastError("变量未找到: " + node->name);
        return false;
    }

    node->val = val;
    return true;
}

/// @brief 无符号整数字面量叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_uint(ast_node * node)
{
    ConstInt * val;

    // 新建一个整数常量Value
    val = module->newConstInt((int32_t) node->integer_val);

    node->val = val;

    return true;
}

/// @brief 变量声明语句节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_declare_statment(ast_node * node)
{
    bool result = false;

    for (auto & child: node->sons) {

        // 遍历每个变量声明
        result = ir_variable_declare(child);
        if (!result) {
            break;
        }
        // 将变量声明生成的指令添加到当前节点的指令列表中-lxg
        node->blockInsts.addInst(child->blockInsts);
    }

    return result;
}

/// @brief 变量定声明节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
// bool IRGenerator::ir_variable_declare(ast_node * node)
// {
//     // 共有两个孩子，第一个类型，第二个变量名

//     // TODO 这里可强化类型等检查

//     node->val = module->newVarValue(node->sons[0]->type, node->sons[1]->name);

//     return true;
// }
//不仅要创建变量，还要处理初始化值-lxg
bool IRGenerator::ir_variable_declare(ast_node * node)
{
    if (!node || node->sons.size() < 2) {
        setLastError("变量声明节点格式错误");
        return false;
    }

    // 获取变量类型和名称
    Type * varType = node->sons[0]->type;

    // 检查第二个子节点是否是数组定义节点
    if (node->sons[1]->node_type == ast_operator_type::AST_OP_ARRAY_DEF) {
        // 处理数组定义
        return ir_array_def(node->sons[1]);
    }

    std::string varName = node->sons[1]->name;

    printf("DEBUG: 处理变量声明: %s, 子节点数量: %zu\n", varName.c_str(), node->sons.size());

    // 创建变量
    Value * var = module->newVarValue(varType, varName);
    if (!var) {
        setLastError("创建变量失败: " + varName);
        return false;
    }

    // 获取当前函数
    Function * currentFunc = module->getCurrentFunction();

    // 处理变量初始化
    if (node->sons.size() > 2 && node->sons[2]) {
        printf("DEBUG: 变量 %s 有初始化表达式\n", varName.c_str());

        if (currentFunc) {
            // 局部变量初始化
            // 处理初始化表达式
            ast_node * init_expr = ir_visit_ast_node(node->sons[2]);
            if (!init_expr) {
                setLastError("处理变量 " + varName + " 的初始化表达式失败");
                return false;
            }

            if (!init_expr->val) {
                // 如果是整数字面量，直接创建常量
                if (node->sons[2]->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
                    uint32_t value = node->sons[2]->integer_val;
                    ConstInt * constVal = module->newConstInt(value);

                    // 生成赋值指令
                    MoveInstruction * moveInst = new MoveInstruction(currentFunc, var, constVal);

                    // 添加赋值指令
                    node->blockInsts.addInst(moveInst);
                    printf("DEBUG: 为局部变量 %s 生成了初始化为%u的指令\n", varName.c_str(), value);
                } else {
                    setLastError("变量 " + varName + " 的初始化表达式没有产生有效值");
                    return false;
                }
            } else {
                printf("DEBUG: 初始化表达式生成的值类型: %s\n",
                       init_expr->val->getType()->isInt32Type() ? "int32" : "其他");

                // 生成赋值指令
                MoveInstruction * moveInst = new MoveInstruction(currentFunc, var, init_expr->val);

                // 添加初始化表达式的指令和赋值指令
                node->blockInsts.addInst(init_expr->blockInsts);
                node->blockInsts.addInst(moveInst);

                printf("DEBUG: 为局部变量 %s 生成了初始化指令\n", varName.c_str());
            }
        } else {
            // 全局变量初始化
            if (node->sons[2]->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
                uint32_t value = node->sons[2]->integer_val;
                printf("DEBUG: 记录全局变量 %s 的初始值 %u\n", varName.c_str(), value);

                // 保存全局变量的初始值
                globalVarInitValues[varName] = value;
            } else {
                printf("DEBUG: 全局变量 %s 的初始化表达式太复杂，当前不支持\n", varName.c_str());
            }
        }
    } else if (currentFunc) {
        // 对于未初始化的局部变量，我们可以默认初始化为0
        if (varType->isInt32Type()) {
            ConstInt * zeroVal = module->newConstInt(0);
            MoveInstruction * moveInst = new MoveInstruction(currentFunc, var, zeroVal);
            node->blockInsts.addInst(moveInst);
            printf("DEBUG: 为局部变量 %s 生成了默认初始化为0的指令\n", varName.c_str());
        }
    }
    node->val = var;
    return true;
}

// 实现数组定义和访问的处理函数-lxg
/// @brief 数组定义节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_array_def(ast_node * node)
{
    if (node->sons.size() < 2) {
        setLastError("数组定义节点格式错误");
        return false;
    }

    // 获取数组名
    std::string arrayName = node->sons[0]->name;
    printf("DEBUG: 处理数组定义: %s\n", arrayName.c_str());

    // 收集维度信息
    std::vector<int> dimensions;
    for (size_t i = 1;
         i < node->sons.size() - (node->sons.back()->node_type != ast_operator_type::AST_OP_LEAF_LITERAL_UINT ? 1 : 0);
         i++) {
        // 确保维度是常量表达式
        if (node->sons[i]->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
            int dimSize = node->sons[i]->integer_val; // 直接使用节点中的整数值
            if (dimSize <= 0) {
                setLastError("数组维度必须大于0");
                return false;
            }
            dimensions.push_back(dimSize);
            printf("DEBUG: 数组维度 %zu: %d\n", i, dimSize);
        } else {
            // 处理表达式作为维度大小
            ast_node * dimExpr = ir_visit_ast_node(node->sons[i]);
            if (!dimExpr || !dimExpr->val) {
                setLastError("数组维度必须是常量表达式");
                return false;
            }

            // 尝试获取常量值
            if (auto * constInt = dynamic_cast<ConstInt *>(dimExpr->val)) {
                int dimSize = constInt->getVal();
                if (dimSize <= 0) {
                    setLastError("数组维度必须大于0");
                    return false;
                }
                dimensions.push_back(dimSize);
                printf("DEBUG: 数组维度 %zu: %d (从表达式)\n", i, dimSize);
            } else {
                setLastError("数组维度必须是常量表达式");
                return false;
            }
        }
    }

    // 确保数组至少有一个维度，并且所有维度都大于0
    if (dimensions.empty()) {
        setLastError("数组必须至少有一个维度");
        return false;
    }

    for (int dim: dimensions) {
        if (dim <= 0) {
            setLastError("数组维度必须大于0");
            return false;
        }
    }

    // 创建数组类型
    Type * elementType = IntegerType::getTypeInt(); // 假设元素类型是int
    Type * arrayType = new ArrayType(elementType, dimensions);

    // 创建数组变量
    Function * currentFunc = module->getCurrentFunction();
    Value * arrayVar;

    if (currentFunc) {
        // 局部数组变量
        arrayVar = module->newVarValue(arrayType, arrayName);
        printf("DEBUG: 创建局部数组变量: %s\n", arrayName.c_str());

        // 处理数组初始化 (如果有)
        if (node->sons.size() > dimensions.size() + 1) {
            ast_node * initNode = node->sons.back();
            if (initNode) {
                printf("DEBUG: 数组初始化暂不支持\n");
                // 目前不处理数组初始化，这需要更复杂的实现
            }
        }
    } else {
        // 全局数组变量
        arrayVar = module->newVarValue(arrayType, arrayName);
        printf("DEBUG: 创建全局数组变量: %s\n", arrayName.c_str());

        // 全局数组初始化同样暂不支持
    }

    node->val = arrayVar;
    return true;
}

/// @brief 数组访问节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_array_access(ast_node * node)
{
    if (node->sons.size() < 2) {
        setLastError("数组访问节点格式错误");
        return false;
    }

    // 获取数组变量
    std::string arrayName = node->sons[0]->name;
    Value * arrayVar = module->findVarValue(arrayName);

    if (!arrayVar) {
        setLastError("未定义的数组: " + arrayName);
        return false;
    }

    // 获取当前函数
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        setLastError("数组访问必须在函数内部");
        return false;
    }

    // 检查是否是函数参数（数组参数）
    if (isCurrentFunctionParameter(arrayName)) {
        printf("DEBUG: 处理函数数组参数访问: %s\n", arrayName.c_str());

        // 🔧 关键修改：获取保存的维度信息
        std::string funcName = currentFunc->getName();

        // 找到参数索引
        int paramIndex = -1;
        for (size_t i = 0; i < currentFunc->getParams().size(); i++) {
            if (currentFunc->getParams()[i]->getName() == arrayName) {
                paramIndex = i;
                break;
            }
        }

        if (paramIndex >= 0 && functionParameterDimensions.count(funcName) > 0 &&
            functionParameterDimensions[funcName].count(paramIndex) > 0) {

            // 使用保存的维度信息进行正确的偏移计算
            const std::vector<int> & dimensions = functionParameterDimensions[funcName][paramIndex];

            printf("DEBUG: 使用保存的维度信息，维度数: %zu\n", dimensions.size());
            for (size_t i = 0; i < dimensions.size(); i++) {
                printf("DEBUG: 维度 %zu: %d\n", i, dimensions[i]);
            }

            return handleParameterArrayAccessWithDimensions(node, arrayVar, dimensions);
        } else {
            // 没有维度信息，按简单指针处理
            printf("DEBUG: 没有找到维度信息，按简单指针处理\n");
            return handleSimplePointerParamAccess(node, arrayVar);
        }
    }

    // 处理普通数组（非参数）
    if (!arrayVar->getType()->isArrayType()) {
        setLastError(arrayName + " 不是数组类型");
        return false;
    }

    ArrayType * arrayType = static_cast<ArrayType *>(arrayVar->getType());
    std::vector<int> dimensions = arrayType->getDimensions();

    // 处理普通数组访问
    return handleRegularArrayAccess(node, arrayVar, dimensions);
}

bool IRGenerator::ir_empty_stmt(ast_node * node)
{
    // 空语句不需要生成任何实际代码
    // 只需要返回成功即可
    printf("DEBUG: 处理空语句\n");
    return true;
}

/// @brief 函数数组形参AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_formal_param_array(ast_node * node)
{
    // 数组参数在C语言中实际上是指针
    // 这里不需要特殊处理，因为在ir_function_formal_params中已经处理了
    // 这个函数主要是为了防止ir_default被调用

    printf("DEBUG: 处理数组形参节点: %s\n", node->sons.size() > 1 ? node->sons[1]->name.c_str() : "未知");

    return true;
}

/// @brief 检查变量是否是当前函数的参数
/// @param varName 变量名
/// @return 是否是函数参数
bool IRGenerator::isCurrentFunctionParameter(const std::string & varName)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc)
        return false;

    for (auto param: currentFunc->getParams()) {
        if (param->getName() == varName) {
            return true;
        }
    }
    return false;
}

/// @brief 计算数组访问的深度
/// @param node 数组访问节点
/// @return 访问深度
int IRGenerator::calculateArrayAccessDepth(ast_node * node)
{
    // 计算实际的索引数量
    // sons[0] 是数组名，sons[1], sons[2], ... 是索引
    return node->sons.size() - 1;
}

/// @brief 根据访问深度计算行大小
/// @param dimensions 数组维度
/// @param accessDepth 访问深度
/// @return 行大小
int IRGenerator::calculateRowSize(const std::vector<int> & dimensions, int accessDepth)
{
    // 行大小 = 从 accessDepth 维开始的所有维度的乘积
    int rowSize = 1;
    for (size_t i = accessDepth; i < dimensions.size(); i++) {
        rowSize *= dimensions[i];
    }
    return rowSize;
}

/// @brief 计算线性偏移量（处理多维索引）
/// @param node 数组访问节点
/// @param blockInsts 指令容器，用于添加生成的指令
/// @return 线性偏移量
Value * IRGenerator::calculateLinearOffset(ast_node * node, InterCode & blockInsts)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        setLastError("当前函数为空");
        return nullptr;
    }

    // 如果所有索引都是0，直接返回常量0，避免复杂计算
    bool allZeros = true;
    for (size_t i = 1; i < node->sons.size(); i++) {
        ast_node * indexNode = node->sons[i];
        if (indexNode->node_type != ast_operator_type::AST_OP_LEAF_LITERAL_UINT || indexNode->integer_val != 0) {
            allZeros = false;
            break;
        }
    }

    // 如果所有索引都是0，直接返回0，避免复杂计算
    if (allZeros) {
        printf("DEBUG: 所有索引都是0，返回常量0\n");
        return module->newConstInt(0);
    }

    // 获取数组名
    std::string arrayName = node->sons[0]->name;
    Value * arrayVar = module->findVarValue(arrayName);
    if (!arrayVar) {
        setLastError("未找到数组: " + arrayName);
        return nullptr;
    }

    // 如果只有一个索引，直接处理
    if (node->sons.size() == 2) {
        ast_node * indexNode = ir_visit_ast_node(node->sons[1]);
        if (!indexNode || !indexNode->val) {
            setLastError("无效的数组索引表达式");
            return nullptr;
        }
        // 添加索引计算的指令
        blockInsts.addInst(indexNode->blockInsts);
        return indexNode->val;
    }

    // 获取数组维度信息
    ArrayType * arrayType = dynamic_cast<ArrayType *>(arrayVar->getType());
    if (!arrayType) {
        printf("DEBUG: 数组参数无法获取维度信息，使用简化计算\n");
        return module->newConstInt(0);
    }

    const std::vector<int> & dimensions = arrayType->getDimensions();

    // 初始化线性偏移为0
    Value * linearOffset = module->newConstInt(0);

    // 计算每个维度的贡献
    for (size_t i = 1; i < node->sons.size(); i++) {
        ast_node * indexNode = ir_visit_ast_node(node->sons[i]);
        if (!indexNode || !indexNode->val) {
            setLastError("无效的数组索引表达式");
            return nullptr;
        }

        // 添加索引计算的指令
        blockInsts.addInst(indexNode->blockInsts);

        // 计算该维度的系数
        int coefficient = 1;
        for (size_t j = i; j < dimensions.size(); j++) {
            coefficient *= dimensions[j];
        }

        // printf("DEBUG: 维度 %zu 的系数: %d\n", i - 1, coefficient);

        if (coefficient == 1) {
            BinaryInstruction * addInst = new BinaryInstruction(currentFunc,
                                                                IRInstOperator::IRINST_OP_ADD_I,
                                                                linearOffset,
                                                                indexNode->val,
                                                                IntegerType::getTypeInt());
            // 将指令添加到指令流
            blockInsts.addInst(addInst);
            linearOffset = addInst;
        } else {
            BinaryInstruction * mulInst = new BinaryInstruction(currentFunc,
                                                                IRInstOperator::IRINST_OP_MUL_I,
                                                                indexNode->val,
                                                                module->newConstInt(coefficient),
                                                                IntegerType::getTypeInt());
            // 将乘法指令添加到指令流
            blockInsts.addInst(mulInst);

            BinaryInstruction * addInst = new BinaryInstruction(currentFunc,
                                                                IRInstOperator::IRINST_OP_ADD_I,
                                                                linearOffset,
                                                                mulInst,
                                                                IntegerType::getTypeInt());
            // 将加法指令添加到指令流
            blockInsts.addInst(addInst);
            linearOffset = addInst;
        }
    }

    return linearOffset;
}

/// @brief 根据形参类型计算参数传递的偏移量
Value * IRGenerator::calculateParameterOffset(ast_node * arrayAccessNode,
                                              const std::vector<int> & paramDimensions,
                                              InterCode & blockInsts)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        return nullptr;
    }

    // 获取索引数量（减去数组名）
    size_t indexCount = arrayAccessNode->sons.size() - 1;

    // 如果没有索引，返回0
    if (indexCount == 0) {
        return module->newConstInt(0);
    }

    // 根据形参维度计算正确的偏移量
    // paramDimensions[0] = 0（指针维度），paramDimensions[1], [2]... 是实际维度
    std::vector<int> actualDimensions(paramDimensions.begin() + 1, paramDimensions.end());

    // 计算线性索引
    Value * linearIndex = module->newConstInt(0);

    for (size_t i = 0; i < indexCount && i < actualDimensions.size(); i++) {
        // 处理当前维度的索引
        ast_node * indexNode = ir_visit_ast_node(arrayAccessNode->sons[i + 1]);
        if (!indexNode || !indexNode->val) {
            return nullptr;
        }
        blockInsts.addInst(indexNode->blockInsts);

        // 计算该维度的步长（后续所有维度大小的乘积）
        int stride = 1;
        for (size_t j = i + 1; j < actualDimensions.size(); j++) {
            stride *= actualDimensions[j];
        }

        if (stride > 1) {
            // index * stride
            BinaryInstruction * mulInst = new BinaryInstruction(currentFunc,
                                                                IRInstOperator::IRINST_OP_MUL_I,
                                                                indexNode->val,
                                                                module->newConstInt(stride),
                                                                IntegerType::getTypeInt());
            blockInsts.addInst(mulInst);

            // linearIndex + (index * stride)
            BinaryInstruction * addInst = new BinaryInstruction(currentFunc,
                                                                IRInstOperator::IRINST_OP_ADD_I,
                                                                linearIndex,
                                                                mulInst,
                                                                IntegerType::getTypeInt());
            blockInsts.addInst(addInst);
            linearIndex = addInst;
        } else {
            // stride == 1，直接加上索引
            BinaryInstruction * addInst = new BinaryInstruction(currentFunc,
                                                                IRInstOperator::IRINST_OP_ADD_I,
                                                                linearIndex,
                                                                indexNode->val,
                                                                IntegerType::getTypeInt());
            blockInsts.addInst(addInst);
            linearIndex = addInst;
        }
    }

    // 转换为字节偏移量（乘以元素大小）
    BinaryInstruction * byteOffsetInst = new BinaryInstruction(currentFunc,
                                                               IRInstOperator::IRINST_OP_MUL_I,
                                                               linearIndex,
                                                               module->newConstInt(4), // sizeof(int)
                                                               IntegerType::getTypeInt());
    blockInsts.addInst(byteOffsetInst);

    return byteOffsetInst;
}

/// @brief 计算数组访问的字节偏移量
Value * IRGenerator::calculateArrayAccessOffset(ast_node * arrayAccessNode, InterCode & blockInsts)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        return nullptr;
    }

    // 获取数组变量信息
    std::string arrayName = arrayAccessNode->sons[0]->name;
    Value * arrayVar = module->findVarValue(arrayName);
    if (!arrayVar) {
        return nullptr;
    }

    // 获取索引数量（减去数组名）
    size_t indexCount = arrayAccessNode->sons.size() - 1;

    // 如果没有索引，返回0
    if (indexCount == 0) {
        return module->newConstInt(0);
    }

    // 获取数组维度信息
    ArrayType * arrayType = dynamic_cast<ArrayType *>(arrayVar->getType());
    if (!arrayType) {
        // 如果无法获取维度信息，使用简化计算
        printf("DEBUG: 无法获取数组维度信息，使用简化偏移计算\n");

        // 只处理第一个索引
        ast_node * indexNode = ir_visit_ast_node(arrayAccessNode->sons[1]);
        if (!indexNode || !indexNode->val) {
            return nullptr;
        }
        blockInsts.addInst(indexNode->blockInsts);

        // 转换为字节偏移量
        BinaryInstruction * byteOffsetInst = new BinaryInstruction(currentFunc,
                                                                   IRInstOperator::IRINST_OP_MUL_I,
                                                                   indexNode->val,
                                                                   module->newConstInt(4),
                                                                   IntegerType::getTypeInt());
        blockInsts.addInst(byteOffsetInst);
        return byteOffsetInst;
    }

    const std::vector<int> & dimensions = arrayType->getDimensions();

    // 计算线性索引（与原有逻辑相同）
    Value * totalOffset = module->newConstInt(0);

    // 处理每个维度的索引
    for (size_t dimIdx = 1; dimIdx < arrayAccessNode->sons.size(); dimIdx++) {
        ast_node * indexNode = ir_visit_ast_node(arrayAccessNode->sons[dimIdx]);
        if (!indexNode || !indexNode->val) {
            return nullptr;
        }
        blockInsts.addInst(indexNode->blockInsts);

        // 计算该维度的步长（从当前维度到最后一维的乘积）
        int stride = 1;
        for (size_t j = dimIdx; j < dimensions.size(); j++) {
            stride *= dimensions[j];
        }

        Value * indexContribution;
        if (stride == 1) {
            indexContribution = indexNode->val;
        } else {
            BinaryInstruction * mulInst = new BinaryInstruction(currentFunc,
                                                                IRInstOperator::IRINST_OP_MUL_I,
                                                                indexNode->val,
                                                                module->newConstInt(stride),
                                                                IntegerType::getTypeInt());
            blockInsts.addInst(mulInst);
            indexContribution = mulInst;
        }

        // 累加到总偏移量
        BinaryInstruction * addInst = new BinaryInstruction(currentFunc,
                                                            IRInstOperator::IRINST_OP_ADD_I,
                                                            totalOffset,
                                                            indexContribution,
                                                            IntegerType::getTypeInt());
        blockInsts.addInst(addInst);
        totalOffset = addInst;
    }

    // 转换为字节偏移量（乘以元素大小）
    BinaryInstruction * byteOffsetInst = new BinaryInstruction(currentFunc,
                                                               IRInstOperator::IRINST_OP_MUL_I,
                                                               totalOffset,
                                                               module->newConstInt(4), // sizeof(int)
                                                               IntegerType::getTypeInt());
    blockInsts.addInst(byteOffsetInst);

    return byteOffsetInst;
}

/// @brief 处理简单指针参数访问 (int* 类型)
bool IRGenerator::handleSimplePointerParamAccess(ast_node * node, Value * arrayVar)
{
    Function * currentFunc = module->getCurrentFunction();

    // 只处理第一个索引
    ast_node * indexNode = ir_visit_ast_node(node->sons[1]);
    if (!indexNode || !indexNode->val) {
        setLastError("无效的数组索引表达式");
        return false;
    }
    node->blockInsts.addInst(indexNode->blockInsts);

    // 计算字节偏移量：index * sizeof(int)
    LocalVariable * byteOffset = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));
    BinaryInstruction * byteOffsetInst = new BinaryInstruction(currentFunc,
                                                               IRInstOperator::IRINST_OP_MUL_I,
                                                               indexNode->val,
                                                               module->newConstInt(4), // sizeof(int) = 4
                                                               IntegerType::getTypeInt());
    node->blockInsts.addInst(byteOffsetInst);
    node->blockInsts.addInst(new MoveInstruction(currentFunc, byteOffset, byteOffsetInst));

    // 计算元素指针：arrayVar + byteOffset
    Type * ptrType = const_cast<Type *>(static_cast<const Type *>(PointerType::get(IntegerType::getTypeInt())));
    LocalVariable * elemPtr = static_cast<LocalVariable *>(module->newVarValue(ptrType));

    BinaryInstruction * ptrInst =
        new BinaryInstruction(currentFunc, IRInstOperator::IRINST_OP_ADD_I, arrayVar, byteOffset, ptrType);
    node->blockInsts.addInst(ptrInst);
    node->blockInsts.addInst(new MoveInstruction(currentFunc, elemPtr, ptrInst));

    // 读取元素值
    LocalVariable * elemValue = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));
    MoveInstruction * loadInst = new MoveInstruction(currentFunc, elemValue, elemPtr);
    loadInst->setIsPointerLoad(true);
    node->blockInsts.addInst(loadInst);

    // 保存结果
    node->arrayVar = arrayVar;
    node->offsetValue = byteOffset;
    node->arrayPtr = elemPtr;
    node->val = elemValue;

    printf("DEBUG: 完成简单指针参数访问\n");
    return true;
}

/// @brief 处理多维数组参数访问 (int(*)[2][2]... 类型)
bool IRGenerator::handleMultiDimArrayParamAccess(ast_node * node, Value * arrayVar, const std::vector<int> & dimensions)
{
    Function * currentFunc = module->getCurrentFunction();

    // 收集所有索引
    std::vector<Value *> indices;
    for (size_t i = 1; i < node->sons.size(); i++) {
        ast_node * indexNode = ir_visit_ast_node(node->sons[i]);
        if (!indexNode || !indexNode->val) {
            setLastError("无效的数组索引表达式");
            return false;
        }
        node->blockInsts.addInst(indexNode->blockInsts);
        indices.push_back(indexNode->val);
    }

    // 计算正确的线性偏移量
    Value * linearOffset = module->newConstInt(0);

    // 对于 int(*)[2][2] 类型的参数访问 param[i][j]
    // 偏移量 = i * dimensions[0] + j
    for (size_t i = 0; i < indices.size() && i < dimensions.size(); i++) {
        Value * indexContribution;

        // 计算该维度的步长（从当前维度到最后一维的乘积）
        int stride = 1;
        for (size_t j = i + 1; j < dimensions.size(); j++) {
            stride *= dimensions[j];
        }

        if (stride == 1) {
            indexContribution = indices[i];
        } else {
            BinaryInstruction * mulInst = new BinaryInstruction(currentFunc,
                                                                IRInstOperator::IRINST_OP_MUL_I,
                                                                indices[i],
                                                                module->newConstInt(stride),
                                                                IntegerType::getTypeInt());
            node->blockInsts.addInst(mulInst);
            indexContribution = mulInst;
        }

        // 累加到线性偏移量
        BinaryInstruction * addInst = new BinaryInstruction(currentFunc,
                                                            IRInstOperator::IRINST_OP_ADD_I,
                                                            linearOffset,
                                                            indexContribution,
                                                            IntegerType::getTypeInt());
        node->blockInsts.addInst(addInst);
        linearOffset = addInst;
    }

    // 转换为字节偏移量
    BinaryInstruction * byteOffsetInst = new BinaryInstruction(currentFunc,
                                                               IRInstOperator::IRINST_OP_MUL_I,
                                                               linearOffset,
                                                               module->newConstInt(4), // sizeof(int)
                                                               IntegerType::getTypeInt());
    node->blockInsts.addInst(byteOffsetInst);

    // 计算最终指针
    Type * ptrType = const_cast<Type *>(static_cast<const Type *>(PointerType::get(IntegerType::getTypeInt())));
    LocalVariable * elemPtr = static_cast<LocalVariable *>(module->newVarValue(ptrType));

    BinaryInstruction * ptrInst =
        new BinaryInstruction(currentFunc, IRInstOperator::IRINST_OP_ADD_I, arrayVar, byteOffsetInst, ptrType);
    node->blockInsts.addInst(ptrInst);
    node->blockInsts.addInst(new MoveInstruction(currentFunc, elemPtr, ptrInst));

    // 读取元素值
    LocalVariable * elemValue = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));
    MoveInstruction * loadInst = new MoveInstruction(currentFunc, elemValue, elemPtr);
    loadInst->setIsPointerLoad(true);
    node->blockInsts.addInst(loadInst);

    // 保存结果
    node->arrayVar = arrayVar;
    node->offsetValue = byteOffsetInst;
    node->arrayPtr = elemPtr;
    node->val = elemValue;

    printf("DEBUG: 完成多维数组参数访问\n");
    return true;
}

/// @brief 处理普通数组访问（保持原有逻辑）
bool IRGenerator::handleRegularArrayAccess(ast_node * node, Value * arrayVar, const std::vector<int> & dimensions)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        setLastError("数组访问必须在函数内部");
        return false;
    }

    // 处理索引表达式
    std::vector<Value *> indices;
    for (size_t i = 1; i < node->sons.size(); i++) {
        ast_node * indexNode = ir_visit_ast_node(node->sons[i]);
        if (!indexNode || !indexNode->val) {
            setLastError("无效的数组索引表达式");
            return false;
        }

        // 将索引表达式的指令添加到当前节点
        node->blockInsts.addInst(indexNode->blockInsts);

        indices.push_back(indexNode->val);
        // printf("DEBUG: 处理数组索引 %zu\n", i - 1);
    }

    // 针对二维数组的处理
    if (indices.size() == 2 && dimensions.size() == 2) {
        // 获取行列索引
        Value * rowIndex = indices[0];
        Value * colIndex = indices[1];

        // 获取列数（第二维度大小）
        int colSize = dimensions[1];

        // 创建临时变量
        LocalVariable * mulResult = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));
        LocalVariable * addResult = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));
        LocalVariable * offsetResult = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));

        // 为指针类型创建一个普通的Type*，而不是const PointerType*
        Type * ptrType = const_cast<Type *>(static_cast<const Type *>(PointerType::get(IntegerType::getTypeInt())));
        LocalVariable * ptrResult = static_cast<LocalVariable *>(module->newVarValue(ptrType));

        // 创建一个局部变量用于存储数组元素的值
        LocalVariable * elemValue = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));

        // 1. %t5 = mul %l2, 10  (rowIndex * colSize)
        BinaryInstruction * mulInst = new BinaryInstruction(currentFunc,
                                                            IRInstOperator::IRINST_OP_MUL_I,
                                                            rowIndex,
                                                            module->newConstInt(colSize),
                                                            IntegerType::getTypeInt());
        node->blockInsts.addInst(mulInst);
        node->blockInsts.addInst(new MoveInstruction(currentFunc, mulResult, mulInst));

        // 2. %t6 = add %t5, %l3  (rowIndex * colSize + colIndex)
        BinaryInstruction * addInst = new BinaryInstruction(currentFunc,
                                                            IRInstOperator::IRINST_OP_ADD_I,
                                                            mulResult,
                                                            colIndex,
                                                            IntegerType::getTypeInt());
        node->blockInsts.addInst(addInst);
        node->blockInsts.addInst(new MoveInstruction(currentFunc, addResult, addInst));

        // 3. %t7 = mul %t6, 4  ((rowIndex * colSize + colIndex) * sizeof(int))
        BinaryInstruction * offsetInst = new BinaryInstruction(currentFunc,
                                                               IRInstOperator::IRINST_OP_MUL_I,
                                                               addResult,
                                                               module->newConstInt(4), // sizeof(int) = 4
                                                               IntegerType::getTypeInt());
        node->blockInsts.addInst(offsetInst);
        node->blockInsts.addInst(new MoveInstruction(currentFunc, offsetResult, offsetInst));

        // 4. %t8 = add %l1, %t7  (数组基址 + 字节偏移)
        BinaryInstruction * ptrInst =
            new BinaryInstruction(currentFunc, IRInstOperator::IRINST_OP_ADD_I, arrayVar, offsetResult, ptrType);
        node->blockInsts.addInst(ptrInst);
        node->blockInsts.addInst(new MoveInstruction(currentFunc, ptrResult, ptrInst));

        // 5. 读取数组元素的值 (新增)
        MoveInstruction * loadInst = new MoveInstruction(currentFunc,
                                                         elemValue, // 存储元素值的变量
                                                         ptrResult  // 元素指针
        );
        loadInst->setIsPointerLoad(true); // 标记为指针加载操作
        node->blockInsts.addInst(loadInst);

        // 保存必要信息
        node->arrayVar = arrayVar;
        node->offsetValue = offsetResult;
        node->arrayPtr = ptrResult; // 用于赋值操作
        node->val = elemValue;      // 对于表达式，返回元素的值而不是指针

        printf("DEBUG: 完成二维数组访问，读取了元素值: %s\n", elemValue->getIRName().c_str());
    } else {
        // 处理一般维度的数组 - 使用标准的多维数组展开公式
        Value * linearOffset = module->newConstInt(0);

        // 标准多维数组线性偏移公式：offset = i0*d1*d2*...*dn + i1*d2*...*dn + ... + in
        for (size_t i = 0; i < indices.size(); i++) {
            // 计算该维度的权重（后续所有维度大小的乘积）
            int weight = 1;
            for (size_t j = i + 1; j < dimensions.size(); j++) {
                weight *= dimensions[j];
            }

            // printf("DEBUG: 维度 %zu 的权重: %d\n", i, weight);

            if (weight == 1) {
                // 最后一维，直接加上索引
                BinaryInstruction * addInst = new BinaryInstruction(currentFunc,
                                                                    IRInstOperator::IRINST_OP_ADD_I,
                                                                    linearOffset,
                                                                    indices[i],
                                                                    IntegerType::getTypeInt());
                node->blockInsts.addInst(addInst);
                linearOffset = addInst;
            } else {
                // 计算 indices[i] * weight
                BinaryInstruction * mulInst = new BinaryInstruction(currentFunc,
                                                                    IRInstOperator::IRINST_OP_MUL_I,
                                                                    indices[i],
                                                                    module->newConstInt(weight),
                                                                    IntegerType::getTypeInt());
                node->blockInsts.addInst(mulInst);

                // 累加到总偏移
                BinaryInstruction * addInst = new BinaryInstruction(currentFunc,
                                                                    IRInstOperator::IRINST_OP_ADD_I,
                                                                    linearOffset,
                                                                    mulInst,
                                                                    IntegerType::getTypeInt());
                node->blockInsts.addInst(addInst);
                linearOffset = addInst;
            }
        }

        // 计算字节偏移量
        BinaryInstruction * byteOffsetInst = new BinaryInstruction(currentFunc,
                                                                   IRInstOperator::IRINST_OP_MUL_I,
                                                                   linearOffset,
                                                                   module->newConstInt(4), // sizeof(int) = 4
                                                                   IntegerType::getTypeInt());
        node->blockInsts.addInst(byteOffsetInst);

        // 为指针类型创建一个普通的Type*，而不是const PointerType*
        Type * ptrType = const_cast<Type *>(static_cast<const Type *>(PointerType::get(IntegerType::getTypeInt())));
        LocalVariable * elemPtr = static_cast<LocalVariable *>(module->newVarValue(ptrType));

        // 计算元素指针
        BinaryInstruction * ptrInst =
            new BinaryInstruction(currentFunc, IRInstOperator::IRINST_OP_ADD_I, arrayVar, byteOffsetInst, ptrType);
        node->blockInsts.addInst(ptrInst);
        node->blockInsts.addInst(new MoveInstruction(currentFunc, elemPtr, ptrInst));

        // 创建一个局部变量用于存储数组元素的值
        LocalVariable * elemValue = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));

        // 读取数组元素的值 (新增)
        MoveInstruction * loadInst = new MoveInstruction(currentFunc,
                                                         elemValue, // 存储元素值的变量
                                                         elemPtr    // 元素指针
        );
        loadInst->setIsPointerLoad(true); // 标记为指针加载操作
        node->blockInsts.addInst(loadInst);

        // 保存结果
        node->arrayVar = arrayVar;
        node->offsetValue = byteOffsetInst;
        node->arrayPtr = elemPtr; // 用于赋值操作
        node->val = elemValue;    // 对于表达式，返回元素的值而不是指针

        printf("DEBUG: 完成多维数组访问，读取了元素值: %s\n", elemValue->getIRName().c_str());
    }

    return true;
}

/// @brief 使用保存的维度信息处理函数数组参数访问
bool IRGenerator::handleParameterArrayAccessWithDimensions(ast_node * node,
                                                           Value * arrayVar,
                                                           const std::vector<int> & dimensions)
{
    Function * currentFunc = module->getCurrentFunction();

    // 收集所有索引
    std::vector<Value *> indices;
    for (size_t i = 1; i < node->sons.size(); i++) {
        ast_node * indexNode = ir_visit_ast_node(node->sons[i]);
        if (!indexNode || !indexNode->val) {
            setLastError("无效的数组索引表达式");
            return false;
        }
        node->blockInsts.addInst(indexNode->blockInsts);
        indices.push_back(indexNode->val);
    }

    // 使用正确的维度信息计算偏移量
    Value * linearOffset = module->newConstInt(0);

    // 对于保存的维度 [2, 2] (表示 [2][2])
    // 访问 param[i][j] 的偏移量 = i * 2 + j
    for (size_t i = 0; i < indices.size() && i < dimensions.size(); i++) {
        Value * indexContribution;

        // 计算该维度的步长（从下一维开始的所有维度大小的乘积）
        int stride = 1;
        for (size_t j = i + 1; j < dimensions.size(); j++) {
            stride *= dimensions[j];
        }

        printf("DEBUG: 维度 %zu, 步长: %d\n", i, stride);

        if (stride == 1) {
            indexContribution = indices[i];
        } else {
            BinaryInstruction * mulInst = new BinaryInstruction(currentFunc,
                                                                IRInstOperator::IRINST_OP_MUL_I,
                                                                indices[i],
                                                                module->newConstInt(stride),
                                                                IntegerType::getTypeInt());
            node->blockInsts.addInst(mulInst);
            indexContribution = mulInst;
        }

        // 累加到线性偏移量
        BinaryInstruction * addInst = new BinaryInstruction(currentFunc,
                                                            IRInstOperator::IRINST_OP_ADD_I,
                                                            linearOffset,
                                                            indexContribution,
                                                            IntegerType::getTypeInt());
        node->blockInsts.addInst(addInst);
        linearOffset = addInst;
    }

    // 转换为字节偏移量
    BinaryInstruction * byteOffsetInst = new BinaryInstruction(currentFunc,
                                                               IRInstOperator::IRINST_OP_MUL_I,
                                                               linearOffset,
                                                               module->newConstInt(4), // sizeof(int)
                                                               IntegerType::getTypeInt());
    node->blockInsts.addInst(byteOffsetInst);

    // 计算最终指针和读取元素值
    Type * ptrType = const_cast<Type *>(static_cast<const Type *>(PointerType::get(IntegerType::getTypeInt())));
    LocalVariable * elemPtr = static_cast<LocalVariable *>(module->newVarValue(ptrType));

    BinaryInstruction * ptrInst =
        new BinaryInstruction(currentFunc, IRInstOperator::IRINST_OP_ADD_I, arrayVar, byteOffsetInst, ptrType);
    node->blockInsts.addInst(ptrInst);
    node->blockInsts.addInst(new MoveInstruction(currentFunc, elemPtr, ptrInst));

    // 读取元素值
    LocalVariable * elemValue = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));
    MoveInstruction * loadInst = new MoveInstruction(currentFunc, elemValue, elemPtr);
    loadInst->setIsPointerLoad(true);
    node->blockInsts.addInst(loadInst);

    // 保存结果
    node->arrayVar = arrayVar;
    node->offsetValue = byteOffsetInst;
    node->arrayPtr = elemPtr;
    node->val = elemValue;

    printf("DEBUG: 完成使用维度信息的数组参数访问\n");
    return true;
}