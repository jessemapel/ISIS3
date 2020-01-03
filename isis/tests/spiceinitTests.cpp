#include <iostream>

#include <QTemporaryFile>

#include "spiceinit.h"

#include "Cube.h"
#include "CubeAttribute.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "PvlKeyword.h"
#include "TestUtilities.h"

#include "gmock/gmock.h"

using namespace Isis;

class spiceinitTestCube : public ::testing::Test {
  protected:
    Cube *testCube;
    QTemporaryFile tempFile;

  void SetUp() override {
    Cube baseCube("$base/testData/isisTruth.cub", "r");
    CubeAttributeOutput cubeAtts;
    tempFile.open();
    testCube = baseCube.copy(tempFile.fileName(), cubeAtts);
  }

  void TearDown() override {
    if (testCube) {
      delete testCube;
    }
  }
};

TEST_F(spiceinitTestCube, PredictAndReconCK) {

  std::istringstream clementineLabelStrm(R"(
    Object = IsisCube
      Object = Core
        StartByte   = 65537
        Format      = Tile
        TileSamples = 384
        TileLines   = 288

        Group = Dimensions
          Samples = 384
          Lines   = 288
          Bands   = 1
        End_Group

        Group = Pixels
          Type       = UnsignedByte
          ByteOrder  = Lsb
          Base       = 0.0
          Multiplier = 1.0
        End_Group
      End_Object

      Group = Instrument
        SpacecraftName           = "CLEMENTINE 1"
        InstrumentId             = UVVIS
        TargetName               = MOON
        StartTime                = 1994-03-05T08:21:22.626
        OrbitNumber              = 063
        FocalPlaneTemperature    = 273.633 <K>
        ExposureDuration         = 20.3904 <ms>
        OffsetModeID             = 6
        GainModeID               = 1
        CryocoolerDuration       = N/A
        EncodingCompressionRatio = 3.55
        EncodingFormat           = CLEM-JPEG-1
      End_Group

      Group = Archive
        ProductID    = LUB5120P.063
        MissionPhase = "LUNAR MAPPING"
      End_Group

      Group = BandBin
        FilterName = B
        Center     = 0.75 <micrometers>
        Width      = 0.01 <micrometers>
      End_Group

      Group = Kernels
        NaifFrameCode = -40022
      End_Group
    End_Object

    Object = Label
      Bytes = 65536
    End_Object

    Object = History
      Name      = IsisCube
      StartByte = 176129
      Bytes     = 456
    End_Object

    Object = OriginalLabel
      Name      = IsisCube
      StartByte = 176585
      Bytes     = 5409
    End_Object
  End
  )");

  Pvl clementineLabel;
  clementineLabelStrm >> clementineLabel;

  testCube->putGroup(clementineLabel.findObject("IsisCube").findGroup("Instrument"));
  testCube->putGroup(clementineLabel.findObject("IsisCube").findGroup("Archive"));
  testCube->putGroup(clementineLabel.findObject("IsisCube").findGroup("BandBin"));
  testCube->putGroup(clementineLabel.findObject("IsisCube").findGroup("Kernels"));

  spiceinitOptions options;
  options.ckrecon = true;
  options.cksmithed = true;
  options.attach = false;

  spiceinit(testCube, options);

  PvlGroup kernels = testCube->group("Kernels");
  ASSERT_TRUE(kernels.hasKeyword("InstrumentPointing"));
  PvlKeyword instrumentPointing = kernels["InstrumentPointing"];
  ASSERT_EQ(instrumentPointing.size(), 3);
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[0], "$Clementine1/kernels/ck/clem_2mn.bck");
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[1], "$Clementine1/kernels/ck/clem_5sc.bck");
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[2], "$clementine1/kernels/fk/clem_v12.tf");
}
