// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#pragma once

#include "scipp/dataset/dataset.h"

/// Helper to provide equivalent of the `items()` method of a Python dict.
template <class T> class items_view {
public:
  items_view(T &obj) : m_obj(&obj) {}
  auto size() const noexcept { return m_obj->size(); }
  auto begin() const { return m_obj->items_begin(); }
  auto end() const { return m_obj->items_end(); }
  auto tostring() const { return to_string(*m_obj); }

private:
  T *m_obj;
};
template <class T> items_view(T &) -> items_view<T>;

/// Helper to provide equivalent of the `values()` method of a Python dict.
template <class T> class values_view {
public:
  values_view(T &obj) : m_obj(&obj) {}
  auto size() const noexcept { return m_obj->size(); }
  auto begin() const {
    if constexpr (std::is_same_v<typename T::mapped_type, scipp::DataArray>)
      return m_obj->begin();
    else
      return m_obj->values_begin();
  }
  auto end() const {
    if constexpr (std::is_same_v<typename T::mapped_type, scipp::DataArray>)
      return m_obj->end();
    else
      return m_obj->values_end();
  }
  auto tostring() const {
    std::stringstream ss;
    ss << "<scipp.Dict.values>";
    for (auto it = this->begin(); it != this->end(); ++it)
      ss << "\n" << to_string(*it);
    return ss.str();
  }

private:
  T *m_obj;
};
template <class T> values_view(T &) -> values_view<T>;

/// Helper to provide equivalent of the `keys()` method of a Python dict.
template <class T> class keys_view {
public:
  keys_view(T &obj) : m_obj(&obj) {}
  auto size() const noexcept { return m_obj->size(); }
  auto begin() const { return m_obj->keys_begin(); }
  auto end() const { return m_obj->keys_end(); }
  auto tostring() const { return dict_keys_to_string(*m_obj); }

private:
  T *m_obj;
};
template <class T> keys_view(T &) -> keys_view<T>;

static constexpr auto dim_to_str = [](auto &&dim) -> decltype(auto) {
  return dim.name();
};

/// Helper to provide equivalent of the `keys()` method of a Python dict.
template <class T> class str_keys_view {
public:
  str_keys_view(T &obj) : m_obj(&obj) {}
  auto size() const noexcept { return m_obj->size(); }
  auto begin() const { return m_obj->keys_begin().transform(dim_to_str); }
  auto end() const { return m_obj->keys_end().transform(dim_to_str); }
  auto tostring() const { return dict_keys_to_string(*m_obj); }

private:
  T *m_obj;
};
template <class T> str_keys_view(T &) -> str_keys_view<T>;

static constexpr auto item_to_str = [](const auto &item) {
  return std::pair(item.first.name(), item.second);
};

/// Helper to provide equivalent of the `items()` method of a Python dict.
template <class T> class str_items_view {
public:
  str_items_view(T &obj) : m_obj(&obj) {}
  auto size() const noexcept { return m_obj->size(); }
  auto begin() const { return m_obj->items_begin().transform(item_to_str); }
  auto end() const { return m_obj->items_end().transform(item_to_str); }
  auto tostring() const { return to_string(*m_obj); }

private:
  T *m_obj;
};
template <class T> str_items_view(T &) -> str_items_view<T>;
