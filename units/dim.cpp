// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include <limits>
#include <mutex>

#include "scipp/common/index.h"
#include "scipp/units/dim.h"

namespace scipp::units {

std::unordered_map<std::string, Dim::Id> Dim::builtin_ids{
    {"energy", Dim::Energy},
    {"event", Dim::Event},
    {"group", Dim::Group},
    {"<invalid>", Dim::Invalid},
    {"position", Dim::Position},
    {"row", Dim::Row},
    {"temperature", Dim::Temperature},
    {"time", Dim::Time},
    {"wavelength", Dim::Wavelength},
    {"x", Dim::X},
    {"y", Dim::Y},
    {"z", Dim::Z}};

std::unordered_map<std::string, Dim::Id> Dim::custom_ids;
std::shared_mutex Dim::mutex;

Dim::Dim(const std::string &label) {
  if (const auto it = builtin_ids.find(label); it != builtin_ids.end()) {
    m_id = it->second;
    return;
  }
  std::shared_lock read_lock(mutex);
  if (const auto it = custom_ids.find(label); it != custom_ids.end()) {
    m_id = it->second;
    return;
  }
  read_lock.unlock();
  const std::unique_lock write_lock(mutex);
  const auto id = scipp::size(custom_ids) + 1000;
  if (id > std::numeric_limits<std::underlying_type<Id>::type>::max())
    throw std::runtime_error(
        "Exceeded maximum number of different dimension labels.");
  m_id = static_cast<Id>(id);
  custom_ids[label] = m_id;
}

std::string Dim::name() const {
  if (static_cast<int64_t>(m_id) < 1000)
    for (const auto &item : builtin_ids)
      if (item.second == m_id)
        return item.first;
  const std::shared_lock read_lock(mutex);
  for (const auto &item : custom_ids)
    if (item.second == m_id)
      return item.first;
  return "unreachable"; // throw or terminate?
}

std::string to_string(const Dim dim) { return dim.name(); }

} // namespace scipp::units
