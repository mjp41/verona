// Copyright Microsoft and Project Verona Contributors.
// SPDX-License-Identifier: MIT
#pragma once

#include "compiler/ast_forward.h"
#include "compiler/type.h"
#include "compiler/context.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace verona::compiler
{

  bool name_resolution(Context& context, Program* p);

  const Method* lookup_method(const Entity* definition, const std::string& name);

  const Field* lookup_field(const Entity* definition, const std::string& name);

  TypePtr lookup_field_type(
    Context& context, const TypePtr& ty, const std::string& name);

  // Lookup a field's type in a set of types.
  //
  // Returns the set of possible field types. Input types that aren't
  // NominalTypes, or which don't have a field with that name are ignored.
  TypeSet lookup_field_types(
    Context& context, const TypeSet& types, const std::string& name);

  std::optional<std::pair<TypeList, const FnSignature*>>
  lookup_method_signature(const TypePtr& ty, const std::string& name);
}
