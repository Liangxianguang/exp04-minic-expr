
// Generated from MiniC.g4 by ANTLR 4.12.0


#include "MiniCVisitor.h"

#include "MiniCParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct MiniCParserStaticData final {
  MiniCParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  MiniCParserStaticData(const MiniCParserStaticData&) = delete;
  MiniCParserStaticData(MiniCParserStaticData&&) = delete;
  MiniCParserStaticData& operator=(const MiniCParserStaticData&) = delete;
  MiniCParserStaticData& operator=(MiniCParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag minicParserOnceFlag;
MiniCParserStaticData *minicParserStaticData = nullptr;

void minicParserInitialize() {
  assert(minicParserStaticData == nullptr);
  auto staticData = std::make_unique<MiniCParserStaticData>(
    std::vector<std::string>{
      "compileUnit", "funcDef", "paramList", "param", "block", "blockItemList", 
      "blockItem", "varDecl", "basicType", "varDef", "statement", "expr", 
      "lorExp", "landExp", "eqExp", "relExp", "addExp", "addOp", "mulDivExp", 
      "mulDivOp", "unaryExp", "primaryExp", "realParamList", "lVal"
    },
    std::vector<std::string>{
      "", "'['", "']'", "'('", "')'", "';'", "'{'", "'}'", "'='", "','", 
      "'+'", "'-'", "'*'", "'/'", "'%'", "'<'", "'>'", "'<='", "'>='", "'=='", 
      "'!='", "'&&'", "'||'", "'!'", "'if'", "'else'", "'while'", "'break'", 
      "'continue'", "'return'", "'int'", "'void'"
    },
    std::vector<std::string>{
      "", "", "", "T_L_PAREN", "T_R_PAREN", "T_SEMICOLON", "T_L_BRACE", 
      "T_R_BRACE", "T_ASSIGN", "T_COMMA", "T_ADD", "T_SUB", "T_MUL", "T_DIV", 
      "T_MOD", "T_LT", "T_GT", "T_LE", "T_GE", "T_EQ", "T_NE", "T_LOGIC_AND", 
      "T_LOGIC_OR", "T_LOGIC_NOT", "T_IF", "T_ELSE", "T_WHILE", "T_BREAK", 
      "T_CONTINUE", "T_RETURN", "T_INT", "T_VOID", "T_ID", "T_DIGIT", "COMMENT", 
      "BLOCK_COMMENT", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,36,260,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,1,0,1,0,5,0,51,8,0,10,0,12,0,54,9,0,1,0,1,0,1,
  	1,1,1,1,1,1,1,3,1,62,8,1,1,1,1,1,1,1,1,2,1,2,1,2,5,2,70,8,2,10,2,12,2,
  	73,9,2,1,3,1,3,1,3,1,4,1,4,3,4,80,8,4,1,4,1,4,1,5,4,5,85,8,5,11,5,12,
  	5,86,1,6,1,6,3,6,91,8,6,1,7,1,7,1,7,1,7,5,7,97,8,7,10,7,12,7,100,9,7,
  	1,7,1,7,1,8,1,8,1,9,1,9,1,9,1,9,1,9,5,9,111,8,9,10,9,12,9,114,9,9,1,9,
  	1,9,1,9,1,9,1,9,5,9,121,8,9,10,9,12,9,124,9,9,1,9,1,9,3,9,128,8,9,1,10,
  	1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,
  	1,10,1,10,3,10,147,8,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,
  	1,10,1,10,3,10,160,8,10,1,10,3,10,163,8,10,1,11,1,11,1,12,1,12,1,12,5,
  	12,170,8,12,10,12,12,12,173,9,12,1,13,1,13,1,13,5,13,178,8,13,10,13,12,
  	13,181,9,13,1,14,1,14,1,14,5,14,186,8,14,10,14,12,14,189,9,14,1,15,1,
  	15,1,15,5,15,194,8,15,10,15,12,15,197,9,15,1,16,1,16,1,16,1,16,5,16,203,
  	8,16,10,16,12,16,206,9,16,1,17,1,17,1,18,1,18,1,18,1,18,5,18,214,8,18,
  	10,18,12,18,217,9,18,1,19,1,19,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,
  	3,20,229,8,20,1,20,3,20,232,8,20,1,21,1,21,1,21,1,21,1,21,1,21,3,21,240,
  	8,21,1,22,1,22,1,22,5,22,245,8,22,10,22,12,22,248,9,22,1,23,1,23,1,23,
  	1,23,1,23,5,23,255,8,23,10,23,12,23,258,9,23,1,23,0,0,24,0,2,4,6,8,10,
  	12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,0,5,1,0,30,31,1,
  	0,19,20,1,0,15,18,1,0,10,11,1,0,12,14,269,0,52,1,0,0,0,2,57,1,0,0,0,4,
  	66,1,0,0,0,6,74,1,0,0,0,8,77,1,0,0,0,10,84,1,0,0,0,12,90,1,0,0,0,14,92,
  	1,0,0,0,16,103,1,0,0,0,18,127,1,0,0,0,20,162,1,0,0,0,22,164,1,0,0,0,24,
  	166,1,0,0,0,26,174,1,0,0,0,28,182,1,0,0,0,30,190,1,0,0,0,32,198,1,0,0,
  	0,34,207,1,0,0,0,36,209,1,0,0,0,38,218,1,0,0,0,40,231,1,0,0,0,42,239,
  	1,0,0,0,44,241,1,0,0,0,46,249,1,0,0,0,48,51,3,2,1,0,49,51,3,14,7,0,50,
  	48,1,0,0,0,50,49,1,0,0,0,51,54,1,0,0,0,52,50,1,0,0,0,52,53,1,0,0,0,53,
  	55,1,0,0,0,54,52,1,0,0,0,55,56,5,0,0,1,56,1,1,0,0,0,57,58,7,0,0,0,58,
  	59,5,32,0,0,59,61,5,3,0,0,60,62,3,4,2,0,61,60,1,0,0,0,61,62,1,0,0,0,62,
  	63,1,0,0,0,63,64,5,4,0,0,64,65,3,8,4,0,65,3,1,0,0,0,66,71,3,6,3,0,67,
  	68,5,9,0,0,68,70,3,6,3,0,69,67,1,0,0,0,70,73,1,0,0,0,71,69,1,0,0,0,71,
  	72,1,0,0,0,72,5,1,0,0,0,73,71,1,0,0,0,74,75,5,30,0,0,75,76,5,32,0,0,76,
  	7,1,0,0,0,77,79,5,6,0,0,78,80,3,10,5,0,79,78,1,0,0,0,79,80,1,0,0,0,80,
  	81,1,0,0,0,81,82,5,7,0,0,82,9,1,0,0,0,83,85,3,12,6,0,84,83,1,0,0,0,85,
  	86,1,0,0,0,86,84,1,0,0,0,86,87,1,0,0,0,87,11,1,0,0,0,88,91,3,20,10,0,
  	89,91,3,14,7,0,90,88,1,0,0,0,90,89,1,0,0,0,91,13,1,0,0,0,92,93,3,16,8,
  	0,93,98,3,18,9,0,94,95,5,9,0,0,95,97,3,18,9,0,96,94,1,0,0,0,97,100,1,
  	0,0,0,98,96,1,0,0,0,98,99,1,0,0,0,99,101,1,0,0,0,100,98,1,0,0,0,101,102,
  	5,5,0,0,102,15,1,0,0,0,103,104,5,30,0,0,104,17,1,0,0,0,105,112,5,32,0,
  	0,106,107,5,1,0,0,107,108,3,22,11,0,108,109,5,2,0,0,109,111,1,0,0,0,110,
  	106,1,0,0,0,111,114,1,0,0,0,112,110,1,0,0,0,112,113,1,0,0,0,113,128,1,
  	0,0,0,114,112,1,0,0,0,115,122,5,32,0,0,116,117,5,1,0,0,117,118,3,22,11,
  	0,118,119,5,2,0,0,119,121,1,0,0,0,120,116,1,0,0,0,121,124,1,0,0,0,122,
  	120,1,0,0,0,122,123,1,0,0,0,123,125,1,0,0,0,124,122,1,0,0,0,125,126,5,
  	8,0,0,126,128,3,22,11,0,127,105,1,0,0,0,127,115,1,0,0,0,128,19,1,0,0,
  	0,129,130,5,29,0,0,130,131,3,22,11,0,131,132,5,5,0,0,132,163,1,0,0,0,
  	133,134,3,46,23,0,134,135,5,8,0,0,135,136,3,22,11,0,136,137,5,5,0,0,137,
  	163,1,0,0,0,138,163,3,8,4,0,139,140,5,24,0,0,140,141,5,3,0,0,141,142,
  	3,22,11,0,142,143,5,4,0,0,143,146,3,20,10,0,144,145,5,25,0,0,145,147,
  	3,20,10,0,146,144,1,0,0,0,146,147,1,0,0,0,147,163,1,0,0,0,148,149,5,26,
  	0,0,149,150,5,3,0,0,150,151,3,22,11,0,151,152,5,4,0,0,152,153,3,20,10,
  	0,153,163,1,0,0,0,154,155,5,27,0,0,155,163,5,5,0,0,156,157,5,28,0,0,157,
  	163,5,5,0,0,158,160,3,22,11,0,159,158,1,0,0,0,159,160,1,0,0,0,160,161,
  	1,0,0,0,161,163,5,5,0,0,162,129,1,0,0,0,162,133,1,0,0,0,162,138,1,0,0,
  	0,162,139,1,0,0,0,162,148,1,0,0,0,162,154,1,0,0,0,162,156,1,0,0,0,162,
  	159,1,0,0,0,163,21,1,0,0,0,164,165,3,24,12,0,165,23,1,0,0,0,166,171,3,
  	26,13,0,167,168,5,22,0,0,168,170,3,26,13,0,169,167,1,0,0,0,170,173,1,
  	0,0,0,171,169,1,0,0,0,171,172,1,0,0,0,172,25,1,0,0,0,173,171,1,0,0,0,
  	174,179,3,28,14,0,175,176,5,21,0,0,176,178,3,28,14,0,177,175,1,0,0,0,
  	178,181,1,0,0,0,179,177,1,0,0,0,179,180,1,0,0,0,180,27,1,0,0,0,181,179,
  	1,0,0,0,182,187,3,30,15,0,183,184,7,1,0,0,184,186,3,30,15,0,185,183,1,
  	0,0,0,186,189,1,0,0,0,187,185,1,0,0,0,187,188,1,0,0,0,188,29,1,0,0,0,
  	189,187,1,0,0,0,190,195,3,32,16,0,191,192,7,2,0,0,192,194,3,32,16,0,193,
  	191,1,0,0,0,194,197,1,0,0,0,195,193,1,0,0,0,195,196,1,0,0,0,196,31,1,
  	0,0,0,197,195,1,0,0,0,198,204,3,36,18,0,199,200,3,34,17,0,200,201,3,36,
  	18,0,201,203,1,0,0,0,202,199,1,0,0,0,203,206,1,0,0,0,204,202,1,0,0,0,
  	204,205,1,0,0,0,205,33,1,0,0,0,206,204,1,0,0,0,207,208,7,3,0,0,208,35,
  	1,0,0,0,209,215,3,40,20,0,210,211,3,38,19,0,211,212,3,40,20,0,212,214,
  	1,0,0,0,213,210,1,0,0,0,214,217,1,0,0,0,215,213,1,0,0,0,215,216,1,0,0,
  	0,216,37,1,0,0,0,217,215,1,0,0,0,218,219,7,4,0,0,219,39,1,0,0,0,220,221,
  	5,11,0,0,221,232,3,40,20,0,222,223,5,23,0,0,223,232,3,40,20,0,224,232,
  	3,42,21,0,225,226,5,32,0,0,226,228,5,3,0,0,227,229,3,44,22,0,228,227,
  	1,0,0,0,228,229,1,0,0,0,229,230,1,0,0,0,230,232,5,4,0,0,231,220,1,0,0,
  	0,231,222,1,0,0,0,231,224,1,0,0,0,231,225,1,0,0,0,232,41,1,0,0,0,233,
  	234,5,3,0,0,234,235,3,22,11,0,235,236,5,4,0,0,236,240,1,0,0,0,237,240,
  	5,33,0,0,238,240,3,46,23,0,239,233,1,0,0,0,239,237,1,0,0,0,239,238,1,
  	0,0,0,240,43,1,0,0,0,241,246,3,22,11,0,242,243,5,9,0,0,243,245,3,22,11,
  	0,244,242,1,0,0,0,245,248,1,0,0,0,246,244,1,0,0,0,246,247,1,0,0,0,247,
  	45,1,0,0,0,248,246,1,0,0,0,249,256,5,32,0,0,250,251,5,1,0,0,251,252,3,
  	22,11,0,252,253,5,2,0,0,253,255,1,0,0,0,254,250,1,0,0,0,255,258,1,0,0,
  	0,256,254,1,0,0,0,256,257,1,0,0,0,257,47,1,0,0,0,258,256,1,0,0,0,25,50,
  	52,61,71,79,86,90,98,112,122,127,146,159,162,171,179,187,195,204,215,
  	228,231,239,246,256
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  minicParserStaticData = staticData.release();
}

}

MiniCParser::MiniCParser(TokenStream *input) : MiniCParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

MiniCParser::MiniCParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  MiniCParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *minicParserStaticData->atn, minicParserStaticData->decisionToDFA, minicParserStaticData->sharedContextCache, options);
}

