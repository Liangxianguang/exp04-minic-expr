///
/// @file IRGenerator.h
/// @brief AST遍历产生线性IR的头文件
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
#pragma once

#include <unordered_map>
#include <map>
#include <vector>
#include <string>

#include "AST.h"
#include "Module.h"

/// @brief AST遍历产生线性IR类
class IRGenerator {

public:
    /// @brief 构造函数
    /// @param root
    /// @param _module
    IRGenerator(ast_node * root, Module * _module);

    /// @brief 析构函数
    ~IRGenerator() = default;

    /// @brief 运行产生IR
    bool run();

    void setLastError(const std::string & error)
    {
        lastError = error;
    }
    std::string getLastError() const
    {
        return lastError;
    }

protected:
    /// @brief 编译单元AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_compile_unit(ast_node * node);

    /// @brief 函数定义AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_function_define(ast_node * node);

    /// @brief 形式参数AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_function_formal_params(ast_node * node);

    /// @brief 函数调用AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_function_call(ast_node * node);

    /// @brief 语句块（含函数体）AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_block(ast_node * node);

    /// @brief 整数加法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_add(ast_node * node);

    /// @brief 整数减法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_sub(ast_node * node);

    ///添加三个新的函数声明ir-mul,ir-div和ir-mod  -lxg
    /// @brief 整数乘法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_mul(ast_node * node);

    /// @brief 整数除法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_div(ast_node * node);

    /// @brief 整数求余AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_mod(ast_node * node);

    /// @brief 一元负号AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_neg(ast_node * node);

    /// @brief 赋值AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败

    /// 关系运算符
    bool ir_lt(ast_node * node);
    bool ir_gt(ast_node * node);
    bool ir_le(ast_node * node);
    bool ir_ge(ast_node * node);
    bool ir_eq(ast_node * node);
    bool ir_ne(ast_node * node);

    /// 逻辑运算符
    bool ir_logic_and(ast_node * node);
    bool ir_logic_or(ast_node * node);
    bool ir_logic_not(ast_node * node);

    /// 控制流语句
    bool ir_if(ast_node * node);
    bool ir_if_else(ast_node * node);
    bool ir_while(ast_node * node);
    bool ir_break(ast_node * node);
    bool ir_continue(ast_node * node);

    /// 辅助函数
    /// 生成条件跳转代码
    Value * generateConditionCode(ast_node * node, Instruction * trueLabel, Instruction * falseLabel);
    /// 整数转布尔值
    bool int_to_bool(Value * val, Value ** bool_val);
    /// 布尔值转整数
    bool bool_to_int(Value * val, Value ** int_val);

    bool ir_assign(ast_node * node);

    // 添加数组相关方法的声明-lxg
    /// @brief 数组定义节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_array_def(ast_node * node);

    /// @brief 数组访问节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_array_access(ast_node * node);

    /// @brief 空语句节点翻译成线性中间IR-lxg
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_empty_stmt(ast_node * node);

    /// @brief 函数数组形参AST节点翻译成线性中间IR-lxg
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_function_formal_param_array(ast_node * node);

    /// @brief 检查变量是否是当前函数的参数-lxg
    /// @param varName 变量名
    /// @return 是否是函数参数
    bool isCurrentFunctionParameter(const std::string & varName);

    /// @brief 计算数组访问的深度
    /// @param node 数组访问节点
    /// @return 访问深度
    int calculateArrayAccessDepth(ast_node * node);

    /// @brief 根据访问深度计算行大小
    /// @param dimensions 数组维度
    /// @param accessDepth 访问深度
    /// @return 行大小
    int calculateRowSize(const std::vector<int> & dimensions, int accessDepth);

    /// @brief 计算线性偏移量（处理多维索引）
    /// @param node 数组访问节点
    /// @return 线性偏移量
    Value * calculateLinearOffset(ast_node * node, InterCode & blockInsts);

    /// @brief 根据形参类型计算参数传递的偏移量-lxg
    /// @param arrayAccessNode 数组访问节点
    /// @param paramDimensions 形参的维度信息（第一维为0）
    /// @return 字节偏移量
    Value * calculateParameterOffset(ast_node * arrayAccessNode,
                                     const std::vector<int> & paramDimensions,
                                     InterCode & blockInsts);

    /// @brief 计算数组访问的字节偏移量
    /// @param arrayAccessNode 数组访问节点
    /// @param blockInsts 指令容器
    /// @return 字节偏移量
    Value * calculateArrayAccessOffset(ast_node * arrayAccessNode, InterCode & blockInsts);

    /// @brief 处理简单指针参数访问 (int* 类型)
    bool handleSimplePointerParamAccess(ast_node * node, Value * arrayVar);

    /// @brief 处理多维数组参数访问 (int(*)[2][2]... 类型)
    bool handleMultiDimArrayParamAccess(ast_node * node, Value * arrayVar, const std::vector<int> & dimensions);

    /// @brief 处理普通数组访问（保持原有逻辑）
    bool handleRegularArrayAccess(ast_node * node, Value * arrayVar, const std::vector<int> & dimensions);

    /// @brief 处理数组访问维度信息
    bool
    handleParameterArrayAccessWithDimensions(ast_node * node, Value * arrayVar, const std::vector<int> & dimensions);

    /// @brief return节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_return(ast_node * node);

    /// @brief 类型叶子节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_leaf_node_type(ast_node * node);

    /// @brief 标识符叶子节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_leaf_node_var_id(ast_node * node);

    /// @brief 无符号整数字面量叶子节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_leaf_node_uint(ast_node * node);

    /// @brief float数字面量叶子节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_leaf_node_float(ast_node * node);

    /// @brief 变量声明语句节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_declare_statment(ast_node * node);

    /// @brief 变量定声明节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_variable_declare(ast_node * node);

    /// @brief 未知节点类型的节点处理
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_default(ast_node * node);

    /// @brief 根据AST的节点运算符查找对应的翻译函数并执行翻译动作
    /// @param node AST节点
    /// @return 成功返回node节点，否则返回nullptr
    ast_node * ir_visit_ast_node(ast_node * node);

    /// @brief AST的节点操作函数
    typedef bool (IRGenerator::*ast2ir_handler_t)(ast_node *);

    /// @brief AST节点运算符与动作函数关联的映射表
    std::unordered_map<ast_operator_type, ast2ir_handler_t> ast2ir_handlers;

private:
    /// @brief 抽象语法树的根
    ast_node * root;

    /// @brief 符号表:模块
    Module * module;
    std::string lastError;
    /// @brief 全局变量初始值映射-lxg
    std::unordered_map<std::string, int> globalVarInitValues;
    // 保存函数参数的原始维度信息-lxg
    std::map<std::string, std::map<int, std::vector<int>>> functionParameterDimensions;
};
