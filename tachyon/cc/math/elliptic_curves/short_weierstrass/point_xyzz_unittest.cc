#include "gtest/gtest.h"

#include "tachyon/c/math/elliptic_curves/bn/bn254/fq_prime_field_traits.h"
#include "tachyon/c/math/elliptic_curves/bn/bn254/g1_point_traits.h"
#include "tachyon/cc/math/elliptic_curves/bn/bn254/g1.h"
#include "tachyon/cc/math/elliptic_curves/point_conversions.h"
#include "tachyon/math/elliptic_curves/bn/bn254/g1.h"

namespace tachyon::cc::math {

namespace {

class PointXYZZTest : public testing::Test {
 public:
  static void SetUpTestSuite() {
    tachyon::math::bn254::G1PointXYZZ::Curve::Init();
  }

  void SetUp() override {
    a_ = tachyon::math::bn254::G1PointXYZZ::Random();
    b_ = tachyon::math::bn254::G1PointXYZZ::Random();

    cc_a_ = bn254::G1PointXYZZ(ToCPointXYZZ(a_));
    cc_b_ = bn254::G1PointXYZZ(ToCPointXYZZ(b_));
  }

 protected:
  tachyon::math::bn254::G1PointXYZZ a_;
  tachyon::math::bn254::G1PointXYZZ b_;
  bn254::G1PointXYZZ cc_a_;
  bn254::G1PointXYZZ cc_b_;
};

}  // namespace

TEST_F(PointXYZZTest, Zero) {
  bn254::G1PointXYZZ c_ret = bn254::G1PointXYZZ::Zero();
  EXPECT_TRUE(ToPointXYZZ(c_ret.ToCPoint()).IsZero());
}

TEST_F(PointXYZZTest, Generator) {
  bn254::G1PointXYZZ c_ret = bn254::G1PointXYZZ::Generator();
  EXPECT_EQ(ToPointXYZZ(c_ret.ToCPoint()),
            tachyon::math::bn254::G1PointXYZZ::Generator());
}

TEST_F(PointXYZZTest, Random) {
  bn254::G1PointXYZZ c_ret = bn254::G1PointXYZZ::Random();
  EXPECT_NE(ToPointXYZZ(c_ret.ToCPoint()), a_);
}

}  // namespace tachyon::cc::math
