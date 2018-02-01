#include <device/anemobox/Nmea2000Source.h>
#include <device/anemobox/Nmea2000Utils.h>
#include <gtest/gtest.h>
#include <queue>
#include <server/common/logging.h>
#include <fstream>
#include <server/common/Env.h>

using namespace sail;

struct CanFrame {
  unsigned long id = 0;
  unsigned char len = 0;
  unsigned char buf[8];
};

// hex#hex
CanFrame canFrameFromDumpString(const std::string& ds) {
  auto at = ds.find('#');
  if (at < ds.length()) {
    CanFrame dst;
    {
      std::stringstream ss;
      ss << std::hex << ds.substr(0, at);
      ss >> dst.id;
    }{
      auto from = at+1;
      auto rest = ds.substr(from);
      if (rest.length() % 2 == 1) {
        LOG(ERROR) << "Should be whole bytes";
        return CanFrame();
      }
      dst.len = rest.length()/2;
      for (int i = 0; i < dst.len; i++) {
        auto part = rest.substr(2*i, 2);
        dst.buf[i] = strtol(part.c_str(), nullptr, 16);
      }
    }
    return dst;
  } else {
    LOG(ERROR) << "Failed to parse CanFrame";
    return CanFrame();
  }
}



class NMEA2000ForTesting : public tNMEA2000 {
public:

  bool isFastPacket(const tN2kMsg& msg) {
    return IsFastPacket(msg);
  }

  bool CANSendFrame(
      unsigned long id, unsigned char len, const unsigned char *buf, bool wait_sent=true) {
    std::cout << "Trying to send frame" << std::endl;
    return false;
  }

  virtual bool CANOpen() {
    return true;
  }

  std::queue<CanFrame> framesToGet;

  virtual bool CANGetFrame(
      unsigned long &id, unsigned char &len, unsigned char *buf) {
    if (framesToGet.empty()) {
      std::cout << "No more frames to get" << std::endl;
      return false;
    }
    const auto& x = framesToGet.front();
    id = x.id;
    len = x.len;
    std::copy(x.buf, x.buf + x.len, buf);
    framesToGet.pop();
    return true;
  }

  void parseCandump(std::istream* src) {
    int count = 0;
    while (src->good()) {
      std::string s0;
      std::getline(*src, s0);
      auto ss = split(s0, ' ');
      if (!ss.empty()) {
        std::string s = ss.last();
        CanFrame frame = canFrameFromDumpString(s);
        if (frame.len == 0) {
          LOG(INFO) << "End of frame data";
          break;
        } else {
          framesToGet.push(frame);
          count++;
        }
      }
    }
    LOG(INFO) << "Processed " << count << " lines";
  }

  virtual ~NMEA2000ForTesting() {}
};

class TestHandler : public tNMEA2000::tMsgHandler {
public:
  std::vector<PgnClasses::GnssPositionData> data;

  TestHandler(tNMEA2000* s) : tNMEA2000::tMsgHandler(129029, s) {}

  void HandleMsg(const tN2kMsg &msg) {
    CHECK(msg.PGN == 129029);
    data.push_back(
        PgnClasses::GnssPositionData(
            msg.Data, msg.DataLen));
  }
};


TEST(Nmea2000SourceTest, DeviceNameTest) {
  NMEA2000ForTesting testInstance;
  Nmea2000Source src(&testInstance, nullptr);

  std::string srcName = "8078AC73008C28C0";

  testInstance.framesToGet.push(
      canFrameFromDumpString(
          "18EEFF80#" + srcName));

  testInstance.ParseMessages();

  auto formattedName = deviceNameToString(src.getSourceName(128).get());

  std::string prefix = "NMEA2000/";
  EXPECT_EQ(prefix, formattedName.substr(0, prefix.length()));

  auto rest = formattedName.substr(prefix.length());

  EXPECT_EQ(rest.length(), srcName.length());
  int n = rest.length()/2;

  // The bytes are reversed w.r.t. each other, but maybe that is
  // not a problem?
  for (int i = 0; i < n; i++) {
    int j = n-i-1;
    auto a = srcName.substr(2*i, 2);
    auto b = rest.substr(2*j, 2);
    EXPECT_EQ(std::tolower(a[0]), b[0]);
    EXPECT_EQ(std::tolower(a[1]), b[1]);
  }
}


