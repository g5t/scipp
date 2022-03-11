// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Owen Arnold
#include "scipp/variable/shape.h"
#include "docstring.h"
#include "pybind11.h"
#include "scipp/dataset/shape.h"
#include "scipp/variable/variable.h"

using namespace scipp;
using namespace scipp::variable;

namespace py = pybind11;

namespace {

template <class T> void bind_broadcast(py::module &m) {
  m.def(
      "broadcast",
      [](const T &self, const std::vector<Dim> &labels,
         const std::vector<scipp::index> &shape) {
        const Dimensions dims(labels, shape);
        return broadcast(self, dims);
      },
      py::arg("x"), py::arg("dims"), py::arg("shape"));
}

template <class T> void bind_concat(py::module &m) {
  m.def(
      "concat",
      [](const std::vector<T> &x, const Dim dim) { return concat(x, dim); },
      py::arg("x"), py::arg("dim"), py::call_guard<py::gil_scoped_release>());
}

template <class T> void bind_fold(pybind11::module &mod) {
  mod.def(
      "fold",
      [](const T &self, const Dim dim, const std::vector<Dim> &labels,
         const std::vector<scipp::index> &shape) {
        const Dimensions dims(labels, shape);
        return fold(self, dim, dims);
      },
      py::arg("x"), py::arg("dim"), py::arg("dims"), py::arg("shape"),
      py::call_guard<py::gil_scoped_release>());
}

template <class T> void bind_flatten(pybind11::module &mod) {
  mod.def(
      "flatten",
      [](const T &self, const std::vector<Dim> &dims, const Dim &to) {
        return flatten(self, dims, to);
      },
      py::arg("x"), py::arg("dims"), py::arg("to"),
      py::call_guard<py::gil_scoped_release>());
}

template <class T> void bind_transpose(pybind11::module &mod) {
  mod.def(
      "transpose",
      [](const T &self, const std::vector<Dim> &dims) {
        return transpose(self, dims);
      },
      py::arg("x"), py::arg("dims") = std::vector<Dim>{});
}

template <class T> void bind_squeeze(pybind11::module &mod) {
  mod.def(
      "squeeze",
      [](const T &self, const std::optional<std::vector<Dim>> &dims) {
        return squeeze(self, dims);
      },
      py::arg("x"), py::arg("dims") = std::nullopt);
}
} // namespace

void init_shape(py::module &m) {
  bind_broadcast<Variable>(m);
  bind_concat<Variable>(m);
  bind_concat<DataArray>(m);
  bind_concat<Dataset>(m);
  bind_fold<Variable>(m);
  bind_fold<DataArray>(m);
  bind_flatten<Variable>(m);
  bind_flatten<DataArray>(m);
  bind_transpose<Variable>(m);
  bind_transpose<DataArray>(m);
  bind_transpose<Dataset>(m);
  bind_squeeze<Variable>(m);
  bind_squeeze<DataArray>(m);
  bind_squeeze<Dataset>(m);
}
