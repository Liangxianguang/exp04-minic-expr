
// Generated from MiniC.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"




class  MiniCLexer : public antlr4::Lexer {
public:
  enum {
    T_L_PAREN = 1, T_R_PAREN = 2, T_SEMICOLON = 3, T_L_BRACE = 4, T_R_BRACE = 5, 
    T_ASSIGN = 6, T_COMMA = 7, T_ADD = 8, T_SUB = 9, T_MUL = 10, T_DIV = 11, 
    T_MOD = 12, T_LT = 13, T_GT = 14, T_LE = 15, T_GE = 16, T_EQ = 17, T_NE = 18, 
    T_LOGIC_AND = 19, T_LOGIC_OR = 20, T_LOGIC_NOT = 21, T_IF = 22, T_ELSE = 23, 
    T_WHILE = 24, T_BREAK = 25, T_CONTINUE = 26, T_RETURN = 27, T_INT = 28, 
    T_VOID = 29, T_ID = 30, T_DIGIT = 31, COMMENT = 32, BLOCK_COMMENT = 33, 
    WS = 34
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

