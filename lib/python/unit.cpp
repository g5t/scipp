// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Scipp contributors (https://github.com/scipp)

#include "unit.h"

#include "scipp/common/overloaded.h"
#include "scipp/core/bucket.h"
#include "scipp/core/dtype.h"
#include "scipp/core/eigen.h"
#include "scipp/core/time_point.h"
#include "scipp/units/string.h"
#include "scipp/variable/variable.h"

#include "dtype.h"

using namespace scipp;
namespace py = pybind11;

namespace {
bool temporal_or_dimensionless(const units::Unit unit) {
  return unit == units::one || unit.has_same_base(units::s);
}
} // namespace

std::tuple<units::Unit, int64_t>
get_time_unit(const std::optional<scipp::units::Unit> value_unit,
              const std::optional<scipp::units::Unit> dtype_unit,
              const units::Unit sc_unit) {
  if (!temporal_or_dimensionless(sc_unit)) {
    throw except::UnitError("Invalid unit for dtype=datetime64: " +
                            to_string(sc_unit));
  }
  if (dtype_unit.value_or(units::one) != units::one &&
      (sc_unit != units::one && *dtype_unit != sc_unit)) {
    throw std::invalid_argument(
        "dtype (datetime64[" + to_string(*dtype_unit) +
        "]) has a different time unit from 'unit' argument (" +
        to_string(sc_unit) + ")");
  }
  units::Unit actual_unit = units::one;
  if (sc_unit != units::one)
    actual_unit = sc_unit;
  else if (dtype_unit.value_or(units::one) != units::one)
    actual_unit = *dtype_unit;
  else if (value_unit.has_value())
    actual_unit = *value_unit;

  // TODO implement
  if (value_unit && value_unit != actual_unit) {
    throw std::runtime_error("Conversion of time units is not implemented.");
  }

  return {actual_unit, 1};
}

std::tuple<units::Unit, int64_t> get_time_unit(const py::buffer &value,
                                               const py::object &dtype,
                                               const units::Unit unit) {
  return get_time_unit(
      value.is_none() || value.attr("dtype").attr("kind").cast<char>() != 'M'
          ? std::optional<units::Unit>{}
          : parse_datetime_dtype(value),
      dtype.is_none() ? std::optional<units::Unit>{}
                      : parse_datetime_dtype(dtype),
      unit);
}

template <>
std::tuple<scipp::units::Unit, scipp::units::Unit>
common_unit<scipp::core::time_point>(const pybind11::object &values,
                                     const scipp::units::Unit unit) {
  if (!temporal_or_dimensionless(unit)) {
    throw except::UnitError("Invalid unit for dtype=datetime64: " +
                            to_string(unit));
  }

  if (values.is_none() || !has_datetime_dtype(values)) {
    return std::tuple{unit, unit};
  }

  const auto value_unit = parse_datetime_dtype(values);
  if (unit == units::one) {
    return std::tuple{value_unit, value_unit};
  } else {
    return std::tuple{value_unit, unit};
  }
}

std::string to_numpy_time_string(const scipp::units::Unit unit) {
  if (unit == units::m) {
    // Would be treated as minute otherwise.
    throw except::UnitError("Invalid time unit, got 'm' which means meter. "
                            "If you meant minute, use unit='min' instead.");
  }
  return unit == units::us            ? std::string("us")
         : unit == units::Unit("min") ? std::string("m")
                                      : to_string(unit);
}

std::string to_numpy_time_string(const ProtoUnit &unit) {
  return std::visit(
      overloaded{
          [](const scipp::units::Unit &u) { return to_numpy_time_string(u); },
          [](const std::string &u) {
            return to_numpy_time_string(scipp::units::Unit(u));
          },
          [](const auto &) { return std::string(); }},
      unit);
}

scipp::units::Unit unit_or_default(const ProtoUnit &unit, const DType type) {
  return std::visit(
      overloaded{[type](DefaultUnit) {
                   if (type == dtype<void>)
                     throw except::UnitError(
                         "Default unit requested but dtype unknown.");
                   return variable::default_unit_for(type);
                 },
                 [](const py::none &) { return units::none; },
                 [](const std::string &u) { return units::Unit(u); },
                 [](const units::Unit &u) { return u; }},
      unit);
}