MiniCParser::~MiniCParser() {
  delete _interpreter;
}

const atn::ATN& MiniCParser::getATN() const {
  return *minicParserStaticData->atn;
}

std::string MiniCParser::getGrammarFileName() const {
  return "MiniC.g4";
}

const std::vector<std::string>& MiniCParser::getRuleNames() const {
  return minicParserStaticData->ruleNames;
}

const dfa::Vocabulary& MiniCParser::getVocabulary() const {
  return minicParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView MiniCParser::getSerializedATN() const {
  return minicParserStaticData->serializedATN;
}


//----------------- CompileUnitContext ------------------------------------------------------------------

MiniCParser::CompileUnitContext::CompileUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::CompileUnitContext::EOF() {
  return getToken(MiniCParser::EOF, 0);
}

std::vector<MiniCParser::FuncDefContext *> MiniCParser::CompileUnitContext::funcDef() {
  return getRuleContexts<MiniCParser::FuncDefContext>();
}

MiniCParser::FuncDefContext* MiniCParser::CompileUnitContext::funcDef(size_t i) {
  return getRuleContext<MiniCParser::FuncDefContext>(i);
}

std::vector<MiniCParser::VarDeclContext *> MiniCParser::CompileUnitContext::varDecl() {
  return getRuleContexts<MiniCParser::VarDeclContext>();
}

MiniCParser::VarDeclContext* MiniCParser::CompileUnitContext::varDecl(size_t i) {
  return getRuleContext<MiniCParser::VarDeclContext>(i);
}


size_t MiniCParser::CompileUnitContext::getRuleIndex() const {
  return MiniCParser::RuleCompileUnit;
}


std::any MiniCParser::CompileUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitCompileUnit(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::CompileUnitContext* MiniCParser::compileUnit() {
  CompileUnitContext *_localctx = _tracker.createInstance<CompileUnitContext>(_ctx, getState());
  enterRule(_localctx, 0, MiniCParser::RuleCompileUnit);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(52);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_INT

    || _la == MiniCParser::T_VOID) {
      setState(50);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
      case 1: {
        setState(48);
        funcDef();
        break;
      }

      case 2: {
        setState(49);
        varDecl();
        break;
      }

      default:
        break;
      }
      setState(54);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(55);
    match(MiniCParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncDefContext ------------------------------------------------------------------

MiniCParser::FuncDefContext::FuncDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::FuncDefContext::T_ID() {
  return getToken(MiniCParser::T_ID, 0);
}

tree::TerminalNode* MiniCParser::FuncDefContext::T_L_PAREN() {
  return getToken(MiniCParser::T_L_PAREN, 0);
}

tree::TerminalNode* MiniCParser::FuncDefContext::T_R_PAREN() {
  return getToken(MiniCParser::T_R_PAREN, 0);
}

MiniCParser::BlockContext* MiniCParser::FuncDefContext::block() {
  return getRuleContext<MiniCParser::BlockContext>(0);
}

tree::TerminalNode* MiniCParser::FuncDefContext::T_INT() {
  return getToken(MiniCParser::T_INT, 0);
}

tree::TerminalNode* MiniCParser::FuncDefContext::T_VOID() {
  return getToken(MiniCParser::T_VOID, 0);
}

MiniCParser::ParamListContext* MiniCParser::FuncDefContext::paramList() {
  return getRuleContext<MiniCParser::ParamListContext>(0);
}


size_t MiniCParser::FuncDefContext::getRuleIndex() const {
  return MiniCParser::RuleFuncDef;
}


std::any MiniCParser::FuncDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitFuncDef(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::FuncDefContext* MiniCParser::funcDef() {
  FuncDefContext *_localctx = _tracker.createInstance<FuncDefContext>(_ctx, getState());
  enterRule(_localctx, 2, MiniCParser::RuleFuncDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(57);
    _la = _input->LA(1);
    if (!(_la == MiniCParser::T_INT

    || _la == MiniCParser::T_VOID)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(58);
    match(MiniCParser::T_ID);
    setState(59);
    match(MiniCParser::T_L_PAREN);
    setState(61);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MiniCParser::T_INT) {
      setState(60);
      paramList();
    }
    setState(63);
    match(MiniCParser::T_R_PAREN);
    setState(64);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParamListContext ------------------------------------------------------------------

MiniCParser::ParamListContext::ParamListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::ParamContext *> MiniCParser::ParamListContext::param() {
  return getRuleContexts<MiniCParser::ParamContext>();
}

MiniCParser::ParamContext* MiniCParser::ParamListContext::param(size_t i) {
  return getRuleContext<MiniCParser::ParamContext>(i);
}

std::vector<tree::TerminalNode *> MiniCParser::ParamListContext::T_COMMA() {
  return getTokens(MiniCParser::T_COMMA);
}

tree::TerminalNode* MiniCParser::ParamListContext::T_COMMA(size_t i) {
  return getToken(MiniCParser::T_COMMA, i);
}


size_t MiniCParser::ParamListContext::getRuleIndex() const {
  return MiniCParser::RuleParamList;
}


std::any MiniCParser::ParamListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitParamList(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::ParamListContext* MiniCParser::paramList() {
  ParamListContext *_localctx = _tracker.createInstance<ParamListContext>(_ctx, getState());
  enterRule(_localctx, 4, MiniCParser::RuleParamList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(66);
    param();
    setState(71);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_COMMA) {
      setState(67);
      match(MiniCParser::T_COMMA);
      setState(68);
      param();
      setState(73);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParamContext ------------------------------------------------------------------

MiniCParser::ParamContext::ParamContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::ParamContext::T_INT() {
  return getToken(MiniCParser::T_INT, 0);
}

tree::TerminalNode* MiniCParser::ParamContext::T_ID() {
  return getToken(MiniCParser::T_ID, 0);
}


size_t MiniCParser::ParamContext::getRuleIndex() const {
  return MiniCParser::RuleParam;
}


std::any MiniCParser::ParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitParam(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::ParamContext* MiniCParser::param() {
  ParamContext *_localctx = _tracker.createInstance<ParamContext>(_ctx, getState());
  enterRule(_localctx, 6, MiniCParser::RuleParam);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(74);
    match(MiniCParser::T_INT);
    setState(75);
    match(MiniCParser::T_ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockContext ------------------------------------------------------------------

MiniCParser::BlockContext::BlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::BlockContext::T_L_BRACE() {
  return getToken(MiniCParser::T_L_BRACE, 0);
}

tree::TerminalNode* MiniCParser::BlockContext::T_R_BRACE() {
  return getToken(MiniCParser::T_R_BRACE, 0);
}

MiniCParser::BlockItemListContext* MiniCParser::BlockContext::blockItemList() {
  return getRuleContext<MiniCParser::BlockItemListContext>(0);
}


size_t MiniCParser::BlockContext::getRuleIndex() const {
  return MiniCParser::RuleBlock;
}


std::any MiniCParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::BlockContext* MiniCParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 8, MiniCParser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(77);
    match(MiniCParser::T_L_BRACE);
    setState(79);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 14990444648) != 0)) {
      setState(78);
      blockItemList();
    }
    setState(81);
    match(MiniCParser::T_R_BRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockItemListContext ------------------------------------------------------------------

MiniCParser::BlockItemListContext::BlockItemListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::BlockItemContext *> MiniCParser::BlockItemListContext::blockItem() {
  return getRuleContexts<MiniCParser::BlockItemContext>();
}

MiniCParser::BlockItemContext* MiniCParser::BlockItemListContext::blockItem(size_t i) {
  return getRuleContext<MiniCParser::BlockItemContext>(i);
}


size_t MiniCParser::BlockItemListContext::getRuleIndex() const {
  return MiniCParser::RuleBlockItemList;
}


std::any MiniCParser::BlockItemListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBlockItemList(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::BlockItemListContext* MiniCParser::blockItemList() {
  BlockItemListContext *_localctx = _tracker.createInstance<BlockItemListContext>(_ctx, getState());
  enterRule(_localctx, 10, MiniCParser::RuleBlockItemList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(84); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(83);
      blockItem();
      setState(86); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 14990444648) != 0));
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockItemContext ------------------------------------------------------------------

MiniCParser::BlockItemContext::BlockItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MiniCParser::StatementContext* MiniCParser::BlockItemContext::statement() {
  return getRuleContext<MiniCParser::StatementContext>(0);
}

MiniCParser::VarDeclContext* MiniCParser::BlockItemContext::varDecl() {
  return getRuleContext<MiniCParser::VarDeclContext>(0);
}


size_t MiniCParser::BlockItemContext::getRuleIndex() const {
  return MiniCParser::RuleBlockItem;
}


std::any MiniCParser::BlockItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBlockItem(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::BlockItemContext* MiniCParser::blockItem() {
  BlockItemContext *_localctx = _tracker.createInstance<BlockItemContext>(_ctx, getState());
  enterRule(_localctx, 12, MiniCParser::RuleBlockItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(90);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case MiniCParser::T_L_PAREN:
      case MiniCParser::T_SEMICOLON:
      case MiniCParser::T_L_BRACE:
      case MiniCParser::T_SUB:
      case MiniCParser::T_LOGIC_NOT:
      case MiniCParser::T_IF:
      case MiniCParser::T_WHILE:
      case MiniCParser::T_BREAK:
      case MiniCParser::T_CONTINUE:
      case MiniCParser::T_RETURN:
      case MiniCParser::T_ID:
      case MiniCParser::T_DIGIT: {
        enterOuterAlt(_localctx, 1);
        setState(88);
        statement();
        break;
      }

      case MiniCParser::T_INT: {
        enterOuterAlt(_localctx, 2);
        setState(89);
        varDecl();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDeclContext ------------------------------------------------------------------

MiniCParser::VarDeclContext::VarDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MiniCParser::BasicTypeContext* MiniCParser::VarDeclContext::basicType() {
  return getRuleContext<MiniCParser::BasicTypeContext>(0);
}

std::vector<MiniCParser::VarDefContext *> MiniCParser::VarDeclContext::varDef() {
  return getRuleContexts<MiniCParser::VarDefContext>();
}

MiniCParser::VarDefContext* MiniCParser::VarDeclContext::varDef(size_t i) {
  return getRuleContext<MiniCParser::VarDefContext>(i);
}

tree::TerminalNode* MiniCParser::VarDeclContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

std::vector<tree::TerminalNode *> MiniCParser::VarDeclContext::T_COMMA() {
  return getTokens(MiniCParser::T_COMMA);
}

tree::TerminalNode* MiniCParser::VarDeclContext::T_COMMA(size_t i) {
  return getToken(MiniCParser::T_COMMA, i);
}


size_t MiniCParser::VarDeclContext::getRuleIndex() const {
  return MiniCParser::RuleVarDecl;
}


std::any MiniCParser::VarDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitVarDecl(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::VarDeclContext* MiniCParser::varDecl() {
  VarDeclContext *_localctx = _tracker.createInstance<VarDeclContext>(_ctx, getState());
  enterRule(_localctx, 14, MiniCParser::RuleVarDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(92);
    basicType();
    setState(93);
    varDef();
    setState(98);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_COMMA) {
      setState(94);
      match(MiniCParser::T_COMMA);
      setState(95);
      varDef();
      setState(100);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(101);
    match(MiniCParser::T_SEMICOLON);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BasicTypeContext ------------------------------------------------------------------

MiniCParser::BasicTypeContext::BasicTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::BasicTypeContext::T_INT() {
  return getToken(MiniCParser::T_INT, 0);
}


size_t MiniCParser::BasicTypeContext::getRuleIndex() const {
  return MiniCParser::RuleBasicType;
}


std::any MiniCParser::BasicTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBasicType(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::BasicTypeContext* MiniCParser::basicType() {
  BasicTypeContext *_localctx = _tracker.createInstance<BasicTypeContext>(_ctx, getState());
  enterRule(_localctx, 16, MiniCParser::RuleBasicType);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(103);
    match(MiniCParser::T_INT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDefContext ------------------------------------------------------------------

MiniCParser::VarDefContext::VarDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::VarDefContext::T_ID() {
  return getToken(MiniCParser::T_ID, 0);
}

std::vector<MiniCParser::ExprContext *> MiniCParser::VarDefContext::expr() {
  return getRuleContexts<MiniCParser::ExprContext>();
}

MiniCParser::ExprContext* MiniCParser::VarDefContext::expr(size_t i) {
  return getRuleContext<MiniCParser::ExprContext>(i);
}

tree::TerminalNode* MiniCParser::VarDefContext::T_ASSIGN() {
  return getToken(MiniCParser::T_ASSIGN, 0);
}


size_t MiniCParser::VarDefContext::getRuleIndex() const {
  return MiniCParser::RuleVarDef;
}


std::any MiniCParser::VarDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitVarDef(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::VarDefContext* MiniCParser::varDef() {
  VarDefContext *_localctx = _tracker.createInstance<VarDefContext>(_ctx, getState());
  enterRule(_localctx, 18, MiniCParser::RuleVarDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(127);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(105);
      match(MiniCParser::T_ID);
      setState(112);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == MiniCParser::T__0) {
        setState(106);
        match(MiniCParser::T__0);
        setState(107);
        expr();
        setState(108);
        match(MiniCParser::T__1);
        setState(114);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(115);
      match(MiniCParser::T_ID);
      setState(122);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == MiniCParser::T__0) {
        setState(116);
        match(MiniCParser::T__0);
        setState(117);
        expr();
        setState(118);
        match(MiniCParser::T__1);
        setState(124);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(125);
      match(MiniCParser::T_ASSIGN);
      setState(126);
      expr();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementContext ------------------------------------------------------------------

MiniCParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t MiniCParser::StatementContext::getRuleIndex() const {
  return MiniCParser::RuleStatement;
}

void MiniCParser::StatementContext::copyFrom(StatementContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- WhileStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::WhileStatementContext::T_WHILE() {
  return getToken(MiniCParser::T_WHILE, 0);
}

tree::TerminalNode* MiniCParser::WhileStatementContext::T_L_PAREN() {
  return getToken(MiniCParser::T_L_PAREN, 0);
}

MiniCParser::ExprContext* MiniCParser::WhileStatementContext::expr() {
  return getRuleContext<MiniCParser::ExprContext>(0);
}

tree::TerminalNode* MiniCParser::WhileStatementContext::T_R_PAREN() {
  return getToken(MiniCParser::T_R_PAREN, 0);
}

MiniCParser::StatementContext* MiniCParser::WhileStatementContext::statement() {
  return getRuleContext<MiniCParser::StatementContext>(0);
}

MiniCParser::WhileStatementContext::WhileStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::WhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitWhileStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BlockStatementContext ------------------------------------------------------------------

MiniCParser::BlockContext* MiniCParser::BlockStatementContext::block() {
  return getRuleContext<MiniCParser::BlockContext>(0);
}

MiniCParser::BlockStatementContext::BlockStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::BlockStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBlockStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AssignStatementContext ------------------------------------------------------------------

MiniCParser::LValContext* MiniCParser::AssignStatementContext::lVal() {
  return getRuleContext<MiniCParser::LValContext>(0);
}

tree::TerminalNode* MiniCParser::AssignStatementContext::T_ASSIGN() {
  return getToken(MiniCParser::T_ASSIGN, 0);
}

MiniCParser::ExprContext* MiniCParser::AssignStatementContext::expr() {
  return getRuleContext<MiniCParser::ExprContext>(0);
}

tree::TerminalNode* MiniCParser::AssignStatementContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

MiniCParser::AssignStatementContext::AssignStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::AssignStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitAssignStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BreakStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::BreakStatementContext::T_BREAK() {
  return getToken(MiniCParser::T_BREAK, 0);
}

tree::TerminalNode* MiniCParser::BreakStatementContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

MiniCParser::BreakStatementContext::BreakStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::BreakStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBreakStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ContinueStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::ContinueStatementContext::T_CONTINUE() {
  return getToken(MiniCParser::T_CONTINUE, 0);
}

tree::TerminalNode* MiniCParser::ContinueStatementContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

MiniCParser::ContinueStatementContext::ContinueStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::ContinueStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitContinueStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ExpressionStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::ExpressionStatementContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

MiniCParser::ExprContext* MiniCParser::ExpressionStatementContext::expr() {
  return getRuleContext<MiniCParser::ExprContext>(0);
}

MiniCParser::ExpressionStatementContext::ExpressionStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::ExpressionStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitExpressionStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ReturnStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::ReturnStatementContext::T_RETURN() {
  return getToken(MiniCParser::T_RETURN, 0);
}

MiniCParser::ExprContext* MiniCParser::ReturnStatementContext::expr() {
  return getRuleContext<MiniCParser::ExprContext>(0);
}

tree::TerminalNode* MiniCParser::ReturnStatementContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

MiniCParser::ReturnStatementContext::ReturnStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::ReturnStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitReturnStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IfStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::IfStatementContext::T_IF() {
  return getToken(MiniCParser::T_IF, 0);
}

tree::TerminalNode* MiniCParser::IfStatementContext::T_L_PAREN() {
  return getToken(MiniCParser::T_L_PAREN, 0);
}

MiniCParser::ExprContext* MiniCParser::IfStatementContext::expr() {
  return getRuleContext<MiniCParser::ExprContext>(0);
}

tree::TerminalNode* MiniCParser::IfStatementContext::T_R_PAREN() {
  return getToken(MiniCParser::T_R_PAREN, 0);
}

std::vector<MiniCParser::StatementContext *> MiniCParser::IfStatementContext::statement() {
  return getRuleContexts<MiniCParser::StatementContext>();
}

MiniCParser::StatementContext* MiniCParser::IfStatementContext::statement(size_t i) {
  return getRuleContext<MiniCParser::StatementContext>(i);
}

tree::TerminalNode* MiniCParser::IfStatementContext::T_ELSE() {
  return getToken(MiniCParser::T_ELSE, 0);
}

MiniCParser::IfStatementContext::IfStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::IfStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitIfStatement(this);
  else
    return visitor->visitChildren(this);
}
MiniCParser::StatementContext* MiniCParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 20, MiniCParser::RuleStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(162);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<MiniCParser::ReturnStatementContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(129);
      match(MiniCParser::T_RETURN);
      setState(130);
      expr();
      setState(131);
      match(MiniCParser::T_SEMICOLON);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<MiniCParser::AssignStatementContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(133);
      lVal();
      setState(134);
      match(MiniCParser::T_ASSIGN);
      setState(135);
      expr();
      setState(136);
      match(MiniCParser::T_SEMICOLON);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<MiniCParser::BlockStatementContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(138);
      block();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<MiniCParser::IfStatementContext>(_localctx);
      enterOuterAlt(_localctx, 4);
      setState(139);
      match(MiniCParser::T_IF);
      setState(140);
      match(MiniCParser::T_L_PAREN);
      setState(141);
      expr();
      setState(142);
      match(MiniCParser::T_R_PAREN);
      setState(143);
      statement();
      setState(146);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx)) {
      case 1: {
        setState(144);
        match(MiniCParser::T_ELSE);
        setState(145);
        statement();
        break;
      }

      default:
        break;
      }
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<MiniCParser::WhileStatementContext>(_localctx);
      enterOuterAlt(_localctx, 5);
      setState(148);
      match(MiniCParser::T_WHILE);
      setState(149);
      match(MiniCParser::T_L_PAREN);
      setState(150);
      expr();
      setState(151);
      match(MiniCParser::T_R_PAREN);
      setState(152);
      statement();
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<MiniCParser::BreakStatementContext>(_localctx);
      enterOuterAlt(_localctx, 6);
      setState(154);
      match(MiniCParser::T_BREAK);
      setState(155);
      match(MiniCParser::T_SEMICOLON);
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<MiniCParser::ContinueStatementContext>(_localctx);
      enterOuterAlt(_localctx, 7);
      setState(156);
      match(MiniCParser::T_CONTINUE);
      setState(157);
      match(MiniCParser::T_SEMICOLON);
      break;
    }

    case 8: {
      _localctx = _tracker.createInstance<MiniCParser::ExpressionStatementContext>(_localctx);
      enterOuterAlt(_localctx, 8);
      setState(159);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 12893292552) != 0)) {
        setState(158);
        expr();
      }
      setState(161);
      match(MiniCParser::T_SEMICOLON);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExprContext ------------------------------------------------------------------

MiniCParser::ExprContext::ExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MiniCParser::LorExpContext* MiniCParser::ExprContext::lorExp() {
  return getRuleContext<MiniCParser::LorExpContext>(0);
}


size_t MiniCParser::ExprContext::getRuleIndex() const {
  return MiniCParser::RuleExpr;
}


std::any MiniCParser::ExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitExpr(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::ExprContext* MiniCParser::expr() {
  ExprContext *_localctx = _tracker.createInstance<ExprContext>(_ctx, getState());
  enterRule(_localctx, 22, MiniCParser::RuleExpr);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(164);
    lorExp();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LorExpContext ------------------------------------------------------------------

MiniCParser::LorExpContext::LorExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::LandExpContext *> MiniCParser::LorExpContext::landExp() {
  return getRuleContexts<MiniCParser::LandExpContext>();
}

MiniCParser::LandExpContext* MiniCParser::LorExpContext::landExp(size_t i) {
  return getRuleContext<MiniCParser::LandExpContext>(i);
}

std::vector<tree::TerminalNode *> MiniCParser::LorExpContext::T_LOGIC_OR() {
  return getTokens(MiniCParser::T_LOGIC_OR);
}

tree::TerminalNode* MiniCParser::LorExpContext::T_LOGIC_OR(size_t i) {
  return getToken(MiniCParser::T_LOGIC_OR, i);
}


size_t MiniCParser::LorExpContext::getRuleIndex() const {
  return MiniCParser::RuleLorExp;
}


std::any MiniCParser::LorExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitLorExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::LorExpContext* MiniCParser::lorExp() {
  LorExpContext *_localctx = _tracker.createInstance<LorExpContext>(_ctx, getState());
  enterRule(_localctx, 24, MiniCParser::RuleLorExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(166);
    landExp();
    setState(171);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_LOGIC_OR) {
      setState(167);
      match(MiniCParser::T_LOGIC_OR);
      setState(168);
      landExp();
      setState(173);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LandExpContext ------------------------------------------------------------------

MiniCParser::LandExpContext::LandExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::EqExpContext *> MiniCParser::LandExpContext::eqExp() {
  return getRuleContexts<MiniCParser::EqExpContext>();
}

MiniCParser::EqExpContext* MiniCParser::LandExpContext::eqExp(size_t i) {
  return getRuleContext<MiniCParser::EqExpContext>(i);
}

std::vector<tree::TerminalNode *> MiniCParser::LandExpContext::T_LOGIC_AND() {
  return getTokens(MiniCParser::T_LOGIC_AND);
}

tree::TerminalNode* MiniCParser::LandExpContext::T_LOGIC_AND(size_t i) {
  return getToken(MiniCParser::T_LOGIC_AND, i);
}


size_t MiniCParser::LandExpContext::getRuleIndex() const {
  return MiniCParser::RuleLandExp;
}


std::any MiniCParser::LandExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitLandExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::LandExpContext* MiniCParser::landExp() {
  LandExpContext *_localctx = _tracker.createInstance<LandExpContext>(_ctx, getState());
  enterRule(_localctx, 26, MiniCParser::RuleLandExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(174);
    eqExp();
    setState(179);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_LOGIC_AND) {
      setState(175);
      match(MiniCParser::T_LOGIC_AND);
      setState(176);
      eqExp();
      setState(181);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EqExpContext ------------------------------------------------------------------

MiniCParser::EqExpContext::EqExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::RelExpContext *> MiniCParser::EqExpContext::relExp() {
  return getRuleContexts<MiniCParser::RelExpContext>();
}

MiniCParser::RelExpContext* MiniCParser::EqExpContext::relExp(size_t i) {
  return getRuleContext<MiniCParser::RelExpContext>(i);
}

std::vector<tree::TerminalNode *> MiniCParser::EqExpContext::T_EQ() {
  return getTokens(MiniCParser::T_EQ);
}

tree::TerminalNode* MiniCParser::EqExpContext::T_EQ(size_t i) {
  return getToken(MiniCParser::T_EQ, i);
}

std::vector<tree::TerminalNode *> MiniCParser::EqExpContext::T_NE() {
  return getTokens(MiniCParser::T_NE);
}

tree::TerminalNode* MiniCParser::EqExpContext::T_NE(size_t i) {
  return getToken(MiniCParser::T_NE, i);
}


size_t MiniCParser::EqExpContext::getRuleIndex() const {
  return MiniCParser::RuleEqExp;
}


std::any MiniCParser::EqExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitEqExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::EqExpContext* MiniCParser::eqExp() {
  EqExpContext *_localctx = _tracker.createInstance<EqExpContext>(_ctx, getState());
  enterRule(_localctx, 28, MiniCParser::RuleEqExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(182);
    relExp();
    setState(187);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_EQ

    || _la == MiniCParser::T_NE) {
      setState(183);
      _la = _input->LA(1);
      if (!(_la == MiniCParser::T_EQ

      || _la == MiniCParser::T_NE)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(184);
      relExp();
      setState(189);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RelExpContext ------------------------------------------------------------------

MiniCParser::RelExpContext::RelExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::AddExpContext *> MiniCParser::RelExpContext::addExp() {
  return getRuleContexts<MiniCParser::AddExpContext>();
}

MiniCParser::AddExpContext* MiniCParser::RelExpContext::addExp(size_t i) {
  return getRuleContext<MiniCParser::AddExpContext>(i);
}

std::vector<tree::TerminalNode *> MiniCParser::RelExpContext::T_LT() {
  return getTokens(MiniCParser::T_LT);
}

tree::TerminalNode* MiniCParser::RelExpContext::T_LT(size_t i) {
  return getToken(MiniCParser::T_LT, i);
}

std::vector<tree::TerminalNode *> MiniCParser::RelExpContext::T_GT() {
  return getTokens(MiniCParser::T_GT);
}

tree::TerminalNode* MiniCParser::RelExpContext::T_GT(size_t i) {
  return getToken(MiniCParser::T_GT, i);
}

std::vector<tree::TerminalNode *> MiniCParser::RelExpContext::T_LE() {
  return getTokens(MiniCParser::T_LE);
}

tree::TerminalNode* MiniCParser::RelExpContext::T_LE(size_t i) {
  return getToken(MiniCParser::T_LE, i);
}

std::vector<tree::TerminalNode *> MiniCParser::RelExpContext::T_GE() {
  return getTokens(MiniCParser::T_GE);
}

tree::TerminalNode* MiniCParser::RelExpContext::T_GE(size_t i) {
  return getToken(MiniCParser::T_GE, i);
}


size_t MiniCParser::RelExpContext::getRuleIndex() const {
  return MiniCParser::RuleRelExp;
}


std::any MiniCParser::RelExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitRelExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::RelExpContext* MiniCParser::relExp() {
  RelExpContext *_localctx = _tracker.createInstance<RelExpContext>(_ctx, getState());
  enterRule(_localctx, 30, MiniCParser::RuleRelExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(190);
    addExp();
    setState(195);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 491520) != 0)) {
      setState(191);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 491520) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(192);
      addExp();
      setState(197);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AddExpContext ------------------------------------------------------------------

MiniCParser::AddExpContext::AddExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::MulDivExpContext *> MiniCParser::AddExpContext::mulDivExp() {
  return getRuleContexts<MiniCParser::MulDivExpContext>();
}

MiniCParser::MulDivExpContext* MiniCParser::AddExpContext::mulDivExp(size_t i) {
  return getRuleContext<MiniCParser::MulDivExpContext>(i);
}

std::vector<MiniCParser::AddOpContext *> MiniCParser::AddExpContext::addOp() {
  return getRuleContexts<MiniCParser::AddOpContext>();
}

MiniCParser::AddOpContext* MiniCParser::AddExpContext::addOp(size_t i) {
  return getRuleContext<MiniCParser::AddOpContext>(i);
}


size_t MiniCParser::AddExpContext::getRuleIndex() const {
  return MiniCParser::RuleAddExp;
}


std::any MiniCParser::AddExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitAddExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::AddExpContext* MiniCParser::addExp() {
  AddExpContext *_localctx = _tracker.createInstance<AddExpContext>(_ctx, getState());
  enterRule(_localctx, 32, MiniCParser::RuleAddExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(198);
    mulDivExp();
    setState(204);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_ADD

    || _la == MiniCParser::T_SUB) {
      setState(199);
      addOp();
      setState(200);
      mulDivExp();
      setState(206);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AddOpContext ------------------------------------------------------------------

MiniCParser::AddOpContext::AddOpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::AddOpContext::T_ADD() {
  return getToken(MiniCParser::T_ADD, 0);
}

tree::TerminalNode* MiniCParser::AddOpContext::T_SUB() {
  return getToken(MiniCParser::T_SUB, 0);
}


size_t MiniCParser::AddOpContext::getRuleIndex() const {
  return MiniCParser::RuleAddOp;
}


std::any MiniCParser::AddOpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitAddOp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::AddOpContext* MiniCParser::addOp() {
  AddOpContext *_localctx = _tracker.createInstance<AddOpContext>(_ctx, getState());
  enterRule(_localctx, 34, MiniCParser::RuleAddOp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(207);
    _la = _input->LA(1);
    if (!(_la == MiniCParser::T_ADD

    || _la == MiniCParser::T_SUB)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MulDivExpContext ------------------------------------------------------------------

MiniCParser::MulDivExpContext::MulDivExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::UnaryExpContext *> MiniCParser::MulDivExpContext::unaryExp() {
  return getRuleContexts<MiniCParser::UnaryExpContext>();
}

MiniCParser::UnaryExpContext* MiniCParser::MulDivExpContext::unaryExp(size_t i) {
  return getRuleContext<MiniCParser::UnaryExpContext>(i);
}

std::vector<MiniCParser::MulDivOpContext *> MiniCParser::MulDivExpContext::mulDivOp() {
  return getRuleContexts<MiniCParser::MulDivOpContext>();
}

MiniCParser::MulDivOpContext* MiniCParser::MulDivExpContext::mulDivOp(size_t i) {
  return getRuleContext<MiniCParser::MulDivOpContext>(i);
}


size_t MiniCParser::MulDivExpContext::getRuleIndex() const {
  return MiniCParser::RuleMulDivExp;
}


std::any MiniCParser::MulDivExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitMulDivExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::MulDivExpContext* MiniCParser::mulDivExp() {
  MulDivExpContext *_localctx = _tracker.createInstance<MulDivExpContext>(_ctx, getState());
  enterRule(_localctx, 36, MiniCParser::RuleMulDivExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(209);
    unaryExp();
    setState(215);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 28672) != 0)) {
      setState(210);
      mulDivOp();
      setState(211);
      unaryExp();
      setState(217);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MulDivOpContext ------------------------------------------------------------------

MiniCParser::MulDivOpContext::MulDivOpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::MulDivOpContext::T_MUL() {
  return getToken(MiniCParser::T_MUL, 0);
}

tree::TerminalNode* MiniCParser::MulDivOpContext::T_DIV() {
  return getToken(MiniCParser::T_DIV, 0);
}

tree::TerminalNode* MiniCParser::MulDivOpContext::T_MOD() {
  return getToken(MiniCParser::T_MOD, 0);
}


size_t MiniCParser::MulDivOpContext::getRuleIndex() const {
  return MiniCParser::RuleMulDivOp;
}


std::any MiniCParser::MulDivOpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitMulDivOp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::MulDivOpContext* MiniCParser::mulDivOp() {
  MulDivOpContext *_localctx = _tracker.createInstance<MulDivOpContext>(_ctx, getState());
  enterRule(_localctx, 38, MiniCParser::RuleMulDivOp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(218);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 28672) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnaryExpContext ------------------------------------------------------------------

MiniCParser::UnaryExpContext::UnaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::UnaryExpContext::T_SUB() {
  return getToken(MiniCParser::T_SUB, 0);
}

MiniCParser::UnaryExpContext* MiniCParser::UnaryExpContext::unaryExp() {
  return getRuleContext<MiniCParser::UnaryExpContext>(0);
}

tree::TerminalNode* MiniCParser::UnaryExpContext::T_LOGIC_NOT() {
  return getToken(MiniCParser::T_LOGIC_NOT, 0);
}

MiniCParser::PrimaryExpContext* MiniCParser::UnaryExpContext::primaryExp() {
  return getRuleContext<MiniCParser::PrimaryExpContext>(0);
}

tree::TerminalNode* MiniCParser::UnaryExpContext::T_ID() {
  return getToken(MiniCParser::T_ID, 0);
}

tree::TerminalNode* MiniCParser::UnaryExpContext::T_L_PAREN() {
  return getToken(MiniCParser::T_L_PAREN, 0);
}

tree::TerminalNode* MiniCParser::UnaryExpContext::T_R_PAREN() {
  return getToken(MiniCParser::T_R_PAREN, 0);
}

MiniCParser::RealParamListContext* MiniCParser::UnaryExpContext::realParamList() {
  return getRuleContext<MiniCParser::RealParamListContext>(0);
}


size_t MiniCParser::UnaryExpContext::getRuleIndex() const {
  return MiniCParser::RuleUnaryExp;
}


std::any MiniCParser::UnaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitUnaryExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::UnaryExpContext* MiniCParser::unaryExp() {
  UnaryExpContext *_localctx = _tracker.createInstance<UnaryExpContext>(_ctx, getState());
  enterRule(_localctx, 40, MiniCParser::RuleUnaryExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(231);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(220);
      match(MiniCParser::T_SUB);
      setState(221);
      unaryExp();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(222);
      match(MiniCParser::T_LOGIC_NOT);
      setState(223);
      unaryExp();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(224);
      primaryExp();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(225);
      match(MiniCParser::T_ID);
      setState(226);
      match(MiniCParser::T_L_PAREN);
      setState(228);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 12893292552) != 0)) {
        setState(227);
        realParamList();
      }
      setState(230);
      match(MiniCParser::T_R_PAREN);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrimaryExpContext ------------------------------------------------------------------

MiniCParser::PrimaryExpContext::PrimaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::PrimaryExpContext::T_L_PAREN() {
  return getToken(MiniCParser::T_L_PAREN, 0);
}

MiniCParser::ExprContext* MiniCParser::PrimaryExpContext::expr() {
  return getRuleContext<MiniCParser::ExprContext>(0);
}

tree::TerminalNode* MiniCParser::PrimaryExpContext::T_R_PAREN() {
  return getToken(MiniCParser::T_R_PAREN, 0);
}

tree::TerminalNode* MiniCParser::PrimaryExpContext::T_DIGIT() {
  return getToken(MiniCParser::T_DIGIT, 0);
}

MiniCParser::LValContext* MiniCParser::PrimaryExpContext::lVal() {
  return getRuleContext<MiniCParser::LValContext>(0);
}


size_t MiniCParser::PrimaryExpContext::getRuleIndex() const {
  return MiniCParser::RulePrimaryExp;
}


std::any MiniCParser::PrimaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitPrimaryExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::PrimaryExpContext* MiniCParser::primaryExp() {
  PrimaryExpContext *_localctx = _tracker.createInstance<PrimaryExpContext>(_ctx, getState());
  enterRule(_localctx, 42, MiniCParser::RulePrimaryExp);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(239);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case MiniCParser::T_L_PAREN: {
        enterOuterAlt(_localctx, 1);
        setState(233);
        match(MiniCParser::T_L_PAREN);
        setState(234);
        expr();
        setState(235);
        match(MiniCParser::T_R_PAREN);
        break;
      }

      case MiniCParser::T_DIGIT: {
        enterOuterAlt(_localctx, 2);
        setState(237);
        match(MiniCParser::T_DIGIT);
        break;
      }

      case MiniCParser::T_ID: {
        enterOuterAlt(_localctx, 3);
        setState(238);
        lVal();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RealParamListContext ------------------------------------------------------------------

MiniCParser::RealParamListContext::RealParamListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::ExprContext *> MiniCParser::RealParamListContext::expr() {
  return getRuleContexts<MiniCParser::ExprContext>();
}

MiniCParser::ExprContext* MiniCParser::RealParamListContext::expr(size_t i) {
  return getRuleContext<MiniCParser::ExprContext>(i);
}

std::vector<tree::TerminalNode *> MiniCParser::RealParamListContext::T_COMMA() {
  return getTokens(MiniCParser::T_COMMA);
}

tree::TerminalNode* MiniCParser::RealParamListContext::T_COMMA(size_t i) {
  return getToken(MiniCParser::T_COMMA, i);
}


size_t MiniCParser::RealParamListContext::getRuleIndex() const {
  return MiniCParser::RuleRealParamList;
}


std::any MiniCParser::RealParamListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitRealParamList(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::RealParamListContext* MiniCParser::realParamList() {
  RealParamListContext *_localctx = _tracker.createInstance<RealParamListContext>(_ctx, getState());
  enterRule(_localctx, 44, MiniCParser::RuleRealParamList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(241);
    expr();
    setState(246);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_COMMA) {
      setState(242);
      match(MiniCParser::T_COMMA);
      setState(243);
      expr();
      setState(248);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LValContext ------------------------------------------------------------------

MiniCParser::LValContext::LValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::LValContext::T_ID() {
  return getToken(MiniCParser::T_ID, 0);
}

std::vector<MiniCParser::ExprContext *> MiniCParser::LValContext::expr() {
  return getRuleContexts<MiniCParser::ExprContext>();
}

MiniCParser::ExprContext* MiniCParser::LValContext::expr(size_t i) {
  return getRuleContext<MiniCParser::ExprContext>(i);
}


size_t MiniCParser::LValContext::getRuleIndex() const {
  return MiniCParser::RuleLVal;
}


std::any MiniCParser::LValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitLVal(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::LValContext* MiniCParser::lVal() {
  LValContext *_localctx = _tracker.createInstance<LValContext>(_ctx, getState());
  enterRule(_localctx, 46, MiniCParser::RuleLVal);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(249);
    match(MiniCParser::T_ID);
    setState(256);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T__0) {
      setState(250);
      match(MiniCParser::T__0);
      setState(251);
      expr();
      setState(252);
      match(MiniCParser::T__1);
      setState(258);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void MiniCParser::initialize() {
  ::antlr4::internal::call_once(minicParserOnceFlag, minicParserInitialize);
}
