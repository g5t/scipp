// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Jan-Lukas Wynen
#pragma once

#include "scipp/variable/variable.h"

#include "scipp-dataset_export.h"

// These functions are placed in scipp::variable for ADL but cannot be easily
// moved into that compilation module since the implementation requires
// DataArray.
namespace scipp::variable {
[[nodiscard]] SCIPP_DATASET_EXPORT Variable bins_sum(const Variable &data);
[[nodiscard]] SCIPP_DATASET_EXPORT Variable bins_max(const Variable &data);
[[nodiscard]] SCIPP_DATASET_EXPORT Variable bins_nanmax(const Variable &data);
[[nodiscard]] SCIPP_DATASET_EXPORT Variable bins_min(const Variable &data);
[[nodiscard]] SCIPP_DATASET_EXPORT Variable bins_nanmin(const Variable &data);
[[nodiscard]] SCIPP_DATASET_EXPORT Variable bins_all(const Variable &data);
[[nodiscard]] SCIPP_DATASET_EXPORT Variable bins_any(const Variable &data);
[[nodiscard]] SCIPP_DATASET_EXPORT Variable bins_mean(const Variable &data);
} // namespace scipp::variable
