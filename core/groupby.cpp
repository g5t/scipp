// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include <numeric>

#include "scipp/core/event.h"
#include "scipp/core/except.h"
#include "scipp/core/groupby.h"
#include "scipp/core/histogram.h"
#include "scipp/core/indexed_slice_view.h"
#include "scipp/core/parallel.h"
#include "scipp/core/tag_util.h"

#include "dataset_operations_common.h"
#include "variable_operations_common.h"

namespace scipp::core {

/// Extract given group as a new data array or dataset
template <class T> T GroupBy<T>::operator[](const scipp::index group) const {
  const auto &slices = groups()[group];
  scipp::index size = 0;
  for (const auto &slice : slices)
    size += slice.end() - slice.begin();
  // This is just the slicing dim, but `slices` may be empty
  const Dim slice_dim = m_data.coords()[dim()].dims().inner();
  auto out = copy(m_data.slice({slice_dim, 0, size}));
  scipp::index current = 0;
  for (const auto &slice : slices) {
    const auto thickness = slice.end() - slice.begin();
    const Slice out_slice(slice_dim, current, current + thickness);
    copy(m_data.slice(slice), out.slice(out_slice));
    current += thickness;
  }
  return out;
}

/// Helper for creating output for "combine" step for "apply" steps that reduce
/// a dimension.
///
/// - Delete anything (but data) that depends on the reduction dimension.
/// - Default-init data.
template <class T>
T GroupBy<T>::makeReductionOutput(const Dim reductionDim) const {
  auto out = resize(m_data, reductionDim, size());
  out.rename(reductionDim, dim());
  out.setCoord(dim(), key());
  return out;
}

template <class T>
template <class Op, class CoordOp>
T GroupBy<T>::reduce(Op op, const Dim reductionDim, CoordOp coord_op) const {
  auto out = makeReductionOutput(reductionDim);
  const auto mask = ~masks_merge_if_contains(m_data.masks(), reductionDim);
  // Apply to each group, storing result in output slice
  const auto process_groups = [&](const auto &range) {
    for (scipp::index group = range.begin(); group != range.end(); ++group) {
      const auto out_slice = out.slice({dim(), group});
      if constexpr (std::is_same_v<T, Dataset>) {
        for (const auto &item : m_data) {
          op(out_slice[item.name()], item, groups()[group], reductionDim, mask);
        }
      } else {
        op(out_slice, m_data, groups()[group], reductionDim, mask);
      }
      if constexpr (!std::is_same_v<CoordOp, void *>)
        for (auto &&[dim, coord] : out_slice.coords())
          coord_op(coord, m_data.coords()[dim], groups()[group], reductionDim,
                   mask);
      else
        static_cast<void>(coord_op);
    }
  };
  parallel::parallel_for(parallel::blocked_range(0, size()), process_groups);
  return out;
}

namespace groupby_detail {
static constexpr auto flatten = [](const DataArrayView &out, const auto &in,
                                   const GroupByGrouping::group &group,
                                   const Dim reductionDim,
                                   const Variable &mask_) {
  const auto no_mask = makeVariable<bool>(Values{true});
  for (const auto &slice : group) {
    auto mask =
        mask_.dims().contains(reductionDim) ? mask_.slice(slice) : no_mask;
    const auto &array = in.slice(slice);
    if (in.hasData()) {
      if (is_events(array.data()))
        flatten_impl(out.data(), array.data(), mask);
      else
        sum_impl(out.data(), array.data() * mask);
    }
  }
};

static constexpr auto flatten_coord =
    [](const VariableView &out, const auto &in,
       const GroupByGrouping::group &group, const Dim reductionDim,
       const Variable &mask_) {
      if (!in.dims().contains(reductionDim))
        return;
      const auto no_mask = makeVariable<bool>(Values{true});
      for (const auto &slice : group) {
        auto mask =
            mask_.dims().contains(reductionDim) ? mask_.slice(slice) : no_mask;
        const auto &array = in.slice(slice);
        if (is_events(out))
          flatten_impl(out, array, mask);
      }
    };

static constexpr auto sum = [](const DataArrayView &out,
                               const auto &data_container,
                               const GroupByGrouping::group &group,
                               const Dim reductionDim, const Variable &mask) {
  for (const auto &slice : group) {
    const auto data_slice = data_container.slice(slice);
    if (mask.dims().contains(reductionDim))
      sum_impl(out.data(), data_slice.data() * mask.slice(slice));
    else
      sum_impl(out.data(), data_slice.data());
  }
};

template <void (*Func)(const VariableView &, const VariableConstView &)>
static constexpr auto reduce_idempotent =
    [](const DataArrayView &out, const auto &data_container,
       const GroupByGrouping::group &group, const Dim reductionDim,
       const Variable &mask) {
      bool first = true;
      for (const auto &slice : group) {
        const auto data_slice = data_container.slice(slice);
        if (mask.dims().contains(reductionDim))
          throw std::runtime_error(
              "This operation does not support masks yet.");
        if (first) {
          out.data().assign(data_slice.data().slice({reductionDim, 0}));
          first = false;
        }
        Func(out.data(), data_slice.data());
      }
    };
} // namespace groupby_detail

/// Flatten provided dimension in each group and return combined data.
///
/// This only supports event data.
template <class T> T GroupBy<T>::flatten(const Dim reductionDim) const {
  return reduce(groupby_detail::flatten, reductionDim,
                groupby_detail::flatten_coord);
}

/// Reduce each group using `sum` and return combined data.
template <class T> T GroupBy<T>::sum(const Dim reductionDim) const {
  return reduce(groupby_detail::sum, reductionDim);
}

/// Reduce each group using `all` and return combined data.
template <class T> T GroupBy<T>::all(const Dim reductionDim) const {
  return reduce(groupby_detail::reduce_idempotent<all_impl>, reductionDim);
}

/// Reduce each group using `any` and return combined data.
template <class T> T GroupBy<T>::any(const Dim reductionDim) const {
  return reduce(groupby_detail::reduce_idempotent<any_impl>, reductionDim);
}

/// Reduce each group using `max` and return combined data.
template <class T> T GroupBy<T>::max(const Dim reductionDim) const {
  return reduce(groupby_detail::reduce_idempotent<max_impl>, reductionDim);
}

/// Reduce each group using `min` and return combined data.
template <class T> T GroupBy<T>::min(const Dim reductionDim) const {
  return reduce(groupby_detail::reduce_idempotent<min_impl>, reductionDim);
}

/// Apply mean to groups and return combined data.
template <class T> T GroupBy<T>::mean(const Dim reductionDim) const {
  // 1. Sum into output slices
  auto out = sum(reductionDim);

  // 2. Compute number of slices N contributing to each out slice
  auto scale = makeVariable<double>(Dims{dim()}, Shape{size()});
  const auto scaleT = scale.template values<double>();
  const auto mask = masks_merge_if_contains(m_data.masks(), reductionDim);
  for (scipp::index group = 0; group < size(); ++group)
    for (const auto &slice : groups()[group]) {
      // N contributing to each slice
      scaleT[group] += slice.end() - slice.begin();
      // N masks for each slice, that need to be subtracted
      if (mask.dims().contains(reductionDim)) {
        const auto masks_sum = core::sum(mask.slice(slice), reductionDim);
        scaleT[group] -= masks_sum.template value<int64_t>();
      }
    }

  scale = 1.0 / scale;

  // 3. sum/N -> mean
  if constexpr (std::is_same_v<T, Dataset>) {
    for (const auto &item : out) {
      if (isInt(item.data().dtype()))
        out.setData(item.name(), item.data() * scale);
      else
        item *= scale;
    }
  } else {
    if (isInt(out.data().dtype()))
      out.setData(out.data() * scale);
    else
      out *= scale;
  }

  return out;
}

static void expectValidGroupbyKey(const VariableConstView &key) {
  if (key.dims().ndim() != 1)
    throw except::DimensionError("Group-by key must be 1-dimensional");
  if (key.hasVariances())
    throw except::VariancesError("Group-by key cannot have variances");
}

template <class T> struct MakeGroups {
  static auto apply(const VariableConstView &key, const Dim targetDim) {
    expectValidGroupbyKey(key);
    const auto &values = key.values<T>();

    const auto dim = key.dims().inner();
    std::map<T, GroupByGrouping::group> indices;
    const auto end = values.end();
    scipp::index i = 0;
    for (auto it = values.begin(); it != end;) {
      // Use contiguous (thick) slices if possible to avoid overhead of slice
      // handling in follow-up "apply" steps.
      const auto begin = i;
      const auto &value = *it;
      while (it != end && *it == value) {
        ++it;
        ++i;
      }
      indices[value].emplace_back(dim, begin, i);
    }

    const Dimensions dims{targetDim, scipp::size(indices)};
    std::vector<T> keys;
    std::vector<GroupByGrouping::group> groups;
    for (auto &item : indices) {
      keys.emplace_back(std::move(item.first));
      groups.emplace_back(std::move(item.second));
    }
    auto keys_ = makeVariable<T>(Dimensions{dims}, Values(std::move(keys)));
    keys_.setUnit(key.unit());
    return GroupByGrouping{std::move(keys_), std::move(groups)};
  }
};

template <class T> struct MakeBinGroups {
  static auto apply(const VariableConstView &key,
                    const VariableConstView &bins) {
    expectValidGroupbyKey(key);
    if (bins.dims().ndim() != 1)
      throw except::DimensionError("Group-by bins must be 1-dimensional");
    if (key.unit() != bins.unit())
      throw except::UnitError("Group-by key must have same unit as bins");
    const auto &values = key.values<T>();
    const auto &edges = bins.values<T>();
    expect::histogram::sorted_edges(edges);

    const auto dim = key.dims().inner();
    std::vector<GroupByGrouping::group> groups(edges.size() - 1);
    for (scipp::index i = 0; i < scipp::size(values);) {
      // Use contiguous (thick) slices if possible to avoid overhead of slice
      // handling in follow-up "apply" steps.
      const auto value = values[i];
      const auto begin = i++;
      auto right = std::upper_bound(edges.begin(), edges.end(), value);
      if (right != edges.end() && right != edges.begin()) {
        auto left = right - 1;
        while (i < scipp::size(values) && (*left <= values[i]) &&
               (values[i] < *right))
          ++i;
        groups[std::distance(edges.begin(), left)].emplace_back(dim, begin, i);
      }
    }
    return GroupByGrouping{Variable(bins), std::move(groups)};
  }
};

/// Create GroupBy<DataArray> object as part of "split-apply-combine" mechanism.
///
/// Groups the slices of `array` according to values in given by a coord.
/// Grouping will create a new coordinate for the dimension of the grouping
/// coord in a later apply/combine step.
GroupBy<DataArray> groupby(const DataArrayConstView &array, const Dim dim) {
  const auto &key = array.coords()[dim];
  return {array,
          CallDType<double, float, int64_t, int32_t, bool,
                    std::string>::apply<MakeGroups>(key.dtype(), key, dim)};
}

/// Create GroupBy<DataArray> object as part of "split-apply-combine" mechanism.
///
/// Groups the slices of `array` according to values in given by a coord.
/// Grouping of a coord is according to given `bins`, which will be added as a
/// new coordinate to the output in a later apply/combine step.
GroupBy<DataArray> groupby(const DataArrayConstView &array, const Dim dim,
                           const VariableConstView &bins) {
  const auto &key = array.coords()[dim];
  return {array,
          CallDType<double, float, int64_t, int32_t>::apply<MakeBinGroups>(
              key.dtype(), key, bins)};
}

/// Create GroupBy<Dataset> object as part of "split-apply-combine" mechanism.
///
/// Groups the slices of `dataset` according to values in given by a coord.
/// Grouping will create a new coordinate for the dimension of the grouping
/// coord in a later apply/combine step.
GroupBy<Dataset> groupby(const DatasetConstView &dataset, const Dim dim) {
  const auto &key = dataset.coords()[dim];
  return {dataset,
          CallDType<double, float, int64_t, int32_t, bool,
                    std::string>::apply<MakeGroups>(key.dtype(), key, dim)};
}

/// Create GroupBy<Dataset> object as part of "split-apply-combine" mechanism.
///
/// Groups the slices of `dataset` according to values in given by a coord.
/// Grouping of a coord is according to given `bins`, which will be added as a
/// new coordinate to the output in a later apply/combine step.
GroupBy<Dataset> groupby(const DatasetConstView &dataset, const Dim dim,
                         const VariableConstView &bins) {
  const auto &key = dataset.coords()[dim];
  return {dataset,
          CallDType<double, float, int64_t, int32_t>::apply<MakeBinGroups>(
              key.dtype(), key, bins)};
}

template class GroupBy<DataArray>;
template class GroupBy<Dataset>;

} // namespace scipp::core
