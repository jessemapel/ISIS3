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
    testCube.setPixelType(UnsignedByte);
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

  std::istringstream labelStrm(R"(
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
  End
  )");

  Pvl label;
  labelStrm >> label;

  createCube(label);

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
  ASSERT_TRUE(kernels.hasKeyword("InstrumentPointingQuality"));
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, kernels["InstrumentPointingQuality"][0], "Reconstructed");
}


TEST_F(spiceinitTestCube, CkConfigFile) {

  std::istringstream labelStrm(R"(
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
  End
  )");

  Pvl label;
  labelStrm >> label;

  createCube(label);

  spiceinit(&testCube);

  PvlGroup kernels = testCube.group("Kernels");
  ASSERT_TRUE(kernels.hasKeyword("InstrumentPointing"));
  PvlKeyword instrumentPointing = kernels["InstrumentPointing"];
  ASSERT_EQ(instrumentPointing.size(), 4);
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[0], "Table");
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[1], "$mro/kernels/ck/mro_crm_psp_110223_101128.bc");
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[2], "$mro/kernels/ck/mro_sc_psp_110222_110228.bc");
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, instrumentPointing[3], "$mro/kernels/fk/mro_v15.tf");
}


TEST_F(spiceinitTestCube, Default) {

  std::istringstream labelStrm(R"(
    Object = IsisCube
      Object = Core
        StartByte   = 65537
        Format      = Tile
        TileSamples = 128
        TileLines   = 128

        Group = Dimensions
          Samples = 1204
          Lines   = 1056
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
        SpacecraftName       = VIKING_ORBITER_1
        InstrumentId         = VISUAL_IMAGING_SUBSYSTEM_CAMERA_B
        TargetName           = MARS
        StartTime            = 1977-07-09T20:05:51
        ExposureDuration     = 0.008480 <seconds>
        SpacecraftClockCount = 33322515
        FloodModeId          = ON
        GainModeId           = HIGH
        OffsetModeId         = ON
      End_Group

      Group = Archive
        DataSetId       = VO1/VO2-M-VIS-2-EDR-V2.0
        ProductId       = 387A06
        MissonPhaseName = EXTENDED_MISSION
        ImageNumber     = 33322515
        OrbitNumber     = 387
      End_Group

      Group = BandBin
        FilterName = CLEAR
        FilterId   = 4
      End_Group

      Group = Kernels
        NaifFrameCode = -27002
      End_Group

      Group = Reseaus
        Line     = (5, 6, 8, 9, 10, 11, 12, 13, 14, 14, 15, 133, 134, 135, 137,
                    138, 139, 140, 141, 141, 142, 143, 144, 263, 264, 266, 267,
                    268, 269, 269, 270, 271, 272, 273, 393, 393, 395, 396, 397,
                    398, 399, 399, 400, 401, 402, 403, 523, 524, 525, 526, 527,
                    527, 528, 529, 530, 530, 532, 652, 652, 654, 655, 656, 657,
                    657, 658, 659, 660, 661, 662, 781, 783, 784, 785, 786, 787,
                    788, 788, 789, 790, 791, 911, 912, 913, 914, 915, 916, 917,
                    918, 918, 919, 920, 921, 1040, 1041, 1043, 1044, 1045, 1045,
                    1046, 1047, 1047, 1048, 1050)
        Sample   = (24, 142, 259, 375, 491, 607, 723, 839, 954, 1070, 1185, 24,
                    84, 201, 317, 433, 549, 665, 780, 896, 1011, 1127, 1183, 25,
                    142, 259, 375, 492, 607, 722, 838, 953, 1068, 1183, 25, 84,
                    201, 317, 433, 549, 665, 779, 895, 1010, 1125, 1182, 25, 143,
                    259, 375, 491, 607, 722, 837, 952, 1067, 1182, 25, 84, 201,
                    317, 433, 548, 664, 779, 894, 1009, 1124, 1181, 25, 142, 258,
                    374, 490, 605, 720, 835, 951, 1066, 1180, 24, 83, 200, 316,
                    431, 547, 662, 776, 892, 1007, 1122, 1179, 23, 140, 257, 373,
                    488, 603, 718, 833, 948, 1063, 1179)
        Type     = (1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                    5, 6, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 4, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 6, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 4, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 6, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 4, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 6, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6)
        Valid    = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
        Template = $viking1/reseaus/vo1.visb.template.cub
        Status   = Nominal
      End_Group
    End_Object
  End
  )");

  Pvl label;
  labelStrm >> label;

  createCube(label);

  spiceinit(&testCube);

  PvlGroup kernels = testCube.group("Kernels");

  EXPECT_TRUE(kernels.hasKeyword("InstrumentPointing"));
  EXPECT_TRUE(kernels.hasKeyword("LeapSecond"));
  EXPECT_TRUE(kernels.hasKeyword("TargetAttitudeShape"));
  EXPECT_TRUE(kernels.hasKeyword("TargetPosition"));
  EXPECT_TRUE(kernels.hasKeyword("InstrumentPointing"));
  EXPECT_TRUE(kernels.hasKeyword("Instrument"));
  EXPECT_TRUE(kernels.hasKeyword("SpacecraftClock"));
  EXPECT_TRUE(kernels.hasKeyword("InstrumentPosition"));
  EXPECT_TRUE(kernels.hasKeyword("InstrumentAddendum"));
  EXPECT_TRUE(kernels.hasKeyword("ShapeModel"));
  EXPECT_TRUE(kernels.hasKeyword("InstrumentPositionQuality"));
  EXPECT_TRUE(kernels.hasKeyword("InstrumentPointingQuality"));
  EXPECT_TRUE(kernels.hasKeyword("CameraVersion"));

  spiceinit(&testCube);

  PvlGroup secondKernels = testCube.group("Kernels");

  EXPECT_PRED_FORMAT2(AssertPvlGroupEqual, secondKernels, kernels);
}


