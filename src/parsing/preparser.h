// Copyright 2012 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_PARSING_PREPARSER_H
#define V8_PARSING_PREPARSER_H

#include "src/ast/scopes.h"
#include "src/parsing/parser-base.h"

namespace v8 {
namespace internal {

// Whereas the Parser generates AST during the recursive descent,
// the PreParser doesn't create a tree. Instead, it passes around minimal
// data objects (PreParserExpression, PreParserIdentifier etc.) which contain
// just enough data for the upper layer functions. PreParserFactory is
// responsible for creating these dummy objects. It provides a similar kind of
// interface as AstNodeFactory, so ParserBase doesn't need to care which one is
// used.

class PreParserIdentifier {
 public:
  PreParserIdentifier() : type_(kUnknownIdentifier) {}
  static PreParserIdentifier Default() {
    return PreParserIdentifier(kUnknownIdentifier);
  }
  static PreParserIdentifier Eval() {
    return PreParserIdentifier(kEvalIdentifier);
  }
  static PreParserIdentifier Arguments() {
    return PreParserIdentifier(kArgumentsIdentifier);
  }
  static PreParserIdentifier Undefined() {
    return PreParserIdentifier(kUndefinedIdentifier);
  }
  static PreParserIdentifier FutureReserved() {
    return PreParserIdentifier(kFutureReservedIdentifier);
  }
  static PreParserIdentifier FutureStrictReserved() {
    return PreParserIdentifier(kFutureStrictReservedIdentifier);
  }
  static PreParserIdentifier Let() {
    return PreParserIdentifier(kLetIdentifier);
  }
  static PreParserIdentifier Static() {
    return PreParserIdentifier(kStaticIdentifier);
  }
  static PreParserIdentifier Yield() {
    return PreParserIdentifier(kYieldIdentifier);
  }
  static PreParserIdentifier Prototype() {
    return PreParserIdentifier(kPrototypeIdentifier);
  }
  static PreParserIdentifier Constructor() {
    return PreParserIdentifier(kConstructorIdentifier);
  }
  static PreParserIdentifier Enum() {
    return PreParserIdentifier(kEnumIdentifier);
  }
  static PreParserIdentifier Await() {
    return PreParserIdentifier(kAwaitIdentifier);
  }
  static PreParserIdentifier Async() {
    return PreParserIdentifier(kAsyncIdentifier);
  }
  bool IsEval() const { return type_ == kEvalIdentifier; }
  bool IsArguments() const { return type_ == kArgumentsIdentifier; }
  bool IsEvalOrArguments() const { return IsEval() || IsArguments(); }
  bool IsUndefined() const { return type_ == kUndefinedIdentifier; }
  bool IsLet() const { return type_ == kLetIdentifier; }
  bool IsStatic() const { return type_ == kStaticIdentifier; }
  bool IsYield() const { return type_ == kYieldIdentifier; }
  bool IsPrototype() const { return type_ == kPrototypeIdentifier; }
  bool IsConstructor() const { return type_ == kConstructorIdentifier; }
  bool IsEnum() const { return type_ == kEnumIdentifier; }
  bool IsAwait() const { return type_ == kAwaitIdentifier; }
  bool IsFutureStrictReserved() const {
    return type_ == kFutureStrictReservedIdentifier ||
           type_ == kLetIdentifier || type_ == kStaticIdentifier ||
           type_ == kYieldIdentifier;
  }

  // Allow identifier->name()[->length()] to work. The preparser
  // does not need the actual positions/lengths of the identifiers.
  const PreParserIdentifier* operator->() const { return this; }
  const PreParserIdentifier raw_name() const { return *this; }

  int position() const { return 0; }
  int length() const { return 0; }

 private:
  enum Type {
    kUnknownIdentifier,
    kFutureReservedIdentifier,
    kFutureStrictReservedIdentifier,
    kLetIdentifier,
    kStaticIdentifier,
    kYieldIdentifier,
    kEvalIdentifier,
    kArgumentsIdentifier,
    kUndefinedIdentifier,
    kPrototypeIdentifier,
    kConstructorIdentifier,
    kEnumIdentifier,
    kAwaitIdentifier,
    kAsyncIdentifier
  };

  explicit PreParserIdentifier(Type type) : type_(type) {}
  Type type_;

  friend class PreParserExpression;
};


class PreParserExpression {
 public:
  PreParserExpression() : code_(TypeField::encode(kEmpty)) {}

  static PreParserExpression Empty() { return PreParserExpression(); }

  static PreParserExpression Default() {
    return PreParserExpression(TypeField::encode(kExpression));
  }

  static PreParserExpression Spread(PreParserExpression expression) {
    return PreParserExpression(TypeField::encode(kSpreadExpression));
  }

  static PreParserExpression FromIdentifier(PreParserIdentifier id) {
    return PreParserExpression(TypeField::encode(kIdentifierExpression) |
                               IdentifierTypeField::encode(id.type_));
  }

  static PreParserExpression BinaryOperation(PreParserExpression left,
                                             Token::Value op,
                                             PreParserExpression right) {
    return PreParserExpression(TypeField::encode(kBinaryOperationExpression));
  }

  static PreParserExpression Assignment() {
    return PreParserExpression(TypeField::encode(kExpression) |
                               ExpressionTypeField::encode(kAssignment));
  }

  static PreParserExpression ObjectLiteral() {
    return PreParserExpression(TypeField::encode(kObjectLiteralExpression));
  }

  static PreParserExpression ArrayLiteral() {
    return PreParserExpression(TypeField::encode(kArrayLiteralExpression));
  }

  static PreParserExpression StringLiteral() {
    return PreParserExpression(TypeField::encode(kStringLiteralExpression));
  }

