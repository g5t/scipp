// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#pragma once
#include "scipp/common/initialization.h" // TODO can remove this?
#include "scipp/core/dimensions.h"
#include "scipp/core/element_array_view.h"
#include "scipp/core/except.h"
#include "scipp/units/unit.h"
#include "scipp/variable/data_model.h"
#include "scipp/variable/except.h"
#include "scipp/variable/transform.h"
#include "scipp/variable/variable_concept.h"

namespace scipp::variable {

/// Implementation of VariableConcept that holds and array with element type T.
template <class T, class Elem, int... N>
class StructuredModel : public VariableConcept {
public:
  using value_type = T;
  static constexpr auto num_element = (N * ...);

  StructuredModel(const scipp::index size, const units::Unit &unit,
                  element_array<Elem> model)
      : VariableConcept(units::one), // unit ignored
        m_elements(std::make_shared<DataModel<Elem>>(size * num_element, unit,
                                                     std::move(model))) {}

  static DType static_dtype() noexcept { return scipp::dtype<T>; }
  DType dtype() const noexcept override { return scipp::dtype<T>; }
  scipp::index size() const override {
    return m_elements->size() / num_element;
  }

  const units::Unit &unit() const override { return m_elements->unit(); }
  void setUnit(const units::Unit &unit) override { m_elements->setUnit(unit); }

  VariableConceptHandle
  makeDefaultFromParent(const scipp::index size) const override;

  VariableConceptHandle
  makeDefaultFromParent(const Variable &shape) const override {
    return makeDefaultFromParent(shape.dims().volume());
  }

  bool equals(const Variable &a, const Variable &b) const override;
  void copy(const Variable &src, Variable &dest) const override;
  void copy(const Variable &src, Variable &&dest) const override;
  void assign(const VariableConcept &other) override;

  void setVariances(const Variable &variances) override;

  VariableConceptHandle clone() const override {
    return std::make_unique<StructuredModel<T, Elem, N...>>(*this);
  }

  bool hasVariances() const noexcept override {
    return m_elements->hasVariances();
  }

  auto values(const core::ElementArrayViewParams &base) const {
    return ElementArrayView(base, get_values());
  }
  auto values(const core::ElementArrayViewParams &base) {
    return ElementArrayView(base, get_values());
  }

  VariableConceptHandle elements() const { return m_elements; }

  scipp::index element_offset(const scipp::index i) const {
    if (((i < 0 || i >= N) || ...))
      throw std::runtime_error("Element index out of range.");
    if constexpr (sizeof...(N) == 1)
      return i;
  }

  scipp::index element_offset(const scipp::index i,
                              const scipp::index j) const {
    // TODO range checks with actual N
    if (i < 0 || i >= 3 || j < 0 || j >= 3)
      throw std::runtime_error("Element index out of range.");
    if constexpr (sizeof...(N) == 2)
      return 3 * i + j;
  }

  scipp::index dtype_size() const override { return sizeof(T); }
  const VariableConceptHandle &bin_indices() const override {
    throw except::TypeError("This data type does not have bin indices.");
  }

  scipp::span<const T> values() const { return {get_values(), size()}; }
  scipp::span<T> values() { return {get_values(), size()}; }

private:
  const T *get_values() const {
    return reinterpret_cast<const T *>(
        requireT<const DataModel<Elem>>(*m_elements).values().data());
  }
  T *get_values() {
    return reinterpret_cast<T *>(
        requireT<DataModel<Elem>>(*m_elements).values().data());
  }
  VariableConceptHandle m_elements;
};

template <class T, class Elem, int... N>
VariableConceptHandle StructuredModel<T, Elem, N...>::makeDefaultFromParent(
    const scipp::index size) const {
  // return std::make_unique<StructuredModel<T, N...>>(size, unit(),
  // element_array<T>(size));
  throw std::runtime_error("todo how to get dims?");
  // return std::make_unique<StructuredModel<T, N...>>();
}

/// Helper for implementing Variable(View)::operator==.
///
/// This method is using virtual dispatch as a trick to obtain T, such that
/// values<T> and variances<T> can be compared.
template <class T, class Elem, int... N>
bool StructuredModel<T, Elem, N...>::equals(const Variable &a,
                                            const Variable &b) const {
  if (a.dims() != b.dims())
    return false;
  const auto &a_elems =
      *requireT<const StructuredModel<T, Elem, N...>>(a.data()).m_elements;
  const auto &b_elems =
      *requireT<const StructuredModel<T, Elem, N...>>(b.data()).m_elements;
  throw std::runtime_error("todo ");
  // return a_elems == b_elems;
}

/// Helper for implementing Variable(View) copy operations.
///
/// This method is using virtual dispatch as a trick to obtain T, such that
/// transform can be called with any T.
template <class T, class Elem, int... N>
void StructuredModel<T, Elem, N...>::copy(const Variable &src,
                                          Variable &dest) const {
  transform_in_place<T>(dest, src, [](auto &a, const auto &b) { a = b; });
}
template <class T, class Elem, int... N>
void StructuredModel<T, Elem, N...>::copy(const Variable &src,
                                          Variable &&dest) const {
  copy(src, dest);
}

template <class T, class Elem, int... N>
void StructuredModel<T, Elem, N...>::assign(const VariableConcept &other) {
  *this = requireT<const StructuredModel<T, Elem, N...>>(other);
}

template <class T, class Elem, int... N>
void StructuredModel<T, Elem, N...>::setVariances(const Variable &) {
  throw except::VariancesError("This data type cannot have variances.");
}

} // namespace scipp::variable
