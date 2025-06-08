
// Generated from /home/code/src/exp04-minic-expr/frontend/antlr4/MiniC.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"




class  MiniCLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, T__1 = 2, T_L_PAREN = 3, T_R_PAREN = 4, T_SEMICOLON = 5, T_L_BRACE = 6, 
    T_R_BRACE = 7, T_ASSIGN = 8, T_COMMA = 9, T_ADD = 10, T_SUB = 11, T_MUL = 12, 
    T_DIV = 13, T_MOD = 14, T_LT = 15, T_GT = 16, T_LE = 17, T_GE = 18, 
    T_EQ = 19, T_NE = 20, T_LOGIC_AND = 21, T_LOGIC_OR = 22, T_LOGIC_NOT = 23, 
    T_IF = 24, T_ELSE = 25, T_WHILE = 26, T_BREAK = 27, T_CONTINUE = 28, 
    T_RETURN = 29, T_INT = 30, T_VOID = 31, T_ID = 32, T_DIGIT = 33, COMMENT = 34, 
    BLOCK_COMMENT = 35, WS = 36
  };

  explicit MiniCLexer(antlr4::CharStream *input);

  ~MiniCLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

