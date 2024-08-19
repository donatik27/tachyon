// Copyright (c) 2022 The Plonky3 Authors
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.plonky3 and the LICENCE-APACHE.plonky3
// file.

#ifndef TACHYON_MATH_POLYNOMIALS_UNIVARIATE_TWO_ADIC_SUBGROUP_H_
#define TACHYON_MATH_POLYNOMIALS_UNIVARIATE_TWO_ADIC_SUBGROUP_H_

#include <optional>
#include <utility>
#include <vector>

#include "tachyon/base/optional.h"
#include "tachyon/math/matrix/matrix_types.h"

namespace tachyon::math {

template <typename F>
class TwoAdicSubgroup {
 public:
  virtual ~TwoAdicSubgroup() = default;

  // Compute the discrete Fourier transform (DFT) of each column in |mat|.
  virtual void FFTBatch(Eigen::MatrixBase<RowMajorMatrix<F>>& mat) = 0;

  // Compute the inverse DFT of each column in |mat|.
  void IFFTBatch(Eigen::MatrixBase<RowMajorMatrix<F>>& mat) {
    if constexpr (F::Config::kModulusBits > 32) {
      NOTREACHED();
    }
    FFTBatch(mat);
    Eigen::Index rows = mat.rows();
    // TODO(chokobole): Use |size_inv_| instead of directly computing the value.
    F inv = unwrap(F(rows).Inverse());

    mat.row(0) *= inv;
    mat.row(rows / 2) *= inv;
    OMP_PARALLEL_FOR(Eigen::Index row = 1; row < rows / 2; ++row) {
      auto row1 = mat.row(row);
      auto row2 = mat.row(rows - row);
      row1 *= inv;
      row2 *= inv;
      row1.swap(row2);
    }
  }

  // Compute the low-degree extension of each column in |mat| onto a coset of
  // a larger subgroup.
  virtual void CosetLDEBatch(Eigen::MatrixBase<RowMajorMatrix<F>>& mat,
                             size_t added_bits, F shift) {
    if constexpr (F::Config::kModulusBits > 32) {
      NOTREACHED();
    }
    IFFTBatch(mat);
    Eigen::Index rows = mat.rows();
    Eigen::Index new_rows = rows << added_bits;
    Eigen::Index cols = mat.cols();

    // Possible crash if the new resized length overflows
    RowMajorMatrix<F> new_mat(new_rows, cols);
    OMP_PARALLEL_FOR(Eigen::Index row = 0; row < new_rows; ++row) {
      if (row < rows) {
        new_mat.row(row) = mat.row(row);
      } else {
        new_mat.row(row).setZero();
      }
    }
    mat = std::move(new_mat);
    CosetFFTBatch(mat, shift);
  }

 private:
  // Compute the "coset DFT" of each column in |mat|. This can be viewed as
  // interpolation onto a coset of a multiplicative subgroup, rather than the
  // subgroup itself.
  void CosetFFTBatch(Eigen::MatrixBase<RowMajorMatrix<F>>& mat, F shift) {
    if constexpr (F::Config::kModulusBits > 32) {
      NOTREACHED();
    }
    // Observe that
    // yᵢ = ∑ⱼ cⱼ (s gⁱ)ʲ
    //    = ∑ⱼ (cⱼ sʲ) (gⁱ)ʲ
    // which has the structure of an ordinary DFT, except each coefficient cⱼ
    // is first replaced by cⱼ s.
    size_t rows = mat.rows();
    base::Parallelize(
        rows, [&mat, &shift](Eigen::Index len, Eigen::Index chunk_offset,
                             Eigen::Index chunk_size) {
          Eigen::Index start = chunk_offset * chunk_size;
          F weight = shift.Pow(start);
          // NOTE: It is not possible to have empty chunk so this is safe
          for (Eigen::Index row = start; row < start + len - 1; ++row) {
            for (Eigen::Index col = 0; col < mat.cols(); ++col) {
              mat(row, col) *= weight;
            }
            weight *= shift;
          }
          for (Eigen::Index col = 0; col < mat.cols(); ++col) {
            mat(start + len - 1, col) *= weight;
          }
        });
    FFTBatch(mat);
  }
};

}  // namespace tachyon::math
#endif  // TACHYON_MATH_POLYNOMIALS_UNIVARIATE_TWO_ADIC_SUBGROUP_H_
