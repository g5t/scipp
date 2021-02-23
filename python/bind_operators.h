// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#pragma once

#include "pybind11.h"
#include "scipp/dataset/arithmetic.h"
#include "scipp/dataset/generated_comparison.h"
#include "scipp/dataset/generated_logical.h"
#include "scipp/variable/arithmetic.h"
#include "scipp/variable/comparison.h"
#include "scipp/variable/logical.h"

namespace py = pybind11;

template <class T, class... Ignored>
void bind_common_operators(pybind11::class_<T, Ignored...> &c) {
  c.def("__repr__", [](T &self) { return to_string(self); });
  c.def("__bool__", [](T &) {
    throw std::runtime_error("The truth value of a variable, data array, or "
                             "dataset is ambiguous. Use any() or all().");
  });
  // For views such as VariableView __copy__ needs to return a Variable. Use
  // result type of operator+ to obtain this type.
  using Copy = decltype(std::declval<T>() + std::declval<T>());
  c.def(
      "copy", [](T &self) { return Copy(self); },
      py::call_guard<py::gil_scoped_release>(), "Return a (deep) copy.");
  c.def(
      "__copy__", [](T &self) { return Copy(self); },
      py::call_guard<py::gil_scoped_release>(), "Return a (deep) copy.");
  c.def(
      "__deepcopy__", [](T &self, py::dict) { return Copy(self); },
      py::call_guard<py::gil_scoped_release>(), "Return a (deep) copy.");
}

template <class T, class... Ignored>
void bind_astype(py::class_<T, Ignored...> &c) {
  c.def(
      "astype",
      [](const T &self, const DType type) { return astype(self, type); },
      py::call_guard<py::gil_scoped_release>(),
      R"(
        Converts a Variable or DataArray to a different type.

        :raises: If the variable cannot be converted to the requested dtype.
        :return: New variable or data array with specified dtype.
        :rtype: Variable or DataArray)");
}

template <class Other, class T, class... Ignored>
void bind_inequality_to_operator(pybind11::class_<T, Ignored...> &c) {
  c.def(
      "__eq__", [](T &a, Other &b) { return a == b; }, py::is_operator(),
      py::call_guard<py::gil_scoped_release>());
  c.def(
      "__ne__", [](T &a, Other &b) { return a != b; }, py::is_operator(),
      py::call_guard<py::gil_scoped_release>());
}

template <class Other, class T, class... Ignored>
void bind_comparison(pybind11::class_<T, Ignored...> &c) {
  c.def(
      "__eq__", [](T &a, Other &b) { return equal(a, b); }, py::is_operator(),
      py::call_guard<py::gil_scoped_release>());
  c.def(
      "__ne__", [](T &a, Other &b) { return not_equal(a, b); },
      py::is_operator(), py::call_guard<py::gil_scoped_release>());
  c.def(
      "__lt__", [](T &a, Other &b) { return less(a, b); }, py::is_operator(),
      py::call_guard<py::gil_scoped_release>());
  c.def(
      "__gt__", [](T &a, Other &b) { return greater(a, b); }, py::is_operator(),
      py::call_guard<py::gil_scoped_release>());
  c.def(
      "__le__", [](T &a, Other &b) { return less_equal(a, b); },
      py::is_operator(), py::call_guard<py::gil_scoped_release>());
  c.def(
      "__ge__", [](T &a, Other &b) { return greater_equal(a, b); },
      py::is_operator(), py::call_guard<py::gil_scoped_release>());
}

struct Identity {
  template <class T> const T &operator()(const T &x) const noexcept {
    return x;
  }
};
struct ScalarToVariable {
  template <class T> Variable operator()(const T &x) const noexcept {
    return x * units::one;
  }
};