  static PreParserExpression UseStrictStringLiteral() {
    return PreParserExpression(TypeField::encode(kStringLiteralExpression) |
                               IsUseStrictField::encode(true));
  }

  static PreParserExpression UseAsmStringLiteral() {
    return PreParserExpression(TypeField::encode(kStringLiteralExpression) |
                               IsUseAsmField::encode(true));
  }

  static PreParserExpression This() {
    return PreParserExpression(TypeField::encode(kExpression) |
                               ExpressionTypeField::encode(kThisExpression));
  }

  static PreParserExpression ThisProperty() {
    return PreParserExpression(
        TypeField::encode(kExpression) |
        ExpressionTypeField::encode(kThisPropertyExpression));
  }

  static PreParserExpression Property() {
    return PreParserExpression(
        TypeField::encode(kExpression) |
        ExpressionTypeField::encode(kPropertyExpression));
  }

  static PreParserExpression Call() {
    return PreParserExpression(TypeField::encode(kExpression) |
                               ExpressionTypeField::encode(kCallExpression));
  }

  static PreParserExpression CallEval() {
    return PreParserExpression(
        TypeField::encode(kExpression) |
        ExpressionTypeField::encode(kCallEvalExpression));
  }

  static PreParserExpression SuperCallReference() {
    return PreParserExpression(
        TypeField::encode(kExpression) |
        ExpressionTypeField::encode(kSuperCallReference));
  }

  static PreParserExpression NoTemplateTag() {
    return PreParserExpression(
        TypeField::encode(kExpression) |
        ExpressionTypeField::encode(kNoTemplateTagExpression));
  }

  bool IsEmpty() const { return TypeField::decode(code_) == kEmpty; }

  bool IsIdentifier() const {
    return TypeField::decode(code_) == kIdentifierExpression;
  }

  PreParserIdentifier AsIdentifier() const {
    DCHECK(IsIdentifier());
    return PreParserIdentifier(IdentifierTypeField::decode(code_));
  }

  bool IsAssignment() const {
    return TypeField::decode(code_) == kExpression &&
           ExpressionTypeField::decode(code_) == kAssignment;
  }

  bool IsObjectLiteral() const {
    return TypeField::decode(code_) == kObjectLiteralExpression;
  }

  bool IsArrayLiteral() const {
    return TypeField::decode(code_) == kArrayLiteralExpression;
  }

  bool IsStringLiteral() const {
    return TypeField::decode(code_) == kStringLiteralExpression;
  }

  bool IsUseStrictLiteral() const {
    return TypeField::decode(code_) == kStringLiteralExpression &&
           IsUseStrictField::decode(code_);
  }

  bool IsUseAsmLiteral() const {
    return TypeField::decode(code_) == kStringLiteralExpression &&
           IsUseAsmField::decode(code_);
  }

  bool IsThis() const {
    return TypeField::decode(code_) == kExpression &&
           ExpressionTypeField::decode(code_) == kThisExpression;
  }

  bool IsThisProperty() const {
    return TypeField::decode(code_) == kExpression &&
           ExpressionTypeField::decode(code_) == kThisPropertyExpression;
  }

  bool IsProperty() const {
    return TypeField::decode(code_) == kExpression &&
           (ExpressionTypeField::decode(code_) == kPropertyExpression ||
            ExpressionTypeField::decode(code_) == kThisPropertyExpression);
  }

  bool IsCall() const {
    return TypeField::decode(code_) == kExpression &&
           (ExpressionTypeField::decode(code_) == kCallExpression ||
            ExpressionTypeField::decode(code_) == kCallEvalExpression);
  }

  bool IsDirectEvalCall() const {
    return TypeField::decode(code_) == kExpression &&
           ExpressionTypeField::decode(code_) == kCallEvalExpression;
  }

  bool IsSuperCallReference() const {
    return TypeField::decode(code_) == kExpression &&
           ExpressionTypeField::decode(code_) == kSuperCallReference;
  }

  bool IsValidReferenceExpression() const {
    return IsIdentifier() || IsProperty();
  }

  // At the moment PreParser doesn't track these expression types.
  bool IsFunctionLiteral() const { return false; }
  bool IsCallNew() const { return false; }

  bool IsNoTemplateTag() const {
    return TypeField::decode(code_) == kExpression &&
           ExpressionTypeField::decode(code_) == kNoTemplateTagExpression;
  }

  bool IsSpread() const {
    return TypeField::decode(code_) == kSpreadExpression;
  }

  PreParserExpression AsFunctionLiteral() { return *this; }

  bool IsBinaryOperation() const {
    return TypeField::decode(code_) == kBinaryOperationExpression;
  }

  // Dummy implementation for making expression->somefunc() work in both Parser
  // and PreParser.
  PreParserExpression* operator->() { return this; }

  // More dummy implementations of things PreParser doesn't need to track:
  void set_index(int index) {}  // For YieldExpressions
  void set_should_eager_compile() {}
  void set_should_be_used_once_hint() {}

  int position() const { return kNoSourcePosition; }
  void set_function_token_position(int position) {}

 private:
  enum Type {
    kEmpty,
    kExpression,
    kIdentifierExpression,
    kStringLiteralExpression,
    kBinaryOperationExpression,
    kSpreadExpression,
    kObjectLiteralExpression,
    kArrayLiteralExpression
  };

  enum ExpressionType {
    kThisExpression,
    kThisPropertyExpression,
    kPropertyExpression,
    kCallExpression,
    kCallEvalExpression,
    kSuperCallReference,
    kNoTemplateTagExpression,
    kAssignment
  };

  explicit PreParserExpression(uint32_t expression_code)
      : code_(expression_code) {}

