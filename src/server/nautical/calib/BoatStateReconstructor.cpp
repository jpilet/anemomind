/*
 * BoatStateReconstructor.cpp
 *
 *  Created on: 29 Sep 2016
 *      Author: jonas
 */

#include <server/nautical/calib/BoatStateReconstructor.h>
#include <server/nautical/calib/Fitness.h>
#include <ceres/ceres.h>
#include <random>
#include <server/common/string.h>

namespace sail {


template <typename T>
void foreachSpan(const TimeStampToIndexMapper &mapper,
    const Array<TimedValue<T> > &values,
    std::function<void(int, Spani)> cb) {
  assert(std::is_sorted(values.begin(), values.end()));
  int currentPosition = 0;
  while (currentPosition < values.size()) {
    int currentIndex = mapper.map(values[currentPosition].time);
    int nextPosition = currentPosition + 1;
    while (nextPosition < values.size()) {
      int nextIndex = mapper.map(values[nextPosition].time);
      if (nextIndex != currentIndex) {
        break;
      }
      nextPosition++;
    }
    if (currentIndex != -1) {
      cb(currentIndex, Spani(currentPosition, nextPosition));
    }
    currentPosition = nextPosition;
  }
}

// Forced instantiation for unit tests.
template void foreachSpan<Velocity<double>>(
    const TimeStampToIndexMapper &mapper,
    const Array<TimedValue<Velocity<double>> > &values,
    std::function<void(int, Spani)> cb);

template <typename T>
ValueAccumulator<T> makeValueAccumulator(
    const BoatParameterLayout::IndexAndOffsetMap &sensorIndices,
  const TimeStampToIndexMapper &mapper,
  const std::map<std::string, Array<TimedValue<T> > > &srcData) {
  typedef ValueAccumulator<T> DstType;
  DstType dst;
  int sampleCounter = 0;
  for (auto kv: srcData) {
    foreachSpan<T>(mapper, kv.second, [&](int sampleIndex, Spani span) {
      sampleCounter += span.width();
    });
  }
  dst.values.reserve(sampleCounter);

  for (auto kv: srcData) {
    auto f = sensorIndices.find(kv.first);
    assert(f != sensorIndices.end());
    int sensorIndex = f->second.sensorIndex;
    foreachSpan<T>(mapper, kv.second, [&](int sampleIndex, Spani span) {
      for (auto i: span) {
        auto tagged = typename DstType::TaggedValue{
          sensorIndex, sampleIndex, kv.second[i].value};
        dst.values.push_back(tagged);
      }
    });
  }
  std::unordered_map<int, Spani> tmp;
  assert(dst.values.size() == sampleCounter);
  std::sort(dst.values.begin(), dst.values.end());
  for (int i = 0; i < dst.values.size(); i++) {
    int index = dst.values[i].sampleIndex;
    auto f = tmp.find(index);
    if (f == tmp.end()) {
      tmp.insert(std::pair<int, Spani>(index, Spani(i, i+1)));
    } else {
      f->second.extend(i);
      f->second.extend(i+1);
    }
  }
  for (auto kv: tmp) {
    dst.valuesPerIndex.push_back(kv);
  }
  std::sort(dst.valuesPerIndex.begin(), dst.valuesPerIndex.end(),
      [](const std::pair<int, Spani> &a,
          const std::pair<int, Spani> &b) {
    return a.first < b.first;
  });
  return dst;
}

// Forced instantiations for unit tests.
template ValueAccumulator<Angle<double>> makeValueAccumulator<Angle<double>>(
    const BoatParameterLayout::IndexAndOffsetMap &layout, const TimeStampToIndexMapper &mapper,
  const std::map<std::string, Array<TimedValue<Angle<double>> > > &srcData);
template ValueAccumulator<Velocity<double>> makeValueAccumulator<Velocity<double>>(
    const BoatParameterLayout::IndexAndOffsetMap &layout, const TimeStampToIndexMapper &mapper,
  const std::map<std::string, Array<TimedValue<Velocity<double>> > > &srcData);

template <typename T>
void evaluateMotionReg(
    const HorizontalMotion<T> &dif,
    T weight, Velocity<T> bw,
    T *dst) {
  dst[0] = weight*(dif[0]/bw);
  dst[1] = weight*(dif[1]/bw);
}

template <typename Settings>
struct RegCost {
  double *globalScale;
  double regWeight;

