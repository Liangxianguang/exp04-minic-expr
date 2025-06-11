///
/// @file Antlr4CSTVisitor.cpp
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

#include <string>

#include "Antlr4CSTVisitor.h"
#include "AST.h"
#include "AttrType.h"

#define Instanceof(res, type, var) auto res = dynamic_cast<type>(var)

/// @brief 构造函数
MiniCCSTVisitor::MiniCCSTVisitor()
{}

/// @brief 析构函数
MiniCCSTVisitor::~MiniCCSTVisitor()
{}

/// @brief 遍历CST产生AST
/// @param root CST语法树的根结点
/// @return AST的根节点
ast_node * MiniCCSTVisitor::run(MiniCParser::CompileUnitContext * root)
{
    return std::any_cast<ast_node *>(visitCompileUnit(root));
}

/// @brief 非终结运算符compileUnit的遍历
/// @param ctx CST上下文
std::any MiniCCSTVisitor::visitCompileUnit(MiniCParser::CompileUnitContext * ctx)
{
    // compileUnit: (funcDef | varDecl)* EOF

    // 请注意这里必须先遍历全局变量后遍历函数。肯定可以确保全局变量先声明后使用的规则，但有些情况却不能检查出。
    // 事实上可能函数A后全局变量B后函数C，这时在函数A中是不能使用变量B的，需要报语义错误，但目前的处理不会。
    // 因此在进行语义检查时，可能追加检查行号和列号，如果函数的行号/列号在全局变量的行号/列号的前面则需要报语义错误
    // TODO 请追加实现。

    ast_node * temp_node;
    ast_node * compileUnitNode = create_contain_node(ast_operator_type::AST_OP_COMPILE_UNIT);

    // 可能多个变量，因此必须循环遍历
    for (auto varCtx: ctx->varDecl()) {

        // 变量函数定义
        temp_node = std::any_cast<ast_node *>(visitVarDecl(varCtx));
        (void) compileUnitNode->insert_son_node(temp_node);
    }

    // 可能有多个函数，因此必须循环遍历
    for (auto funcCtx: ctx->funcDef()) {

        // 变量函数定义
        temp_node = std::any_cast<ast_node *>(visitFuncDef(funcCtx));
        (void) compileUnitNode->insert_son_node(temp_node);
    }

    return compileUnitNode;
}

/// @brief 非终结运算符funcDef的遍历
/// @param ctx CST上下文
// std::any MiniCCSTVisitor::visitFuncDef(MiniCParser::FuncDefContext * ctx)
// {
//     // 识别的文法产生式：funcDef : T_INT T_ID T_L_PAREN T_R_PAREN block;

//     // 函数返回类型，终结符
//     type_attr funcReturnType{BasicType::TYPE_INT, (int64_t) ctx->T_INT()->getSymbol()->getLine()};

//     // 创建函数名的标识符终结符节点，终结符
//     char * id = strdup(ctx->T_ID()->getText().c_str());

//     var_id_attr funcId{id, (int64_t) ctx->T_ID()->getSymbol()->getLine()};

//     // // 形参结点目前没有，设置为空指针
//     // ast_node * formalParamsNode = nullptr;
// 	// 形参列表节点-lxg
//     ast_node * formalParamsNode = nullptr;
//     if (ctx->paramList()) {
//         formalParamsNode = std::any_cast<ast_node *>(visitParamList(ctx->paramList()));
//     } else {
//         // 如果没有参数，创建一个空的形参列表节点
//         formalParamsNode = new ast_node(ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS);
//     }
//     // 遍历block结点创建函数体节点，非终结符
//     auto blockNode = std::any_cast<ast_node *>(visitBlock(ctx->block()));

//     // 创建函数定义的节点，孩子有类型，函数名，语句块和形参(实际上无)
//     //
//     create_func_def函数内会释放funcId中指向的标识符空间，切记，之后不要再释放，之前一定要是通过strdup函数或者malloc分配的空间
//     return create_func_def(funcReturnType, funcId, blockNode, formalParamsNode);
// }
// 修改 visitFuncDef 方法,使其支持void类型-lxg
std::any MiniCCSTVisitor::visitFuncDef(MiniCParser::FuncDefContext * ctx)
{
    // 识别的文法产生式：funcDef : (T_INT|T_VOID) T_ID T_L_PAREN paramList? T_R_PAREN block;

    // 函数返回类型，终结符
    type_attr funcReturnType;
    if (ctx->T_INT()) {
        funcReturnType = {BasicType::TYPE_INT, (int64_t) ctx->T_INT()->getSymbol()->getLine()};
    } else if (ctx->T_VOID()) {
        funcReturnType = {BasicType::TYPE_VOID, (int64_t) ctx->T_VOID()->getSymbol()->getLine()};
    }

    // 创建函数名的标识符终结符节点，终结符
    char * id = strdup(ctx->T_ID()->getText().c_str());

    var_id_attr funcId{id, (int64_t) ctx->T_ID()->getSymbol()->getLine()};

    // 形参列表节点
    ast_node * formalParamsNode = nullptr;
    if (ctx->paramList()) {
        formalParamsNode = std::any_cast<ast_node *>(visitParamList(ctx->paramList()));
    } else {
        // 如果没有参数，创建一个空的形参列表节点
        formalParamsNode = new ast_node(ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS);
    }

    // 遍历block结点创建函数体节点，非终结符
    auto blockNode = std::any_cast<ast_node *>(visitBlock(ctx->block()));

    // 创建函数定义的节点，孩子有类型，函数名，语句块和形参
    return create_func_def(funcReturnType, funcId, blockNode, formalParamsNode);
}

/// @brief 非终结运算符block的遍历
/// @param ctx CST上下文
std::any MiniCCSTVisitor::visitBlock(MiniCParser::BlockContext * ctx)
{
    // 识别的文法产生式：block : T_L_BRACE blockItemList? T_R_BRACE';
    if (!ctx->blockItemList()) {
        // 语句块没有语句

        // 为了方便创建一个空的Block节点
        return create_contain_node(ast_operator_type::AST_OP_BLOCK);
    }

    // 语句块含有语句

    // 内部创建Block节点，并把语句加入，这里不需要创建Block节点
    return visitBlockItemList(ctx->blockItemList());
}