  // The first three bits are for the Type.
  typedef BitField<Type, 0, 3> TypeField;

  // The high order bit applies only to nodes which would inherit from the
  // Expression ASTNode --- This is by necessity, due to the fact that
  // Expression nodes may be represented as multiple Types, not exclusively
  // through kExpression.
  // TODO(caitp, adamk): clean up PreParserExpression bitfields.
  typedef BitField<bool, 31, 1> ParenthesizedField;

  // The rest of the bits are interpreted depending on the value
  // of the Type field, so they can share the storage.
  typedef BitField<ExpressionType, TypeField::kNext, 3> ExpressionTypeField;
  typedef BitField<bool, TypeField::kNext, 1> IsUseStrictField;
  typedef BitField<bool, IsUseStrictField::kNext, 1> IsUseAsmField;
  typedef BitField<PreParserIdentifier::Type, TypeField::kNext, 10>
      IdentifierTypeField;
  typedef BitField<bool, TypeField::kNext, 1> HasCoverInitializedNameField;

  uint32_t code_;
};


// The pre-parser doesn't need to build lists of expressions, identifiers, or
// the like.
template <typename T>
class PreParserList {
 public:
  // These functions make list->Add(some_expression) work (and do nothing).
  PreParserList() : length_(0) {}
  PreParserList* operator->() { return this; }
  void Add(T, void*) { ++length_; }
  int length() const { return length_; }
  static PreParserList Null() { return PreParserList(-1); }
  bool IsNull() const { return length_ == -1; }

 private:
  explicit PreParserList(int n) : length_(n) {}
  int length_;
};

typedef PreParserList<PreParserExpression> PreParserExpressionList;

class PreParserStatement;
typedef PreParserList<PreParserStatement> PreParserStatementList;

class PreParserStatement {
 public:
  static PreParserStatement Default() {
    return PreParserStatement(kUnknownStatement);
  }

  static PreParserStatement Jump() {
    return PreParserStatement(kJumpStatement);
  }

  static PreParserStatement FunctionDeclaration() {
    return PreParserStatement(kFunctionDeclaration);
  }

  // Creates expression statement from expression.
  // Preserves being an unparenthesized string literal, possibly
  // "use strict".
  static PreParserStatement ExpressionStatement(
      PreParserExpression expression) {
    if (expression.IsUseStrictLiteral()) {
      return PreParserStatement(kUseStrictExpressionStatement);
    }
    if (expression.IsUseAsmLiteral()) {
      return PreParserStatement(kUseAsmExpressionStatement);
    }
    if (expression.IsStringLiteral()) {
      return PreParserStatement(kStringLiteralExpressionStatement);
    }
    return Default();
  }

  bool IsStringLiteral() {
    return code_ == kStringLiteralExpressionStatement || IsUseStrictLiteral() ||
           IsUseAsmLiteral();
  }

  bool IsUseStrictLiteral() {
    return code_ == kUseStrictExpressionStatement;
  }

  bool IsUseAsmLiteral() { return code_ == kUseAsmExpressionStatement; }

  bool IsFunctionDeclaration() {
    return code_ == kFunctionDeclaration;
  }

  bool IsJumpStatement() {
    return code_ == kJumpStatement;
  }

  // Dummy implementation for making statement->somefunc() work in both Parser
  // and PreParser.
  PreParserStatement* operator->() { return this; }

  PreParserStatementList statements() { return PreParserStatementList(); }

 private:
  enum Type {
    kUnknownStatement,
    kJumpStatement,
    kStringLiteralExpressionStatement,
    kUseStrictExpressionStatement,
    kUseAsmExpressionStatement,
    kFunctionDeclaration
  };

