// Copyright Microsoft and Project Verona Contributors.
// SPDX-License-Identifier: MIT
#include "compiler/ast.h"

#include "compiler/format.h"
#include "compiler/instantiation.h"
#include "compiler/intern.h"
#include "compiler/printing.h"
#include "compiler/zip.h"

namespace verona::compiler
{
  using pegmatite::ASTConstant;
  using pegmatite::ASTStack;
  using pegmatite::ASTStackEntry;
  using pegmatite::ErrorReporter;
  using pegmatite::InputRange;
  using pegmatite::popFromASTStack;

  namespace
  {
    /**
     * Get a string describing the type arguments for given generics and
     * instantiation.
     *
     * The string will be empty if there are no generics, or will be of the form
     * "[T1, T2]" otherwise.
     */
    std::string instantiated_generics(
      const Generics& generics, const Instantiation& instantiation)
    {
      using format::optional_list;
      using format::to_string;

      auto map_parameter = [&](const std::unique_ptr<TypeParameterDef>& elem) {
        return instantiation.types().at(elem->index);
      };

      return to_string(optional_list(generics.types, map_parameter));
    }
  }

  const Name& Symbol::name() const
  {
    return match(
      *this,
      [](const ErrorSymbol&) -> const Name& {
        throw std::logic_error("Found error symbol");
      },
      [](const auto* inner) -> const Name& { return inner->name; });
  }

  std::string Entity::path() const
  {
    return name;
  }

  std::string
  instantiated_path(const Entity& entity, const Instantiation& instantiation)
  {
    return fmt::format(
      "{}{}", entity.name, instantiated_generics(*entity.generics, instantiation));
  }

  std::string Member::path() const
  {
    if (parent == nullptr)
      throw std::logic_error("path called before resolution");

    return parent->path() + "." + get_name();
  }
  std::string instantiated_path(const Method& method, const Instantiation& instantiation)
  {
    if (method.parent == nullptr)
      throw std::logic_error("instantiated_path called before resolution");

    return fmt::format(
      "{}.{}{}",
      instantiated_path(*method.parent, instantiation),
      method.name,
      instantiated_generics(*method.signature->generics, instantiation));
  }
  
  std::string instantiated_path(const Field& field, const Instantiation& instantiation)
  {
    if (field.parent == nullptr)
      throw std::logic_error("instantiated_path called before resolution");

    std::string buf;
    buf.append(instantiated_path(*field.parent, instantiation));
    buf.append(".");
    buf.append(field.name);
    return buf;
  }

  /**
   * Binary operators are desugared into method calls, based on the name
   * returned by this function.
   */
  std::string_view binary_operator_method_name(BinaryOperator op)
  {
    switch (op)
    {
      case BinaryOperator::Add:
        return "add";
      case BinaryOperator::Sub:
        return "sub";
      case BinaryOperator::Mul:
        return "mul";
      case BinaryOperator::Div:
        return "div";
      case BinaryOperator::Mod:
        return "mod";
      case BinaryOperator::Shl:
        return "shl";
      case BinaryOperator::Shr:
        return "shr";
      case BinaryOperator::Lt:
        return "lt";
      case BinaryOperator::Le:
        return "le";
      case BinaryOperator::Gt:
        return "gt";
      case BinaryOperator::Ge:
        return "ge";
      case BinaryOperator::Eq:
        return "eq";
      case BinaryOperator::Ne:
        return "ne";
      case BinaryOperator::And:
        return "and";
      case BinaryOperator::Or:
        return "or";

        EXHAUSTIVE_SWITCH
    }
  }

  bool is_a_class(const Entity* e)
  {
    return e->kind->value() == Entity::Class;
  }

  const std::string long_name(const Method& m)
  {
    return m.path();
  }

  const TypeSignature& type_signature(const Method& method)
  {
    return method.signature->types;
  }

  /**
   * Returns a suitably specialised version of the containing class.
   */
  TypePtr containing_type(Context& context, const Method& method)
  {
    auto entity = method.parent;

    TypeList arguments;
    for (const auto& param : entity->generics->types)
    {
      arguments.push_back(
        context.mk_type_parameter(param.get(), TypeParameter::Expanded::No));
    }
    return context.mk_entity_type(entity, arguments);
  }

  const Entity* find_entity(const Program& program, const std::string& name)
  {
    return program.find_entity(name);
  }

  const Entity* parent(const Method& method)
  {
    return method.parent;
  }

  const std::list<std::unique_ptr<TypeParameterDef>>& generics_for_entity(const Entity& entity)
  {
    return entity.generics->types;
  }

  const std::list<std::unique_ptr<TypeParameterDef>>& generics_for_method(const Method& method)
  {
    return method.signature->generics->types;
  }

  TypePtr get_bound(const TypeParameterDef& type_parameter)
  {
    return type_parameter.bound;
  }

  size_t get_index(const TypeParameterDef& type_parameter)
  {
    return type_parameter.index;
  }

  const SourceManager::SourceRange& entity_name_source_range(const Entity& e)
  {
    return e.name.source_range;
  }

  bool is_valid_main_method(Context& context, const Method& main)
  {
    return main.signature->generics->types.empty() && main.signature->receiver == nullptr &&
      main.signature->types.arguments.empty() &&
      main.signature->types.return_type == context.mk_unit();
  }

  EntityKind get_entity_kind(const Entity& e)
  {
    return e.kind->value();
  }

  void for_each_field(const Entity& entity, std::function<void(const Field&)> f)
  {
    for (const auto& member : entity.members)
    {
      if (const Field* fld = member->get_as<Field>())
      {
        f(*fld);
      }
    }
  }

  const std::string name(const Field& f)
  {
    return f.name;
  }

  const std::string name(const Method& m)
  {
    return m.name;
  }

  const std::string name(const Entity& m)
  {
    return m.name;
  }

  bool is_a_builtin(const Method& m)
  {
    return m.kind() == Method::Builtin;
  }

}
