// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#pragma once
#include <Eigen/Core>
#include <Eigen/Geometry>

#include "scipp/core/dtype.h"

namespace scipp::core {

class RotationTransform {
private:
  // Store as quaterniond as this is more space efficient than storing as matrix
  // (4 doubles for quat vs 9 doubles for 3x3 matrix).
  Eigen::Quaterniond quat;

public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  RotationTransform() : quat(Eigen::Quaterniond::Identity()){};
  explicit RotationTransform(const Eigen::Quaterniond &x) : quat(x){};

  [[nodiscard]] Eigen::Matrix3d matrix() const {
    return quat.toRotationMatrix();
  }

  bool operator==(const RotationTransform &other) const {
    return this->quat.w() == other.quat.w() && 
        this->quat.x() == other.quat.x() && 
        this->quat.y() == other.quat.y() && 
        this->quat.z() == other.quat.z();
  }
};

class ScalingTransform {
private:
  Eigen::DiagonalMatrix<double, 3> mat;

public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  ScalingTransform()
      : mat(Eigen::Matrix3d::Identity().diagonal().asDiagonal()){};
  explicit ScalingTransform(const Eigen::DiagonalMatrix<double, 3> &x)
      : mat(x){};

  [[nodiscard]] Eigen::Matrix3d matrix() const { return mat; }

  bool operator==(const ScalingTransform &other) const {
    return this->mat.diagonal() == other.mat.diagonal();
  }
};

class TranslationTransform {
private:
  Eigen::Vector3d vec;

public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  TranslationTransform() : vec(Eigen::Vector3d(0, 0, 0)){};
  explicit TranslationTransform(const Eigen::Vector3d &x) : vec(x){};

  [[nodiscard]] Eigen::Vector3d vector() const { return vec; }

  bool operator==(const TranslationTransform &other) const {
    return this->vector() == other.vector();
  }
};

[[nodiscard]] inline RotationTransform operator*(const RotationTransform &lhs,
                                                 const RotationTransform &rhs) {
  return RotationTransform(Eigen::Quaterniond(lhs.matrix() * rhs.matrix()));
};

[[nodiscard]] inline Eigen::Matrix3d operator*(const RotationTransform &lhs,
                                               const ScalingTransform &rhs) {
  return lhs.matrix() * rhs.matrix();
};

[[nodiscard]] inline Eigen::Matrix3d operator*(const RotationTransform &lhs,
                                               const Eigen::Matrix3d &rhs) {
  return lhs.matrix() * rhs;
};

[[nodiscard]] inline Eigen::Affine3d
operator*(const RotationTransform &lhs, const TranslationTransform &rhs) {
  return lhs.matrix() * Eigen::Translation<double, 3>(rhs.vector());
};

[[nodiscard]] inline Eigen::Affine3d operator*(const RotationTransform &lhs,
                                               const Eigen::Affine3d &rhs) {
  return lhs.matrix() * rhs;
};

[[nodiscard]] inline Eigen::Vector3d operator*(const RotationTransform &lhs,
                                               const Eigen::Vector3d &rhs) {
  return lhs.matrix() * rhs;
};

[[nodiscard]] inline ScalingTransform operator*(const ScalingTransform &lhs,
                                                const ScalingTransform &rhs) {
  return ScalingTransform(
      (lhs.matrix() * rhs.matrix()).diagonal().asDiagonal());
};

[[nodiscard]] inline Eigen::Matrix3d operator*(const ScalingTransform &lhs,
                                               const RotationTransform &rhs) {
  return lhs.matrix() * rhs.matrix();
};

[[nodiscard]] inline Eigen::Matrix3d operator*(const ScalingTransform &lhs,
                                               const Eigen::Matrix3d &rhs) {
  return lhs.matrix() * rhs;
};

[[nodiscard]] inline Eigen::Affine3d
operator*(const ScalingTransform &lhs, const TranslationTransform &rhs) {
  return lhs.matrix() * Eigen::Translation<double, 3>(rhs.vector());
};

[[nodiscard]] inline Eigen::Affine3d operator*(const ScalingTransform &lhs,
                                               const Eigen::Affine3d &rhs) {
  return lhs.matrix() * rhs;
};

[[nodiscard]] inline Eigen::Vector3d operator*(const ScalingTransform &lhs,
                                               const Eigen::Vector3d &rhs) {
  return lhs.matrix() * rhs;
};

[[nodiscard]] inline Eigen::Affine3d operator*(const TranslationTransform &lhs,
                                               const ScalingTransform &rhs) {
  return Eigen::Translation<double, 3>(lhs.vector()) * rhs.matrix();
};

[[nodiscard]] inline Eigen::Affine3d operator*(const TranslationTransform &lhs,
                                               const RotationTransform &rhs) {
  return Eigen::Translation<double, 3>(lhs.vector()) * rhs.matrix();
};

[[nodiscard]] inline Eigen::Affine3d operator*(const TranslationTransform &lhs,
                                               const Eigen::Matrix3d &rhs) {
  return Eigen::Translation<double, 3>(lhs.vector()) * rhs;
};

[[nodiscard]] inline TranslationTransform
operator*(const TranslationTransform &lhs, const TranslationTransform &rhs) {
  return TranslationTransform(lhs.vector() + rhs.vector());
};

[[nodiscard]] inline Eigen::Affine3d operator*(const TranslationTransform &lhs,
                                               const Eigen::Affine3d &rhs) {
  return Eigen::Translation<double, 3>(lhs.vector()) * rhs;
};

[[nodiscard]] inline Eigen::Vector3d operator*(const TranslationTransform &lhs,
                                               const Eigen::Vector3d &rhs) {
  return lhs.vector() + rhs;
};

[[nodiscard]] inline Eigen::Affine3d
operator*(const Eigen::Affine3d &lhs, const TranslationTransform &rhs) {
  return lhs * Eigen::Translation<double, 3>(rhs.vector());
}

[[nodiscard]] inline Eigen::Affine3d operator*(const Eigen::Affine3d &lhs,
                                               const RotationTransform &rhs) {
  return Eigen::Affine3d(lhs * rhs.matrix());
}

[[nodiscard]] inline Eigen::Affine3d operator*(const Eigen::Affine3d &lhs,
                                               const ScalingTransform &rhs) {
  return Eigen::Affine3d(lhs * rhs.matrix());
}

[[nodiscard]] inline Eigen::Affine3d
operator*(const Eigen::Matrix3d &lhs, const TranslationTransform &rhs) {
  return lhs * Eigen::Translation<double, 3>(rhs.vector());
}

[[nodiscard]] inline Eigen::Matrix3d operator*(const Eigen::Matrix3d &lhs,
                                               const RotationTransform &rhs) {
  return lhs * rhs.matrix();
}

[[nodiscard]] inline Eigen::Matrix3d operator*(const Eigen::Matrix3d &lhs,
                                               const ScalingTransform &rhs) {
  return lhs * rhs.matrix();
}

template <> inline constexpr DType dtype<Eigen::Matrix3d>{5001};
template <> inline constexpr DType dtype<Eigen::Affine3d>{5002};
template <> inline constexpr DType dtype<TranslationTransform>{5003};
template <> inline constexpr DType dtype<ScalingTransform>{5004};
template <> inline constexpr DType dtype<RotationTransform>{5005};

} // namespace scipp::core