/// @brief 非终结运算符blockItemList的遍历
/// @param ctx CST上下文
std::any MiniCCSTVisitor::visitBlockItemList(MiniCParser::BlockItemListContext * ctx)
{
    // 识别的文法产生式：blockItemList : blockItem +;
    // 正闭包 循环 至少一个blockItem
    auto block_node = create_contain_node(ast_operator_type::AST_OP_BLOCK);

    for (auto blockItemCtx: ctx->blockItem()) {

        // 非终结符，需遍历
        auto blockItem = std::any_cast<ast_node *>(visitBlockItem(blockItemCtx));

        // 插入到块节点中
        (void) block_node->insert_son_node(blockItem);
    }

    return block_node;
}

///
/// @brief 非终结运算符blockItem的遍历
/// @param ctx CST上下文
///
std::any MiniCCSTVisitor::visitBlockItem(MiniCParser::BlockItemContext * ctx)
{
    // 识别的文法产生式：blockItem : statement | varDecl
    if (ctx->statement()) {
        // 语句识别

        return visitStatement(ctx->statement());
    } else if (ctx->varDecl()) {
        return visitVarDecl(ctx->varDecl());
    }

    return nullptr;
}

/// @brief 非终结运算符statement中的遍历
/// @param ctx CST上下文
std::any MiniCCSTVisitor::visitStatement(MiniCParser::StatementContext * ctx)
{
    // 识别的文法产生式：statement: T_ID T_ASSIGN expr T_SEMICOLON  # assignStatement
    // | T_RETURN expr T_SEMICOLON # returnStatement
    // | block  # blockStatement
    // | expr ? T_SEMICOLON #expressionStatement;
    // 识别的文法产生式：statement:
    // T_RETURN expr T_SEMICOLON                 # returnStatement
    // | lVal T_ASSIGN expr T_SEMICOLON          # assignStatement
    // | block                                   # blockStatement
    // | T_IF T_L_PAREN expr T_R_PAREN statement (T_ELSE statement)?  # ifStatement
    // | T_WHILE T_L_PAREN expr T_R_PAREN statement  # whileStatement
    // | T_BREAK T_SEMICOLON                     # breakStatement
    // | T_CONTINUE T_SEMICOLON                  # continueStatement
    // | expr? T_SEMICOLON                       # expressionStatement
    // ;
    if (Instanceof(assignCtx, MiniCParser::AssignStatementContext *, ctx)) {
        return visitAssignStatement(assignCtx);
    } else if (Instanceof(returnCtx, MiniCParser::ReturnStatementContext *, ctx)) {
        return visitReturnStatement(returnCtx);
    } else if (Instanceof(blockCtx, MiniCParser::BlockStatementContext *, ctx)) {
        return visitBlockStatement(blockCtx);
    } else if (Instanceof(exprCtx, MiniCParser::ExpressionStatementContext *, ctx)) {
        return visitExpressionStatement(exprCtx);
    } else if (Instanceof(ifCtx, MiniCParser::IfStatementContext *, ctx)) {
        return visitIfStatement(ifCtx);
    } else if (Instanceof(whileCtx, MiniCParser::WhileStatementContext *, ctx)) {
        return visitWhileStatement(whileCtx);
    } else if (Instanceof(breakCtx, MiniCParser::BreakStatementContext *, ctx)) {
        return visitBreakStatement(breakCtx);
    } else if (Instanceof(continueCtx, MiniCParser::ContinueStatementContext *, ctx)) {
        return visitContinueStatement(continueCtx);
    }

    return nullptr;
}

///
/// @brief 非终结运算符statement中的returnStatement的遍历
/// @param ctx CST上下文
///
std::any MiniCCSTVisitor::visitReturnStatement(MiniCParser::ReturnStatementContext * ctx)
{
    // // 识别的文法产生式：returnStatement -> T_RETURN expr T_SEMICOLON

    // // 非终结符，表达式expr遍历
    // auto exprNode = std::any_cast<ast_node *>(visitExpr(ctx->expr()));

    // // 创建返回节点，其孩子为Expr
    // return create_contain_node(ast_operator_type::AST_OP_RETURN, exprNode);
    // 识别的文法产生式：returnStatement -> T_RETURN expr? T_SEMICOLON -lxg
    // 检查是否有返回值表达式
    if (ctx->expr() != nullptr) {
        // 有返回值：return expression;
        // 非终结符，表达式expr遍历
        auto exprNode = std::any_cast<ast_node *>(visitExpr(ctx->expr()));

        // 创建返回节点，其孩子为Expr
        return create_contain_node(ast_operator_type::AST_OP_RETURN, exprNode);
    } else {
        // 无返回值：return;（void函数）
        // 创建返回节点，没有子节点
        return create_contain_node(ast_operator_type::AST_OP_RETURN);
    }
}

/// @brief 非终结运算符expr的遍历
/// @param ctx CST上下文
std::any MiniCCSTVisitor::visitExpr(MiniCParser::ExprContext * ctx)
{
    // 识别产生式：expr: addExp;
    // return visitAddExp(ctx->addExp());
    // 修改为: expr: lorExp;-lxg
    return visitLorExp(ctx->lorExp());
}

std::any MiniCCSTVisitor::visitAssignStatement(MiniCParser::AssignStatementContext * ctx)
{
    // 识别文法产生式：assignStatement: lVal T_ASSIGN expr T_SEMICOLON

    // 赋值左侧左值Lval遍历产生节点
    auto lvalNode = std::any_cast<ast_node *>(visitLVal(ctx->lVal()));

    // 赋值右侧expr遍历
    auto exprNode = std::any_cast<ast_node *>(visitExpr(ctx->expr()));

    // 创建一个AST_OP_ASSIGN类型的中间节点，孩子为Lval和Expr
    return ast_node::New(ast_operator_type::AST_OP_ASSIGN, lvalNode, exprNode, nullptr);
}

std::any MiniCCSTVisitor::visitBlockStatement(MiniCParser::BlockStatementContext * ctx)
{
    // 识别文法产生式 blockStatement: block

    return visitBlock(ctx->block());
}

//修改visitAddExp函数-lxg
std::any MiniCCSTVisitor::visitAddExp(MiniCParser::AddExpContext * ctx)
{
    // 识别的文法产生式：addExp : mulDivExp (addOp mulDivExp)*;

    if (ctx->addOp().empty()) {
        // 没有addOp运算符，则说明闭包识别为0，只识别了第一个非终结符mulDivExp
        return visitMulDivExp(ctx->mulDivExp()[0]);
    }

    ast_node *left, *right;

    // 存在addOp运算符
    auto opsCtxVec = ctx->addOp();

    // 有操作符，肯定会进循环，使得right设置正确的值
    for (int k = 0; k < (int) opsCtxVec.size(); k++) {
        // 获取运算符
        ast_operator_type op = std::any_cast<ast_operator_type>(visitAddOp(opsCtxVec[k]));

        if (k == 0) {
            // 左操作数
            left = std::any_cast<ast_node *>(visitMulDivExp(ctx->mulDivExp()[k]));
        }

        // 右操作数
        right = std::any_cast<ast_node *>(visitMulDivExp(ctx->mulDivExp()[k + 1]));

        // 新建结点作为下一个运算符的右操作符
        left = ast_node::New(op, left, right, nullptr);
    }

    return left;
}

