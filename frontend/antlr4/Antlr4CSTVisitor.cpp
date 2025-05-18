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
std::any MiniCCSTVisitor::visitFuncDef(MiniCParser::FuncDefContext * ctx)
{
    // 识别的文法产生式：funcDef : T_INT T_ID T_L_PAREN T_R_PAREN block;

    // 函数返回类型，终结符
    type_attr funcReturnType{BasicType::TYPE_INT, (int64_t) ctx->T_INT()->getSymbol()->getLine()};

    // 创建函数名的标识符终结符节点，终结符
    char * id = strdup(ctx->T_ID()->getText().c_str());

    var_id_attr funcId{id, (int64_t) ctx->T_ID()->getSymbol()->getLine()};

    // // 形参结点目前没有，设置为空指针
    // ast_node * formalParamsNode = nullptr;
	// 形参列表节点-lxg
    ast_node * formalParamsNode = nullptr;
    if (ctx->paramList()) {
        formalParamsNode = std::any_cast<ast_node *>(visitParamList(ctx->paramList()));
    } else {
        // 如果没有参数，创建一个空的形参列表节点
        formalParamsNode = new ast_node(ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS);
    }
    // 遍历block结点创建函数体节点，非终结符
    auto blockNode = std::any_cast<ast_node *>(visitBlock(ctx->block()));

    // 创建函数定义的节点，孩子有类型，函数名，语句块和形参(实际上无)
    // create_func_def函数内会释放funcId中指向的标识符空间，切记，之后不要再释放，之前一定要是通过strdup函数或者malloc分配的空间
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
    // 识别的文法产生式：returnStatement -> T_RETURN expr T_SEMICOLON

    // 非终结符，表达式expr遍历
    auto exprNode = std::any_cast<ast_node *>(visitExpr(ctx->expr()));

    // 创建返回节点，其孩子为Expr
    return create_contain_node(ast_operator_type::AST_OP_RETURN, exprNode);
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
    // 识别文法产生式：unaryExp: T_SUB unaryExp | T_LOGIC_NOT unaryExp | primaryExp | T_ID T_L_PAREN realParamList? T_R_PAREN;-lxg
	// 添加对逻辑非的支持
    if (ctx->T_LOGIC_NOT()) {
        // 逻辑非表达式
        auto unaryNode = std::any_cast<ast_node *>(visitUnaryExp(ctx->unaryExp()));
        return ast_node::New(ast_operator_type::AST_OP_LOGIC_NOT, unaryNode, nullptr, nullptr);
    } else if (ctx->T_SUB()) {
        // 负号表达式 - 注意这里改为递归处理unaryExp
        auto unaryNode = std::any_cast<ast_node *>(visitUnaryExp(ctx->unaryExp()));
        
        // 如果unaryNode是常量，直接取负
        if (unaryNode && unaryNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
            unaryNode->integer_val = -((int32_t)unaryNode->integer_val);
            return unaryNode;
        }
        // 否则保留一元负号节点
        return ast_node::New(ast_operator_type::AST_OP_NEG, unaryNode, nullptr, nullptr);
    } else if (ctx->primaryExp()) {
        // 普通表达式
        return visitPrimaryExp(ctx->primaryExp());
    } else if (ctx->T_ID()) {
        // 创建函数调用名终结符节点
        ast_node * funcname_node = ast_node::New(ctx->T_ID()->getText(), (int64_t) ctx->T_ID()->getSymbol()->getLine());

        // 实参列表
        ast_node * paramListNode = nullptr;

        // 函数调用
        if (ctx->realParamList()) {
            // 有参数
            paramListNode = std::any_cast<ast_node *>(visitRealParamList(ctx->realParamList()));
        }

        // 创建函数调用节点，其孩子为被调用函数名和实参
        return create_func_call(funcname_node, paramListNode);
    } else {
        return nullptr;
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
            val = std::stoul(text, nullptr, 8);  // 八进制
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

std::any MiniCCSTVisitor::visitLVal(MiniCParser::LValContext * ctx)
{
    // 识别文法产生式：lVal: T_ID;
    // 获取ID的名字
    auto varId = ctx->T_ID()->getText();

    // 获取行号
    int64_t lineNo = (int64_t) ctx->T_ID()->getSymbol()->getLine();

    return ast_node::New(varId, lineNo);
}

std::any MiniCCSTVisitor::visitVarDecl(MiniCParser::VarDeclContext * ctx)
{
    // varDecl: basicType varDef (T_COMMA varDef)* T_SEMICOLON;

    // 声明语句节点
    ast_node * stmt_node = create_contain_node(ast_operator_type::AST_OP_DECL_STMT);

    // 类型节点
    type_attr typeAttr = std::any_cast<type_attr>(visitBasicType(ctx->basicType()));

    for (auto & varCtx: ctx->varDef()) {
        // 变量名节点
        ast_node * id_node = std::any_cast<ast_node *>(visitVarDef(varCtx));

        // 创建类型节点
        ast_node * type_node = create_type_node(typeAttr);

        // 创建变量定义节点
        ast_node * decl_node = ast_node::New(ast_operator_type::AST_OP_VAR_DECL, type_node, id_node, nullptr);

        // 插入到变量声明语句
        (void) stmt_node->insert_son_node(decl_node);
    }

    return stmt_node;
}

std::any MiniCCSTVisitor::visitVarDef(MiniCParser::VarDefContext * ctx)
{
    // varDef: T_ID;

    auto varId = ctx->T_ID()->getText();

    // 获取行号
    int64_t lineNo = (int64_t) ctx->T_ID()->getSymbol()->getLine();

    return ast_node::New(varId, lineNo);
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

    auto paramListNode = create_contain_node(ast_operator_type::AST_OP_FUNC_REAL_PARAMS);

    for (auto paramCtx: ctx->expr()) {

        auto paramNode = std::any_cast<ast_node *>(visitExpr(paramCtx));

        paramListNode->insert_son_node(paramNode);
    }

    return paramListNode;
}

std::any MiniCCSTVisitor::visitExpressionStatement(MiniCParser::ExpressionStatementContext * ctx)
{
    // 识别文法产生式  expr ? T_SEMICOLON #expressionStatement;
    if (ctx->expr()) {
        // 表达式语句

        // 遍历expr非终结符，创建表达式节点后返回
        return visitExpr(ctx->expr());
    } else {
        // 空语句

        // 直接返回空指针，需要再把语句加入到语句块时要注意判断，空语句不要加入
        return nullptr;
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
    
    if (ctx->T_LOGIC_OR().empty()) {
        // 没有逻辑或运算符，只有一个landExp
        return visitLandExp(ctx->landExp()[0]);
    }

    ast_node *left, *right;
    
    // 存在逻辑或运算符
    auto orOps = ctx->T_LOGIC_OR();
    
    for (int k = 0; k < (int)orOps.size(); k++) {
        if (k == 0) {
            // 第一个左操作数
            left = std::any_cast<ast_node *>(visitLandExp(ctx->landExp()[k]));
        }
        
        // 右操作数
        right = std::any_cast<ast_node *>(visitLandExp(ctx->landExp()[k + 1]));
        
        // 创建逻辑或节点
        left = ast_node::New(ast_operator_type::AST_OP_LOGIC_OR, left, right, nullptr);
    }
    
    return left;
}

std::any MiniCCSTVisitor::visitLandExp(MiniCParser::LandExpContext * ctx)
{
    // 识别的文法产生式：landExp: eqExp (T_LOGIC_AND eqExp)*;
    
    if (ctx->T_LOGIC_AND().empty()) {
        // 没有逻辑与运算符，只有一个eqExp
        return visitEqExp(ctx->eqExp()[0]);
    }

    ast_node *left, *right;
    
    // 存在逻辑与运算符
    auto andOps = ctx->T_LOGIC_AND();
    
    for (int k = 0; k < (int)andOps.size(); k++) {
        if (k == 0) {
            // 第一个左操作数
            left = std::any_cast<ast_node *>(visitEqExp(ctx->eqExp()[k]));
        }
        
        // 右操作数
        right = std::any_cast<ast_node *>(visitEqExp(ctx->eqExp()[k + 1]));
        
        // 创建逻辑与节点
        left = ast_node::New(ast_operator_type::AST_OP_LOGIC_AND, left, right, nullptr);
    }
    
    return left;
}
//添加相等和关系表达式的访问方法-lxg
std::any MiniCCSTVisitor::visitEqExp(MiniCParser::EqExpContext * ctx)
{
    // 识别的文法产生式：eqExp: relExp ((T_EQ | T_NE) relExp)*;
    
    // 没有相等运算符时直接返回relExp
    if (ctx->T_EQ().empty() && ctx->T_NE().empty()) {
        return visitRelExp(ctx->relExp()[0]);
    }
    
    // 获取第一个关系表达式作为初始左操作数
    ast_node *left = std::any_cast<ast_node *>(visitRelExp(ctx->relExp()[0]));
    
    // 跟踪当前处理的操作符位置
    size_t eqPos = 0;
    size_t nePos = 0;
    
    // 处理所有后续的操作符和操作数
    for (size_t i = 1; i < ctx->relExp().size(); i++) {
        ast_operator_type op;
        
        // 确定使用哪个操作符
        if (eqPos < ctx->T_EQ().size() && 
            (nePos >= ctx->T_NE().size() || 
             ctx->T_EQ()[eqPos]->getSymbol()->getTokenIndex() < 
             ctx->T_NE()[nePos]->getSymbol()->getTokenIndex())) {
            op = ast_operator_type::AST_OP_EQ;
            eqPos++;
        } else {
            op = ast_operator_type::AST_OP_NE;
            nePos++;
        }
        
        // 获取右操作数
        ast_node *right = std::any_cast<ast_node *>(visitRelExp(ctx->relExp()[i]));
        
        // 创建新的表达式节点
        left = ast_node::New(op, left, right, nullptr);
    }
    
    return left;
}

std::any MiniCCSTVisitor::visitRelExp(MiniCParser::RelExpContext * ctx)
{
    // 识别的文法产生式：relExp: addExp ((T_LT | T_GT | T_LE | T_GE) addExp)*;
    // 没有关系运算符时直接返回addExp
    if (ctx->T_LT().empty() && ctx->T_GT().empty() && 
        ctx->T_LE().empty() && ctx->T_GE().empty()) {
        return visitAddExp(ctx->addExp()[0]);
    }
    
    // 获取第一个加法表达式作为初始左操作数
    ast_node *left = std::any_cast<ast_node *>(visitAddExp(ctx->addExp()[0]));
    
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
        if (op == ast_operator_type::AST_OP_LT) ltPos++;
        else if (op == ast_operator_type::AST_OP_GT) gtPos++;
        else if (op == ast_operator_type::AST_OP_LE) lePos++;
        else if (op == ast_operator_type::AST_OP_GE) gePos++;
        
        // 获取右操作数
        ast_node *right = std::any_cast<ast_node *>(visitAddExp(ctx->addExp()[i]));
        
        // 创建新的表达式节点
        left = ast_node::New(op, left, right, nullptr);
    }
    
    return left;
}

//添加控制流语句的访问方法
std::any MiniCCSTVisitor::visitIfStatement(MiniCParser::IfStatementContext * ctx)
{
    // 处理if语句
    // 语法: T_IF T_L_PAREN expr T_R_PAREN statement (T_ELSE statement)?
    
    // 条件表达式
    auto condExpr = std::any_cast<ast_node *>(visitExpr(ctx->expr()));
    
    // if语句体
    auto thenStmt = std::any_cast<ast_node *>(visitStatement(ctx->statement(0)));
    
    if (ctx->T_ELSE()) {
        // if-else语句
        auto elseStmt = std::any_cast<ast_node *>(visitStatement(ctx->statement(1)));
        return ast_node::New(ast_operator_type::AST_OP_IF_ELSE, condExpr, thenStmt, elseStmt, nullptr);
    } else {
        // 单独的if语句
        return ast_node::New(ast_operator_type::AST_OP_IF, condExpr, thenStmt, nullptr);
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
    for (auto paramCtx : ctx->param()) {
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
    
    // 创建类型节点 - 直接使用 create_type_node 而不是先转换类型再创建
    ast_node * typeNode = create_type_node(paramType);
    
    // 获取参数名称
    std::string paramName = ctx->T_ID()->getText();
    int64_t lineno = (int64_t) ctx->T_ID()->getSymbol()->getLine();
    
    // 创建名称节点
    ast_node * nameNode = ast_node::New(paramName, lineno);
    
    // 创建形参节点
    ast_node * paramNode = new ast_node(ast_operator_type::AST_OP_FUNC_FORMAL_PARAM);
    paramNode->insert_son_node(typeNode);
    paramNode->insert_son_node(nameNode);
    
    return paramNode;
}