  RegCost(double *gs, double w) : globalScale(gs), regWeight(w) {}

  template <typename T>
  bool operator()(const T *X, const T *Y, T *dst) {
    ReconstructedBoatState<T, Settings> A, B;
    A.readFrom(X);
    B.readFrom(Y);
    T weight = MakeConstant<T>::apply(regWeight);

    auto vbw = BandWidthForType<T, Velocity<double>>::get();
    auto abw = BandWidthForType<T, Angle<double>>::get();

    evaluateMotionReg<T>(
        A.boatOverGround - B.boatOverGround,
        weight, vbw, dst + 0);
    evaluateMotionReg<T>(
        A.windOverGround - B.windOverGround,
        weight, vbw, dst + 2);
    evaluateMotionReg<T>(
        A.currentOverGround - B.currentOverGround,
        weight, vbw, dst + 4);
    dst[6] = weight*((A.heel - B.heel)/abw);
    dst[7] = weight*((A.pitch - B.pitch)/abw);
    dst[8] = weight*((A.heading - B.heading)/abw);
    static_assert(9 == A.valueDimension, "Bad dim");
    return true;
  }
};

template <typename Settings>
struct DataCost;

template <typename BoatStateSettings>
class BoatStateReconstructor {
public:
  static_assert(AreFitnessSettings<BoatStateSettings>::value,
      "The parameter you passed are not valid settings");

  template <typename T>
  using State = ReconstructedBoatState<T, BoatStateSettings>;

  //static const int dynamicStateDim = State<double>::valueDimension;

  BoatStateReconstructor(
      const Array<CalibDataChunk> &chunk,
      const ReconstructionSettings &settings,
      const HtmlNode::Ptr &logNode) : _log(logNode)
/* define INITIALIZE_VALUE_ACC(HANDLE) \
  HANDLE(makeValueAccumulators<HANDLE>(chunks)),
FOREACH_MEASURE_TO_CONSIDER(INITIALIZE_VALUE_ACC)
undef INITIALIZE_VALUE_ACC*/
  {}

  Array<BoatState<double> > reconstruct(
      const BoatParameters<double> &boatParams) const {
    double globalScale = 1.0;
    /*int n = _initialStates.size();
    ceres::Problem problem;

    Array<double> reconstructedStates(
        n*dynamicStateDim);
    std::default_random_engine rng;
    std::uniform_real_distribution<double> hdgDistrib(0.0, 2.0*M_PI);
    for (int i = 0; i < n; i++) {
      State<double> state;
      state.heading.value = HorizontalMotion<double>::polar(
          0.0001_mps, hdgDistrib(rng)*1.0_rad);

      double *p = reconstructedStates.blockPtr(
          i, dynamicStateDim);
      problem.AddParameterBlock(p, dynamicStateDim);

      state.write(&p);
    }

    Array<double> reconstructedBoatParameters(boatParams.paramCount());
    boatParams.writeTo(reconstructedBoatParameters.ptr());
    problem.AddParameterBlock(
        reconstructedBoatParameters.ptr(),
        boatParams.paramCount());

    { // Regularization
      int regCount = n-1;
      for (int i = 0; i < regCount; i++) {
        typedef RegCost<BoatStateSettings> RegFunctor;
        problem.AddResidualBlock(
            new ceres::AutoDiffCostFunction<RegFunctor,
              dynamicStateDim, dynamicStateDim, dynamicStateDim>(
                  new RegFunctor(&globalScale, 1.0)),
            nullptr,
            reconstructedStates.blockPtr(i, dynamicStateDim),
            reconstructedStates.blockPtr(i+1, dynamicStateDim));
      }
    }

    { // Data
      int dataCount = n;
      for (int i = 0; i < dataCount; i++) {
        typedef DataCost<BoatStateSettings> DataFunctor;
        auto f = new DataFunctor();
        auto cost = new ceres::DynamicAutoDiffCostFunction<DataFunctor>(f);
        cost->AddParameterBlock(dynamicStateDim);
        cost->AddParameterBlock(boatParams.paramCount());
        cost-SetNumResiduals(f->outputCount());
        std::vector<double*> params{
          reconstructedStates.blockPtr(i, dynamicStateDim),
          reconstructedBoatParameters.ptr()
        };
        problem.AddResidualBlock(cost, nullptr, params);
      }
    }
*/

    return Array<BoatState<double>>();
  }

#define DECLARE_VALUE_ACC(HANDLE) \
  Array<ValueAccumulator<typename TypeForCode<HANDLE>::type>> HANDLE;
  FOREACH_MEASURE_TO_CONSIDER(DECLARE_VALUE_ACC)
#undef DECLARE_VALUE_ACC

