// Copyright Microsoft and Project Verona Contributors.
// SPDX-License-Identifier: MIT
#include "compiler/codegen/codegen.h"

#include "compiler/ast_forward.h"
#include "compiler/codegen/builtins.h"
#include "compiler/codegen/descriptor.h"
#include "compiler/codegen/function.h"
#include "compiler/codegen/generator.h"
#include "compiler/codegen/reachability.h"
#include "compiler/instantiation.h"
#include "compiler/resolution.h"
#include "ds/helpers.h"
#include "interpreter/bytecode.h"

namespace verona::compiler
{
  using bytecode::Opcode;
  using bytecode::SelectorIdx;

  /**
   * Search for the program entrypoint and check it has the right signature.
   *
   * Returns nullopt and raises errors in the context if the entrypoint isn't
   * found or is invalid.
   */
  std::optional<std::pair<CodegenItem<Entity>, CodegenItem<Method>>>
  find_entry(Context& context, const Program& program)
  {
    const Entity* main_class = find_entity(program, "Main");
    if (!main_class)
    {
      context.print_global_diagnostic(
        std::cerr, DiagnosticKind::Error, Diagnostic::NoMainClass);
      return std::nullopt;
    }

    if (!is_a_class(main_class))
    {
      context.print_diagnostic(
        std::cerr,
        entity_name_source_range(*main_class).first,
        DiagnosticKind::Error,
        Diagnostic::MainNotAClass);
      context.print_line_diagnostic(std::cerr, entity_name_source_range(*main_class));
      return std::nullopt;
    }

    if (!generics_for_entity(*main_class).empty())
    {
      context.print_diagnostic(
        std::cerr,
        entity_name_source_range(*main_class).first,
        DiagnosticKind::Error,
        Diagnostic::MainClassIsGeneric);
      context.print_line_diagnostic(std::cerr, entity_name_source_range(*main_class));
      return std::nullopt;
    }

    const Method* main_method = lookup_method(main_class, "main");
    if (!main_method)
    {
      context.print_diagnostic(
        std::cerr,
        entity_name_source_range(*main_class).first,
        DiagnosticKind::Error,
        Diagnostic::NoMainMethod);
      context.print_line_diagnostic(std::cerr, entity_name_source_range(*main_class));
      return std::nullopt;
    }

    if (!is_valid_main_method(context, *main_method))
    {
      context.print_diagnostic(
        std::cerr,
        entity_name_source_range(*main_class).first,
        DiagnosticKind::Error,
        Diagnostic::InvalidMainSignature);
      context.print_line_diagnostic(std::cerr, entity_name_source_range(*main_class));
      return std::nullopt;
    }

    CodegenItem<Entity> class_item(main_class, Instantiation::empty());
    CodegenItem<Method> method_item(main_method, Instantiation::empty());
    return std::make_pair(class_item, method_item);
  }

  /**
   * Writes the magic numbers to the bytecode
   * @param code Generator to which the bytes should be emitted
   */
  void write_magic_number(Generator& code)
  {
    code.u32(bytecode::MAGIC_NUMBER);
  }

  std::vector<uint8_t> codegen(
    Context& context, const Program& program, const AnalysisResults& analysis)
  {
    auto entry = find_entry(context, program);
    if (!entry)
      return {};

    std::vector<uint8_t> code;

    Generator gen(code);
    write_magic_number(gen);

    Reachability reachability = compute_reachability(
      context, program, gen, entry->first, entry->second, analysis);
    SelectorTable selectors = SelectorTable::build(reachability);

    emit_program_header(program, reachability, selectors, gen, entry->first);
    emit_functions(context, analysis, reachability, selectors, gen);

    gen.finish();

    return code;
  }
}
