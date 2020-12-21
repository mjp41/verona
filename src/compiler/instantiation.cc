// Copyright Microsoft and Project Verona Contributors.
// SPDX-License-Identifier: MIT

#include "instantiation.h"

#include "compiler/ast_forward.h"

namespace verona::compiler
{
  TypePtr
  Instantiation::Applier::visit_type_parameter(const TypeParameterPtr& ty)
  {
    return instance_.types_.at(get_index(*ty->definition));
  }
}
