/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/RealCalibData.h>
#include <server/common/PathBuilder.h>
#include <server/common/Env.h>


namespace sail {

Array<Poco::Path> getRealDatasets() {
  auto ds = PathBuilder::makeDirectory(Env::SOURCE_DIR).pushDirectory("datasets");
  auto irene = ds.pushDirectory("Irene");
  return Array<Poco::Path>{
    ds.pushDirectory("exocet").get(),
    ds.pushDirectory("psaros33_Banque_Sturdza").get(),

  };
}

} /* namespace mmm */
