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
  const TimeStampToIndexMapper &mapper,
  const std::map<std::string, Array<TimedValue<T> > > &srcData) {
  typedef ValueAccumulator<T> DstType;
  DstType dst;
  int sensorCounter = 0;
  int sampleCounter = 0;
  for (auto kv: srcData) {
    dst.sensorIndices[kv.first] = sensorCounter++;
    foreachSpan<T>(mapper, kv.second, [&](int sampleIndex, Spani span) {
      sampleCounter += span.width();
    });
  }
  dst.values.reserve(sampleCounter);

  assert(sensorCounter == dst.sensorIndices.size());
  for (auto kv: srcData) {
    assert(dst.sensorIndices.count(kv.first) == 1);
    int sensorIndex = dst.sensorIndices[kv.first];
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
  const TimeStampToIndexMapper &mapper,
  const std::map<std::string, Array<TimedValue<Angle<double>> > > &srcData);
template ValueAccumulator<Velocity<double>> makeValueAccumulator<Velocity<double>>(
  const TimeStampToIndexMapper &mapper,
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
  template <typename T>
  using State = ReconstructedBoatState<T, BoatStateSettings>;

  static const int dynamicStateDim = State<double>::valueDimension;

  BoatStateReconstructor(
      const Array<BoatState<double> > &initialStates,
      const TimeStampToIndexMapper &mapper,
      const CalibDataChunk &chunk) :
#define INITIALIZE_VALUE_ACC(HANDLE) \
  HANDLE(makeValueAccumulator(mapper, chunk.HANDLE)),
FOREACH_MEASURE_TO_CONSIDER(INITIALIZE_VALUE_ACC)
#undef INITIALIZE_VALUE_ACC
        _timeMapper(mapper),
        _initialStates(initialStates) {
    CHECK(initialStates.size() == mapper.sampleCount);
  }

  Array<BoatState<double> > reconstruct(
      const BoatParameters<double> &boatParams) const {
    double globalScale = 1.0;
    int n = _initialStates.size();
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


    return Array<BoatState<double>>();
  }

#define DECLARE_VALUE_ACC(HANDLE) \
  ValueAccumulator<typename TypeForCode<HANDLE>::type> HANDLE;
  FOREACH_MEASURE_TO_CONSIDER(DECLARE_VALUE_ACC)
#undef DECLARE_VALUE_ACC
private:
  TimeStampToIndexMapper _timeMapper;
  Array<BoatState<double> > _initialStates;
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

ReconstructionResults resconstruct(
    const Array<CalibDataChunk> &chunks,
    const ReconstructionSettings &settings,
    HtmlNode::Ptr logNode) {
  return ReconstructionResults();
}





}