/// @brief 非终结运算符addOp的遍历
/// @param ctx CST上下文
std::any MiniCCSTVisitor::visitAddOp(MiniCParser::AddOpContext * ctx)
{
    // 识别的文法产生式：addOp : T_ADD | T_SUB

    if (ctx->T_ADD()) {
        return ast_operator_type::AST_OP_ADD;
    } else {
        return ast_operator_type::AST_OP_SUB;
    }
}
//修改visitUnaryExp函数-lxg
std::any MiniCCSTVisitor::visitUnaryExp(MiniCParser::UnaryExpContext * ctx)
{
    // 识别文法产生式：unaryExp: primaryExp | T_ID T_L_PAREN realParamList? T_R_PAREN;
    // 识别文法产生式：unaryExp: T_SUB primaryExp | primaryExp | T_ID T_L_PAREN realParamList? T_R_PAREN;-lxg
    // 识别文法产生式：unaryExp: T_SUB unaryExp | primaryExp | T_ID T_L_PAREN realParamList? T_R_PAREN;-lxg
    // 识别文法产生式：unaryExp: T_SUB unaryExp | T_LOGIC_NOT unaryExp | primaryExp | T_ID T_L_PAREN realParamList?
    // T_R_PAREN;-lxg 添加对逻辑非的支持
    try {
        if (ctx->T_LOGIC_NOT()) {
            // 逻辑非表达式
            std::any result = visitUnaryExp(ctx->unaryExp());
            if (!result.has_value()) {
                // 提供默认节点
                digit_int_attr attr{0, -1};
                return ast_node::New(attr);
            }

            ast_node * unaryNode = std::any_cast<ast_node *>(result);
            if (!unaryNode || reinterpret_cast<intptr_t>(unaryNode) < 0x1000) {
                // 无效节点，提供默认值
                digit_int_attr attr{0, -1};
                unaryNode = ast_node::New(attr);
            }

            return ast_node::New(ast_operator_type::AST_OP_LOGIC_NOT, unaryNode, nullptr, nullptr);

        } else if (ctx->T_SUB()) {
            // 安全获取表达式
            std::any result = visitUnaryExp(ctx->unaryExp());
            ast_node * unaryNode = nullptr;

            if (result.has_value()) {
                try {
                    unaryNode = std::any_cast<ast_node *>(result);
                } catch (...) {
                    // 提供默认节点
                    digit_int_attr attr{0, -1};
                    unaryNode = ast_node::New(attr);
                }
            } else {
                // 提供默认节点
                digit_int_attr attr{0, -1};
                unaryNode = ast_node::New(attr);
            }

            // 确保节点有效
            if (!unaryNode || reinterpret_cast<intptr_t>(unaryNode) < 0x1000) {
                digit_int_attr attr{0, -1};
                unaryNode = ast_node::New(attr);
            }

            // 处理常量情况
            if (unaryNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
                unaryNode->integer_val = -((int32_t) unaryNode->integer_val);
                return unaryNode;
            }

            // 创建负号节点
            return ast_node::New(ast_operator_type::AST_OP_NEG, unaryNode, nullptr, nullptr);

        } else if (ctx->primaryExp()) {
            // 普通表达式
            return visitPrimaryExp(ctx->primaryExp());

        } else if (ctx->T_ID()) {
            // 函数调用 - 安全处理
            std::string funcName = ctx->T_ID()->getText();
            int64_t lineNo = ctx->T_ID()->getSymbol()->getLine();

            // 创建函数名节点
            ast_node * funcname_node = ast_node::New(funcName, lineNo);

            // 安全处理参数
            ast_node * paramListNode = nullptr;
            if (ctx->realParamList()) {
                try {
                    auto result = visitRealParamList(ctx->realParamList());
                    if (result.has_value()) {
                        paramListNode = std::any_cast<ast_node *>(result);
                    }
                } catch (...) {
                    // 参数处理失败，使用空参数列表
                    paramListNode = nullptr;
                }
            }

            // 创建函数调用
            return create_func_call(funcname_node, paramListNode);
        }

        // 默认情况：创建默认节点
        digit_int_attr attr{0, -1};
        return ast_node::New(attr);

    } catch (...) {
        // 任何异常情况：创建默认节点
        digit_int_attr attr{0, -1};
        return ast_node::New(attr);
    }
}

std::any MiniCCSTVisitor::visitPrimaryExp(MiniCParser::PrimaryExpContext * ctx)
{
    // 识别文法产生式 primaryExp: T_L_PAREN expr T_R_PAREN | T_DIGIT | lVal;

    ast_node * node = nullptr;

    // if (ctx->T_DIGIT()) {
    //     // 无符号整型字面量
    //     // 识别 primaryExp: T_DIGIT

    //     uint32_t val = (uint32_t) stoull(ctx->T_DIGIT()->getText());
    //     int64_t lineNo = (int64_t) ctx->T_DIGIT()->getSymbol()->getLine();
    //     node = ast_node::New(digit_int_attr{val, lineNo});
    // } else if (ctx->lVal()) {
    //     // 具有左值的表达式
    //     // 识别 primaryExp: lVal
    //     node = std::any_cast<ast_node *>(visitLVal(ctx->lVal()));
    // } else if (ctx->expr()) {
    //     // 带有括号的表达式
    //     // primaryExp: T_L_PAREN expr T_R_PAREN
    //     node = std::any_cast<ast_node *>(visitExpr(ctx->expr()));
    // }
    //修改visitPrimaryExp函数，可识别八进制、十六进制-lxg
    if (ctx->T_DIGIT()) {
        // 支持十进制、八进制、十六进制
        std::string text = ctx->T_DIGIT()->getText();
        uint32_t val = 0;
        if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
            val = std::stoul(text, nullptr, 16); // 十六进制
        } else if (text.size() > 1 && text[0] == '0') {
            val = std::stoul(text, nullptr, 8); // 八进制
        } else {
            val = std::stoul(text, nullptr, 10); // 十进制
        }
        int64_t lineNo = (int64_t) ctx->T_DIGIT()->getSymbol()->getLine();
        node = ast_node::New(digit_int_attr{val, lineNo});
    } else if (ctx->lVal()) {
        // 具有左值的表达式
        node = std::any_cast<ast_node *>(visitLVal(ctx->lVal()));
    } else if (ctx->expr()) {
        // 带有括号的表达式
        node = std::any_cast<ast_node *>(visitExpr(ctx->expr()));
    }

    return node;
}