  explicit PreParserStatement(Type code) : code_(code) {}
  Type code_;
};


class PreParserFactory {
 public:
  explicit PreParserFactory(void* unused_value_factory) {}
  PreParserExpression NewStringLiteral(PreParserIdentifier identifier,
                                       int pos) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewNumberLiteral(double number,
                                       int pos) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewRegExpLiteral(PreParserIdentifier js_pattern,
                                       int js_flags, int literal_index,
                                       int pos) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewArrayLiteral(PreParserExpressionList values,
                                      int literal_index,
                                      int pos) {
    return PreParserExpression::ArrayLiteral();
  }
  PreParserExpression NewArrayLiteral(PreParserExpressionList values,
                                      int first_spread_index, int literal_index,
                                      int pos) {
    return PreParserExpression::ArrayLiteral();
  }
  PreParserExpression NewClassLiteralProperty(PreParserExpression key,
                                              PreParserExpression value,
                                              ClassLiteralProperty::Kind kind,
                                              bool is_static,
                                              bool is_computed_name) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewObjectLiteralProperty(PreParserExpression key,
                                               PreParserExpression value,
                                               ObjectLiteralProperty::Kind kind,
                                               bool is_computed_name) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewObjectLiteralProperty(PreParserExpression key,
                                               PreParserExpression value,
                                               bool is_computed_name) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewObjectLiteral(PreParserExpressionList properties,
                                       int literal_index,
                                       int boilerplate_properties,
                                       int pos) {
    return PreParserExpression::ObjectLiteral();
  }
  PreParserExpression NewVariableProxy(void* variable) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewProperty(PreParserExpression obj,
                                  PreParserExpression key,
                                  int pos) {
    if (obj.IsThis()) {
      return PreParserExpression::ThisProperty();
    }
    return PreParserExpression::Property();
  }
  PreParserExpression NewUnaryOperation(Token::Value op,
                                        PreParserExpression expression,
                                        int pos) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewBinaryOperation(Token::Value op,
                                         PreParserExpression left,
                                         PreParserExpression right, int pos) {
    return PreParserExpression::BinaryOperation(left, op, right);
  }
  PreParserExpression NewCompareOperation(Token::Value op,
                                          PreParserExpression left,
                                          PreParserExpression right, int pos) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewRewritableExpression(PreParserExpression expression) {
    return expression;
  }
  PreParserExpression NewAssignment(Token::Value op,
                                    PreParserExpression left,
                                    PreParserExpression right,
                                    int pos) {
    return PreParserExpression::Assignment();
  }
  PreParserExpression NewYield(PreParserExpression generator_object,
                               PreParserExpression expression, int pos,
                               Yield::OnException on_exception) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewConditional(PreParserExpression condition,
                                     PreParserExpression then_expression,
                                     PreParserExpression else_expression,
                                     int pos) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewCountOperation(Token::Value op,
                                        bool is_prefix,
                                        PreParserExpression expression,
                                        int pos) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewCall(
      PreParserExpression expression, PreParserExpressionList arguments,
      int pos, Call::PossiblyEval possibly_eval = Call::NOT_EVAL) {
    if (possibly_eval == Call::IS_POSSIBLY_EVAL) {
      DCHECK(expression.IsIdentifier() && expression.AsIdentifier().IsEval());
      return PreParserExpression::CallEval();
    }
    return PreParserExpression::Call();
  }
  PreParserExpression NewCallNew(PreParserExpression expression,
                                 PreParserExpressionList arguments,
                                 int pos) {
    return PreParserExpression::Default();
  }
  PreParserExpression NewCallRuntime(const AstRawString* name,
                                     const Runtime::Function* function,
                                     PreParserExpressionList arguments,
                                     int pos) {
    return PreParserExpression::Default();
  }
  PreParserStatement NewReturnStatement(PreParserExpression expression,
                                        int pos) {
    return PreParserStatement::Default();
  }
  PreParserExpression NewFunctionLiteral(
      PreParserIdentifier name, Scope* scope, PreParserStatementList body,
      int materialized_literal_count, int expected_property_count,
      int parameter_count,
      FunctionLiteral::ParameterFlag has_duplicate_parameters,
      FunctionLiteral::FunctionType function_type,
      FunctionLiteral::EagerCompileHint eager_compile_hint, FunctionKind kind,
      int position) {
    return PreParserExpression::Default();
  }

  PreParserExpression NewSpread(PreParserExpression expression, int pos,
                                int expr_pos) {
    return PreParserExpression::Spread(expression);
  }

  PreParserExpression NewEmptyParentheses(int pos) {
    return PreParserExpression::Default();
  }

  PreParserStatement NewEmptyStatement(int pos) {
    return PreParserStatement::Default();
  }

  PreParserStatement NewBlock(ZoneList<const AstRawString*>* labels,
                              int capacity, bool ignore_completion_value,
                              int pos) {
    return PreParserStatement::Default();
  }

  // Return the object itself as AstVisitor and implement the needed
  // dummy method right in this class.
  PreParserFactory* visitor() { return this; }
  int* ast_properties() {
    static int dummy = 42;
    return &dummy;
  }
};


struct PreParserFormalParameters : FormalParametersBase {
  explicit PreParserFormalParameters(DeclarationScope* scope)
      : FormalParametersBase(scope) {}
  int arity = 0;

  int Arity() const { return arity; }
  PreParserIdentifier at(int i) { return PreParserIdentifier(); }  // Dummy
};


class PreParser;

class PreParserTarget {
 public:
  PreParserTarget(ParserBase<PreParser>* preparser,
                  PreParserStatement statement) {}
};

class PreParserTargetScope {
 public:
  explicit PreParserTargetScope(ParserBase<PreParser>* preparser) {}
};

template <>
struct ParserTypes<PreParser> {
  typedef ParserBase<PreParser> Base;
  typedef PreParser Impl;

  // PreParser doesn't need to store generator variables.
  typedef void GeneratorVariable;

  // Return types for traversing functions.
  typedef PreParserIdentifier Identifier;
  typedef PreParserExpression Expression;
  typedef PreParserExpression FunctionLiteral;
  typedef PreParserExpression ObjectLiteralProperty;
  typedef PreParserExpression ClassLiteralProperty;
  typedef PreParserExpressionList ExpressionList;
  typedef PreParserExpressionList PropertyList;
  typedef PreParserFormalParameters FormalParameters;
  typedef PreParserStatement Statement;
  typedef PreParserStatementList StatementList;
  typedef PreParserStatement Block;

  // For constructing objects returned by the traversing functions.
  typedef PreParserFactory Factory;

  typedef PreParserTarget Target;
  typedef PreParserTargetScope TargetScope;
};


// Preparsing checks a JavaScript program and emits preparse-data that helps
// a later parsing to be faster.
// See preparse-data-format.h for the data format.

// The PreParser checks that the syntax follows the grammar for JavaScript,
// and collects some information about the program along the way.
// The grammar check is only performed in order to understand the program
// sufficiently to deduce some information about it, that can be used
// to speed up later parsing. Finding errors is not the goal of pre-parsing,
// rather it is to speed up properly written and correct programs.
// That means that contextual checks (like a label being declared where
// it is used) are generally omitted.
class PreParser : public ParserBase<PreParser> {
  friend class ParserBase<PreParser>;
  friend class v8::internal::ExpressionClassifier<ParserTypes<PreParser>>;

 public:
  typedef PreParserIdentifier Identifier;
  typedef PreParserExpression Expression;
  typedef PreParserStatement Statement;

  enum PreParseResult {
    kPreParseStackOverflow,
    kPreParseAbort,
    kPreParseSuccess
  };

