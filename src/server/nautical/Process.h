/*
 * Process.h
 *
 *  Created on: 28 Jul 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_PROCESS_H_
#define SERVER_COMMON_PROCESS_H_

#include <server/nautical/logimport/LogLoader.h>
#include <server/common/Span.h>
#include <server/common/DOMUtils.h>
#include <server/math/QuadForm.h>
#include <array>
#include <server/common/DynamicInterface.h>
#include "../math/SegmentOutlierFilter.h"

namespace sail {

SerializationInfo toDynamicObject(
    const DataCode& x, Poco::Dynamic::Var* dst);
SerializationInfo fromDynamicObject(
    const Poco::Dynamic::Var& src, DataCode *x);

struct ProcessSettings {
  bool outputHtml = true;
  std::string resultsDir = "processed";
  double downsampleMinPeriodSeconds = 0.1;
  std::map<std::string, std::set<DataCode>> samplesToRemove;
  double timeGapMinutes = 60.0;
  sof::Settings gpsOutlierSettings;

  template <typename V>
  void visitFields(V* v) {
    v->visit("output_html", outputHtml);
    v->visit("results_dir", resultsDir);
    v->visit("samples_to_remove", samplesToRemove);
    v->visit("downsample_minperiod_seconds",
        downsampleMinPeriodSeconds);
    v->visit("time_gap_minutes", timeGapMinutes);
    v->visit("gps_outlier_settings", gpsOutlierSettings);
  }
};



std::vector<LogFileInfo> adjustBoundaries(
    const std::vector<LogFileInfo>& src);

int countLogs(const std::vector<LogFileInfo>& logs);

std::vector<Span<TimeStamp>> presegmentData(
    const std::vector<LogFileInfo>& logFiles,
    const ProcessSettings& settings,
    DOM::Node* output);

} /* namespace sail */

#endif /* SERVER_COMMON_PROCESS_H_ */
