// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include "scipp/dataset/bins.h"
#include "scipp/dataset/dataset.h"
#include "scipp/dataset/shape.h"
#include "scipp/dataset/string.h"
#include "scipp/variable/bin_variable.tcc"
#include "scipp/variable/bins.h"
#include "scipp/variable/string.h"

namespace scipp::variable {

INSTANTIATE_BIN_VARIABLE(DatasetView, bucket<Dataset>)
INSTANTIATE_BIN_VARIABLE(DataArrayView, bucket<DataArray>)
INSTANTIATE_BIN_VARIABLE(DataArrayView_observer, bucket<DataArrayView>)
INSTANTIATE_BIN_VARIABLE(DataArrayConstView_observer,
                         bucket<DataArrayConstView>)

} // namespace scipp::variable

namespace scipp::dataset {

class BinVariableMakerDataArray : public variable::BinVariableMaker<DataArray> {
private:
  Variable call_make_bins(const VariableConstView &parent,
                          const VariableConstView &indices, const Dim dim,
                          const DType type, const Dimensions &dims,
                          const units::Unit &unit,
                          const bool variances) const override {
    const auto &source = std::get<2>(parent.constituents<bucket<DataArray>>());
    if (parent.dims() !=
        indices
            .dims()) // would need to select and copy slices from source coords
      throw std::runtime_error(
          "Shape changing operations with bucket<DataArray> not supported yet");
    // TODO This may also fail if the input buffer has extra capacity (rows not
    // in any bucket).
    auto buffer = DataArray(
        variable::variableFactory().create(type, dims, unit, variances),
        source.coords(), source.masks(), source.attrs());
    return make_bins(Variable(indices), dim, std::move(buffer));
  }
  VariableConstView data(const VariableConstView &var) const override {
    return std::get<2>(var.constituents<bucket<DataArray>>()).data();
  }
  VariableView data(const VariableView &var) const override {
    return std::get<2>(var.constituents<bucket<DataArray>>()).data();
  }
  core::ElementArrayViewParams
  array_params(const VariableConstView &var) const override {
    const auto &[indices, dim, buffer] = var.constituents<bucket<DataArray>>();
    auto params = var.array_params();
    return {0, // no offset required in buffer since access via indices
            params.dims(),
            params.dataDims(),
            {dim, buffer.dims(), indices.values<scipp::index_pair>().data()}};
  }
};

/// This is currently a dummy implemented just to make `is_bins` work.
class BinVariableMakerDataset
    : public variable::BinVariableMakerCommon<Dataset> {
  Variable create(const DType, const Dimensions &, const units::Unit &,
                  const bool,
                  const std::vector<VariableConstView> &) const override {
    throw std::runtime_error("not implemented");
  }
  Dim elem_dim(const VariableConstView &) const override {
    throw std::runtime_error("undefined");
  }
  DType elem_dtype(const VariableConstView &) const override {
    throw std::runtime_error("undefined");
  }
  units::Unit elem_unit(const VariableConstView &) const override {
    throw std::runtime_error("undefined");
  }
  void expect_can_set_elem_unit(const VariableView &,
                                const units::Unit &) const override {
    throw std::runtime_error("undefined");
  }
  void set_elem_unit(const VariableView &, const units::Unit &) const override {
    throw std::runtime_error("undefined");
  }
  bool hasVariances(const VariableConstView &) const override {
    throw std::runtime_error("undefined");
  }
};

namespace {
auto register_dataset_types(
    (variable::formatterRegistry().emplace(
         dtype<bucket<Dataset>>,
         std::make_unique<variable::Formatter<bucket<Dataset>>>()),
     variable::formatterRegistry().emplace(
         dtype<bucket<DataArray>>,
         std::make_unique<variable::Formatter<bucket<DataArray>>>()),
     variable::formatterRegistry().emplace(
         dtype<bucket<DataArrayView>>,
         std::make_unique<variable::Formatter<bucket<DataArrayView>>>()),
     variable::formatterRegistry().emplace(
         dtype<bucket<DataArrayConstView>>,
         std::make_unique<variable::Formatter<bucket<DataArrayConstView>>>()),
     0));
auto register_variable_maker_bucket_DataArray(
    (variable::variableFactory().emplace(
         dtype<bucket<DataArray>>,
         std::make_unique<BinVariableMakerDataArray>()),
     variable::variableFactory().emplace(
         dtype<bucket<Dataset>>, std::make_unique<BinVariableMakerDataset>()),
     0));
} // namespace
} // namespace scipp::dataset
