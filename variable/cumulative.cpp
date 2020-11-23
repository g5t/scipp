// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include "scipp/variable/cumulative.h"
#include "scipp/core/element/cumulative.h"
#include "scipp/variable/transform.h"
#include "scipp/variable/util.h"

using namespace scipp;

namespace scipp::variable {

/// Return std::exclusive_scan along dim.
Variable exclusive_scan(const VariableConstView &var, const Dim dim) {
  if (var.dims()[dim] == 0)
    return Variable{var};
  Variable cumulative(var.slice({dim, 0}));
  fill_zeros(cumulative);
  Variable out(var);
  accumulate_in_place(cumulative, out, core::element::exclusive_scan);
  return out;
}

Variable exclusive_scan_bins(const VariableConstView &var) {
  Variable out(var);
  auto cumulative = Variable(variable::variableFactory().elem_dtype(var),
                             var.dims(), var.unit());
  accumulate_in_place(cumulative, out, core::element::exclusive_scan);
  return out;
}

} // namespace scipp::variable