  HtmlNode::Ptr _log;
  Array<CalibDataChunk> _chunks;
};

template <typename Settings>
struct DataCost {
  BoatStateReconstructor<Settings> *rec;

  DataCost(BoatStateReconstructor<Settings> *p) :
    rec(p) {}

  template <typename T>
  bool operator()(T const* const* parameters,
      T *residuals) const {
    ReconstructedBoatState<T, Settings> state;
    state.readFrom(parameters[0]);

    BoatParameters<T> boatParams;
    boatParams.readFrom(parameters[1]);

    return true;
  }
};

namespace {

  template <FunctionCode fcode, DataCode code>
  void initializeFields(const CalibDataChunk &src,
      SensorFunctionSet<fcode, double> *dst) {
    typedef typename
        SensorFunctionType<fcode, double, code>::type FType;
    const auto &srcMap =
        *ChannelFieldAccess<code>::template get<CalibDataChunk>(src);
    auto &dstMap =
        *ChannelFieldAccess<code>
          ::template get<SensorFunctionSet<fcode, double> >(*dst);
    for (const auto &kv: srcMap) {
      dstMap[kv.first] = FType();
    }
  }

  template <FunctionCode code>
  void initializeSensorSetFromChunk(
      const CalibDataChunk &chunk,
      SensorFunctionSet<code, double> *dst) {
    initializeFields<code, AWA>(chunk, dst);
    initializeFields<code, AWS>(chunk, dst);
    initializeFields<code, MAG_HEADING>(chunk, dst);
    initializeFields<code, WAT_SPEED>(chunk, dst);
  }

  template <FunctionCode code>
  SensorFunctionSet<code, double> initializeSensorSet(
      const Array<CalibDataChunk> &chunks) {
    SensorFunctionSet<code, double> dst;
    for (auto chunk: chunks) {
      initializeSensorSetFromChunk<code>(chunk, &dst);
    }
    return dst;
  }
}

namespace {
  struct CalibChunkSampleVisitor {
    std::map<DataCode, int> counts;
    template <DataCode code, typename T, typename Data>
    void visit(const Data &d) {
      int n = 0;
      for (auto kv: d) {
        n += kv.second.size();
      }
      counts[code] = n;
    }
  };

