///
/// @file Antlr4CSTVisitor.h
/// @brief Antlr4的具体语法树的遍历产生AST
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

#include "AST.h"
#include "MiniCBaseVisitor.h"

/// @brief 遍历具体语法树产生抽象语法树
class MiniCCSTVisitor : public MiniCBaseVisitor {

public:
    /// @brief 构造函数
    MiniCCSTVisitor();

    /// @brief 析构函数
    virtual ~MiniCCSTVisitor();

    /// @brief 遍历CST产生AST
    /// @param root CST语法树的根结点
    /// @return AST的根节点
    ast_node * run(MiniCParser::CompileUnitContext * root);

protected:
    /* 下面的函数都是从MiniCBaseVisitor继承下来的虚拟函数，需要重载实现 */

    /// @brief 非终结运算符compileUnit的遍历
    /// @param ctx CST上下文
    /// @return AST的节点
    std::any visitCompileUnit(MiniCParser::CompileUnitContext * ctx) override;

    /// @brief 非终结运算符funcDef的遍历
    /// @param ctx CST上下文
    /// @return AST的节点
    std::any visitFuncDef(MiniCParser::FuncDefContext * ctx) override;

    /// @brief 非终结运算符block的遍历
    /// @param ctx CST上下文
    /// @return AST的节点
    std::any visitBlock(MiniCParser::BlockContext * ctx) override;

    /// @brief 非终结运算符blockItemList的遍历
    /// @param ctx CST上下文
    /// @return AST的节点
    std::any visitBlockItemList(MiniCParser::BlockItemListContext * ctx) override;

    /// @brief 非终结运算符blockItem的遍历
    /// @param ctx CST上下文
    /// @return AST的节点
    std::any visitBlockItem(MiniCParser::BlockItemContext * ctx) override;

    /// @brief 非终结运算符statement中的遍历
    /// @param ctx CST上下文
    /// @return AST的节点
    std::any visitStatement(MiniCParser::StatementContext * ctx);

    /// @brief 非终结运算符statement中的returnStatement的遍历
    /// @param ctx CST上下文
    /// @return AST的节点
    std::any visitReturnStatement(MiniCParser::ReturnStatementContext * ctx) override;

    /// @brief 非终结运算符expr的遍历
    /// @param ctx CST上下文
    /// @return AST的节点
    std::any visitExpr(MiniCParser::ExprContext * ctx) override;

    ///
    /// @brief 内部产生的非终结符assignStatement的分析
    /// @param ctx CST上下文
    /// @return std::any AST的节点
    ///
    std::any visitAssignStatement(MiniCParser::AssignStatementContext * ctx) override;

    ///
    /// @brief 内部产生的非终结符blockStatement的分析
    /// @param ctx CST上下文
    /// @return std::any AST的节点
    ///
    std::any visitBlockStatement(MiniCParser::BlockStatementContext * ctx) override;

    ///
    /// @brief 非终结符AddExp的分析
    /// @param ctx CST上下文
    /// @return std::any AST的节点
    ///
    std::any visitAddExp(MiniCParser::AddExpContext * ctx) override;

    ///
    /// @brief 非终结符addOp的分析
    /// @param ctx CST上下文
    /// @return std::any 类型
    ///
    std::any visitAddOp(MiniCParser::AddOpContext * ctx) override;

    ///
    /// @brief 非终结符unaryExp的分析
    /// @param ctx CST上下文
    /// @return std::any AST的节点
    ///
    std::any visitUnaryExp(MiniCParser::UnaryExpContext * ctx) override;

    ///
    /// @brief 非终结符PrimaryExp的分析
    /// @param ctx CST上下文
    /// @return std::any AST的节点
    ///
    std::any visitPrimaryExp(MiniCParser::PrimaryExpContext * ctx) override;

    ///
    /// @brief 非终结符LVal的分析
    /// @param ctx CST上下文
    /// @return std::any AST的节点
    ///
    std::any visitLVal(MiniCParser::LValContext * ctx) override;

    ///
    /// @brief 非终结符VarDecl的分析
    /// @param ctx CST上下文
    /// @return std::any AST的节点
    ///
    std::any visitVarDecl(MiniCParser::VarDeclContext * ctx) override;

