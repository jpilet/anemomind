/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulationPrecomp.h>
#include <server/nautical/synthtest/NavalSimulationJson.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/common/JsonIO.h>
#include <Poco/File.h>
#include <server/common/logging.h>

namespace sail {

    NavalSimulation loadOrMake(std::function<NavalSimulation()> srcFunction, std::string filename) {

      auto dir = PathBuilder::makeDirectory(Env::SOURCE_DIR)
        .pushDirectory("datasets").pushDirectory("synth");
      {
        Poco::File dst(dir.get());

        // Ignore the return value. Just
        // make sure that the directory exists.
        dst.createDirectory();
      }

      auto p = dir.makeFile(filename).get();
      Poco::File file(p);
      if (file.exists()) {
        NavalSimulation dst;
        if (json::deserialize(json::load(p.toString()), &dst)) {
          return dst;
        }
        LOG(WARNING) << "Failed to deserialize data from " << p.toString();
      }
      LOG(INFO) << "No precomputed results available from " << p.toString() << ", generate...";
      auto results =  srcFunction();
      if (!results.hasBoatData()) {
        LOG(FATAL) << "No boat data was generated. This is probably a bug.";
      }

      LOG(INFO) << "Done generating them. Convert to json structure.";

      auto jsondata = json::serialize(results);
      if (jsondata.isEmpty()) {
        LOG(FATAL) << "Failed to serialize the data to a json datastructure.";
      }

      LOG(INFO) << "Save them to " << p.toString();
      json::save(p.toString(), jsondata);
      return results;
    }



NavalSimulation getNavSimFractalWindOriented() {
  return loadOrMake(&makeNavSimFractalWindOriented,
      "navSimFractalWindOriented.json");
}

} /* namespace mmm */