  PreParser(Zone* zone, Scanner* scanner, AstValueFactory* ast_value_factory,
            ParserRecorder* log, uintptr_t stack_limit)
      : ParserBase<PreParser>(zone, scanner, stack_limit, NULL,
                              ast_value_factory, log),
        use_counts_(nullptr) {}

  // Pre-parse the program from the character stream; returns true on
  // success (even if parsing failed, the pre-parse data successfully
  // captured the syntax error), and false if a stack-overflow happened
  // during parsing.
  PreParseResult PreParseProgram(int* materialized_literals = 0,
                                 bool is_module = false) {
    DCHECK_NULL(scope_state_);
    DeclarationScope* scope = NewScriptScope();

    // ModuleDeclarationInstantiation for Source Text Module Records creates a
    // new Module Environment Record whose outer lexical environment record is
    // the global scope.
    if (is_module) scope = NewModuleScope(scope);

    FunctionState top_scope(&function_state_, &scope_state_, scope,
                            kNormalFunction);
    bool ok = true;
    int start_position = scanner()->peek_location().beg_pos;
    parsing_module_ = is_module;
    PreParserStatementList body;
    ParseStatementList(body, Token::EOS, &ok);
    if (stack_overflow()) return kPreParseStackOverflow;
    if (!ok) {
      ReportUnexpectedToken(scanner()->current_token());
    } else if (is_strict(this->scope()->language_mode())) {
      CheckStrictOctalLiteral(start_position, scanner()->location().end_pos,
                              &ok);
      CheckDecimalLiteralWithLeadingZero(use_counts_, start_position,
                                         scanner()->location().end_pos);
    }
    if (materialized_literals) {
      *materialized_literals = function_state_->materialized_literal_count();
    }
    return kPreParseSuccess;
  }

  // Parses a single function literal, from the opening parentheses before
  // parameters to the closing brace after the body.
  // Returns a FunctionEntry describing the body of the function in enough
  // detail that it can be lazily compiled.
  // The scanner is expected to have matched the "function" or "function*"
  // keyword and parameters, and have consumed the initial '{'.
  // At return, unless an error occurred, the scanner is positioned before the
  // the final '}'.
  PreParseResult PreParseLazyFunction(LanguageMode language_mode,
                                      FunctionKind kind,
                                      bool has_simple_parameters,
                                      bool parsing_module, ParserRecorder* log,
                                      bool may_abort, int* use_counts);

 private:
  // These types form an algebra over syntactic categories that is just
  // rich enough to let us recognize and propagate the constructs that
  // are either being counted in the preparser data, or is important
  // to throw the correct syntax error exceptions.

  // All ParseXXX functions take as the last argument an *ok parameter
  // which is set to false if parsing failed; it is unchanged otherwise.
  // By making the 'exception handling' explicit, we are forced to check
  // for failure at the call sites.
  Statement ParseScopedStatement(bool legacy, bool* ok);
  Statement ParseHoistableDeclaration(ZoneList<const AstRawString*>* names,
                                      bool default_export, bool* ok);
  Statement ParseHoistableDeclaration(int pos, ParseFunctionFlags flags,
                                      ZoneList<const AstRawString*>* names,
                                      bool default_export, bool* ok);
  Statement ParseFunctionDeclaration(bool* ok);
  Statement ParseAsyncFunctionDeclaration(ZoneList<const AstRawString*>* names,
                                          bool default_export, bool* ok);
  Expression ParseAsyncFunctionExpression(bool* ok);
  Statement ParseClassDeclaration(ZoneList<const AstRawString*>* names,
                                  bool default_export, bool* ok);
  Statement ParseBlock(ZoneList<const AstRawString*>* labels, bool* ok);
  Statement ParseVariableStatement(VariableDeclarationContext var_context,
                                   ZoneList<const AstRawString*>* names,
                                   bool* ok);
  Statement ParseExpressionOrLabelledStatement(
      ZoneList<const AstRawString*>* names,
      AllowLabelledFunctionStatement allow_function, bool* ok);
  Statement ParseIfStatement(ZoneList<const AstRawString*>* labels, bool* ok);
  Statement ParseContinueStatement(bool* ok);
  Statement ParseBreakStatement(ZoneList<const AstRawString*>* labels,
                                bool* ok);
  Statement ParseReturnStatement(bool* ok);
  Statement ParseWithStatement(ZoneList<const AstRawString*>* labels, bool* ok);
  Statement ParseSwitchStatement(ZoneList<const AstRawString*>* labels,
                                 bool* ok);
  Statement ParseDoWhileStatement(ZoneList<const AstRawString*>* labels,
                                  bool* ok);
  Statement ParseWhileStatement(ZoneList<const AstRawString*>* labels,
                                bool* ok);
  Statement ParseForStatement(ZoneList<const AstRawString*>* labels, bool* ok);
  Statement ParseThrowStatement(bool* ok);
  Statement ParseTryStatement(bool* ok);
  Statement ParseDebuggerStatement(bool* ok);
  Expression ParseConditionalExpression(bool accept_IN, bool* ok);
  Expression ParseObjectLiteral(bool* ok);
  Expression ParseV8Intrinsic(bool* ok);
  Expression ParseDoExpression(bool* ok);

  V8_INLINE PreParserStatementList ParseEagerFunctionBody(
      PreParserIdentifier function_name, int pos,
      const PreParserFormalParameters& parameters, FunctionKind kind,
      FunctionLiteral::FunctionType function_type, bool* ok);

