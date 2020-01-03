#ifndef spiceinit_h
#define spiceinit_h

#include <vector>

#include <QString>

#include "Cube.h"
#include "Pvl.h"
#include "UserInterface.h"

namespace Isis {

  struct spiceinitOptions {
    enum shapeSource {
      SYSTEM,
      ELLIPSOID,
      RINGPLANE,
      SYSTEM,
      USER
    };

    bool web = false;
    bool attach = true;
    bool cksmithed = false;
    bool ckrecon = true;
    bool ckpredicted = false;
    bool cknadir = false;
    bool spksmithed = false;
    bool spkrecon = true;
    bool spkpredicted = false;
    std::vector<QString> lsk = std::vector<QString>();
    std::vector<QString> pck = std::vector<QString>();
    std::vector<QString> tspk = std::vector<QString>();
    std::vector<QString> ik = std::vector<QString>();
    std::vector<QString> sclk = std::vector<QString>();
    std::vector<QString> ck = std::vector<QString>();
    std::vector<QString> fk = std::vector<QString>();
    std::vector<QString> spk = std::vector<QString>();
    std::vector<QString> iak = std::vector<QString>();
    std::vector<QString> extra = std::vector<QString>();
    std::vector<QString> model = std::vector<QString>();
    shapeSource shape = SYSTEM;
    double startpad = 0.0;
    double endpad = 0.0;
    QString url = "https://services.isis.astrogeology.usgs.gov/cgi-bin/spiceinit.cgi";
    int port = 443;
  };

  extern void spiceinit(UserInterface &ui,
                        Pvl *log = nullptr);
  extern void spiceinit(Cube *icube,
                        const spiceinitOptions &options = spiceinitOptions(),
                        Pvl *log = nullptr);
}

#endif
