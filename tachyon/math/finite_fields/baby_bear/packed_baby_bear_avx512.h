// Copyright (c) 2022 The Plonky3 Authors
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.plonky3 and the LICENCE-APACHE.plonky3
// file.

#ifndef TACHYON_MATH_FINITE_FIELDS_BABY_BEAR_PACKED_BABY_BEAR_AVX512_H_
#define TACHYON_MATH_FINITE_FIELDS_BABY_BEAR_PACKED_BABY_BEAR_AVX512_H_

#include <stddef.h>

#include "tachyon/math/finite_fields/baby_bear/baby_bear.h"
#include "tachyon/math/finite_fields/packed_prime_field_base.h"

namespace tachyon::math {

class PackedBabyBearAVX512;

template <>
struct PackedPrimeFieldTraits<PackedBabyBearAVX512> {
  using PrimeField = BabyBear;

  constexpr static size_t N = 16;
};

class PackedBabyBearAVX512 : public PackedPrimeFieldBase<PackedBabyBearAVX512> {
 public:
  using PrimeField = BabyBear;

  constexpr static size_t N = 16;

  PackedBabyBearAVX512() = default;
  PackedBabyBearAVX512(const PackedBabyBearAVX512& other) = default;
  PackedBabyBearAVX512& operator=(const PackedBabyBearAVX512& other) = default;
  PackedBabyBearAVX512(PackedBabyBearAVX512&& other) = default;
  PackedBabyBearAVX512& operator=(PackedBabyBearAVX512&& other) = default;

  static void Init();

  static PackedBabyBearAVX512 Zero();

  static PackedBabyBearAVX512 One();

  static PackedBabyBearAVX512 Broadcast(const PrimeField& value);

  // AdditiveSemigroup methods
  PackedBabyBearAVX512 Add(const PackedBabyBearAVX512& other) const;

  // AdditiveGroup methods
  PackedBabyBearAVX512 Sub(const PackedBabyBearAVX512& other) const;

  PackedBabyBearAVX512 Negate() const;

  // MultiplicativeSemigroup methods
  PackedBabyBearAVX512 Mul(const PackedBabyBearAVX512& other) const;
};

}  // namespace tachyon::math

#endif  //  TACHYON_MATH_FINITE_FIELDS_BABY_BEAR_PACKED_BABY_BEAR_AVX512_H_
