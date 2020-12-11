#include <string>
#include <memory>
#include <list>

namespace verona::compiler
{
  struct Program;
  struct Method;
  struct Field;
  struct Entity;
  struct FnSignature;

  struct Program;
  struct Entity;
  struct Member;

  struct StaticAssertion;

  struct FnSignature;
  struct FnBody;
  struct Receiver;
  struct FnParameter;
  struct Generics;
  struct TypeParameterDef;

  struct WhereClause;
  struct WhereClauseTerm;

  struct Type;
  struct NewParent;
  struct TypeSignature;

  struct Expression;
  struct Argument;
  struct MatchArm;
  struct Constraint;

  struct TypeExpression;

  struct LocalID;

  class Context;
  struct Type;
  typedef std::shared_ptr<const Type> TypePtr;

  bool is_a_class(const Entity*);
  const std::string long_name(const Method&);
  const TypeSignature& type_signature(const Method&);
  TypePtr containing_type(Context& context, const Method&);
  const Entity* find_entity(const Program& program, const std::string& name);
  // TODO insane return type.
  const std::list<std::unique_ptr<TypeParameterDef>>& generics_for_entity(const Entity&);
  TypePtr get_bound(const TypeParameterDef&);

}

