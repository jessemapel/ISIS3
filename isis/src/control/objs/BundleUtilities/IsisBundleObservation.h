#ifndef IsisBundleObservation_h
#define IsisBundleObservation_h

/** This is free and unencumbered software released into the public domain.

The authors of ISIS do not claim copyright on the contents of this file.
For more details about the LICENSE terms and the AUTHORS, you will
find files of those names at the top level of this repository. **/

/* SPDX-License-Identifier: CC0-1.0 */

#include <QStringList>
#include <QVector>

#include "BundleImage.h"
#include "BundleObservationSolveSettings.h"
#include "BundleSettings.h"
#include "BundleTargetBody.h"
#include "LinearAlgebra.h"
#include "BundleObservation.h"
#include "BundleMeasure.h"
#include "SurfacePoint.h"

namespace Isis {
  class BundleObservationSolveSettings;
  class SpicePosition;
  class SpiceRotation;

  /**
   * @brief Class for bundle observations
   *
   * This class is used for creating a bundle observation. Contained BundleImages are stored as
   * shared pointers, so they will be automatically deleted when all shared pointers are deleted.
   *
   * @ingroup ControlNetworks
   *
   * @author 2014-07-09 Ken Edmundson
   *
   * @internal
   *   @history 2014-07-09 Ken Edmundson - Original version.
   *   @history 2014-07-16 Jeannie Backer - Replaced QVectors with QLists.
   *   @history 2014-07-17 Kimberly Oyama - Added member variables and accessors for the images and
   *                           parameters in this observation. They will be used for the correlation
   *                           matrix.
   *   @history 2014-07-23 Jeannie Backer - Replaced QVectors with QLists.
   *   @history 2015-02-20 Jeannie Backer - Brought closer to Isis coding standards.
   *   @history 2016-08-03 Jesse Mapel - Changed contained member type to a QSharedPointer.
   *                           Also changed m_solveSettings to a QSharedPointer. Fixes #4150.
   *   @history 2016-08-03 Ian Humphrey - Updated documentation and coding standards. Fixes #4078.
   *   @history 2016-08-10 Jeannie Backer - Replaced boost vector with Isis::LinearAlgebra::Vector.
   *                           References #4163.
   *   @history 2016-08-15 Jesse Mapel - Added a map between cube serial number and contained
   *                           bundle image.  References #4159.
   *   @history 2016-08-23 Ian Humphrey - The applyParameterCorrections() method now throws the
   *                           last exception. Fixes #4153.
   *   @history 2016-10-06 Tyler Wilson - Modified the function formatBundleOutputString so
   *                           that it can be used by BundleSolutionInfo::outputCSVImages()
   *                           function.  Fixes #4314.
   *   @history 2016-10-26 Ian Humphrey - Modified formatBundleOutputString() to provided default
   *                           values for all solve parameters, whether they are being solved for
   *                           or not. Fixes #4464.
   *   @history 2016-10-27 Tyler Wilson - Modified formatBundleOutputString to change N/A to FREE
   *                           in the output under POINTS DETAIL when no lat/lon sigmas were entered.
   *                           Fixes #4317.
   *   @history 2016-11-14 Ken Edmundson - Modified the following...
   *                           -changed adjustedSigma from 0.0 to N/A if error propagation is off
   *                            when writing bundleout.txt OR images.csv.
   *                           -changed sigma default from -1.0 to N/A for position and pointing
   *                            parameters when writing images.csv.
   *   @history 2019-05-14 Tyler Wilson - Added the bundleOutputString(std::ofstream &fpOut,
   *                            bool errorPropagation) function which is called by
   *                            BundleSolutionInfo::outputText(). This function is a refactor of
   *                            the formatBundleOutputString and uses the traditional C function
   *                            sprintf instead of QString arg chaining because it's easier to
   *                            make the output columns align nicely.  Also, it maintains
   *                            consistency with text output in BundleSolutionInfo.
   *   @history 2019-06-03  Tyler Wilson - Deleted the formatBundleOutputString and added the
   *                            functions bundleOutputCSV/bundleOutputFetchData. Combined
   *                            with bundleOutputString these three functions will fulfill
   *                            the same functional role formerly occuped by
   *                            formatBundleOutputString but with reduced code duplication.
   *   @history 2019-08-15  Adam Paquette - Readded the formatBundleOutputString function
   *                            and added deprication warnings to formatBundleOutputString.
   *   @history 2019-09-10  Adam Paquette - Changed how bundleOutputString formats the text
   *                            that is written to the bundleout.txt file.
   *   @history 2021-06-07  Tim Giroux - Rename BundleObservation to IsisBundleObservation
   *
   */
  class IsisBundleObservation : public BundleObservation {

    public:
      // default constructor
      IsisBundleObservation();

      // constructor
      IsisBundleObservation(BundleImageQsp image, QString observationNumber, QString instrumentId,
                        BundleTargetBodyQsp bundleTargetBody);

      // copy constructor
      IsisBundleObservation(const IsisBundleObservation &src);

      // destructor
      ~IsisBundleObservation();

      // equals operator
      IsisBundleObservation &operator=(const IsisBundleObservation &src);

      // copy method
      void copy(const IsisBundleObservation &src);

      virtual bool setSolveSettings(BundleObservationSolveSettings solveSettings);

      int numberPositionParameters();
      int numberPointingParameters();
      int numberParameters();

      SpiceRotation *spiceRotation();
      SpicePosition *spicePosition();

      const BundleObservationSolveSettingsQsp solveSettings();

      bool applyParameterCorrections(LinearAlgebra::Vector corrections);
      bool initializeExteriorOrientation();
      void initializeBodyRotation();
      void updateBodyRotation();

      void bundleOutputFetchData(QVector<double> &finalParameterValues,
                            int &nPositionCoefficients, int &nPointingCoefficients,
                            bool &useDefaultPosition, bool &useDefaultPointing,
                            bool &useDefaultTwist);
      void bundleOutputString(std::ostream &fpOut,bool errorPropagation);
      QString bundleOutputCSV(bool errorPropagation);

      QString formatBundleOutputString(bool errorPropagation, bool imageCSV=false);

      virtual QStringList parameterList();

      bool computeTargetPartials(LinearAlgebra::Matrix &coeffTarget, BundleMeasure &measure, BundleSettingsQsp &bundleSettings, BundleTargetBodyQsp &bundleTargetBody);
      bool computeImagePartials(LinearAlgebra::Matrix &coeffImage, BundleMeasure &measure);
      bool computePoint3DPartials(LinearAlgebra::Matrix &coeffPoint3D, BundleMeasure &measure, SurfacePoint::CoordinateType coordType);
      bool computeRHSPartials(LinearAlgebra::Vector &coeffRHS, BundleMeasure &measure);
      double computeObservationValue(BundleMeasure &measure, double deltaVal);

   private:
      bool initParameterWeights();
      BundleObservationSolveSettingsQsp m_solveSettings; //!< Solve settings for this observation.

      SpiceRotation *m_instrumentRotation;   //!< Instrument spice rotation (in primary image).
      SpicePosition *m_instrumentPosition;   //!< Instrument spice position (in primary image).

      BundleTargetBodyQsp m_bundleTargetBody;       //!< QShared pointer to BundleTargetBody.
  };

  //! Typdef for IsisBundleObservation QSharedPointer.
  typedef QSharedPointer<IsisBundleObservation> IsisBundleObservationQsp;
}

#endif // IsisBundleObservation_h