  void outputChunkOverview(
      const Array<CalibDataChunk> &chunks,
      HtmlNode::Ptr log) {
    std::map<std::pair<int, DataCode>, int> countMap;
    for (int i = 0; i < chunks.size(); i++) {
      CalibChunkSampleVisitor visitor;
      visitFieldsConst(
          chunks[i], &visitor);
      for (auto kv: visitor.counts) {
        countMap.insert({{i, kv.first}, kv.second});
      }
    }

    auto codes = getAllDataCodes();
    auto h1 = SubTable::header(1, 1,
        SubTable::constant("Index"));
    auto chunkIndices = SubTable::header(chunks.size(), 1,
        [](HtmlNode::Ptr dst, int i, int j) {dst->stream() << i+1;});
    auto leftCol = vcat(h1, chunkIndices);
    auto channelHeaders = SubTable::header(1, codes.size(),
        [&](HtmlNode::Ptr dst, int i, int j) {
      dst->stream() << wordIdentifierForCode(codes[j]);
    });
    auto sampleCounts = SubTable::cell(chunks.size(), codes.size(),
        [&](HtmlNode::Ptr dst, int i, int j) {
      auto f = countMap.find({i, codes[j]});
      if (f != countMap.end()) {
        dst->stream() << f->second;
      }
    });
    auto miscHeaders = SubTable::header(1, 3,
        [&](HtmlNode::Ptr dst,
        int i, int j) {
      const char *h[3] = {"Reconstruction sample count",
          "Sampling period", "Duration"};
      dst->stream() << h[j];
    });
    auto misc = SubTable::cell(chunks.size(), 3,
        [&](HtmlNode::Ptr dst, int i, int j) {
      auto x = chunks[i].timeMapper;
      switch (j) {
      case 0:
        dst->stream() << x.sampleCount;
        break;
      case 1:
        dst->stream() << x.period.seconds() << " s";
        break;
      case 2:
        dst->stream() << (double(x.sampleCount-1)*x.period).str();
        break;
      default:
        break;
      };
    });

    auto all = hcat(
        hcat(leftCol, vcat(channelHeaders, sampleCounts)),
        vcat(miscHeaders, misc));
    renderTable(log, all);
  }

  struct InitialStateCharacteristics {
    bool allMotionsDefined = true;
    bool allPositionsDefined = true;
    Velocity<double> maxGpsSpeed = 0.0_kn;
  };

  InitialStateCharacteristics analyzeInitialStates(
      const Array<BoatState<double>> &src) {
    InitialStateCharacteristics c;
    for (auto x: src) {
      auto goodMotion = isFinite(x.boatOverGround());
      c.allMotionsDefined &= goodMotion;
      c.allPositionsDefined &= isFinite(x.position());
      if (goodMotion) {
        c.maxGpsSpeed = std::max(
            c.maxGpsSpeed, x.boatOverGround().norm());
      }
    }
    return c;
  }

  void outputInitialStateOverview(
      const Array<CalibDataChunk> &chunks,
      HtmlNode::Ptr logNode) {
    std::vector<InitialStateCharacteristics> ch;
    for (auto chunk: chunks) {
      ch.push_back(analyzeInitialStates(chunk.initialStates));
    }
    const char *headerStrings[4] = {
        "Index", "Motions defined", "Positions defined", "Max gps speed"
    };
    auto h = SubTable::header(1, 4,
        [&](HtmlNode::Ptr dst, int i, int j) {
      dst->stream() << headerStrings[j];
    });
    auto inds = SubTable::header(chunks.size(), 1,
        [&](HtmlNode::Ptr dst, int i, int j) {
      dst->stream() << i+1;
    });
    auto yesOrNo = [](HtmlNode::Ptr dst, bool v) {
      if (v) {
        HtmlTag::tagWithData(dst, "p", {{"class", "success"}}, "Yes");
      } else {
        HtmlTag::tagWithData(dst, "p", {{"class", "warning"}}, "No");
      }
    };
    auto motionsDefined = SubTable::cell(chunks.size(), 1,
            [&](HtmlNode::Ptr dst, int i, int j) {
          yesOrNo(dst, ch[i].allMotionsDefined);
        });
    auto positionsDefined = SubTable::cell(chunks.size(), 1,
            [&](HtmlNode::Ptr dst, int i, int j) {
          yesOrNo(dst, ch[i].allPositionsDefined);
        });
    auto speeds = SubTable::cell(chunks.size(), 1,
        [&](HtmlNode::Ptr dst, int i, int j) {
      auto vel = ch[i].maxGpsSpeed.knots();
      dst->stream() << vel << " knots";
    });
    auto data = hcat(motionsDefined, hcat(positionsDefined, speeds));
    renderTable(logNode, vcat(h, hcat(inds, data)));
  }