  V8_INLINE LazyParsingResult
  SkipLazyFunctionBody(int* materialized_literal_count,
                       int* expected_property_count, bool may_abort, bool* ok) {
    UNREACHABLE();
    return kLazyParsingComplete;
  }
  Expression ParseFunctionLiteral(
      Identifier name, Scanner::Location function_name_location,
      FunctionNameValidity function_name_validity, FunctionKind kind,
      int function_token_pos, FunctionLiteral::FunctionType function_type,
      LanguageMode language_mode, bool* ok);
  LazyParsingResult ParseLazyFunctionLiteralBody(bool may_abort, bool* ok);

  PreParserExpression ParseClassLiteral(PreParserIdentifier name,
                                        Scanner::Location class_name_location,
                                        bool name_is_strict_reserved, int pos,
                                        bool* ok);

  struct TemplateLiteralState {};

  V8_INLINE TemplateLiteralState OpenTemplateLiteral(int pos) {
    return TemplateLiteralState();
  }
  V8_INLINE void AddTemplateExpression(TemplateLiteralState* state,
                                       PreParserExpression expression) {}
  V8_INLINE void AddTemplateSpan(TemplateLiteralState* state, bool tail) {}
  V8_INLINE PreParserExpression CloseTemplateLiteral(
      TemplateLiteralState* state, int start, PreParserExpression tag);
  V8_INLINE void CheckConflictingVarDeclarations(Scope* scope, bool* ok) {}

  V8_INLINE void SetLanguageMode(Scope* scope, LanguageMode mode) {
    scope->SetLanguageMode(mode);
  }
  V8_INLINE void SetAsmModule() {}

  V8_INLINE void MarkCollectedTailCallExpressions() {}
  V8_INLINE void MarkTailPosition(PreParserExpression expression) {}

  void ParseAsyncArrowSingleExpressionBody(PreParserStatementList body,
                                           bool accept_IN,
                                           int pos, bool* ok);

  V8_INLINE PreParserExpressionList
  PrepareSpreadArguments(PreParserExpressionList list) {
    return list;
  }

  V8_INLINE PreParserExpression SpreadCall(PreParserExpression function,
                                           PreParserExpressionList args,
                                           int pos);
  V8_INLINE PreParserExpression SpreadCallNew(PreParserExpression function,
                                              PreParserExpressionList args,
                                              int pos);

  V8_INLINE void RewriteDestructuringAssignments() {}

  V8_INLINE PreParserExpression RewriteExponentiation(PreParserExpression left,
                                                      PreParserExpression right,
                                                      int pos) {
    return left;
  }
  V8_INLINE PreParserExpression RewriteAssignExponentiation(
      PreParserExpression left, PreParserExpression right, int pos) {
    return left;
  }

  V8_INLINE PreParserExpression
  RewriteAwaitExpression(PreParserExpression value, int pos) {
    return value;
  }
  V8_INLINE PreParserExpression RewriteYieldStar(PreParserExpression generator,
                                                 PreParserExpression expression,
                                                 int pos) {
    return PreParserExpression::Default();
  }
  V8_INLINE void RewriteNonPattern(bool* ok) { ValidateExpression(ok); }

  V8_INLINE void DeclareAndInitializeVariables(
      PreParserStatement block,
      const DeclarationDescriptor* declaration_descriptor,
      const DeclarationParsingResult::Declaration* declaration,
      ZoneList<const AstRawString*>* names, bool* ok) {}

  V8_INLINE void QueueDestructuringAssignmentForRewriting(
      PreParserExpression assignment) {}
  V8_INLINE void QueueNonPatternForRewriting(PreParserExpression expr,
                                             bool* ok) {}

  // Helper functions for recursive descent.
  V8_INLINE bool IsEval(PreParserIdentifier identifier) const {
    return identifier.IsEval();
  }

  V8_INLINE bool IsArguments(PreParserIdentifier identifier) const {
    return identifier.IsArguments();
  }

  V8_INLINE bool IsEvalOrArguments(PreParserIdentifier identifier) const {
    return identifier.IsEvalOrArguments();
  }

  V8_INLINE bool IsUndefined(PreParserIdentifier identifier) const {
    return identifier.IsUndefined();
  }

  V8_INLINE bool IsAwait(PreParserIdentifier identifier) const {
    return identifier.IsAwait();
  }

  V8_INLINE bool IsFutureStrictReserved(PreParserIdentifier identifier) const {
    return identifier.IsFutureStrictReserved();
  }

  // Returns true if the expression is of type "this.foo".
  V8_INLINE static bool IsThisProperty(PreParserExpression expression) {
    return expression.IsThisProperty();
  }

  V8_INLINE static bool IsIdentifier(PreParserExpression expression) {
    return expression.IsIdentifier();
  }

  V8_INLINE static PreParserIdentifier AsIdentifier(
      PreParserExpression expression) {
    return expression.AsIdentifier();
  }

  V8_INLINE bool IsPrototype(PreParserIdentifier identifier) const {
    return identifier.IsPrototype();
  }

  V8_INLINE bool IsConstructor(PreParserIdentifier identifier) const {
    return identifier.IsConstructor();
  }

  V8_INLINE bool IsDirectEvalCall(PreParserExpression expression) const {
    return expression.IsDirectEvalCall();
  }

  V8_INLINE static bool IsBoilerplateProperty(PreParserExpression property) {
    // PreParser doesn't count boilerplate properties.
    return false;
  }

  V8_INLINE static bool IsArrayIndex(PreParserIdentifier string,
                                     uint32_t* index) {
    return false;
  }

  V8_INLINE bool IsUseStrictDirective(PreParserStatement statement) const {
    return statement.IsUseStrictLiteral();
  }

  V8_INLINE bool IsUseAsmDirective(PreParserStatement statement) const {
    return statement.IsUseAsmLiteral();
  }

