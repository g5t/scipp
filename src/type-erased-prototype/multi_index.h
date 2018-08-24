#ifndef MULTI_INDEX_H
#define MULTI_INDEX_H

#include "dimensions.h"

class MultiIndex {
public:
  MultiIndex(const Dimensions &parentDimensions,
             const std::vector<Dimensions> &subdimensions) {
    if (parentDimensions.count() > 4)
      throw std::runtime_error("MultiIndex supports at most 4 dimensions.");
    if (subdimensions.size() > 4)
      throw std::runtime_error("MultiIndex supports at most 4 subindices.");
    m_dims = parentDimensions.count();
    for (gsl::index d = 0; d < m_dims; ++d)
      m_extent[d] = parentDimensions.size(d);

    m_numberOfSubindices = subdimensions.size();
    for (gsl::index j = 0; j < m_numberOfSubindices; ++j) {
      const auto &dimensions = subdimensions[j];
      gsl::index factor{1};
      gsl::index k = 0;
      for (gsl::index i = 0; i < dimensions.count(); ++i) {
        const auto dimension = dimensions.label(i);
        if (parentDimensions.contains(dimension)) {
          m_offsets[j][k] = parentDimensions.index(dimension);
          m_factors[j][k] = factor;
          ++k;
        }
        factor *= dimensions.size(i);
      }
      m_subdims[j] = k;
    }
    gsl::index offset{1};
    for (gsl::index d = 0; d < m_dims; ++d) {
      setIndex(offset);
      m_delta[4 * d + 0] = get<0>();
      m_delta[4 * d + 1] = get<1>();
      m_delta[4 * d + 2] = get<2>();
      m_delta[4 * d + 3] = get<3>();
      if (d > 0) {
        setIndex(offset - 1);
        m_delta[4 * d + 0] -= get<0>();
        m_delta[4 * d + 1] -= get<1>();
        m_delta[4 * d + 2] -= get<2>();
        m_delta[4 * d + 3] -= get<3>();
      }
      for (gsl::index d2 = 0; d2 < d; ++d2) {
        m_delta[4 * d + 0] -= m_delta[4 * d2 + 0];
        m_delta[4 * d + 1] -= m_delta[4 * d2 + 1];
        m_delta[4 * d + 2] -= m_delta[4 * d2 + 2];
        m_delta[4 * d + 3] -= m_delta[4 * d2 + 3];
      }
      offset *= m_extent[d];
    }
    setIndex(0);
  }

  void increment() {
    // gcc does not vectorize the addition for some reason.
    for (int i = 0; i < 4; ++i)
      m_index[i] += m_delta[0 + i];
    ++m_coord[0];
    if (m_coord[0] == m_extent[0]) {
      for (int i = 0; i < 4; ++i)
        m_index[i] += m_delta[4 + i];
      if (m_dims > 1) {
        m_coord[0] = 0;
        ++m_coord[1];
        if (m_coord[1] == m_extent[1]) {
          for (int i = 0; i < 4; ++i)
            m_index[i] += m_delta[8 + i];
          if (m_dims > 2) {
            m_coord[1] = 0;
            ++m_coord[2];
            if (m_coord[2] == m_extent[2]) {
              for (int i = 0; i < 4; ++i)
                m_index[i] += m_delta[12 + i];
              if (m_dims > 3) {
                m_coord[2] = 0;
                ++m_coord[3];
              }
            }
          }
        }
      }
    }
    ++m_fullIndex;
  }

  void setIndex(const gsl::index index) {
    m_fullIndex = index;
    if (m_dims == 0)
      return;
    auto remainder{index};
    for (gsl::index d = 0; d < m_dims - 1; ++d) {
      m_coord[d] = remainder % m_extent[d];
      remainder /= m_extent[d];
    }
    m_coord[m_dims - 1] = remainder;
    for (gsl::index i = 0; i < m_numberOfSubindices; ++i) {
      m_index[i] = 0;
      for (gsl::index j = 0; j < m_subdims[i]; ++j)
        m_index[i] += m_factors[i][j] * m_coord[m_offsets[i][j]];
    }
  }

  template <int N> gsl::index get() const { return m_index[N]; }
  gsl::index index() const { return m_fullIndex; }

  bool operator==(const MultiIndex &other) const {
    return m_fullIndex == other.m_fullIndex;
  }

private:
  // alignas does not help, for some reason gcc does not generate SIMD
  // instructions.
  // Using std::array is 1.5x slower, for some reason intermediate values of
  // m_index are always stored instead of merely being kept in registers.
  alignas(32) gsl::index m_index[4]{0, 0, 0, 0};
  alignas(32) gsl::index m_delta[16]{0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0};
  alignas(32) gsl::index m_coord[4]{0, 0, 0, 0};
  alignas(32) gsl::index m_extent[4]{0, 0, 0, 0};
  gsl::index m_fullIndex;
  gsl::index m_dims;
  gsl::index m_numberOfSubindices;
  std::array<gsl::index, 4> m_subdims;
  std::array<std::array<gsl::index, 4>, 4> m_offsets;
  std::array<std::array<gsl::index, 4>, 4> m_factors;
};

#endif // MULTI_INDEX_H