// std::any MiniCCSTVisitor::visitLVal(MiniCParser::LValContext * ctx)
// {
//     // 识别文法产生式：lVal: T_ID;
//     // 获取ID的名字
//     auto varId = ctx->T_ID()->getText();

//     // 获取行号
//     int64_t lineNo = (int64_t) ctx->T_ID()->getSymbol()->getLine();

//     return ast_node::New(varId, lineNo);
// }

//修改 visitLVal 方法以支持数组访问-lxg
std::any MiniCCSTVisitor::visitLVal(MiniCParser::LValContext * ctx)
{
    // 语法规则：lVal: T_ID ('[' expr ']')*;

    // 获取数组名
    auto varId = ctx->T_ID()->getText();
    int64_t lineNo = (int64_t) ctx->T_ID()->getSymbol()->getLine();

    // 创建数组名节点
    ast_node * name_node = ast_node::New(varId, lineNo);

    // 检查是否有索引表达式
    if (ctx->expr().empty()) {
        // 如果没有索引，就是普通变量访问
        return name_node;
    } else {
        // 有索引，是数组访问
        std::vector<ast_node *> indices;

        // 收集所有索引表达式
        for (auto exprCtx: ctx->expr()) {
            ast_node * index = std::any_cast<ast_node *>(visitExpr(exprCtx));
            indices.push_back(index);
        }

        // 创建数组访问节点
        ast_node * array_access_node = create_array_access(name_node, indices);

        //关键修改：设置访问深度
        array_access_node->access_depth = indices.size();

        return array_access_node;
    }
}

// std::any MiniCCSTVisitor::visitVarDecl(MiniCParser::VarDeclContext * ctx)
// {
//     // varDecl: basicType varDef (T_COMMA varDef)* T_SEMICOLON;

//     // 声明语句节点
//     ast_node * stmt_node = create_contain_node(ast_operator_type::AST_OP_DECL_STMT);

//     // 类型节点
//     type_attr typeAttr = std::any_cast<type_attr>(visitBasicType(ctx->basicType()));

//     for (auto & varCtx: ctx->varDef()) {
//         // 变量名节点
//         ast_node * id_node = std::any_cast<ast_node *>(visitVarDef(varCtx));

//         // 创建类型节点
//         ast_node * type_node = create_type_node(typeAttr);

//         // 创建变量定义节点
//         ast_node * decl_node = ast_node::New(ast_operator_type::AST_OP_VAR_DECL, type_node, id_node, nullptr);

//         // 插入到变量声明语句
//         (void) stmt_node->insert_son_node(decl_node);
//     }

//     return stmt_node;
// }
// 修改 visitVarDecl 方法-lxg
std::any MiniCCSTVisitor::visitVarDecl(MiniCParser::VarDeclContext * ctx)
{
    // varDecl: basicType varDef (T_COMMA varDef)* T_SEMICOLON;

    // 声明语句节点
    ast_node * stmt_node = create_contain_node(ast_operator_type::AST_OP_DECL_STMT);

    // 类型节点
    type_attr typeAttr = std::any_cast<type_attr>(visitBasicType(ctx->basicType()));

    for (auto & varCtx: ctx->varDef()) {
        // 获取变量定义节点
        ast_node * var_node = std::any_cast<ast_node *>(visitVarDef(varCtx));

        // 创建类型节点
        ast_node * type_node = create_type_node(typeAttr);

        // 如果是带初始化的变量定义
        if (var_node->node_type == ast_operator_type::AST_OP_VAR_DEF_WITH_INIT) {
            // 提取变量名和初始化表达式
            ast_node * id_node = var_node->sons[0];
            ast_node * init_expr = var_node->sons[1];

            // 创建变量定义节点，包含三个子节点：类型、变量名、初始化表达式
            ast_node * decl_node =
                create_contain_node(ast_operator_type::AST_OP_VAR_DECL, type_node, id_node, init_expr);

            // 插入到变量声明语句
            (void) stmt_node->insert_son_node(decl_node);

            // 释放临时节点
            delete var_node;
        } else {
            // 普通变量定义，没有初始化表达式
            ast_node * decl_node = create_contain_node(ast_operator_type::AST_OP_VAR_DECL, type_node, var_node);

            // 插入到变量声明语句
            (void) stmt_node->insert_son_node(decl_node);
        }
    }

    return stmt_node;
}

std::any MiniCCSTVisitor::visitVarDef(MiniCParser::VarDefContext * ctx)
{
    // varDef: T_ID;
    // varDef: T_ID | T_ID T_ASSIGN expr;-lxg
    auto varId = ctx->T_ID()->getText();

    // 获取行号
    int64_t lineNo = (int64_t) ctx->T_ID()->getSymbol()->getLine();

    // 创建变量名节点
    ast_node * id_node = ast_node::New(varId, lineNo);

    // 检查是否是数组定义 - 需要有方括号才是数组
    // 通过直接检查方括号的数量来判断是否是数组定义
    // MiniC.g4中的数组语法是 T_ID ('[' expr ']')*
    // 所以我们需要检查'['和']'的数量
    size_t leftBracketCount = 0;
    for (auto token: ctx->children) {
        if (token->getText() == "[") {
            leftBracketCount++;
        }
    }

    if (leftBracketCount > 0) {
        // 这是数组定义，处理维度表达式
        std::vector<ast_node *> dimensions;

        // 收集维度表达式，每个维度占用两个表达式（[和]中间的表达式）
        for (size_t i = 0; i < leftBracketCount; i++) {
            ast_node * dimExpr = std::any_cast<ast_node *>(visitExpr(ctx->expr()[i]));
            dimensions.push_back(dimExpr);
        }

        // 检查是否有初始化表达式
        if (ctx->T_ASSIGN()) {
            // 获取初始化表达式
            ast_node * initExpr = std::any_cast<ast_node *>(visitExpr(ctx->expr()[leftBracketCount]));

            // 创建数组定义节点，包含初始化表达式
            return create_array_def(id_node, dimensions, initExpr);
        } else {
            // 创建数组定义节点，无初始化表达式
            return create_array_def(id_node, dimensions);
        }
    }

    // 处理普通变量定义（没有方括号）
    if (ctx->T_ASSIGN() && !ctx->expr().empty()) {
        // 获取初始化表达式节点
        ast_node * init_expr = std::any_cast<ast_node *>(visitExpr(ctx->expr()[0]));

        // 返回包含初始化表达式的节点
        return ast_node::New(ast_operator_type::AST_OP_VAR_DEF_WITH_INIT, id_node, init_expr, nullptr);
    }

    return id_node;
}

