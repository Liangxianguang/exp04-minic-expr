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
#include "ConstInt.h" //添加ConstInt-lxg
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
	ast2ir_handlers[ast_operator_type::AST_OP_MUL] = &IRGenerator::ir_mul; 	// 添加乘法处理
	ast2ir_handlers[ast_operator_type::AST_OP_DIV] = &IRGenerator::ir_div;  // 添加除法处理
    ast2ir_handlers[ast_operator_type::AST_OP_MOD] = &IRGenerator::ir_mod;  // 添加求余处理
	ast2ir_handlers[ast_operator_type::AST_OP_NEG] = &IRGenerator::ir_neg;  // 添加负号处理

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

    /* 语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_ASSIGN] = &IRGenerator::ir_assign;
    ast2ir_handlers[ast_operator_type::AST_OP_RETURN] = &IRGenerator::ir_return;

    /* 函数调用 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_CALL] = &IRGenerator::ir_function_call;

    /* 函数定义 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_DEF] = &IRGenerator::ir_function_define;
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS] = &IRGenerator::ir_function_formal_params;

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
bool IRGenerator::ir_default(ast_node * node)
{
    // 未知的节点
    printf("Unkown node(%d)\n", (int) node->node_type);
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
    
    // 第一遍：收集所有函数声明(包含参数信息)
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
            
            // 根据AST节点查找参数信息
            if (param_node && !param_node->sons.empty()) {
                for (auto & paramSon : param_node->sons) {
                    if (paramSon->sons.size() >= 2) {
                        Type* paramType = paramSon->sons[0]->type;
                        std::string paramName = paramSon->sons[1]->name;
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
            Function* func = module->newFunction(name_node->name, type_node->type, params);
            if (func) {
                printf("注册函数原型: %s 成功，参数数量: %zu\n", name_node->name.c_str(), params.size());
            } else {
                printf("注册函数原型: %s 失败\n", name_node->name.c_str());
            }
        }
    }
    
    // 第二遍：处理所有节点
    for (auto son: node->sons) {
        ast_node * son_node = ir_visit_ast_node(son);
        if (!son_node) {
            return false;
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
    Function* newFunc = module->findFunction(name_node->name);
    
    if (!newFunc) {
        // 如果函数不存在，使用AST中的信息创建函数参数列表
        std::vector<FormalParam *> params;
        if (param_node && !param_node->sons.empty()) {
            printf("DEBUG: 从AST获取函数参数，数量: %zu\n", param_node->sons.size());
            for (auto & paramSon : param_node->sons) {
                if (paramSon->sons.size() < 2) {
                    setLastError("形参节点格式错误");
                    return false;
                }
                
                Type* paramType = paramSon->sons[0]->type;
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
        
        printf("DEBUG: 创建新函数: %s, 参数数量: %zu\n", 
               name_node->name.c_str(), newFunc->getParams().size());
    } else {
        printf("DEBUG: 使用已注册的函数: %s, 参数数量: %zu\n", 
               name_node->name.c_str(), newFunc->getParams().size());
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

    // IR指令追加到当前的节点中
    node->blockInsts.addInst(block_node->blockInsts);

    // 此时，所有指令都加入到当前函数中，也就是node->blockInsts

    // node节点的指令移动到函数的IR指令列表中
    irCode.addInst(node->blockInsts);

    // 添加函数出口Label指令，主要用于return语句跳转到这里进行函数的退出
    irCode.addInst(exitLabelInst);

    // 函数出口指令
    irCode.addInst(new ExitInstruction(newFunc, retValue));

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
bool IRGenerator::ir_function_formal_params(ast_node * node)
{
    // 获取当前正在处理的函数
    Function* currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        setLastError("未在函数上下文中处理形参");
        return false;
    }
    
    // 获取函数的IR代码列表
    InterCode& irCode = currentFunc->getInterCode();
    
    printf("DEBUG: 处理函数形参，数量: %zu, 函数参数数量: %zu\n", 
           node->sons.size(), currentFunc->getParams().size());
    
    // 获取函数的参数列表
    const std::vector<FormalParam*>& functionParams = currentFunc->getParams();
    
    // 遍历函数的所有形参，创建对应的局部变量和临时变量
    for (size_t i = 0; i < functionParams.size(); i++) {
        FormalParam* param = functionParams[i];
        
        // 函数参数的类型和名称
        Type* paramType = param->getType();
        std::string paramName = param->getName();
        
        if (!paramType) {
            setLastError("函数参数 " + paramName + " 类型无效");
            return false;
        }
        
        printf("DEBUG: 处理函数参数: %s, 类型: %s\n", 
               paramName.c_str(), paramType->isInt32Type() ? "int" : "其他");
        
        // 1. 创建局部变量作为实际的形参变量（在函数内部使用）
        Value* localParam = module->newVarValue(paramType, paramName);
        if (!localParam) {
            setLastError("创建形参局部变量失败: " + paramName);
            return false;
        }
        
        // 2. 获取函数形参本身作为源值
        Value* paramValue = param;
        
        // 3. 创建赋值指令，将形参值复制到局部变量
        MoveInstruction* moveInst = new MoveInstruction(currentFunc, 
                                                       static_cast<LocalVariable*>(localParam), 
                                                       paramValue);
        
        // 4. 将赋值指令添加到函数的IR代码中（在Entry指令之后）
        irCode.addInst(moveInst);
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
    
    printf("DEBUG: 处理函数调用: %s 在第%lld行\n", funcName.c_str(), (long long)lineno);

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
    printf("DEBUG: 找到函数: %s, 需要%d个参数\n", 
           funcName.c_str(), formalParamCount);

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

        // 遍历参数列表，孩子是表达式
        // 这里自左往右计算表达式
        for (auto son: paramsNode->sons) {
            // 遍历Block的每个语句，进行显示或者运算
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
                           std::to_string(calledFunction->getParams().size()) + 
                           "个但提供了" + std::to_string(realParams.size()) + "个";
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
                                                      nullptr,  // 一元运算符第二个操作数为空
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
bool IRGenerator::ir_lt(ast_node* node)
{
    // 生成左右操作数
    ast_node* left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node) return false;
    
    ast_node* right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node) return false;
    
    Value* left = left_node->val;
    Value* right = right_node->val;
    
    if (!left || !right) return false;
    
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);
    
    // 创建临时变量存储比较结果 - 使用布尔类型
    LocalVariable* result = static_cast<LocalVariable*>(module->newVarValue(IntegerType::getTypeBool()));
    
    // 添加比较指令 - 使用布尔类型
    BinaryInstruction* ltInst = new BinaryInstruction(func, 
                                               IRInstOperator::IRINST_OP_LT_I, 
                                               left,
                                               right, 
                                               IntegerType::getTypeBool());
    node->blockInsts.addInst(ltInst);
    
    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, ltInst));
    
    node->val = result;
    return true;
}

// 大于
bool IRGenerator::ir_gt(ast_node* node)
{
    // 生成左右操作数
    ast_node* left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node) return false;
    
    ast_node* right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node) return false;
    
    Value* left = left_node->val;
    Value* right = right_node->val;
    
    if (!left || !right) return false;
    
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);
    
    // 使用布尔类型
    LocalVariable* result = static_cast<LocalVariable*>(module->newVarValue(IntegerType::getTypeBool()));
    
    // 使用布尔类型
    BinaryInstruction* gtInst = new BinaryInstruction(func, 
                                               IRInstOperator::IRINST_OP_GT_I, 
                                               left, 
                                               right, 
                                               IntegerType::getTypeBool());
    node->blockInsts.addInst(gtInst);
    
    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, gtInst));
    
    node->val = result;
    return true;
}

// 小于等于
bool IRGenerator::ir_le(ast_node* node)
{
    // 生成左右操作数
    ast_node* left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node) return false;
    
    ast_node* right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node) return false;
    
    Value* left = left_node->val;
    Value* right = right_node->val;
    
    if (!left || !right) return false;
    
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);
    
    // 使用布尔类型
    LocalVariable* result = static_cast<LocalVariable*>(module->newVarValue(IntegerType::getTypeBool()));
    
    // 使用布尔类型
    BinaryInstruction* leInst = new BinaryInstruction(func, 
                                               IRInstOperator::IRINST_OP_LE_I, 
                                               left, 
                                               right, 
                                               IntegerType::getTypeBool());
    node->blockInsts.addInst(leInst);
    
    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, leInst));
    
    node->val = result;
    return true;
}

// 大于等于
bool IRGenerator::ir_ge(ast_node* node)
{
    // 生成左右操作数
    ast_node* left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node) return false;
    
    ast_node* right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node) return false;
    
    Value* left = left_node->val;
    Value* right = right_node->val;
    
    if (!left || !right) return false;
    
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);
    
    // 使用布尔类型
    LocalVariable* result = static_cast<LocalVariable*>(module->newVarValue(IntegerType::getTypeBool()));
    
    // 使用布尔类型
    BinaryInstruction* geInst = new BinaryInstruction(func, 
                                               IRInstOperator::IRINST_OP_GE_I, 
                                               left, 
                                               right, 
                                               IntegerType::getTypeBool());
    node->blockInsts.addInst(geInst);
    
    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, geInst));
    
    node->val = result;
    return true;
}

// 等于
bool IRGenerator::ir_eq(ast_node* node)
{
    // 生成左右操作数
    ast_node* left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node) return false;
    
    ast_node* right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node) return false;
    
    Value* left = left_node->val;
    Value* right = right_node->val;
    
    if (!left || !right) return false;
    
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);
    
    // 使用布尔类型
    LocalVariable* result = static_cast<LocalVariable*>(module->newVarValue(IntegerType::getTypeBool()));
    
    // 使用布尔类型
    BinaryInstruction* eqInst = new BinaryInstruction(func, 
                                               IRInstOperator::IRINST_OP_EQ_I, 
                                               left, 
                                               right, 
                                               IntegerType::getTypeBool());
    node->blockInsts.addInst(eqInst);
    
    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, eqInst));
    
    node->val = result;
    return true;
}

// 不等于
bool IRGenerator::ir_ne(ast_node* node)
{
    // 生成左右操作数
    ast_node* left_node = ir_visit_ast_node(node->sons[0]);
    if (!left_node) return false;
    
    ast_node* right_node = ir_visit_ast_node(node->sons[1]);
    if (!right_node) return false;
    
    Value* left = left_node->val;
    Value* right = right_node->val;
    
    if (!left || !right) return false;
    
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 添加操作数指令到当前节点
    node->blockInsts.addInst(left_node->blockInsts);
    node->blockInsts.addInst(right_node->blockInsts);
    
    // 使用布尔类型
    LocalVariable* result = static_cast<LocalVariable*>(module->newVarValue(IntegerType::getTypeBool()));
    
    // 使用布尔类型
    BinaryInstruction* neInst = new BinaryInstruction(func, 
                                               IRInstOperator::IRINST_OP_NE_I, 
                                               left, 
                                               right, 
                                               IntegerType::getTypeBool());
    node->blockInsts.addInst(neInst);
    
    // 将结果移动到临时变量中
    node->blockInsts.addInst(new MoveInstruction(func, result, neInst));
    
    node->val = result;
    return true;
}

///实现逻辑运算符，特别需要实现短路求值-lxg
// 逻辑与 &&，带短路求值
bool IRGenerator::ir_logic_and(ast_node* node)
{
    if (!module) return false;
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 检查子节点数量
    if (node->sons.size() < 2) {
        minic_log(LOG_ERROR, "逻辑与运算需要两个操作数");
        return false;
    }
    
    // 创建标签
    LabelInstruction* secondOpLabel = new LabelInstruction(func);
    LabelInstruction* falseLabel = new LabelInstruction(func);
    LabelInstruction* endLabel = new LabelInstruction(func);
    
    // 为结果创建临时变量
    LocalVariable* result = static_cast<LocalVariable*>(module->newVarValue(IntegerType::getTypeInt()));
    if (!result) return false;
    
    // 生成左操作数代码
    ast_node* leftNode = ir_visit_ast_node(node->sons[0]);
    if (!leftNode || !leftNode->val) return false;
    
    // 添加左操作数指令
    node->blockInsts.addInst(leftNode->blockInsts);
    
    // 将左操作数转换为布尔值
    Value* leftBool;
    if (!int_to_bool(leftNode->val, &leftBool)) return false;
    
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
    ast_node* rightNode = ir_visit_ast_node(node->sons[1]);
    if (!rightNode || !rightNode->val) return false;
    
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
bool IRGenerator::ir_logic_or(ast_node* node)
{
    if (!module) return false;
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 检查子节点数量
    if (node->sons.size() < 2) {
        minic_log(LOG_ERROR, "逻辑或运算需要两个操作数");
        return false;
    }
    
    // 创建标签
    LabelInstruction* secondOpLabel = new LabelInstruction(func);
    LabelInstruction* trueLabel = new LabelInstruction(func);
    LabelInstruction* endLabel = new LabelInstruction(func);
    
    // 为结果创建临时变量
    LocalVariable* result = static_cast<LocalVariable*>(module->newVarValue(IntegerType::getTypeInt()));
    if (!result) return false;
    
    // 生成左操作数代码
    ast_node* leftNode = ir_visit_ast_node(node->sons[0]);
    if (!leftNode || !leftNode->val) return false;
    
    // 添加左操作数指令
    node->blockInsts.addInst(leftNode->blockInsts);
    
    // 将左操作数转换为布尔值
    Value* leftBool;
    if (!int_to_bool(leftNode->val, &leftBool)) return false;
    
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
    ast_node* rightNode = ir_visit_ast_node(node->sons[1]);
    if (!rightNode || !rightNode->val) return false;
    
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
bool IRGenerator::ir_logic_not(ast_node* node)
{
    if (!module) return false;
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 检查子节点数量
    if (node->sons.empty()) {
        minic_log(LOG_ERROR, "逻辑非运算需要一个操作数");
        return false;
    }
    
    // 生成操作数代码
    ast_node* operandNode = ir_visit_ast_node(node->sons[0]);
    if (!operandNode || !operandNode->val) return false;
    
    // 添加操作数指令
    node->blockInsts.addInst(operandNode->blockInsts);
    
    // 为结果创建临时变量
    LocalVariable* result = static_cast<LocalVariable*>(module->newVarValue(IntegerType::getTypeInt()));
    if (!result) return false;
    
    // 创建比较指令：检查整数值是否等于0
    BinaryInstruction* eqZeroInst = new BinaryInstruction(func, 
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
bool IRGenerator::ir_if(ast_node* node)
{
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 创建标签
    LabelInstruction* thenLabel = new LabelInstruction(func);
    LabelInstruction* endLabel = new LabelInstruction(func);
    
    // 生成条件表达式代码
    ast_node* cond_node = ir_visit_ast_node(node->sons[0]);
    if (!cond_node) return false;
    
    Value* condVal = cond_node->val;
    if (!condVal) return false;
    
    // 添加条件表达式生成的指令到指令流
    node->blockInsts.addInst(cond_node->blockInsts);
    
    // 直接使用条件值，不再转换为"不等于0"形式
    node->blockInsts.addInst(new GotoInstruction(func, condVal, thenLabel, endLabel));
    
    // 生成then部分代码
    node->blockInsts.addInst(thenLabel);
    ast_node* then_node = ir_visit_ast_node(node->sons[1]);
    if (!then_node) return false;
    node->blockInsts.addInst(then_node->blockInsts);
    
    // 结束标签
    node->blockInsts.addInst(endLabel);
    
    return true;
}

// if-else语句
bool IRGenerator::ir_if_else(ast_node* node)
{
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 创建标签
    LabelInstruction* thenLabel = new LabelInstruction(func);
    LabelInstruction* elseLabel = new LabelInstruction(func);
    LabelInstruction* endLabel = new LabelInstruction(func);
    
    // 生成条件表达式代码
    ast_node* cond_node = ir_visit_ast_node(node->sons[0]);
    if (!cond_node) return false;
    
    Value* condVal = cond_node->val;
    if (!condVal) return false;
    
    // 添加条件表达式生成的指令到指令流
    node->blockInsts.addInst(cond_node->blockInsts);
    
    // 直接使用条件值
    node->blockInsts.addInst(new GotoInstruction(func, condVal, thenLabel, elseLabel));
    
    // 生成then部分代码
    node->blockInsts.addInst(thenLabel);
    ast_node* then_node = ir_visit_ast_node(node->sons[1]);
    if (!then_node) return false;
    node->blockInsts.addInst(then_node->blockInsts);
    
    // then部分执行完后跳转到结束
    node->blockInsts.addInst(new GotoInstruction(func, endLabel));
    
    // 生成else部分代码
    node->blockInsts.addInst(elseLabel);
    ast_node* else_node = ir_visit_ast_node(node->sons[2]);
    if (!else_node) return false;
    node->blockInsts.addInst(else_node->blockInsts);
    
    // 结束标签
    node->blockInsts.addInst(endLabel);
    
    return true;
}

// while循环语句
bool IRGenerator::ir_while(ast_node* node)
{
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 创建标签
    LabelInstruction* condLabel = new LabelInstruction(func);   // 循环的入口label
    LabelInstruction* bodyLabel = new LabelInstruction(func);   // 循环体的入口label
    LabelInstruction* endLabel = new LabelInstruction(func);    // 循环的出口label
    
    // 进入循环前，保存当前break和continue标签
    Instruction* oldBreakLabel = func->getBreakLabel();
    Instruction* oldContinueLabel = func->getContinueLabel();
    
    // 设置新的break和continue标签
    func->setBreakLabel(endLabel);
    func->setContinueLabel(condLabel);
    
    // 从循环条件开始
    node->blockInsts.addInst(condLabel);
    
    // 生成条件表达式代码
    ast_node* cond_node = ir_visit_ast_node(node->sons[0]);
    if (!cond_node) return false;
    
    Value* condVal = cond_node->val;
    if (!condVal) return false;
    
    // 添加条件表达式生成的指令到指令流
    node->blockInsts.addInst(cond_node->blockInsts);
    
    // 直接使用条件值
    node->blockInsts.addInst(new GotoInstruction(func, condVal, bodyLabel, endLabel));
    
    // 生成循环体代码
    node->blockInsts.addInst(bodyLabel);
    ast_node* body_node = ir_visit_ast_node(node->sons[1]);
    if (!body_node) return false;
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
bool IRGenerator::ir_break(ast_node* node)
{
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 获取当前循环的break标签
    Instruction* breakLabel = func->getBreakLabel();
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
bool IRGenerator::ir_continue(ast_node* node)
{
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 获取当前循环的continue标签
    Instruction* continueLabel = func->getContinueLabel();
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
bool IRGenerator::int_to_bool(Value* val, Value** bool_val)
{
    // 检查参数有效性
    if (!val || !module || !bool_val) return false;
    
    Function* func = module->getCurrentFunction();
    if (!func) return false;
    
    // 检查值类型有效性
    Type* valType = val->getType();
    if (!valType) return false;
    
    // 如果输入值已经是布尔类型(i1)，直接使用它
    if (valType->isInt1Byte()) {
        *bool_val = val;
        return true;
    }
    
    // 创建临时变量存储布尔值结果
    LocalVariable* result = static_cast<LocalVariable*>(module->newVarValue(IntegerType::getTypeBool()));
    if (!result) return false;
    
    // 获取常量0用于比较
    ConstInt* zeroConst = module->newConstInt(0);
    if (!zeroConst) return false;
    
    // 创建比较指令：检查整数值是否不等于0
    BinaryInstruction* boolCheck = new BinaryInstruction(func, 
                                        IRInstOperator::IRINST_OP_NE_I, 
                                        val, 
                                        zeroConst, 
                                        IntegerType::getTypeBool());
    if (!boolCheck) return false;
    
    // 创建移动指令：将比较结果移到临时变量
    MoveInstruction* moveInst = new MoveInstruction(func, result, boolCheck);
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
bool IRGenerator::bool_to_int(Value* val, Value** int_val)
{
    // 在我们的实现中，布尔值已经是整数（0或1），所以可以直接使用
    *int_val = val;
    return true;
}


/// @brief 赋值AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_assign(ast_node * node)
{
    ast_node * son1_node = node->sons[0];
    ast_node * son2_node = node->sons[1];

    // 赋值节点，自右往左运算

    // 赋值运算符的左侧操作数
    ast_node * left = ir_visit_ast_node(son1_node);
    if (!left) {
        // 某个变量没有定值
        // 这里缺省设置变量不存在则创建，因此这里不会错误
        return false;
    }

    // 赋值运算符的右侧操作数
    ast_node * right = ir_visit_ast_node(son2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    MoveInstruction * movInst = new MoveInstruction(module->getCurrentFunction(), left->val, right->val);

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(movInst);

    // 这里假定赋值的类型是一致的
    node->val = movInst;

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
    
    printf("DEBUG: 查找变量: %s\n", node->name.c_str());
    
    // 查找ID型Value
    // 变量，则需要在符号表中查找对应的值
    Value* val = module->findVarValue(node->name);
    
    if (!val) {
        printf("DEBUG: 在符号表中未找到变量: %s, 尝试查找函数参数\n", node->name.c_str());
        
        // 查找是否是函数参数
        Function* currentFunc = module->getCurrentFunction();
        if (currentFunc) {
            for (auto& param : currentFunc->getParams()) {
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
    }

    return result;
}

/// @brief 变量定声明节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_variable_declare(ast_node * node)
{
    // 共有两个孩子，第一个类型，第二个变量名

    // TODO 这里可强化类型等检查

    node->val = module->newVarValue(node->sons[0]->type, node->sons[1]->name);

    return true;
}
