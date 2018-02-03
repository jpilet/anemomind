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
    CanFrame f;
    f.id = id;
    f.len = len;
    std::copy(buf, buf + len, f.buf);
    framesToTransmit.push(f);
    return true;
  }

  virtual bool CANOpen() {
    return true;
  }

  std::queue<CanFrame> framesToReceive;
  std::queue<CanFrame> framesToTransmit;

  virtual bool CANGetFrame(
      unsigned long &id, unsigned char &len, unsigned char *buf) {
    if (framesToReceive.empty()) {
      std::cout << "No more frames to get" << std::endl;
      return false;
    }
    const auto& x = framesToReceive.front();
    id = x.id;
    len = x.len;
    std::copy(x.buf, x.buf + x.len, buf);
    framesToReceive.pop();
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
          framesToReceive.push(frame);
          count++;
        }
      }
    }
    LOG(INFO) << "Processed " << count << " lines";
  }

  virtual ~NMEA2000ForTesting() {}
};


template <typename PgnClass>
class TestHandler : public tNMEA2000::tMsgHandler {
public:
  std::vector<PgnClass> data;

  TestHandler(tNMEA2000* s) : tNMEA2000::tMsgHandler(
      PgnClass::ThisPgn, s) {}

  void HandleMsg(const tN2kMsg &msg) {
    CHECK(msg.PGN == PgnClass::ThisPgn);
    data.push_back(
        PgnClass(
            msg.Data, msg.DataLen));
  }
};