std::any MiniCCSTVisitor::visitBasicType(MiniCParser::BasicTypeContext * ctx)
{
    // basicType: T_INT;
    type_attr attr{BasicType::TYPE_VOID, -1};
    if (ctx->T_INT()) {
        attr.type = BasicType::TYPE_INT;
        attr.lineno = (int64_t) ctx->T_INT()->getSymbol()->getLine();
    }

    return attr;
}

std::any MiniCCSTVisitor::visitRealParamList(MiniCParser::RealParamListContext * ctx)
{
    // 识别的文法产生式：realParamList : expr (T_COMMA expr)*;
    try {
        auto paramListNode = create_contain_node(ast_operator_type::AST_OP_FUNC_REAL_PARAMS);

        for (auto paramCtx: ctx->expr()) {
            try {
                auto result = visitExpr(paramCtx);
                if (result.has_value()) {
                    auto paramNode = std::any_cast<ast_node *>(result);
                    if (paramNode) {
                        paramListNode->insert_son_node(paramNode);
                    } else {
                        // 参数为空，添加默认参数
                        digit_int_attr attr{0, (int64_t) paramCtx->getStart()->getLine()};
                        auto defaultParam = ast_node::New(attr);
                        paramListNode->insert_son_node(defaultParam);
                    }
                } else {
                    // 没有返回值，添加默认参数
                    digit_int_attr attr{0, (int64_t) paramCtx->getStart()->getLine()};
                    auto defaultParam = ast_node::New(attr);
                    paramListNode->insert_son_node(defaultParam);
                }
            } catch (...) {
                // 异常处理，添加默认参数
                digit_int_attr attr{0, -1};
                auto defaultParam = ast_node::New(attr);
                paramListNode->insert_son_node(defaultParam);
            }
        }

        return paramListNode;
    } catch (...) {
        // 如果发生异常，返回空参数列表
        return create_contain_node(ast_operator_type::AST_OP_FUNC_REAL_PARAMS);
    }
}

// std::any MiniCCSTVisitor::visitExpressionStatement(MiniCParser::ExpressionStatementContext * ctx)
// {
//     // 识别文法产生式  expr ? T_SEMICOLON #expressionStatement;
//     if (ctx->expr()) {
//         // 表达式语句

//         // 遍历expr非终结符，创建表达式节点后返回
//         return visitExpr(ctx->expr());
//     } else {
//         // 空语句

//         // 直接返回空指针，需要再把语句加入到语句块时要注意判断，空语句不要加入
//         return nullptr;
//     }
// }
//修改visitExpressionStatement函数，使其为空语句创建一个合法的AST节点，而不是返回nullptr-lxg
std::any MiniCCSTVisitor::visitExpressionStatement(MiniCParser::ExpressionStatementContext * ctx)
{
    // 识别文法产生式  expr ? T_SEMICOLON #expressionStatement;
    if (ctx->expr()) {
        // 表达式语句
        // 遍历expr非终结符，创建表达式节点后返回
        return visitExpr(ctx->expr());
    } else {
        // 空语句
        // 创建一个空语句节点，而不是返回nullptr
        int64_t lineNo = ctx->getStart()->getLine(); // 获取语句的行号

        // 创建空语句节点
        ast_node * emptyStmt = create_contain_node(ast_operator_type::AST_OP_EMPTY_STMT);
        emptyStmt->line_no = lineNo;

        return emptyStmt;
    }
}

//添加visitMulDivExp函数-lxg
std::any MiniCCSTVisitor::visitMulDivExp(MiniCParser::MulDivExpContext * ctx)
{
    // 识别的文法产生式：mulDivExp: unaryExp (mulDivOp unaryExp)*;

    if (ctx->mulDivOp().empty()) {
        // 没有mulDivOp运算符，则说明闭包识别为0，只识别了第一个非终结符unaryExp
        return visitUnaryExp(ctx->unaryExp()[0]);
    }

    ast_node *left, *right;

    // 存在mulDivOp运算符
    auto opsCtxVec = ctx->mulDivOp();

    // 有操作符，肯定会进循环，使得right设置正确的值
    for (int k = 0; k < (int) opsCtxVec.size(); k++) {
        // 获取运算符
        ast_operator_type op = std::any_cast<ast_operator_type>(visitMulDivOp(opsCtxVec[k]));

        if (k == 0) {
            // 左操作数
            left = std::any_cast<ast_node *>(visitUnaryExp(ctx->unaryExp()[k]));
        }

        // 右操作数
        right = std::any_cast<ast_node *>(visitUnaryExp(ctx->unaryExp()[k + 1]));

        // 新建结点作为下一个运算符的右操作符
        left = ast_node::New(op, left, right, nullptr);
    }

    return left;
}
//添加visitMulDivOp函数-lxg
std::any MiniCCSTVisitor::visitMulDivOp(MiniCParser::MulDivOpContext * ctx)
{
    if (ctx->T_MUL()) {
        return ast_operator_type::AST_OP_MUL;
    } else if (ctx->T_DIV()) {
        return ast_operator_type::AST_OP_DIV;
    } else if (ctx->T_MOD()) {
        return ast_operator_type::AST_OP_MOD;
    }
    return ast_operator_type::AST_OP_MAX;
}

