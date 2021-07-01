// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
#include <gtest/gtest.h>

#include "scipp/variable/bins.h"
#include "scipp/variable/string.h"
#include "scipp/variable/to_unit.h"
#include "test_macros.h"

using namespace scipp;

TEST(ToUnitTest, not_compatible) {
  const Dimensions dims(Dim::X, 2);
  const auto var = makeVariable<float>(dims, units::Unit("m"), Values{1, 2});
  EXPECT_THROW_DISCARD(to_unit(var, units::Unit("s")), except::UnitError);
}

TEST(ToUnitTest, buffer_handling) {
  const Dimensions dims(Dim::X, 2);
  const auto var = makeVariable<float>(dims, units::Unit("m"), Values{1, 2});
  const auto same = to_unit(var, var.unit());
  EXPECT_TRUE(same.is_same(var)); // not modified => not copied
  const auto different = to_unit(var, units::Unit("mm"));
  EXPECT_FALSE(different.is_same(var)); // modified => copied
}

TEST(ToUnitTest, same) {
  const Dimensions dims(Dim::X, 2);
  const auto var = makeVariable<float>(dims, units::Unit("m"), Values{1, 2});
  EXPECT_EQ(to_unit(var, var.unit()), var);
}

TEST(ToUnitTest, copy) {
  const Dimensions dims(Dim::X, 2);
  const auto var = makeVariable<float>(dims, units::Unit("m"), Values{1, 2});
  const auto no_copy = to_unit(var, var.unit(), CopyPolicy::TryAvoid);
  EXPECT_EQ(no_copy.values<float>().data(), var.values<float>().data());
  const auto force_copy = to_unit(var, var.unit(), CopyPolicy::Always);
  EXPECT_NE(force_copy.values<float>().data(), var.values<float>().data());
  const auto required_copy =
      to_unit(var, units::Unit("mm"), CopyPolicy::TryAvoid);
  EXPECT_NE(required_copy.values<float>().data(), var.values<float>().data());
}

TEST(ToUnitTest, m_to_mm) {
  const Dimensions dims(Dim::X, 2);
  const auto var = makeVariable<float>(dims, units::Unit("m"), Values{1, 2});
  EXPECT_EQ(to_unit(var, units::Unit("mm")),
            makeVariable<float>(dims, units::Unit("mm"), Values{1000, 2000}));
}

TEST(ToUnitTest, mm_to_m) {
  const Dimensions dims(Dim::X, 2);
  const auto var =
      makeVariable<float>(dims, units::Unit("mm"), Values{100, 1000});
  EXPECT_EQ(to_unit(var, units::Unit("m")),
            makeVariable<float>(dims, units::Unit("m"), Values{0.1, 1.0}));
}

TEST(ToUnitTest, ints) {
  const Dimensions dims(Dim::X, 2);
  const auto var =
      makeVariable<int32_t>(dims, units::Unit("mm"), Values{100, 2000});
  EXPECT_EQ(to_unit(var, units::Unit("m")),
            makeVariable<int32_t>(dims, units::Unit("m"), Values{0, 2}));
  EXPECT_EQ(
      to_unit(var, units::Unit("um")),
      makeVariable<int32_t>(dims, units::Unit("um"), Values{100000, 2000000}));
}

TEST(ToUnitTest, time_point) {
  const Dimensions dims(Dim::X, 8);
  const auto var = makeVariable<core::time_point>(
      dims, units::Unit("s"),
      Values{core::time_point{10}, core::time_point{20}, core::time_point{30},
             core::time_point{40}, core::time_point{10 + 60},
             core::time_point{20 + 60}, core::time_point{30 + 60},
             core::time_point{40 + 60}});
  EXPECT_EQ(
      to_unit(var, units::Unit("min")),
      makeVariable<core::time_point>(
          dims, units::Unit("min"),
          Values{core::time_point{0}, core::time_point{0}, core::time_point{1},
                 core::time_point{1}, core::time_point{1}, core::time_point{1},
                 core::time_point{2}, core::time_point{2}}));
  EXPECT_EQ(to_unit(var, units::Unit("ms")),
            makeVariable<core::time_point>(
                dims, units::Unit("ms"),
                Values{core::time_point{10000}, core::time_point{20000},
                       core::time_point{30000}, core::time_point{40000},
                       core::time_point{10000 + 60000},
                       core::time_point{20000 + 60000},
                       core::time_point{30000 + 60000},
                       core::time_point{40000 + 60000}}));
}

TEST(ToUnitTest, time_point_bad_units) {
  const auto do_to_unit = [](const char *initial, const char *target) {
    return to_unit(makeVariable<core::time_point>(Dims{}, units::Unit(initial)),
                   units::Unit(target));
  };

  // Conversions to or from time points with unit day or larger are complicated
  // and not implemented.
  const auto small_unit_names = {"h", "min", "s", "ns"};
  const auto large_unit_names = {"Y", "M", "D"};
  for (const char *initial : small_unit_names) {
    for (const char *target : small_unit_names) {
      EXPECT_NO_THROW_DISCARD(do_to_unit(initial, target));
    }
    for (const char *target : large_unit_names) {
      EXPECT_THROW_DISCARD(do_to_unit(initial, target), except::UnitError);
    }
  }
  for (const char *initial : large_unit_names) {
    for (const char *target : small_unit_names) {
      EXPECT_THROW_DISCARD(do_to_unit(initial, target), except::UnitError);
    }
    for (const char *target : large_unit_names) {
      if (initial == target)
        EXPECT_NO_THROW_DISCARD(do_to_unit(initial, target));
      else
        EXPECT_THROW_DISCARD(do_to_unit(initial, target), except::UnitError);
    }
  }
}

TEST(ToUnitTest, binned) {
  const auto indices = makeVariable<scipp::index_pair>(
      Dims{Dim::Y}, Shape{2}, Values{std::pair{0, 2}, std::pair{2, 4}});
  const auto input_buffer =
      makeVariable<double>(Dims{Dim::X}, Shape{4},
                           Values{1000, 2000, 3000, 4000}, units::Unit{"mm"});
  const auto expected_buffer = to_unit(input_buffer, units::Unit("m"));
  const auto var = make_bins(indices, Dim::X, input_buffer);
  EXPECT_EQ(to_unit(var, units::Unit{"m"}),
            make_bins(indices, Dim::X, expected_buffer));
}
