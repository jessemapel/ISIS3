#include "BundleResults.h"

#include "SurfacePoint.h"

#include <gtest/gtest.h>

using namespace Isis;

TEST(BundleResults, DefaultConstructor) {
  BundleResults testResults(NULL);

  EXPECT_TRUE(testResults.rmsImageSampleResiduals().isEmpty());
  EXPECT_TRUE(testResults.rmsImageLineResiduals().isEmpty());
  EXPECT_TRUE(testResults.rmsImageResiduals().isEmpty());
  EXPECT_TRUE(testResults.rmsImageXSigmas().isEmpty());
  EXPECT_TRUE(testResults.rmsImageYSigmas().isEmpty());
  EXPECT_TRUE(testResults.rmsImageZSigmas().isEmpty());
  EXPECT_TRUE(testResults.rmsImageRASigmas().isEmpty());
  EXPECT_TRUE(testResults.rmsImageDECSigmas().isEmpty());
  EXPECT_TRUE(testResults.rmsImageTWISTSigmas().isEmpty());

  EXPECT_EQ(SurfacePoint::Latitudinal, testResults.coordTypeReports());

  EXPECT_EQ(1.0e+12, testResults.minSigmaCoord1Distance.meters());
  EXPECT_EQ(0.0, testResults.maxSigmaCoord1Distance.meters());
  EXPECT_EQ(1.0e+12, testResults.minSigmaCoord2Distance.meters());
  EXPECT_EQ(0.0, testResults.maxSigmaCoord2Distance.meters());
  EXPECT_EQ(1.0e+12, testResults.minSigmaCoord3Distance.meters());
  EXPECT_EQ(0.0, testResults.maxSigmaCoord3Distance.meters());

  EXPECT_TRUE(testResults.minSigmaCoord1PointId().isEmpty());
  EXPECT_TRUE(testResults.maxSigmaCoord1PointId().isEmpty());
  EXPECT_TRUE(testResults.minSigmaCoord2PointId().isEmpty());
  EXPECT_TRUE(testResults.maxSigmaCoord2PointId().isEmpty());
  EXPECT_TRUE(testResults.minSigmaCoord3PointId().isEmpty());
  EXPECT_TRUE(testResults.maxSigmaCoord3PointId().isEmpty());
}
