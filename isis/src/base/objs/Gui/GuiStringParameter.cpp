/** This is free and unencumbered software released into the public domain.
The authors of ISIS do not claim copyright on the contents of this file.
For more details about the LICENSE terms and the AUTHORS, you will
find files of those names at the top level of this repository. **/

/* SPDX-License-Identifier: CC0-1.0 */
#include <QHBoxLayout>

#include "GuiStringParameter.h"
#include "UserInterface.h"

namespace Isis {

  GuiStringParameter::GuiStringParameter(QGridLayout *grid, UserInterface &ui,
                                         int group, int param) :
    GuiParameter(grid, ui, group, param) {

    p_lineEdit = new QLineEdit;
    connect(p_lineEdit, SIGNAL(textChanged(const QString &)), this, SIGNAL(ValueChanged()));
    grid->addWidget(p_lineEdit, param, 2);

    if(p_ui->HelpersSize(group, param) != 0) {
      grid->addWidget(AddHelpers(p_lineEdit), param, 3);
    }

    RememberWidget(p_lineEdit);

    p_type = StringWidget;
  }


  GuiStringParameter::~GuiStringParameter() {}

  void GuiStringParameter::Set(QString newValue) {
    p_lineEdit->setText(newValue);
  }


  QString GuiStringParameter::Value() {
    return p_lineEdit->text();
  }


}