TEST(Nmea2000SourceTest, GnssPositionData) {

  // Here we are testing with a fast packet (129029), so
  // we certainly need the tNMEA2000 instance in order to
  // form one big tN2kMsg.
  NMEA2000ForTesting testInstance;
  {
    // sed -n '300,600p' youtoo-n2k-log > ~/prog/
    //     anemomind/datasets/candump_youtoo.txt

    std::ifstream ss(
        std::string(sail::Env::SOURCE_DIR)
      + "/datasets/candump_youtoo.txt");

    testInstance.parseCandump(&ss);
  }



  Dispatcher dispatcher;
  Nmea2000Source source(
      &testInstance,
      &dispatcher);
  TestHandler handler(&testInstance);

  testInstance.AttachMsgHandler(&handler);
  while (!testInstance.framesToGet.empty()) {
    testInstance.ParseMessages();
  }

  EXPECT_EQ(2, handler.data.size());

  PgnClasses::GnssPositionData x = handler.data[1];

  // Obtained by calling 'canplayer' on the cat of 'candump_youtoo.txt'
  // Then watching the Actisense software.
  EXPECT_TRUE(x.sid.defined());
  EXPECT_EQ(x.sid.get(), 77);
  EXPECT_EQ(x.timeStamp().toIso8601String(), "2016-09-19T08:48:57Z");
  EXPECT_NEAR(41.34, x.latitude.get().degrees(), 0.2);
  EXPECT_NEAR(2.19, x.longitude.get().degrees(), 0.2);
  EXPECT_NEAR(52.9, x.altitude.get().meters(), 0.2);
  EXPECT_EQ(x.method.get(),
      PgnClasses::GnssPositionData::Method::GNSS_fix);
  EXPECT_EQ(x.integrity.get(),
      PgnClasses::GnssPositionData::Integrity::No_integrity_checking);
  EXPECT_EQ(x.numberOfSvs.get(), 9);
  EXPECT_NEAR(x.geoidalSeparation.get().meters(), 51.28, 0.1);
  EXPECT_EQ(0, x.referenceStations.get());


  EXPECT_TRUE(dispatcher.get<GPS_POS>()->dispatcher()->hasValue());
  auto v = dispatcher.val<GPS_POS>();
  EXPECT_NEAR(v.lat().degrees(), 41.34, 0.2);
  EXPECT_NEAR(v.lon().degrees(), 2.19, 0.2);

  EXPECT_TRUE(dispatcher.get<DATE_TIME>()->dispatcher()->hasValue());
  TimeStamp val = dispatcher.val<DATE_TIME>();
  EXPECT_EQ(val.toIso8601String().substr(0, 54-37),
        "2016-09-19T08:48:");
}


TEST(Nmea2000SourceTest, SystemTime) {
  Dispatcher dispatcher;
  Nmea2000Source source(nullptr, &dispatcher);
  
  // The following data gets parser by canboat analyser as:
  // System Time:  SID = 0; Source = GPS; Date = 2015.03.06; Time = 04:56:12
  //
  // Command line in canboat:
  // candump2analyzer  < samples/candumpSample2.txt | analyzer -raw
  const std::vector<uint8_t> data{
    0x00, 0xf0, 0x74, 0x40, 0xc0, 0xca, 0x97, 0x0a
  };

  N2kMsgBuilder builder;
  builder.PGN = 126992;
  builder.source = 82;
  builder.destination = 83;

  source.HandleMsg(builder.make(data));

  EXPECT_TRUE(dispatcher.get<DATE_TIME>()->dispatcher()->hasValue());
  TimeStamp val = dispatcher.val<DATE_TIME>();
  TimeStamp truth = TimeStamp::UTC(2015, 3, 6, 4, 56, 12);
  EXPECT_NEAR((truth - val).seconds(), 0, 1e-2);
}

TEST(Nmea2000SourceTest, WindData) {
  Dispatcher dispatcher;
  Nmea2000Source source(nullptr, &dispatcher);
  
  const std::vector<uint8_t> data{
    0x00, 0x34, 0x12, 0x50, 0x33, 0x03, 0x0, 0x0
  };

  N2kMsgBuilder builder;
  builder.source = 82;
  builder.destination = 83;
  builder.PGN = 130306;
  auto msg = builder.make(data);

  source.HandleMsg(msg);

  EXPECT_TRUE(dispatcher.get<TWA>()->dispatcher()->hasValue());
  EXPECT_NEAR(dispatcher.val<TWA>().degrees(), 75.3, .1);
}


TEST(Nmea2000SourceTest, RudderAngle) {
  Dispatcher dispatcher;
  Nmea2000Source source(nullptr, &dispatcher);
  
  const std::vector<uint8_t> data{
    0xfc, 0xf8, 0xff, 0x7f, 0x06, 0xfd, 0xff, 0xff
  };

  N2kMsgBuilder builder;
  builder.PGN = 127245;
  builder.source = 82;
  builder.destination = 83;
  auto msg = builder.make(data);

  source.HandleMsg(msg);

  EXPECT_TRUE(dispatcher.get<RUDDER_ANGLE>()->dispatcher()->hasValue());
  EXPECT_NEAR(dispatcher.val<RUDDER_ANGLE>().degrees(), -4.4, .1);
}