//添加逻辑表达式的访问方法-lxg
std::any MiniCCSTVisitor::visitLorExp(MiniCParser::LorExpContext * ctx)
{
    // 识别的文法产生式：lorExp: landExp (T_LOGIC_OR landExp)*;

    try {
        if (ctx->T_LOGIC_OR().empty()) {
            // 没有逻辑或运算符，只有一个landExp
            auto result = visitLandExp(ctx->landExp()[0]);
            // 确保结果有值
            if (!result.has_value()) {
                // 返回默认节点而非null
                digit_int_attr attr{0, -1};
                return ast_node::New(attr);
            }
            return result;
        }

        // 获取第一个表达式作为基础
        auto firstExprResult = visitLandExp(ctx->landExp()[0]);
        ast_node * expr = nullptr;

        if (firstExprResult.has_value()) {
            try {
                expr = std::any_cast<ast_node *>(firstExprResult);
                if (!expr) {
                    // 无效节点，创建默认节点
                    digit_int_attr attr{0, -1};
                    expr = ast_node::New(attr);
                }
            } catch (...) {
                digit_int_attr attr{0, -1};
                expr = ast_node::New(attr);
            }
        } else {
            digit_int_attr attr{0, -1};
            expr = ast_node::New(attr);
        }

        // 逐个处理后续的逻辑或表达式
        for (size_t i = 0; i < ctx->T_LOGIC_OR().size(); i++) {
            // 获取右侧表达式
            auto rightExprResult = visitLandExp(ctx->landExp()[i + 1]);
            ast_node * rightExpr = nullptr;

            if (rightExprResult.has_value()) {
                try {
                    rightExpr = std::any_cast<ast_node *>(rightExprResult);
                    if (!rightExpr) {
                        // 无效节点，创建默认节点
                        digit_int_attr attr{0, -1};
                        rightExpr = ast_node::New(attr);
                    }
                } catch (...) {
                    digit_int_attr attr{0, -1};
                    rightExpr = ast_node::New(attr);
                }
            } else {
                digit_int_attr attr{0, -1};
                rightExpr = ast_node::New(attr);
            }

            // 创建逻辑或节点，使用标准的函数添加子节点
            ast_node * tempNode = create_contain_node(ast_operator_type::AST_OP_LOGIC_OR);

            // 使用标准的insert_son_node函数添加子节点
            tempNode->insert_son_node(expr);
            tempNode->insert_son_node(rightExpr);

            // 更新表达式节点为新创建的节点
            expr = tempNode;
        }

        return expr;
    } catch (...) {
        // 为任何未处理的异常提供安全的返回值
        digit_int_attr attr{0, -1};
        return ast_node::New(attr);
    }
}

//修改visitLandExp函数，增加更多安全检查-lxg
std::any MiniCCSTVisitor::visitLandExp(MiniCParser::LandExpContext * ctx)
{
    // 识别的文法产生式：landExp: eqExp (T_LOGIC_AND eqExp)*;

    try {
        if (ctx->T_LOGIC_AND().empty()) {
            // 没有逻辑与运算符，只有一个eqExp
            auto result = visitEqExp(ctx->eqExp()[0]);
            // 确保结果有值
            if (!result.has_value()) {
                // 返回默认节点而非null
                digit_int_attr attr{0, -1};
                return ast_node::New(attr);
            }
            return result;
        }

        // 获取第一个表达式作为基础
        auto firstExprResult = visitEqExp(ctx->eqExp()[0]);
        ast_node * expr = nullptr;

        if (firstExprResult.has_value()) {
            try {
                expr = std::any_cast<ast_node *>(firstExprResult);
                if (!expr) {
                    // 无效节点，创建默认节点
                    digit_int_attr attr{0, -1};
                    expr = ast_node::New(attr);
                }
            } catch (...) {
                digit_int_attr attr{0, -1};
                expr = ast_node::New(attr);
            }
        } else {
            digit_int_attr attr{0, -1};
            expr = ast_node::New(attr);
        }

        // 逐个处理后续的逻辑与表达式
        for (size_t i = 0; i < ctx->T_LOGIC_AND().size(); i++) {
            // 获取右侧表达式
            auto rightExprResult = visitEqExp(ctx->eqExp()[i + 1]);
            ast_node * rightExpr = nullptr;

            if (rightExprResult.has_value()) {
                try {
                    rightExpr = std::any_cast<ast_node *>(rightExprResult);
                    if (!rightExpr) {
                        // 无效节点，创建默认节点
                        digit_int_attr attr{0, -1};
                        rightExpr = ast_node::New(attr);
                    }
                } catch (...) {
                    digit_int_attr attr{0, -1};
                    rightExpr = ast_node::New(attr);
                }
            } else {
                digit_int_attr attr{0, -1};
                rightExpr = ast_node::New(attr);
            }

            // 创建逻辑与节点，使用标准的函数添加子节点
            ast_node * tempNode = create_contain_node(ast_operator_type::AST_OP_LOGIC_AND);

            // 使用标准的insert_son_node函数添加子节点
            tempNode->insert_son_node(expr);
            tempNode->insert_son_node(rightExpr);

            // 更新表达式节点为新创建的节点
            expr = tempNode;
        }

        return expr;
    } catch (...) {
        // 为任何未处理的异常提供安全的返回值
        digit_int_attr attr{0, -1};
        return ast_node::New(attr);
    }
}

//添加相等和关系表达式的访问方法-lxg
std::any MiniCCSTVisitor::visitEqExp(MiniCParser::EqExpContext * ctx)
{
    // 识别的文法产生式：eqExp: relExp ((T_EQ | T_NE) relExp)*;

    try {
        // 没有相等运算符时直接返回relExp
        if (ctx->T_EQ().empty() && ctx->T_NE().empty()) {
            auto result = visitRelExp(ctx->relExp()[0]);
            // 确保结果有值
            if (!result.has_value()) {
                digit_int_attr attr{0, -1};
                return ast_node::New(attr);
            }
            return result;
        }

        // 获取第一个关系表达式作为初始左操作数
        auto leftResult = visitRelExp(ctx->relExp()[0]);
        ast_node * left = nullptr;

        if (leftResult.has_value()) {
            try {
                left = std::any_cast<ast_node *>(leftResult);
                if (!left) {
                    digit_int_attr attr{0, -1};
                    left = ast_node::New(attr);
                }
            } catch (...) {
                digit_int_attr attr{0, -1};
                left = ast_node::New(attr);
            }
        } else {
            digit_int_attr attr{0, -1};
            left = ast_node::New(attr);
        }

        // 跟踪当前处理的操作符位置
        size_t eqPos = 0;
        size_t nePos = 0;

        // 处理所有后续的操作符和操作数
        for (size_t i = 1; i < ctx->relExp().size(); i++) {
            ast_operator_type op;

            // 确定使用哪个操作符
            if (eqPos < ctx->T_EQ().size() &&
                (nePos >= ctx->T_NE().size() ||
                 ctx->T_EQ()[eqPos]->getSymbol()->getTokenIndex() < ctx->T_NE()[nePos]->getSymbol()->getTokenIndex())) {
                op = ast_operator_type::AST_OP_EQ;
                eqPos++;
            } else {
                op = ast_operator_type::AST_OP_NE;
                nePos++;
            }

            // 获取右操作数
            auto rightResult = visitRelExp(ctx->relExp()[i]);
            ast_node * right = nullptr;

            if (rightResult.has_value()) {
                try {
                    right = std::any_cast<ast_node *>(rightResult);
                    if (!right) {
                        digit_int_attr attr{0, -1};
                        right = ast_node::New(attr);
                    }
                } catch (...) {
                    digit_int_attr attr{0, -1};
                    right = ast_node::New(attr);
                }
            } else {
                digit_int_attr attr{0, -1};
                right = ast_node::New(attr);
            }

            // 创建新的表达式节点
            left = ast_node::New(op, left, right, nullptr);
        }

        return left;
    } catch (...) {
        digit_int_attr attr{0, -1};
        return ast_node::New(attr);
    }
}