    ///
    /// @brief 非终结符VarDecl的分析
    /// @param ctx CST上下文
    /// @return std::any AST的节点
    ///
    std::any visitVarDef(MiniCParser::VarDefContext * ctx) override;

    ///
    /// @brief 非终结符BasicType的分析
    /// @param ctx CST上下文
    /// @return std::any AST的节点
    ///
    std::any visitBasicType(MiniCParser::BasicTypeContext * ctx) override;

    ///
    /// @brief 非终结符RealParamList的分析
    /// @param ctx CST上下文
    /// @return std::any AST的节点
    ///
    std::any visitRealParamList(MiniCParser::RealParamListContext * ctx) override;

    ///
    /// @brief 非终结符ExpressionStatement的分析
    /// @param ctx CST上下文
    /// @return std::any AST的节点
    ///
    std::any visitExpressionStatement(MiniCParser::ExpressionStatementContext * context) override;

	///新增加visitMulDivExp和visitMulDivOp函数声明-lxg
	///
	/// @brief 非终结符MulDivExp的分析
	/// @param ctx CST上下文
	/// @return std::any AST的节点
	///
	std::any visitMulDivExp(MiniCParser::MulDivExpContext * ctx) override;

	///
	/// @brief 非终结符MulDivOp的分析
	/// @param ctx CST上下文
	/// @return std::any 类型
	///
	std::any visitMulDivOp(MiniCParser::MulDivOpContext * ctx) override;

	///新增关系表达式visitRelExp和visitRelOp函数声明-lxg
	///
	/// @brief 非终结符RelExp的分析
	/// @param ctx CST上下文
	/// @return std::any AST的节点
	///
	std::any visitRelExp(MiniCParser::RelExpContext * ctx) override;

	///新增逻辑表达式visitLorExp和visitLandExp函数声明
	///
	/// @brief 非终结符LorExp的分析
	/// @param ctx CST上下文
	/// @return std::any AST的节点
	///
	std::any visitLorExp(MiniCParser::LorExpContext * ctx) override;

	///
	/// @brief 非终结符LandExp的分析
	/// @param ctx CST上下文
	/// @return std::any AST的节点
	///
	std::any visitLandExp(MiniCParser::LandExpContext * ctx) override;

	///新增相等性表达式visitEqExp和visitEqOp函数声明
	/// @brief 非终结符EqExp的分析
	/// @param ctx CST上下文
	/// @return std::any AST的节点
	///
	std::any visitEqExp(MiniCParser::EqExpContext * ctx) override;

	///新增控制流语句visitIfStatement、visitWhileStatement、visitBreakStatement、visitContinueStatement函数声明
	///
	/// @brief 非终结符IfStatement的分析
	/// @param ctx CST上下文
	/// @return std::any AST的节点
	///
	std::any visitIfStatement(MiniCParser::IfStatementContext * ctx) override;

	///
	/// @brief 非终结符WhileStatement的分析
	/// @param ctx CST上下文
	/// @return std::any AST的节点
	///
	std::any visitWhileStatement(MiniCParser::WhileStatementContext * ctx) override;

	///
	/// @brief 非终结符BreakStatement的分析
	/// @param ctx CST上下文
	/// @return std::any AST的节点
	///
	std::any visitBreakStatement(MiniCParser::BreakStatementContext * ctx) override;

	///
	/// @brief 非终结符ContinueStatement的分析
	/// @param ctx CST上下文
	/// @return std::any AST的节点
	///
	std::any visitContinueStatement(MiniCParser::ContinueStatementContext * ctx) override;
	
	///添加对 paramList 和 param 的访问方法-lxg
	/// @brief 非终结运算符paramList的遍历
	/// @param ctx CST上下文
	/// @return AST的节点
	std::any visitParamList(MiniCParser::ParamListContext * ctx) override;

	/// @brief 非终结运算符param的遍历
	/// @param ctx CST上下文
	/// @return AST的节点
	std::any visitParam(MiniCParser::ParamContext * ctx) override;
};
