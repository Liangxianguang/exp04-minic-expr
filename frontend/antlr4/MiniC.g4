grammar MiniC;

// 词法规则名总是以大写字母开头

// 语法规则名总是以小写字母开头

// 每个非终结符尽量多包含闭包、正闭包或可选符等的EBNF范式描述

// 若非终结符由多个产生式组成，则建议在每个产生式的尾部追加# 名称来区分，详细可查看非终结符statement的描述

// 语法规则描述：EBNF范式

// 源文件编译单元定义
compileUnit: (funcDef | varDecl)* EOF;

// 函数定义，目前不支持形参，也不支持返回void类型等 funcDef: T_INT T_ID T_L_PAREN T_R_PAREN block; funcDef: T_INT T_ID
// T_L_PAREN paramList? T_R_PAREN block; 修改函数定义，目前支持形参，也支持返回void类型等-lxg
funcDef: (T_INT | T_VOID) T_ID T_L_PAREN paramList? T_R_PAREN block;
paramList: param (',' param)*;
// 修改 param 规则以支持数组参数 param: T_INT T_ID; param: T_INT T_ID ('[' ']')*;
// 修改参数规则，使其支持inta[][3]之类的数组定义-lxg
param:
	T_INT T_ID // 简单参数
	| T_INT T_ID ('[' ']') ('[' T_DIGIT ']')* // 第一维省略，其余必须指定
	| T_INT T_ID ( '[' T_DIGIT ']')+; // 所有维度都指定

// 语句块看用作函数体，这里允许多个语句，并且不含任何语句
block: T_L_BRACE blockItemList? T_R_BRACE;

// 每个ItemList可包含至少一个Item
blockItemList: blockItem+;

// 每个Item可以是一个语句，或者变量声明语句
blockItem: statement | varDecl;

// 变量声明，目前不支持变量含有初值
varDecl: basicType varDef (T_COMMA varDef)* T_SEMICOLON;

// 基本类型
basicType: T_INT;

// 变量定义 varDef: T_ID; 修改变量定义，使其支持变量初始化 varDef: T_ID | T_ID T_ASSIGN expr; 修改类型声明,使其支持数组
varDef:
	T_ID ('[' expr ']')*
	| T_ID ('[' expr ']')* T_ASSIGN expr;

// 目前语句支持return和赋值语句 statement:T_RETURN expr T_SEMICOLON # returnStatement | lVal T_ASSIGN expr
// T_SEMICOLON # assignStatement | block # blockStatement | expr? T_SEMICOLON # expressionStatement;

// 修改语句规则，增加分支和循环语句-lxg
statement:
	T_RETURN expr? T_SEMICOLON										# returnStatement // 支持return;语句
	| lVal T_ASSIGN expr T_SEMICOLON								# assignStatement
	| block															# blockStatement
	| T_IF T_L_PAREN expr T_R_PAREN statement (T_ELSE statement)?	# ifStatement
	| T_WHILE T_L_PAREN expr T_R_PAREN statement					# whileStatement
	| T_BREAK T_SEMICOLON											# breakStatement
	| T_CONTINUE T_SEMICOLON										# continueStatement
	| expr? T_SEMICOLON												# expressionStatement;

// 表达式文法 expr : AddExp 表达式目前只支持加法与减法运算 expr: addExp; 表达支持加减乘除模运算 修改表达式文法，增加关系和逻辑表达式-lxg
expr: lorExp;

// 逻辑或表达式
lorExp: landExp (T_LOGIC_OR landExp)*;

// 逻辑与表达式
landExp: eqExp (T_LOGIC_AND eqExp)*;

// 相等性表达式
eqExp: relExp ((T_EQ | T_NE) relExp)*;

// 关系表达式
relExp: addExp ((T_LT | T_GT | T_LE | T_GE) addExp)*;

// 加减表达式 addExp: unaryExp (addOp unaryExp)*;
addExp: mulDivExp (addOp mulDivExp)*;

// 加减运算符
addOp: T_ADD | T_SUB;

// 乘除求余表达式
mulDivExp: unaryExp (mulDivOp unaryExp)*;

// 乘除求余运算符
mulDivOp: T_MUL | T_DIV | T_MOD;

// 一元表达式 unaryExp: primaryExp | T_ID T_L_PAREN realParamList? T_R_PAREN; unaryExp: T_SUB unaryExp |
// primaryExp | T_ID T_L_PAREN realParamList? T_R_PAREN;

// 修改一元表达式，增加逻辑非
unaryExp:
	T_SUB unaryExp
	| T_LOGIC_NOT unaryExp
	| primaryExp
	| T_ID T_L_PAREN realParamList? T_R_PAREN;

// 基本表达式：括号表达式、整数、左值表达式
primaryExp: T_L_PAREN expr T_R_PAREN | T_DIGIT | lVal;

// 实参列表
realParamList: expr (T_COMMA expr)*;

// 左值表达式 lVal: T_ID; 修改为左值表达式，支持数组访问
lVal: T_ID ('[' expr ']')*;

// 用正规式来进行词法规则的描述

T_L_PAREN: '(';
T_R_PAREN: ')';
T_SEMICOLON: ';';
T_L_BRACE: '{';
T_R_BRACE: '}';

T_ASSIGN: '=';
T_COMMA: ',';

T_ADD: '+';
T_SUB: '-';
T_MUL: '*';
T_DIV: '/';
T_MOD: '%';
//添加关系运算符和逻辑运算符-lxg
T_LT: '<';
T_GT: '>';
T_LE: '<=';
T_GE: '>=';
T_EQ: '==';
T_NE: '!=';
T_LOGIC_AND: '&&';
T_LOGIC_OR: '||';
T_LOGIC_NOT: '!';
//添加控制流关键字-lxg
T_IF: 'if';
T_ELSE: 'else';
T_WHILE: 'while';
T_BREAK: 'break';
T_CONTINUE: 'continue';
// 要注意关键字同样也属于T_ID，因此必须放在T_ID的前面，否则会识别成T_ID
T_RETURN: 'return';
T_INT: 'int';
T_VOID: 'void';

T_ID: [a-zA-Z_][a-zA-Z0-9_]*;
//T_DIGIT: '0' | [1-9][0-9]* | '0'[0-7]+ | '0'[xX][0-9a-fA-F]+;
T_DIGIT:
	'0'
	| [1-9][0-9]*
	| '0' [0-7]+
	| '0' [xX][0-9a-fA-F]+;

// 添加注释规则-lxg
COMMENT: '//' ~[\r\n]* -> skip; // 单行注释
BLOCK_COMMENT: '/*' .*? '*/' -> skip; // 多行注释

/* 空白符丢弃 */
WS: [ \r\n\t]+ -> skip;