// Copyright (c) 2022 The Plonky3 Authors
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.plonky3 and the LICENCE-APACHE.plonky3
// file.

#include "tachyon/crypto/commitments/merkle_tree/field_merkle_tree/field_merkle_tree_mmcs.h"

#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "tachyon/crypto/hashes/sponge/padding_free_sponge.h"
#include "tachyon/crypto/hashes/sponge/poseidon2/param_traits/poseidon2_baby_bear.h"
#include "tachyon/crypto/hashes/sponge/poseidon2/poseidon2.h"
#include "tachyon/crypto/hashes/sponge/poseidon2/poseidon2_params.h"
#include "tachyon/crypto/hashes/sponge/truncated_permutation.h"
#include "tachyon/math/finite_fields/test/finite_field_test.h"

namespace tachyon::crypto {

constexpr size_t kRate = 8;
constexpr size_t kChunk = 8;
constexpr size_t kN = 2;

using F = math::BabyBear;
using PackedF = math::PackedBabyBear;
using Params = Poseidon2Params<Poseidon2Vendor::kPlonky3,
                               Poseidon2Vendor::kPlonky3, F, 15, 7>;
using PackedParams = Poseidon2Params<Poseidon2Vendor::kPlonky3,
                                     Poseidon2Vendor::kPlonky3, PackedF, 15, 7>;
using Poseidon2 = Poseidon2Sponge<Params>;
using PackedPoseidon2 = Poseidon2Sponge<PackedParams>;
using MyHasher = PaddingFreeSponge<Poseidon2, kRate, kChunk>;
using MyPackedHasher = PaddingFreeSponge<PackedPoseidon2, kRate, kChunk>;
using MyCompressor = TruncatedPermutation<Poseidon2, kChunk, kN>;
using MyPackedCompressor = TruncatedPermutation<PackedPoseidon2, kChunk, kN>;
using Tree = FieldMerkleTree<F, kChunk>;
using MMCS = FieldMerkleTreeMMCS<F, MyHasher, MyPackedHasher, MyCompressor,
                                 MyPackedCompressor, kChunk>;

namespace {

class FieldMerkleTreeMMCSTest : public math::FiniteFieldTest<PackedF> {
 protected:
  MMCS mmcs_;
};

}  // namespace

TEST_F(FieldMerkleTreeMMCSTest, Commit) {
  std::vector<F> vector{F(2), F(1), F(2), F(2), F(0), F(0), F(1), F(0)};
  const math::RowMajorMatrix<F> owned_matrix{
      {F(2)}, {F(1)}, {F(2)}, {F(2)}, {F(0)}, {F(0)}, {F(1)}, {F(0)},
  };
  Eigen::Map<const math::RowMajorMatrix<F>> matrix = math::Map(owned_matrix);
  std::vector<Eigen::Map<const math::RowMajorMatrix<F>>> matrices = {matrix};

  std::vector<Eigen::Map<const math::RowMajorMatrix<F>>> matrices_tmp =
      matrices;
  Tree tree =
      Tree::BuildCpu(mmcs_.hasher(), mmcs_.packed_hasher(), mmcs_.compressor(),
                     mmcs_.packed_compressor(), std::move(matrices_tmp));

  {
    std::array<F, kChunk> commitment;
    Tree prover_data;
    ASSERT_TRUE(mmcs_.Commit(vector, &commitment, &prover_data));
    EXPECT_EQ(commitment, tree.GetRoot());
  }
  {
    std::array<F, kChunk> commitment;
    Tree prover_data;
    ASSERT_TRUE(mmcs_.Commit(matrix, &commitment, &prover_data));
    EXPECT_EQ(commitment, tree.GetRoot());
  }
  {
    std::array<F, kChunk> commitment;
    Tree prover_data;
    ASSERT_TRUE(mmcs_.Commit(std::move(matrices), &commitment, &prover_data));
    EXPECT_EQ(commitment, tree.GetRoot());
  }
}

TEST_F(FieldMerkleTreeMMCSTest, CommitOwned) {
  std::vector<F> vector{F(2), F(1), F(2), F(2), F(0), F(0), F(1), F(0)};
  math::RowMajorMatrix<F> matrix{
      {F(2)}, {F(1)}, {F(2)}, {F(2)}, {F(0)}, {F(0)}, {F(1)}, {F(0)},
  };
  std::vector<math::RowMajorMatrix<F>> matrices = {matrix};

  std::vector<math::RowMajorMatrix<F>> matrices_tmp = matrices;
  Tree tree = Tree::BuildOwnedCpu(mmcs_.hasher(), mmcs_.packed_hasher(),
                                  mmcs_.compressor(), mmcs_.packed_compressor(),
                                  std::move(matrices_tmp));

  {
    std::array<F, kChunk> commitment;
    Tree prover_data;
    ASSERT_TRUE(mmcs_.CommitOwned(vector, &commitment, &prover_data));
    EXPECT_EQ(commitment, tree.GetRoot());
  }
  {
    std::array<F, kChunk> commitment;
    Tree prover_data;
    ASSERT_TRUE(
        mmcs_.CommitOwned(std::move(matrix), &commitment, &prover_data));
    EXPECT_EQ(commitment, tree.GetRoot());
  }
  {
    std::array<F, kChunk> commitment;
    Tree prover_data;
    ASSERT_TRUE(
        mmcs_.CommitOwned(std::move(matrices), &commitment, &prover_data));
    EXPECT_EQ(commitment, tree.GetRoot());
  }
}

TEST_F(FieldMerkleTreeMMCSTest, CommitAndVerify) {
  struct Config {
    size_t num;
    math::Dimensions dimensions;
  };

  struct {
    std::vector<Config> configs;
    size_t index;
  } tests[] = {
      {{{4, {8, 1000}}, {5, {8, 70}}, {6, {8, 8}}}, 6},
      {{{1, {1, 32}},
        {1, {2, 32}},
        {1, {3, 32}},
        {1, {4, 32}},
        {1, {5, 32}},
        {1, {6, 32}},
        {1, {7, 32}},
        {1, {8, 32}},
        {1, {9, 32}},
        {1, {10, 32}}},
       17},
  };

  for (const auto& test : tests) {
    std::vector<math::RowMajorMatrix<F>> matrices;
    std::vector<math::Dimensions> dimensions_vec;
    for (size_t i = 0; i < test.configs.size(); ++i) {
      math::Dimensions dimensions = test.configs[i].dimensions;
      for (size_t j = 0; j < test.configs[i].num; ++j) {
        matrices.push_back(math::RowMajorMatrix<F>::Random(dimensions.height,
                                                           dimensions.width));
        dimensions_vec.push_back(dimensions);
      }
    }

    std::array<F, kChunk> commitment;
    Tree prover_data;
    ASSERT_TRUE(
        mmcs_.CommitOwned(std::move(matrices), &commitment, &prover_data));

    std::vector<std::vector<F>> openings;
    std::vector<std::array<F, kChunk>> proof;
    ASSERT_TRUE(
        mmcs_.CreateOpeningProof(test.index, prover_data, &openings, &proof));
    ASSERT_TRUE(mmcs_.VerifyOpeningProof(commitment, dimensions_vec, test.index,
                                         openings, proof));
  }
}

}  // namespace tachyon::crypto
