// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include "scipp/dataset/except.h"
#include "scipp/dataset/dataset.h"

namespace scipp::except {

DataArrayError::DataArrayError(const std::string &msg) : Error{msg} {}

template <>
void throw_mismatch_error(const dataset::DataArray &expected,
                          const dataset::DataArray &actual,
                          const std::string &optional_message) {
  throw DataArrayError("Expected DataArray " + to_string(expected) + ", got " +
                       to_string(actual) + '.' + optional_message);
}

DatasetError::DatasetError(const std::string &msg) : Error{msg} {}

template <>
void throw_mismatch_error(const dataset::Dataset &expected,
                          const dataset::Dataset &actual,
                          const std::string &optional_message) {
  throw DatasetError("Expected Dataset " + to_string(expected) + ", got " +
                     to_string(actual) + '.' + optional_message);
}

CoordMismatchError::CoordMismatchError(const Dim dim, const Variable &expected,
                                       const Variable &actual)
    : DatasetError{"Mismatch in coordinate '" + to_string(dim) +
                   "', expected\n" + format_variable(expected) + ", got\n" +
                   format_variable(actual)} {}

CoordMismatchError::CoordMismatchError(const Dim dim, const Variable &expected,
                                       const Variable &actual,
                                       const std::string_view opname)
    : DatasetError{"Mismatch in coordinate '" + to_string(dim) +
                   "' in operation '" + std::string(opname) + "':\n" +
                   format_variable(expected) + "\nvs\n" +
                   format_variable(actual)} {}

} // namespace scipp::except

namespace scipp::dataset::expect {
void coords_are_superset(const Coords &a_coords, const Coords &b_coords) {
  for (const auto &b_coord : b_coords) {
    if (a_coords[b_coord.first] != b_coord.second)
      throw except::CoordMismatchError(b_coord.first, a_coords[b_coord.first],
                                       b_coord.second);
  }
}

void coords_are_superset(const DataArray &a, const DataArray &b) {
  coords_are_superset(a.coords(), b.coords());
}

void matching_coord(const Dim dim, const Variable &a, const Variable &b,
                    const std::string_view opname) {
  if (a != b)
    throw except::CoordMismatchError(dim, a, b, opname);
}

void is_key(const Variable &key) {
  if (key.dims().ndim() != 1)
    throw except::DimensionError(
        "Coord for binning or grouping must be 1-dimensional");
  if (key.has_variances())
    throw except::VariancesError(
        "Coord for binning or grouping cannot have variances");
}

} // namespace scipp::dataset::expect