TEST_F(spiceinitTestCube, Nadir) {

  std::istringstream labelStrm(R"(
    Object = IsisCube
      Object = Core
        StartByte   = 65537
        Format      = Tile
        TileSamples = 128
        TileLines   = 128

        Group = Dimensions
          Samples = 1536
          Lines   = 2688
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
        SpacecraftName        = "MARS GLOBAL SURVEYOR"
        InstrumentId          = MOC-NA
        TargetName            = Mars
        StartTime             = 2000-03-19T04:51:46.63
        StopTime              = 2000-03-19T04:51:47.92
        CrosstrackSumming     = 1
        DowntrackSumming      = 1
        FocalPlaneTemperature = 270.3
        GainModeId            = 0A
        LineExposureDuration  = 0.482100 <milliseconds>
        MissionPhaseName      = MAPPING
        OffsetModeId          = 38
        SpacecraftClockCount  = 637908733:72
        RationaleDesc         = "Sample of smooth plains in highlands "
        OrbitNumber           = 4604
        FirstLineSample       = 1
      End_Group

      Group = Archive
        DataSetId           = MGS-M-MOC-NA/WA-2-DSDP-L0-V1.0
        ProductId           = M13/01260
        ProducerId          = MGS_MOC_TEAM
        ProductCreationTime = 2001-03-01T03:00:38
        SoftwareName        = "makepds 1.9"
        UploadId            = UNK
        DataQualityDesc     = OK
        ImageNumber         = 07901260
        ImageKeyId          = 6379001260
      End_Group

      Group = BandBin
        FilterName   = BROAD_BAND
        OriginalBand = 1
        Center       = 0.7 <micrometers>
        Width        = 0.4 <micrometers>
      End_Group

      Group = Kernels
        NaifFrameCode = -94031
      End_Group
    End_Object
  End
  )");

  Pvl label;
  labelStrm >> label;

  createCube(label);

  spiceinit(&testCube);

  spiceinitOptions options;
  options.cknadir = true;
  options.attach = false;

  PvlGroup kernels = testCube.group("Kernels");

  ASSERT_TRUE(kernels.hasKeyword("InstrumentPointing"));
  ASSERT_EQ(kernels["InstrumentPointing"].size(), 1);
  EXPECT_PRED_FORMAT2(AssertQStringsEqual, kernels["InstrumentPointing"][0], "Nadir");
}