  V8_INLINE bool IsStringLiteral(PreParserStatement statement) const {
    return statement.IsStringLiteral();
  }

  V8_INLINE static PreParserExpression GetPropertyValue(
      PreParserExpression property) {
    return PreParserExpression::Default();
  }

  // Functions for encapsulating the differences between parsing and preparsing;
  // operations interleaved with the recursive descent.
  V8_INLINE static void PushLiteralName(PreParserIdentifier id) {}
  V8_INLINE static void PushVariableName(PreParserIdentifier id) {}
  V8_INLINE void PushPropertyName(PreParserExpression expression) {}
  V8_INLINE static void InferFunctionName(PreParserExpression expression) {}

  V8_INLINE static void CheckAssigningFunctionLiteralToProperty(
      PreParserExpression left, PreParserExpression right) {}

  V8_INLINE static PreParserExpression MarkExpressionAsAssigned(
      PreParserExpression expression) {
    // TODO(marja): To be able to produce the same errors, the preparser needs
    // to start tracking which expressions are variables and which are assigned.
    return expression;
  }

  V8_INLINE bool ShortcutNumericLiteralBinaryExpression(PreParserExpression* x,
                                                        PreParserExpression y,
                                                        Token::Value op,
                                                        int pos) {
    return false;
  }

  V8_INLINE PreParserExpression BuildUnaryExpression(
      PreParserExpression expression, Token::Value op, int pos) {
    return PreParserExpression::Default();
  }

  V8_INLINE PreParserExpression BuildIteratorResult(PreParserExpression value,
                                                    bool done) {
    return PreParserExpression::Default();
  }

  V8_INLINE PreParserExpression
  NewThrowReferenceError(MessageTemplate::Template message, int pos) {
    return PreParserExpression::Default();
  }

  V8_INLINE PreParserExpression NewThrowSyntaxError(
      MessageTemplate::Template message, PreParserIdentifier arg, int pos) {
    return PreParserExpression::Default();
  }

  V8_INLINE PreParserExpression NewThrowTypeError(
      MessageTemplate::Template message, PreParserIdentifier arg, int pos) {
    return PreParserExpression::Default();
  }

  // Reporting errors.
  V8_INLINE void ReportMessageAt(Scanner::Location source_location,
                                 MessageTemplate::Template message,
                                 const char* arg = NULL,
                                 ParseErrorType error_type = kSyntaxError) {
    log_->LogMessage(source_location.beg_pos, source_location.end_pos, message,
                     arg, error_type);
  }

  V8_INLINE void ReportMessageAt(Scanner::Location source_location,
                                 MessageTemplate::Template message,
                                 const AstRawString* arg,
                                 ParseErrorType error_type = kSyntaxError) {
    UNREACHABLE();
  }

  // "null" return type creators.
  V8_INLINE static PreParserIdentifier EmptyIdentifier() {
    return PreParserIdentifier::Default();
  }
  V8_INLINE static PreParserExpression EmptyExpression() {
    return PreParserExpression::Empty();
  }
  V8_INLINE static PreParserExpression EmptyLiteral() {
    return PreParserExpression::Default();
  }
  V8_INLINE static PreParserExpression EmptyObjectLiteralProperty() {
    return PreParserExpression::Default();
  }
  V8_INLINE static PreParserExpression EmptyClassLiteralProperty() {
    return PreParserExpression::Default();
  }
  V8_INLINE static PreParserExpression EmptyFunctionLiteral() {
    return PreParserExpression::Default();
  }

  V8_INLINE static bool IsEmptyExpression(PreParserExpression expr) {
    return expr.IsEmpty();
  }

  V8_INLINE static PreParserExpressionList NullExpressionList() {
    return PreParserExpressionList::Null();
  }

  V8_INLINE static bool IsNullExpressionList(PreParserExpressionList exprs) {
    return exprs.IsNull();
  }

  V8_INLINE static PreParserStatementList NullStatementList() {
    return PreParserStatementList::Null();
  }

  V8_INLINE static bool IsNullStatementList(PreParserStatementList stmts) {
    return stmts.IsNull();
  }

  V8_INLINE static PreParserStatement NullStatement() {
    return PreParserStatement::Default();
  }

  V8_INLINE bool IsNullOrEmptyStatement(PreParserStatement stmt) {
    // TODO(nikolaos): See if this needs to be consistent for the preparser.
    return false;
  }

  V8_INLINE static PreParserStatement NullBlock() {
    return PreParserStatement::Default();
  }

  V8_INLINE PreParserIdentifier EmptyIdentifierString() const {
    return PreParserIdentifier::Default();
  }

  // Odd-ball literal creators.
  V8_INLINE PreParserExpression GetLiteralTheHole(int position) {
    return PreParserExpression::Default();
  }

  V8_INLINE PreParserExpression GetLiteralUndefined(int position) {
    return PreParserExpression::Default();
  }

  // Producing data during the recursive descent.
  PreParserIdentifier GetSymbol() const;

  V8_INLINE PreParserIdentifier GetNextSymbol() const {
    return PreParserIdentifier::Default();
  }

  V8_INLINE PreParserIdentifier GetNumberAsSymbol() const {
    return PreParserIdentifier::Default();
  }

  V8_INLINE PreParserExpression ThisExpression(int pos = kNoSourcePosition) {
    return PreParserExpression::This();
  }

  V8_INLINE PreParserExpression NewSuperPropertyReference(int pos) {
    return PreParserExpression::Default();
  }

  V8_INLINE PreParserExpression NewSuperCallReference(int pos) {
    return PreParserExpression::SuperCallReference();
  }