  template <DataCode code>
  void registerSensors(const CalibDataChunk &chunk,
      BoatParameters<double> *dst) {
    const auto &src0 = ChannelFieldAccess<code>::get(chunk);
    for (const auto &kv: *src0) {
      auto &dst0 = *(ChannelFieldAccess<code>::get(dst->sensors));
      dst0[kv.first] = DistortionModel<double, code>();
    }
  }

  template <DataCode code>
  void outputChannelTableRow(
      const std::map<std::string, DistortionModel<double, code>> &map,
      HtmlNode::Ptr dst) {
    int n = map.size();
    if (0 < n) {
      auto row = HtmlTag::make(dst, "tr");
      HtmlTag::tagWithData(row, "td", wordIdentifierForCode(code));
      auto sensorData = HtmlTag::make(row, "td");
      {
        HtmlTag::tagWithData(sensorData, "p",
                stringFormat("%d sensors", n));
        auto sensorTable = HtmlTag::make(sensorData, "table");
        for (auto kv: map) {
          auto row = HtmlTag::make(sensorTable, "tr");
          HtmlTag::tagWithData(row, "td", kv.first);
          auto p = HtmlTag::make(row, "td");
          kv.second.outputSummary(&(p->stream()));
        }
      }
    }
  }

  void outputBoatParameters(
      const BoatParameters<double> &params,
      HtmlNode::Ptr dst) {
    auto table = HtmlTag::make(dst, "table");
    {
      auto row = HtmlTag::make(table, "tr");
      HtmlTag::tagWithData(row, "th", "Type");
      HtmlTag::tagWithData(row, "th", "Data");
    }{
      auto row = HtmlTag::make(table, "tr");
      HtmlTag::tagWithData(row, "td", "Heel constant");
      HtmlTag::tagWithData(row, "td",
          stringFormat("%.3g",
              params.heelConstant/params.heelConstantUnit()));
    }{
      auto row = HtmlTag::make(table, "tr");
      HtmlTag::tagWithData(row, "td", "Leeway constant");
      HtmlTag::tagWithData(row, "td",
          stringFormat("%.3g",
              params.leewayConstant/params.leewayConstantUnit()));
    }
#define OUTPUT_CHANNEL_TABLE_ROW(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  outputChannelTableRow<HANDLE>(*(ChannelFieldAccess<HANDLE>::get(params.sensors)), table);
FOREACH_CHANNEL(OUTPUT_CHANNEL_TABLE_ROW)
#undef OUTPUT_CHANNEL_TABLE_ROW
  }
}

BoatParameters<double> initializeBoatParameters(
    const Array<CalibDataChunk> &chunks) {
  BoatParameters<double> dst;
  for (auto chunk: chunks) {
#define REGISTER_SENSORS(CODE) \
    registerSensors<CODE>(chunk, &dst);
FOREACH_MEASURE_TO_CONSIDER(REGISTER_SENSORS)
#undef REGISTER_SENSORS
  }
  return dst;
}

namespace {
  template <DataCode code>
  int layoutSensors(int offset,
      const std::map<std::string, DistortionModel<double, code>> &params,
      BoatParameterLayout::PerType *dst) {
    int offset0 = offset;
    dst->offset = offset;
    int counter = 0;
    for (auto kv: params) {
      dst->sensors[kv.first] =
          BoatParameterLayout::IndexAndOffset{counter, offset};
      offset += DistortionModel<double, code>::paramCount;
      counter++;
    }
    dst->paramCount = offset - offset0;
    assert(counter == params.size());
    return offset;
  }

