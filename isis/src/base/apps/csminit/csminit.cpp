#include "csminit.h"

#include "QList.h"
#include "QString.h"
#include "QStringList.h"

#include "Isd.h"
#include "Model.h"
#include "NitfIsd.h"
#include "Plugin.h"

#include "Cube.h"
#include "IException.h"
#include "Process.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "PvlKeyword.h"

using namespace std;

namespace Isis {

  /**
   * csminit a cube in an Application
   *
   * @param ui The Application UI
   * @param(out) log The Pvl that attempted models will be logged to
   */
  void csminit(UserInterface &ui, Pvl *log) {
    QString isdFilePath = ui.GetFileName("ISD").expanded();

    QList<QStringList> possibleModels;
    for (const Plugin * plugin : csm::Plugin::getList()) {
      QString pluginName = QString::fromStdString(plugin->getPluginName());
      if (ui.WasEntered("PLUGINNAME") && pluginName != ui.GetString("PLUGINNAME")) {
        continue;
      }

      for (size_t modelIndex = 0; modelIndex < plugin->getNumModels(); modelIndex++) {
        QString modelName = QString::fromStdString(plugin->getModelName(modelIndex));
        if (ui.WasEntered("MODELNAME") && modelName != ui.GetString("MODELNAME")) {
          continue;
        }

        Isd fileIsd(isdFilePath.toStdString());
        if (plugin->canModelBeConstructedFromISD(fileIsd, modelName.toStdString())) {
          QStringList modelSpec = {pluginName, modelName, QString::fromStdString(fileIsd.format())};
          possibleModels.append(modelSpec);
          continue; // If the file ISD works, don't check the others
        }

        Nitf21Isd nitf21Isd(isdFilePath.toStdString());
        if (plugin->canModelBeConstructedFromISD(nitf21Isd, modelName.toStdString())) {
          QStringList modelSpec = {pluginName, modelName, QString::fromStdString(nitf21Isd.format())};
          possibleModels.append(modelSpec);
          continue; // If the NITF 2.1 ISD works, don't check the others
        }
      }
    }

    if (possibleModels.size() > 1) {
      QString message = "Multiple models can be created from the ISD [" + isdFilePath + "]. "
                        "Re-run with the PLUGINNAME and MODELNAME parameters. "
                        "Possible plugin & model names:\n";
      for (const QStringList &modelSpec : possibleModels) {
        message += "Plugin [" + modelSpec[0] + "], Model [" + modelSpec[1] + "]\n";
      }
      throw IException(IException::User, message, _FILEINFO_);
    }

    if (possibleModels.empty()) {
      QString message = "No loaded model could be created from the ISD [" + isdFilePath + "]."
                        "Loaded plugin & model names:\n";
      for (const Plugin * plugin : csm::Plugin::getList()) {
        QString pluginName = QString::fromStdString(plugin->getPluginName());
        for (size_t modelIndex = 0; modelIndex < plugin->getNumModels(); modelIndex++) {
          QString modelName = QString::fromStdString(plugin->getModelName(modelIndex));
          message += "Plugin [" + pluginName + "], Model [" + modelName + "]\n";
        }
      }
      throw IException(IException::User, message, _FILEINFO_);
    }

    // If we are here, then we have exactly 1 model
    QStringList modelSpec = possibleModels.front();
    if (modelSpec.size() != 3) {
      QString message = "Model specification [" + modelSpec.join(" ") + "] has [" + modelSpec.size() + "] elements "
                        "when it should have 3 elements.";
      throw IException(IException::Programmer, message, _FILEINFO_);
    }
    const Plugin *plugin = Plugin::findPlugin(modelSpec[0].toStdString());
    Model *model;
    Isd fileIsd(isdFilePath.toStdString());
    Nitf21Isd nitf21Isd(isdFilePath.toStdString());
    if (modelSpec[2] == QString::fromStdString(fileIsd.format())) {
      model = plugin->constructModelFromISD(fileIsd, modelSpec[1].toStdString());
    }
    else if (modelSpec[2] == QString::fromStdString(nitf21Isd.format())) {
      model = plugin->constructModelFromISD(nitf21Isd, modelSpec[1].toStdString());
    }
    else {
      QString message = "Invalid ISD format specifications [" + modelSpec[2] + "].";
      throw IException(IException::Programmer, message, _FILEINFO_);
    }

    string modelState = model->getModelState();

    // We are not processing the image data, so this process object is just for
    // managing the Cube in memory and adding history
    Process p;
    Cube *cube = p.SetInputCube(ui.GetFileName("FROM"), ui.GetInputAttribute("FROM"), ReadWrite);

    // Really should be
    // if (cube->camera()->type() == ISIS)
    //
    // Maybe just do spiceinit clean-up routine instead
    try {
      cube->camera();
      QString message = "Input cube [" + ui.GetFileName("FROM").expanded() + "]. "
                        "Already has an ISIS camera model associated with it. CSM "
                        "models cannot be added to cubes with an ISIS camera model.";
      throw IException(IException::Programmer, message, _FILEINFO_);
    }
    catch(IException &e) {
      // no operation, continue
    }

    cube->deleteBlob("String", "CSMState");
    cube->deleteGroup("Instrument");
    cube->deleteGroup("Kernels");

    PvlGroup instrumentGroup("Instrument");
    instrumentGroup += PvlKeyword("SpacecraftName", QString::fromStdString(model->getPlatformIdentifier()));
    instrumentGroup += PvlKeyword("InstrumentId", QString::fromStdString(model->getSensorIdentifier()));
    instrumentGroup += PvlKeyword("TargetName", ui.GetString("TARGETNAME"));
    instrumentGroup += PvlKeyword("ReferenceTime", QString::fromStdString(model->getReferenceDateAndTime()));
    cube->putGroup(instrumentGroup);

    PvlGroup kernelsGroup("Kernels");
    if (ui.WasEntered("SHAPEMODEL")) {
      // TODO validate the shapemodel
      kernelsGroup += PvlKeyword("ShapeModel", ui.WasEntered());
    }
    else {
      kernelsGroup += PvlKeyword("ShapeModel", "Ellipsoid");
    }

    // Create our CSM State blob

    // Write CSM State blob to cube

    p.WriteHistory(*cube);
  }

}