std::any MiniCCSTVisitor::visitRelExp(MiniCParser::RelExpContext * ctx)
{
    // 识别的文法产生式：relExp: addExp ((T_LT | T_GT | T_LE | T_GE) addExp)*;

    try {
        // 没有关系运算符时直接返回addExp
        if (ctx->T_LT().empty() && ctx->T_GT().empty() && ctx->T_LE().empty() && ctx->T_GE().empty()) {
            auto result = visitAddExp(ctx->addExp()[0]);
            if (!result.has_value()) {
                digit_int_attr attr{0, -1};
                return ast_node::New(attr);
            }
            return result;
        }

        // 获取第一个加法表达式作为初始左操作数
        auto leftResult = visitAddExp(ctx->addExp()[0]);
        ast_node * left = nullptr;

        if (leftResult.has_value()) {
            try {
                left = std::any_cast<ast_node *>(leftResult);
                if (!left) {
                    digit_int_attr attr{0, -1};
                    left = ast_node::New(attr);
                }
            } catch (...) {
                digit_int_attr attr{0, -1};
                left = ast_node::New(attr);
            }
        } else {
            digit_int_attr attr{0, -1};
            left = ast_node::New(attr);
        }

        // 跟踪操作符位置
        size_t ltPos = 0, gtPos = 0, lePos = 0, gePos = 0;

        // 处理所有后续操作符和操作数
        for (size_t i = 1; i < ctx->addExp().size(); i++) {
            ast_operator_type op;
            int minIndex = INT_MAX;
            int tokenIndex;

            // 找出最早出现的操作符
            if (ltPos < ctx->T_LT().size()) {
                tokenIndex = ctx->T_LT()[ltPos]->getSymbol()->getTokenIndex();
                if (tokenIndex < minIndex) {
                    minIndex = tokenIndex;
                    op = ast_operator_type::AST_OP_LT;
                }
            }

            if (gtPos < ctx->T_GT().size()) {
                tokenIndex = ctx->T_GT()[gtPos]->getSymbol()->getTokenIndex();
                if (tokenIndex < minIndex) {
                    minIndex = tokenIndex;
                    op = ast_operator_type::AST_OP_GT;
                }
            }

            if (lePos < ctx->T_LE().size()) {
                tokenIndex = ctx->T_LE()[lePos]->getSymbol()->getTokenIndex();
                if (tokenIndex < minIndex) {
                    minIndex = tokenIndex;
                    op = ast_operator_type::AST_OP_LE;
                }
            }

            if (gePos < ctx->T_GE().size()) {
                tokenIndex = ctx->T_GE()[gePos]->getSymbol()->getTokenIndex();
                if (tokenIndex < minIndex) {
                    minIndex = tokenIndex;
                    op = ast_operator_type::AST_OP_GE;
                }
            }

            // 更新操作符位置计数器
            if (op == ast_operator_type::AST_OP_LT)
                ltPos++;
            else if (op == ast_operator_type::AST_OP_GT)
                gtPos++;
            else if (op == ast_operator_type::AST_OP_LE)
                lePos++;
            else if (op == ast_operator_type::AST_OP_GE)
                gePos++;

            // 获取右操作数
            auto rightResult = visitAddExp(ctx->addExp()[i]);
            ast_node * right = nullptr;

            if (rightResult.has_value()) {
                try {
                    right = std::any_cast<ast_node *>(rightResult);
                    if (!right) {
                        digit_int_attr attr{0, -1};
                        right = ast_node::New(attr);
                    }
                } catch (...) {
                    digit_int_attr attr{0, -1};
                    right = ast_node::New(attr);
                }
            } else {
                digit_int_attr attr{0, -1};
                right = ast_node::New(attr);
            }

            // 创建新的表达式节点
            left = ast_node::New(op, left, right, nullptr);
        }

        return left;
    } catch (...) {
        digit_int_attr attr{0, -1};
        return ast_node::New(attr);
    }
}

//添加控制流语句的访问方法
std::any MiniCCSTVisitor::visitIfStatement(MiniCParser::IfStatementContext * ctx)
{
    // 处理if语句
    // 语法: T_IF T_L_PAREN expr T_R_PAREN statement (T_ELSE statement)?

    // 条件表达式
    ast_node * condExpr = nullptr;
    try {
        auto result = visitExpr(ctx->expr());
        if (result.has_value()) {
            condExpr = std::any_cast<ast_node *>(result);

            // 健壮性检查 - 确保condExpr指针有效
            if (!condExpr || reinterpret_cast<intptr_t>(condExpr) < 0x1000) {
                // 创建默认条件表达式
                condExpr = ast_node::New(ast_operator_type::AST_OP_LEAF_LITERAL_UINT);
                condExpr->integer_val = 1; // 设为true
            }
        } else {
            // 创建默认条件表达式
            condExpr = ast_node::New(ast_operator_type::AST_OP_LEAF_LITERAL_UINT);
            condExpr->integer_val = 1; // 设为true
        }
    } catch (const std::exception & e) {
        // 创建默认条件表达式
        condExpr = ast_node::New(ast_operator_type::AST_OP_LEAF_LITERAL_UINT);
        condExpr->integer_val = 1; // 设为true
    }

    // if语句体
    ast_node * thenStmt = nullptr;
    try {
        auto result = visitStatement(ctx->statement(0));
        if (result.has_value()) {
            thenStmt = std::any_cast<ast_node *>(result);
            // 验证语句节点的有效性
            if (!thenStmt || reinterpret_cast<intptr_t>(thenStmt) < 0x1000) {
                thenStmt = ast_node::New(ast_operator_type::AST_OP_BLOCK);
            }
        } else {
            // 空语句创建空块
            thenStmt = ast_node::New(ast_operator_type::AST_OP_BLOCK);
        }
    } catch (const std::exception & e) {
        // 创建空块作为默认语句体
        thenStmt = ast_node::New(ast_operator_type::AST_OP_BLOCK);
    }

    if (ctx->T_ELSE()) {
        // if-else语句
        ast_node * elseStmt = nullptr;
        try {
            auto result = visitStatement(ctx->statement(1));
            if (result.has_value()) {
                elseStmt = std::any_cast<ast_node *>(result);
                // 验证语句节点的有效性
                if (!elseStmt || reinterpret_cast<intptr_t>(elseStmt) < 0x1000) {
                    elseStmt = ast_node::New(ast_operator_type::AST_OP_BLOCK);
                }
            } else {
                elseStmt = ast_node::New(ast_operator_type::AST_OP_BLOCK);
            }
        } catch (const std::exception & e) {
            elseStmt = ast_node::New(ast_operator_type::AST_OP_BLOCK);
        }

        // 创建if-else节点，确保所有子节点有效
        ast_node * ifElseNode = nullptr;
        try {
            ifElseNode = ast_node::New(ast_operator_type::AST_OP_IF_ELSE, condExpr, thenStmt, elseStmt, nullptr);
        } catch (...) {
            // 发生异常，创建默认节点
            ifElseNode = ast_node::New(ast_operator_type::AST_OP_BLOCK);
        }
        return ifElseNode;
    } else {
        // 单独的if语句
        ast_node * ifNode = nullptr;
        try {
            ifNode = ast_node::New(ast_operator_type::AST_OP_IF, condExpr, thenStmt, nullptr);
        } catch (...) {
            // 发生异常，创建默认节点
            ifNode = ast_node::New(ast_operator_type::AST_OP_BLOCK);
        }
        return ifNode;
    }
}