TEST(Nmea2000SourceTest, DeviceNameTest) {
  NMEA2000ForTesting testInstance;
  Nmea2000Source src(&testInstance, nullptr);

  std::string srcName = "8078AC73008C28C0";

  testInstance.framesToReceive.push(
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
  TestHandler<PgnClasses::GnssPositionData> handler(&testInstance);

  testInstance.AttachMsgHandler(&handler);
  while (!testInstance.framesToReceive.empty()) {
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

void prepareN2k(tNMEA2000* n2k) {
  n2k->Open(); // Seems like Open needs to be called early?

  // Code copy/pasted from
  // https://github.com/ttlappalainen/NMEA2000/blob/master/Examples/BatteryMonitor/BatteryMonitor.ino
  const tNMEA2000::tProductInformation BatteryMonitorProductInformation ={
                                         1300,                        // N2kVersion
                                         100,                         // Manufacturer's product code
                                         "Simple battery monitor",    // Manufacturer's Model ID
                                         "1.1.0.14 (2017-06-11)",     // Manufacturer's Software version code
                                         "1.1.0.0 (2017-06-11)",      // Manufacturer's Model version
                                         "00000001",                  // Manufacturer's Model serial code
                                         0,                           // SertificationLevel
                                         1                            // LoadEquivalency
                                        };

  // ---  Example of using PROGMEM to hold Configuration information.  However, doing this will prevent any updating of
  //      these details outside of recompiling the program.
  const char BatteryMonitorManufacturerInformation [] = "John Doe, john.doe@unknown.com";
  const char BatteryMonitorInstallationDescription1 [] = "Just for sample";
  const char BatteryMonitorInstallationDescription2 [] = "No real information send to bus";

  // Set Product information
  n2k->SetProductInformation(&BatteryMonitorProductInformation );
  // Set Configuration information
  n2k->SetProgmemConfigurationInformation(BatteryMonitorManufacturerInformation,BatteryMonitorInstallationDescription1,BatteryMonitorInstallationDescription2);
  // Set device information
  n2k->SetDeviceInformation(1,      // Unique number. Use e.g. Serial number.
                                  170,    // Device function=Battery. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                  35,     // Device class=Electrical Generation. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                  2046    // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
                                 );


  // Uncomment 3 rows below to see, what device will send to bus
  //Serial.begin(115200);
  //NMEA2000.SetForwardStream(&Serial);
  // NMEA2000.SetForwardType(tNMEA2000::fwdt_Text);     // Show in clear text. Leave uncommented for default Actisense format.
  // If you also want to see all traffic on the bus use N2km_ListenAndNode instead of N2km_NodeOnly below
  n2k->SetMode(tNMEA2000::N2km_NodeOnly,22);
  // NMEA2000.SetDebugMode(tNMEA2000::dm_ClearText);     // Uncomment this, so you can test code without CAN bus chips on Arduino Mega
  // NMEA2000.EnableForward(false);                      // Disable all msg forwarding to USB (=Serial)

  //  NMEA2000.SetN2kCANMsgBufSize(2);                    // For this simple example, limit buffer size to 2, since we are only sending data
}

TEST(Nmea2000SourceTest, SendTest) {
  NMEA2000ForTesting n2k;
  Dispatcher dispatcher;

  Nmea2000Source source(&n2k, &dispatcher);
  PgnClasses::PositionRapidUpdate msg;
  msg.latitude = 13.4_deg;
  msg.longitude = 51.9_deg;

  // No devices, so it should be impossible to send it.
  EXPECT_FALSE(source.send(0, msg));

  prepareN2k(&n2k);

  // Now the tNMEA2000 instance is both open and has
  // a device from which we can send.
  EXPECT_TRUE(n2k.framesToTransmit.empty());

  EXPECT_TRUE(source.send(0, msg));

  // n2k.ParseMessages(); // Doesn't seem to be necessary to call this

  EXPECT_FALSE(n2k.framesToTransmit.empty());

  // Parse the message that we just sent.
  TestHandler<PgnClasses::PositionRapidUpdate> handler(&n2k);
  n2k.framesToReceive.push(n2k.framesToTransmit.back());
  n2k.ParseMessages();

  EXPECT_FALSE(handler.data.empty());
  EXPECT_EQ(1, handler.data.size());
  auto pos = handler.data.back();
  EXPECT_NEAR(pos.latitude.get().degrees(), 13.4, 0.01);
  EXPECT_NEAR(pos.longitude.get().degrees(), 51.9, 0.01);
}

#define DBG(X) (([&]{auto x = X; std::cout << "///////////7Value of " #X " = " << x << std::endl; return x;})())

void testSuccessfullySentRapidPos(
    const std::map<std::string, TaggedValue>& input,
    bool expectedOutcome,
    Angle<double> expectedLon,
    Angle<double> expectedLat) {
  NMEA2000ForTesting n2k;
  Dispatcher dispatcher;

  Nmea2000Source source(&n2k, &dispatcher);

  n2k.Open();
  prepareN2k(&n2k);

  // Now the tNMEA2000 instance is both open and has
  // a device from which we can send.
  EXPECT_TRUE(n2k.framesToTransmit.empty());

  auto outcome = source.send({input});
  EXPECT_EQ(outcome.success, expectedOutcome);
  if (outcome.success) {
    n2k.ParseMessages(); // Doesn't seem to be necessary to call this

    EXPECT_FALSE(n2k.framesToTransmit.empty());

    // Parse the message that we just sent.
    TestHandler<PgnClasses::PositionRapidUpdate> handler(&n2k);
    n2k.framesToReceive.push(n2k.framesToTransmit.back());

    n2k.ParseMessages();

    EXPECT_FALSE(handler.data.empty());
    EXPECT_EQ(1, handler.data.size());
    auto pos = handler.data.back();
    EXPECT_NEAR(pos.latitude.get().degrees(),
        expectedLat.degrees(), 0.01);
    EXPECT_NEAR(pos.longitude.get().degrees(),
        expectedLon.degrees(), 0.01);
  }
}

TEST(Nmea2000SourceTest, SendTaggedValues) {
  testSuccessfullySentRapidPos({
     {"longitude", 9.3},
     {"latitude", 4.5}
  }, true, 9.3_deg, 4.5_deg);
}
