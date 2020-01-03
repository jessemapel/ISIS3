#include <iostream>

#include <QTemporaryFile>

#include "spiceinit.h"

#include "Cube.h"
#include "CubeAttribute.h"
#include "PixelType.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "PvlKeyword.h"
#include "TestUtilities.h"

#include "gmock/gmock.h"

using namespace Isis;

class spiceinitTestCube : public ::testing::Test {
  protected:
    Cube testCube;
    QTemporaryFile tempFile;

  void SetUp() override {
    tempFile.open();
    testCube.setPixelType(PixelType::UNSIGNEDBYTE);
  }

  void TearDown() override {
    if (testCube.isOpen()) {
      testCube.close();
    }
  }

  void createCube(Pvl &label) {
    PvlObject cubeLabel = label.findObject("IsisCube");
    PvlGroup dimensions = cubeLabel.findObject("Core").findGroup("Dimensions");
    testCube.setDimensions(dimensions["Samples"],
                          dimensions["Lines"],
                          dimensions["Bands"]);
    testCube.create(tempFile.fileName());

    for (auto grpIt = cubeLabel.beginGroup(); grpIt!= cubeLabel.endGroup(); grpIt++) {
      testCube.putGroup(*grpIt);
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

  createCube(clementineLabel);

  spiceinitOptions options;
  options.ckrecon = true;
  options.cksmithed = true;
  options.attach = false;

  spiceinit(&testCube, options);

  PvlGroup kernels = testCube.group("Kernels");
  ASSERT_TRUE(kernels.hasKeyword("InstrumentPointing"));
  PvlKeyword instrumentPointing = kernels["InstrumentPointing"];
  ASSERT_EQ(instrumentPointing.size(), 3);
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[0], "$Clementine1/kernels/ck/clem_2mn.bck");
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[1], "$Clementine1/kernels/ck/clem_5sc.bck");
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[2], "$clementine1/kernels/fk/clem_v12.tf");
}

TEST_F(spiceinitTestCube, CkConfigFile) {

  std::istringstream crismLabelStrm(R"(
    Object = IsisCube
      Object = Core
        StartByte   = 65537
        Format      = Tile
        TileSamples = 320
        TileLines   = 420

        Group = Dimensions
          Samples = 640
          Lines   = 420
          Bands   = 1
        End_Group

        Group = Pixels
          Type       = Real
          ByteOrder  = Lsb
          Base       = 0.0
          Multiplier = 1.0
        End_Group
      End_Object

      Group = Instrument
        SpacecraftName            = "MARS RECONNAISSANCE ORBITER"
        InstrumentId              = CRISM
        TargetName                = Mars
        StartTime                 = 2011-02-25T01:51:05.839
        StopTime                  = 2011-02-25T01:52:57.573
        SpacecraftClockStartCount = 10/0983065897.48805
        SpacecraftClockStopCount  = 10/0983066009.31381
        SensorId                  = S
        ShutterModeId             = OPEN
        FrameRate                 = 3.75 <HZ>
        ExposureParameter         = 184
        PixelAveragingWidth       = 1
        ScanModeId                = SHORT
        SamplingModeId            = HYPERSPEC
      End_Group

      Group = Archive
        DataSetId               = MRO-M-CRISM-3-RDR-TARGETED-V1.0
        ProductId               = FRT0001CFD8_07_IF124S_TRR3
        ProductType             = TARGETED_RDR
        ProductCreationTime     = 2011-03-02T10:59:47
        ObservationType         = FRT
        ObservationId           = 16#0001CFD8#
        ObservationNumber       = 16#07#
        ActivityId              = IF124
        DetectorTemperature     = -53.687
        OpticalBenchTemperature = -41.003
        SpectrometerHousingTemp = -64.976
        SphereTemperature       = -41.062
        FpeTemperature          = 6.847
        ProductVersionId        = 3
        SoftwareName            = crism_imagecal
      End_Group

      Group = Kernels
        NaifIkCode = -74017
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

  Pvl crismLabel;
  crismLabelStrm >> crismLabel;

  createCube(crismLabel);

  spiceinit(testCube);

  PvlGroup kernels = testCube->group("Kernels");
  ASSERT_TRUE(kernels.hasKeyword("InstrumentPointing"));
  PvlKeyword instrumentPointing = kernels["InstrumentPointing"];
  ASSERT_EQ(instrumentPointing.size(), 4);
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[0], "Table");
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[0], "$mro/kernels/ck/mro_crm_psp_110223_101128.bc");
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[1], "$mro/kernels/ck/mro_sc_psp_110222_110228.bc");
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[2], "$mro/kernels/fk/mro_v15.tf");
}