template <class RHSSetup> struct OpBinder {
  template <class Other, class T, class... Ignored>
  static void in_place_binary(pybind11::class_<T, Ignored...> &c) {
    // In-place operators return py::object due to the way in-place operators
    // work in Python (assigning return value to this). This avoids extra
    // copies, and additionally ensures that all references to the object keep
    // referencing the same object after the operation.
    c.def(
        "__iadd__",
        [](py::object &a, Other &b) {
          a.cast<T &>() += RHSSetup{}(b);
          return a;
        },
        py::is_operator(), py::call_guard<py::gil_scoped_release>());
    c.def(
        "__isub__",
        [](py::object &a, Other &b) {
          a.cast<T &>() -= RHSSetup{}(b);
          return a;
        },
        py::is_operator(), py::call_guard<py::gil_scoped_release>());
    c.def(
        "__imul__",
        [](py::object &a, Other &b) {
          a.cast<T &>() *= RHSSetup{}(b);
          return a;
        },
        py::is_operator(), py::call_guard<py::gil_scoped_release>());
    c.def(
        "__itruediv__",
        [](py::object &a, Other &b) {
          a.cast<T &>() /= RHSSetup{}(b);
          return a;
        },
        py::is_operator(), py::call_guard<py::gil_scoped_release>());
    if constexpr (!(std::is_same_v<T, Dataset> ||
                    std::is_same_v<T, DatasetView> ||
                    std::is_same_v<Other, Dataset> ||
                    std::is_same_v<Other, DatasetView>)) {
      c.def(
          "__imod__",
          [](py::object &a, Other &b) {
            a.cast<T &>() %= RHSSetup{}(b);
            return a;
          },
          py::is_operator(), py::call_guard<py::gil_scoped_release>());
    }
  }

  template <class Other, class T, class... Ignored>
  static void binary(pybind11::class_<T, Ignored...> &c) {
    c.def(
        "__add__", [](T &a, Other &b) { return a + RHSSetup{}(b); },
        py::is_operator(), py::call_guard<py::gil_scoped_release>());
    c.def(
        "__sub__", [](T &a, Other &b) { return a - RHSSetup{}(b); },
        py::is_operator(), py::call_guard<py::gil_scoped_release>());
    c.def(
        "__mul__", [](T &a, Other &b) { return a * RHSSetup{}(b); },
        py::is_operator(), py::call_guard<py::gil_scoped_release>());
    c.def(
        "__truediv__", [](T &a, Other &b) { return a / RHSSetup{}(b); },
        py::is_operator(), py::call_guard<py::gil_scoped_release>());
    if constexpr (!(std::is_same_v<T, Dataset> ||
                    std::is_same_v<T, DatasetView> ||
                    std::is_same_v<Other, Dataset> ||
                    std::is_same_v<Other, DatasetView>)) {
      c.def(
          "__floordiv__",
          [](T &a, Other &b) { return floor_divide(a, RHSSetup{}(b)); },
          py::is_operator(), py::call_guard<py::gil_scoped_release>());
      c.def(
          "__mod__", [](T &a, Other &b) { return a % RHSSetup{}(b); },
          py::is_operator(), py::call_guard<py::gil_scoped_release>());
    }
  }
};

template <class Other, class T, class... Ignored>
static void bind_in_place_binary(pybind11::class_<T, Ignored...> &c) {
  OpBinder<Identity>::in_place_binary<Other>(c);
}

template <class Other, class T, class... Ignored>
static void bind_binary(pybind11::class_<T, Ignored...> &c) {
  OpBinder<Identity>::binary<Other>(c);
}

template <class T, class... Ignored>
void bind_in_place_binary_scalars(pybind11::class_<T, Ignored...> &c) {
  OpBinder<ScalarToVariable>::in_place_binary<float>(c);
  OpBinder<ScalarToVariable>::in_place_binary<double>(c);
  OpBinder<ScalarToVariable>::in_place_binary<int32_t>(c);
  OpBinder<ScalarToVariable>::in_place_binary<int64_t>(c);
}

template <class T, class... Ignored>
void bind_binary_scalars(pybind11::class_<T, Ignored...> &c) {
  OpBinder<ScalarToVariable>::binary<float>(c);
  OpBinder<ScalarToVariable>::binary<double>(c);
  OpBinder<ScalarToVariable>::binary<int32_t>(c);
  OpBinder<ScalarToVariable>::binary<int64_t>(c);
}

template <class T, class... Ignored>
void bind_unary(pybind11::class_<T, Ignored...> &c) {
  c.def(
      "__neg__", [](T &a) { return -a; }, py::is_operator(),
      py::call_guard<py::gil_scoped_release>());
}

template <class T, class... Ignored>
void bind_boolean_unary(pybind11::class_<T, Ignored...> &c) {
  c.def(
      "__invert__", [](T &a) { return ~a; }, py::is_operator(),
      py::call_guard<py::gil_scoped_release>());
}

template <class Other, class T, class... Ignored>
void bind_logical(pybind11::class_<T, Ignored...> &c) {
  using T1 = const typename T::const_view_type;
  using T2 = const typename Other::const_view_type;
  c.def(
      "__or__", [](T1 &a, T2 &b) { return a | b; }, py::is_operator(),
      py::call_guard<py::gil_scoped_release>());
  c.def(
      "__xor__", [](T1 &a, T2 &b) { return a ^ b; }, py::is_operator(),
      py::call_guard<py::gil_scoped_release>());
  c.def(
      "__and__", [](T1 &a, T2 &b) { return a & b; }, py::is_operator(),
      py::call_guard<py::gil_scoped_release>());
  c.def(
      "__ior__",
      [](py::object &a, T2 &b) {
        a.cast<T &>() |= b;
        return a;
      },
      py::is_operator(), py::call_guard<py::gil_scoped_release>());
  c.def(
      "__ixor__",
      [](py::object &a, T2 &b) {
        a.cast<T &>() ^= b;
        return a;
      },
      py::is_operator(), py::call_guard<py::gil_scoped_release>());
  c.def(
      "__iand__",
      [](py::object &a, T2 &b) {
        a.cast<T &>() &= b;
        return a;
      },
      py::is_operator(), py::call_guard<py::gil_scoped_release>());
}