  V8_INLINE PreParserExpression NewTargetExpression(int pos) {
    return PreParserExpression::Default();
  }

  V8_INLINE PreParserExpression FunctionSentExpression(int pos) {
    return PreParserExpression::Default();
  }

  V8_INLINE PreParserExpression ExpressionFromLiteral(Token::Value token,
                                                      int pos) {
    return PreParserExpression::Default();
  }

  V8_INLINE PreParserExpression ExpressionFromIdentifier(
      PreParserIdentifier name, int start_position, int end_position,
      InferName infer = InferName::kYes) {
    return PreParserExpression::FromIdentifier(name);
  }

  V8_INLINE PreParserExpression ExpressionFromString(int pos) {
    if (scanner()->UnescapedLiteralMatches("use strict", 10)) {
      return PreParserExpression::UseStrictStringLiteral();
    }
    return PreParserExpression::StringLiteral();
  }

  V8_INLINE PreParserExpressionList NewExpressionList(int size) const {
    return PreParserExpressionList();
  }

  V8_INLINE PreParserExpressionList NewPropertyList(int size) const {
    return PreParserExpressionList();
  }

  V8_INLINE PreParserStatementList NewStatementList(int size) const {
    return PreParserStatementList();
  }

  V8_INLINE static PreParserStatement NewBlock(
      ZoneList<const AstRawString*>* labels, int capacity,
      bool ignore_completion_value, int pos) {
    return PreParserStatement::Default();
  }

  V8_INLINE void AddParameterInitializationBlock(
      const PreParserFormalParameters& parameters, PreParserStatementList body,
      bool is_async, bool* ok) {}

  V8_INLINE void AddFormalParameter(PreParserFormalParameters* parameters,
                                    PreParserExpression pattern,
                                    PreParserExpression initializer,
                                    int initializer_end_position,
                                    bool is_rest) {
    ++parameters->arity;
  }

  V8_INLINE void DeclareFormalParameter(DeclarationScope* scope,
                                        PreParserIdentifier parameter) {
    if (!classifier()->is_simple_parameter_list()) {
      scope->SetHasNonSimpleParameters();
    }
  }

  V8_INLINE void ParseArrowFunctionFormalParameterList(
      PreParserFormalParameters* parameters, PreParserExpression params,
      const Scanner::Location& params_loc, Scanner::Location* duplicate_loc,
      const Scope::Snapshot& scope_snapshot, bool* ok) {
    // TODO(wingo): Detect duplicated identifiers in paramlists.  Detect
    // parameter
    // lists that are too long.
  }

  V8_INLINE void ReindexLiterals(const PreParserFormalParameters& parameters) {}

  V8_INLINE PreParserExpression NoTemplateTag() {
    return PreParserExpression::NoTemplateTag();
  }

  V8_INLINE static bool IsTaggedTemplate(const PreParserExpression tag) {
    return !tag.IsNoTemplateTag();
  }

  V8_INLINE void MaterializeUnspreadArgumentsLiterals(int count) {
    for (int i = 0; i < count; ++i) {
      function_state_->NextMaterializedLiteralIndex();
    }
  }

  V8_INLINE PreParserExpression
  ExpressionListToExpression(PreParserExpressionList args) {
    return PreParserExpression::Default();
  }

  V8_INLINE void AddAccessorPrefixToFunctionName(bool is_get,
                                                 PreParserExpression function,
                                                 PreParserIdentifier name) {}
  V8_INLINE void SetFunctionNameFromPropertyName(PreParserExpression property,
                                                 PreParserIdentifier name) {}
  V8_INLINE void SetFunctionNameFromIdentifierRef(
      PreParserExpression value, PreParserExpression identifier) {}

  V8_INLINE ZoneList<typename ExpressionClassifier::Error>*
  GetReportedErrorList() const {
    return function_state_->GetReportedErrorList();
  }

  V8_INLINE ZoneList<PreParserExpression>* GetNonPatternList() const {
    return function_state_->non_patterns_to_rewrite();
  }

  // Preparser's private field members.

  int* use_counts_;
};

PreParserExpression PreParser::SpreadCall(PreParserExpression function,
                                          PreParserExpressionList args,
                                          int pos) {
  return factory()->NewCall(function, args, pos);
}

PreParserExpression PreParser::SpreadCallNew(PreParserExpression function,
                                             PreParserExpressionList args,
                                             int pos) {
  return factory()->NewCallNew(function, args, pos);
}

PreParserStatementList PreParser::ParseEagerFunctionBody(
    PreParserIdentifier function_name, int pos,
    const PreParserFormalParameters& parameters, FunctionKind kind,
    FunctionLiteral::FunctionType function_type, bool* ok) {
  ParsingModeScope parsing_mode(this, PARSE_EAGERLY);
  PreParserStatementList result;

  Scope* inner_scope = scope();
  if (!parameters.is_simple) inner_scope = NewScope(BLOCK_SCOPE);

  {
    BlockState block_state(&scope_state_, inner_scope);
    ParseStatementList(result, Token::RBRACE, ok);
    if (!*ok) return PreParserStatementList();
  }

  Expect(Token::RBRACE, ok);
  return result;
}

PreParserExpression PreParser::CloseTemplateLiteral(TemplateLiteralState* state,
                                                    int start,
                                                    PreParserExpression tag) {
  if (IsTaggedTemplate(tag)) {
    // Emulate generation of array literals for tag callsite
    // 1st is array of cooked strings, second is array of raw strings
    function_state_->NextMaterializedLiteralIndex();
    function_state_->NextMaterializedLiteralIndex();
  }
  return EmptyExpression();
}

}  // namespace internal
}  // namespace v8

#endif  // V8_PARSING_PREPARSER_H