std::any MiniCCSTVisitor::visitWhileStatement(MiniCParser::WhileStatementContext * ctx)
{
    // 处理while循环语句
    // 语法: T_WHILE T_L_PAREN expr T_R_PAREN statement

    // 条件表达式
    auto condExpr = std::any_cast<ast_node *>(visitExpr(ctx->expr()));

    // 循环体
    auto bodyStmt = std::any_cast<ast_node *>(visitStatement(ctx->statement()));

    return ast_node::New(ast_operator_type::AST_OP_WHILE, condExpr, bodyStmt, nullptr);
}

std::any MiniCCSTVisitor::visitBreakStatement(MiniCParser::BreakStatementContext * ctx)
{
    // 处理break语句
    // 语法: T_BREAK T_SEMICOLON
    return ast_node::New(ast_operator_type::AST_OP_BREAK, nullptr);
}

std::any MiniCCSTVisitor::visitContinueStatement(MiniCParser::ContinueStatementContext * ctx)
{
    // 处理continue语句
    // 语法: T_CONTINUE T_SEMICOLON
    return ast_node::New(ast_operator_type::AST_OP_CONTINUE, nullptr);
}

///实现visitParamList和visitParam方法-lxg
std::any MiniCCSTVisitor::visitParamList(MiniCParser::ParamListContext * ctx)
{
    // 创建形参列表节点
    ast_node * paramsNode = new ast_node(ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS);

    // 遍历所有参数
    for (auto paramCtx: ctx->param()) {
        // 处理每个参数
        ast_node * paramNode = std::any_cast<ast_node *>(visitParam(paramCtx));
        paramsNode->insert_son_node(paramNode);
    }

    return paramsNode;
}

std::any MiniCCSTVisitor::visitParam(MiniCParser::ParamContext * ctx)
{
    // 获取参数类型
    type_attr paramType{BasicType::TYPE_INT, (int64_t) ctx->T_INT()->getSymbol()->getLine()};

    // 创建类型节点
    ast_node * typeNode = create_type_node(paramType);

    // 获取参数名称
    std::string paramName = ctx->T_ID()->getText();
    int64_t lineno = (int64_t) ctx->T_ID()->getSymbol()->getLine();

    // 创建名称节点
    ast_node * nameNode = ast_node::New(paramName, lineno);

    // **关键修改：正确提取维度信息**
    printf("DEBUG: 处理参数 %s，子节点数量: %zu\n", paramName.c_str(), ctx->children.size());

    // 检查是否有T_DIGIT节点（维度值）
    std::vector<std::string> digits;
    for (auto digitNode: ctx->T_DIGIT()) {
        digits.push_back(digitNode->getText());
        // printf("DEBUG: 找到维度值: %s\n", digitNode->getText().c_str());
    }

    // 计算方括号数量
    int arrayDimCount = 0;
    for (auto child: ctx->children) {
        if (child->getText() == "[") {
            arrayDimCount++;
        }
    }

    printf("DEBUG: 参数 %s 方括号数量: %d，维度值数量: %zu\n", paramName.c_str(), arrayDimCount, digits.size());

    if (arrayDimCount > 0) {
        // 数组参数
        ast_node * paramNode = new ast_node(ast_operator_type::AST_OP_FUNC_FORMAL_PARAM_ARRAY);
        paramNode->insert_son_node(typeNode);
        paramNode->insert_son_node(nameNode);

        // **关键：添加维度信息**
        if (digits.empty()) {
            // 情况：int a[] 或 int a[][2][3] （第一维为空）
            // 第一维总是0（表示指针）
            ast_node * firstDimNode = ast_node::New(digit_int_attr{0, lineno});
            paramNode->insert_son_node(firstDimNode);
            printf("DEBUG: 添加第一维（空）: 0\n");

            // 如果还有其他方括号但没有数字，可能都是空的
            for (int i = 1; i < arrayDimCount; i++) {
                // 这种情况在语法上不太可能，但为了安全处理
                ast_node * dimNode = ast_node::New(digit_int_attr{0, lineno});
                paramNode->insert_son_node(dimNode);
                printf("DEBUG: 添加额外空维度: 0\n");
            }
        } else {
            // 情况：int a[][2] 或 int a[][2][3] （有具体维度值）
            // 第一维总是0（空的）
            ast_node * firstDimNode = ast_node::New(digit_int_attr{0, lineno});
            paramNode->insert_son_node(firstDimNode);
            printf("DEBUG: 添加第一维（空）: 0\n");

            // 添加后续的具体维度
            for (const std::string & dimStr: digits) {
                uint32_t dimValue = std::stoul(dimStr);
                ast_node * dimNode = ast_node::New(digit_int_attr{dimValue, lineno});
                paramNode->insert_son_node(dimNode);
                // printf("DEBUG: 添加具体维度: %u\n", dimValue);
            }
        }

        printf("DEBUG: 数组参数 %s 总共有 %zu 个子节点\n", paramName.c_str(), paramNode->sons.size());

        return paramNode;
    } else {
        // 普通参数
        printf("DEBUG: 普通参数: %s\n", paramName.c_str());
        ast_node * paramNode = new ast_node(ast_operator_type::AST_OP_FUNC_FORMAL_PARAM);
        paramNode->insert_son_node(typeNode);
        paramNode->insert_son_node(nameNode);
        return paramNode;
    }
}