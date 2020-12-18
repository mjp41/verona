#pragma once

#include <string>
#include <memory>
#include <list>
#include "compiler/source_manager.h"
#include <functional>

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
  struct Generics;
  struct TypeParameterDef;

  struct WhereClause;
  struct WhereClauseTerm;

  struct Type;
  struct TypeSignature;

  struct Expression;
  struct Constraint;

  struct TypeExpression;

  struct LocalID;

  class Context;
  struct Instantiation;
  struct Type;
  typedef std::shared_ptr<const Type> TypePtr;


  enum class EntityKind
  {
    Class,
    Interface,
    Primitive,
  };

/**
 * The following methods are accessors for bits of the AST that the rest of the
 * pipeline depends upon.
 *
 * TODO: Design this better and abstract.
 */
  const std::string name(const Entity&);
  bool is_a_class(const Entity*);
  EntityKind get_entity_kind(const Entity&);
  const Entity* find_entity(const Program& program, const std::string& name);

  const std::string name(const Method&);
  const Entity* parent(const Method&);
  const std::string long_name(const Method&);
  bool is_a_builtin(const Method&);
  const TypeSignature& type_signature(const Method&);
  TypePtr containing_type(Context& context, const Method&);
  // TODO insane return type.

  const std::list<std::unique_ptr<TypeParameterDef>>& generics_for_entity(const Entity&);
  const std::list<std::unique_ptr<TypeParameterDef>>& generics_for_method(const Method&);
  TypePtr get_bound(const TypeParameterDef&);
  size_t get_index(const TypeParameterDef&);

  const SourceManager::SourceRange& entity_name_source_range(const Entity& e);
  bool is_valid_main_method(Context& context, const Method& main);

  const std::string name(const Field&);
  void for_each_field(const Entity&, std::function<void(const Field&)>);

  std::string
  instantiated_path(const Entity& entity, const Instantiation& instantiation);
  std::string instantiated_path(const Method& method, const Instantiation& instantiation);
  std::string instantiated_path(const Field& field, const Instantiation& instantiation);

}