  void outputSensorOffsetData(DataCode code,
      BoatParameterLayout::PerType data,
      HtmlNode::Ptr dst0) {
    if (!data.sensors.empty()) {
      auto row0 = HtmlTag::make(dst0, "tr");
      HtmlTag::tagWithData(row0, "td", wordIdentifierForCode(code));
      auto dst = HtmlTag::make(row0, "td");

      {
        auto overview = HtmlTag::make(dst, "table");
        {
          auto row = HtmlTag::make(overview, "tr");
          HtmlTag::tagWithData(row, "td", "offset");
          HtmlTag::tagWithData(row, "td",
              stringFormat("%d", data.offset));
        }{
          auto row = HtmlTag::make(overview, "tr");
          HtmlTag::tagWithData(row, "td", "param count");
          HtmlTag::tagWithData(row, "td",
              stringFormat("%d", data.paramCount));
        }
      }{
        auto perSensor = HtmlTag::make(dst, "table");
        {
          auto row = HtmlTag::make(perSensor, "tr");
          HtmlTag::tagWithData(row, "th", "Source");
          HtmlTag::tagWithData(row, "th", "Offset");
          HtmlTag::tagWithData(row, "th", "Index");
        }
        for (auto kv: data.sensors) {
          auto row = HtmlTag::make(perSensor, "tr");
          HtmlTag::tagWithData(row, "td", kv.first);
          HtmlTag::tagWithData(row, "td",
              stringFormat("%d", kv.second.sensorOffset));
          HtmlTag::tagWithData(row, "td",
              stringFormat("%d", kv.second.sensorIndex));
        }
      }
    }
  }

  void outputParamLayout(const BoatParameterLayout &src,
      HtmlNode::Ptr dst) {
    {
      auto table = HtmlTag::make(dst, "table");
      {
        auto row = HtmlTag::make(table, "tr");
        HtmlTag::tagWithData(row, "th", "Type");
        HtmlTag::tagWithData(row, "th", "Data");
      }{
        auto row = HtmlTag::make(table, "tr");
        HtmlTag::tagWithData(row, "td", "Heel offset");
        HtmlTag::tagWithData(row, "td",
            stringFormat("%d", src.heelConstantOffset));
      }{
        auto row = HtmlTag::make(table, "tr");
        HtmlTag::tagWithData(row, "td", "Leeway offset");
        HtmlTag::tagWithData(row, "td",
            stringFormat("%d", src.leewayConstantOffset));
      }
  #define OUTPUT_SENSOR_OFFSET(HANDLE, INDEX, SHORTNAME, TYPE, DESCRIPTION) \
      outputSensorOffsetData(DataCode::HANDLE, src.HANDLE, dst);
  FOREACH_CHANNEL(OUTPUT_SENSOR_OFFSET)
  #undef OUTPUT_SENSOR_OFFSET
    }
    HtmlTag::tagWithData(dst, "p",
        stringFormat("Total parameter count: %d", src.paramCount));
  }
}

BoatParameterLayout::BoatParameterLayout(
    const BoatParameters<double> &parameters) {
  int offset = 2; // after leeway and heel.
#define LAYOUT_SENSORS(HANDLE, INDEX, SHORTNAME, TYPE, DESCRIPTION) \
  offset = layoutSensors<DataCode::HANDLE>(offset, parameters.sensors.HANDLE, &HANDLE);
FOREACH_CHANNEL(LAYOUT_SENSORS)
#undef LAYOUT_SENSORS
  paramCount = offset;
  assert(paramCount == parameters.paramCount());
}

ReconstructionResults reconstruct(
    const Array<CalibDataChunk> &chunks,
    const ReconstructionSettings &settings,
    HtmlNode::Ptr logNode) {
  HtmlTag::tagWithData(logNode, "h1",
      stringFormat("Reconstruction results for %d chunks",
          chunks.size()));
  if (logNode) {
    HtmlTag::tagWithData(logNode, "h2", "Input data");
    HtmlTag::tagWithData(logNode, "h3", "Chunks");
    outputChunkOverview(chunks, logNode);
    HtmlTag::tagWithData(logNode, "h3", "Initial states");
    outputInitialStateOverview(chunks, logNode);
  }

  auto initialParameters = initializeBoatParameters(chunks);
  BoatParameterLayout layout(initialParameters);
  if (logNode) {
    HtmlTag::tagWithData(logNode, "h2", "Initial parameters");
    outputBoatParameters(initialParameters, logNode);
    HtmlTag::tagWithData(logNode, "h2", "Parameter layout");
    outputParamLayout(layout, logNode);
  }

  BoatStateReconstructor<FullSettings> reconstructor(
      chunks, settings, logNode);
  return ReconstructionResults();
}





}


