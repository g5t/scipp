// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
/// @author Neil Vaytet
#include <regex>
#include <stdexcept>

#include <units/units.hpp>
#include <units/units_util.hpp>

#include "scipp/units/except.h"
#include "scipp/units/unit.h"

namespace scipp::units {

Unit::Unit(const std::string &unit)
    : Unit(llnl::units::unit_from_string(unit == "dimensionless" ? "" : unit)) {
  if (!is_valid(m_unit))
    throw except::UnitError("Failed to convert string `" + unit +
                            "` to valid unit.");
}

std::string Unit::name() const {
  auto repr = to_string(m_unit);
  repr = std::regex_replace(repr, std::regex("^u"), "µ");
  repr = std::regex_replace(repr, std::regex("item"), "count");
  repr = std::regex_replace(repr, std::regex("count(?!s)"), "counts");
  return repr == "" ? "dimensionless" : repr;
}

bool Unit::isCounts() const { return *this == counts; }

bool Unit::isCountDensity() const {
  return !isCounts() && m_unit.base_units().count() != 0;
}

bool Unit::operator==(const Unit &other) const {
  return m_unit == other.m_unit;
}
bool Unit::operator!=(const Unit &other) const { return !(*this == other); }

Unit &Unit::operator+=(const Unit &other) { return *this = *this + other; }

Unit &Unit::operator-=(const Unit &other) { return *this = *this - other; }

Unit &Unit::operator*=(const Unit &other) { return *this = *this * other; }

Unit &Unit::operator/=(const Unit &other) { return *this = *this / other; }

Unit &Unit::operator%=(const Unit &other) { return operator/=(other); }

Unit operator+(const Unit &a, const Unit &b) {
  if (a == b)
    return a;
  throw except::UnitError("Cannot add " + a.name() + " and " + b.name() + ".");
}

Unit operator-(const Unit &a, const Unit &b) {
  if (a == b)
    return a;
  throw except::UnitError("Cannot subtract " + a.name() + " and " + b.name() +
                          ".");
}

Unit operator*(const Unit &a, const Unit &b) {
  if (llnl::units::times_overflows(a.underlying(), b.underlying()))
    throw except::UnitError("Unsupported unit as result of multiplication: (" +
                            a.name() + ") * (" + b.name() + ')');
  return {a.underlying() * b.underlying()};
}

Unit operator/(const Unit &a, const Unit &b) {
  if (llnl::units::divides_overflows(a.underlying(), b.underlying()))
    throw except::UnitError("Unsupported unit as result of division: (" +
                            a.name() + ") / (" + b.name() + ')');
  return {a.underlying() / b.underlying()};
}

Unit operator%(const Unit &a, const Unit &b) { return a / b; }

Unit operator-(const Unit &a) { return a; }

Unit abs(const Unit &a) { return a; }

Unit sqrt(const Unit &a) {
  if (llnl::units::is_error(sqrt(a.underlying())))
    throw except::UnitError("Unsupported unit as result of sqrt: sqrt(" +
                            a.name() + ").");
  return {sqrt(a.underlying())};
}

Unit pow(const Unit &a, const int64_t power) {
  if (llnl::units::pow_overflows(a.underlying(), power))
    throw except::UnitError("Unsupported unit as result of pow: pow(" +
                            a.name() + ", " + std::to_string(power) + ").");
  return {a.underlying().pow(power)};
}

Unit trigonometric(const Unit &a) {
  if (a == units::rad || a == units::deg)
    return units::dimensionless;
  throw except::UnitError(
      "Trigonometric function requires rad or deg unit, got " + a.name() + ".");
}

Unit inverse_trigonometric(const Unit &a) {
  if (a == units::dimensionless)
    return units::rad;
  throw except::UnitError(
      "Inverse trigonometric function requires dimensionless unit, got " +
      a.name() + ".");
}

Unit sin(const Unit &a) { return trigonometric(a); }
Unit cos(const Unit &a) { return trigonometric(a); }
Unit tan(const Unit &a) { return trigonometric(a); }
Unit asin(const Unit &a) { return inverse_trigonometric(a); }
Unit acos(const Unit &a) { return inverse_trigonometric(a); }
Unit atan(const Unit &a) { return inverse_trigonometric(a); }
Unit atan2(const Unit &y, const Unit &x) {
  if (x == y)
    return units::rad;
  throw except::UnitError(
      "atan2 function requires matching units for input, got a " + x.name() +
      " b " + y.name() + ".");
}

} // namespace scipp::units
