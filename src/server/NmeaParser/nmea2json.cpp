#include <IreneTargetSpeed/IreneTargetSpeed.h>
#include <NmeaParser/NmeaParser.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>
#include <vector>

using std::cout;
using std::make_pair;
using std::string;
using std::vector;

namespace {

time_t getTime(const NmeaParser &p) {
  struct tm t;
  memset(&t, 0, sizeof(t));
  t.tm_year = 100 + p.year();
  t.tm_mon = p.month();
  t.tm_mday = p.day();
  t.tm_hour = p.hour();
  t.tm_min = p.min();
  t.tm_sec = p.sec();
  return mktime(&t);
}

template <typename T>
std::string format(T a) {
  std::stringstream r;
  r << a;
  return r.str();
}

void printRow(const GeoRef &ref, const NmeaParser &p) {
  std::vector<std::pair<string, string> > data;
  data.push_back(make_pair("time", format(getTime(p))));
  data.push_back(make_pair("gpsSpeed", p.gpsSpeedAsString()));
  data.push_back(make_pair("awa", p.awaAsString()));
  data.push_back(make_pair("aws", p.awsAsString()));
  data.push_back(make_pair("twa", p.twaAsString()));
  data.push_back(make_pair("tws", p.twsAsString()));
  data.push_back(make_pair("magHdg", p.magHdgAsString()));
  data.push_back(make_pair("watSpeed", p.watSpeedAsString()));
  data.push_back(make_pair("gpsBearing", p.gpsBearingAsString()));
  data.push_back(make_pair("lat", format(p.pos().lat.toDouble())));
  data.push_back(make_pair("lon", format(p.pos().lon.toDouble())));
  data.push_back(make_pair("cwd", p.cwdAsString()));
  data.push_back(make_pair("wd", p.wdAsString()));
  data.push_back(make_pair("speedRatio", format(getSpeedRatio(p.twa(), p.tws(), p.gpsSpeed()))));

  ProjectedPos pos(ref, p.pos());
  data.push_back(make_pair("x", format(pos.x())));
  data.push_back(make_pair("y", format(pos.y())));

  for (auto it = data.begin(); it != data.end(); ++it) {
    if (it == data.begin()) {
      cout << "{";
    } else {
      cout << ",";
    }
    cout << "\"" << it->first << "\":" << it->second;
  }
  cout << "}";
}

}  // namespace

int main(int argc, char *argv[]) {
  int c;
  FILE *f = stdin;

  NmeaParser np;

  np.setIgnoreWrongChecksum(true);

  if (argc>=2) f = fopen(argv[1],"r");

  if (!f) {
    perror(argv[1]);
    return -1;
  }

  printf("[\n");
  std::unique_ptr<GeoRef> ref;
  bool started = false;
  for(c=0; c!=EOF; c = fgetc(f)) {
    if (NmeaParser::isCycleMark(np.processByte(c))) {
      if (!ref) {
        ref.reset(new GeoRef(np.pos(), 0));
      }
      if (started) {
        printf(",");
      }
      started = true;
      printRow(*ref, np);
    }
  }
  printf("]\n");

  fprintf(stderr, "Summary:\nBytes: %ld\nErrors: %d\n Sentences: %d\n",
          np.numBytes(),
          np.numErr(),
          np.numSentences());
  return 0;
}


