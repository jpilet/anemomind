/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 *
 *  A tool to easily look at data while developing.
 */

#include <server/common/ArgMap.h>
#include <server/common/logging.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/NavNmeaScan.h>
#include <iostream>
#include <server/common/Span.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/math/CleanNumArray.h>
#include <server/nautical/GeographicReference.h>

using namespace sail;

/*
 * Code taken from Plot.cpp. Don't know why we need it.
 */
Duration<double> getRawTime(const Nav &n) {
  return Duration<double>::seconds(double(n.time().toMilliSecondsSince1970()/int64_t(1000)));
}

Arrayd getSeconds(Array<Nav> navs) {
  return navs.map<double>([&](const Nav &x) {
    return getRawTime(x).seconds();
  });
}

namespace {

  typedef std::function<Arrayd(const Array<Nav> &x)> DataExtractor;

  DataExtractor
    makeAngleExtractor(std::function<Angle<double>(Nav)> f) {
    return [=](Array<Nav> navs) {
      return cleanContinuousAngles(navs.map<Angle<double> >(f)).map<double>(
      [](Angle<double> x) {
        return x.degrees();
      });
    };
  }

  DataExtractor
    makeSpeedExtractor(std::function<Velocity<double>(Nav)> f) {
    return [=](Array<Nav> navs) {
      return navs.map<double>([=](const Nav &x) {
        return f(x).knots();
      });
    };
  }

  auto awaExtractor = makeAngleExtractor([](const Nav &x) {return x.awa();});
  auto magHdgExtractor = makeAngleExtractor([](const Nav &x) {return x.magHdg();});
  auto gpsBearingExtractor = makeAngleExtractor([](const Nav &x) {return x.gpsBearing();});
  auto awsExtractor = makeSpeedExtractor([](const Nav &x) {return x.aws();});
  auto watSpeedExtractor = makeSpeedExtractor([](const Nav &x) {return x.watSpeed();});
  auto gpsSpeedExtractor = makeSpeedExtractor([](const Nav &x) {return x.gpsSpeed();});

  void setTimeFormatForAxis(GnuplotExtra &plot, int i) {
    const char axis[3] = {'x', 'y', 'z'};
    char sym = axis[i];
    plot.cmd(stringFormat("set %cdata time\n", sym));
    plot.cmd("set timefmt \"%s\"\n");
    plot.cmd(stringFormat("set format %c \"%s\"\n",
                          sym, "%m/%d/%Y %H:%M:%S"));
    plot.cmd("set xtics nomirror rotate by -45\n");
  }

  void plotData(std::string label, std::function<Arrayd(Array<Nav>)> extractor,
    Array<Nav> navs) {
    auto time = getSeconds(navs);
    auto values = extractor(navs);
    GnuplotExtra plot;
    setTimeFormatForAxis(plot, 0);
    plot.plot_xy(time, values);
    plot.set_xlabel("Time");
    plot.set_ylabel(label);
    plot.show();
  }
}




Array<Poco::Path> getPaths(ArgMap &amap) {
  return amap.optionArgs("--dir").map<Poco::Path>([=](ArgMap::Arg* arg) {
    return PathBuilder::makeDirectory(arg->value()).get();
  });
}

int selectFromAlternatives(Array<std::string> alternatives) {
  int n = alternatives.size();
  int x = 0;
  do {
    std::cout << "Select an alternative:\n";
    for (int i = 0; i < n; i++) {
      std::cout << "  " << i+1 << ") " << alternatives[i] << "\n";
    }
    std::cout << "Which? ";
    std::cin >> x;
  } while (!(1 <= x && x <= n));
  return x-1;
}

Duration<double> inputFreeDuration(std::string ask) {
  std::cout << ask << std::endl;
  Duration<double> units[4] = {
      Duration<double>::seconds(1.0),
      Duration<double>::minutes(1.0),
      Duration<double>::hours(1.0),
      Duration<double>::days(1.0)
  };
  auto unitLabels = Array<std::string>{"seconds", "minutes", "hours", "days"};
  int index = selectFromAlternatives(unitLabels);
  std::cout << "How many " << unitLabels[index] << "? ";
  double amount = 0;
  std::cin >> amount;
  return amount*units[index];
}

Duration<double> inputDuration(std::string ask) {
  std::cout << ask << std::endl;
  int choice = selectFromAlternatives(Array<std::string>{
    "1 minute",
    "1 hour",
    "1 day",
    "1 week",
    "1 month",
    "other duration"
  });
  Array<Duration<double> > durations{
    Duration<double>::minutes(1.0),
    Duration<double>::hours(1.0),
    Duration<double>::days(1.0),
    Duration<double>::weeks(1.0),
    Duration<double>::days(30),
  };
  if (choice < 5) {
    return durations[choice];
  }
  return inputFreeDuration(ask);
}

Array<Nav> slice(Array<Nav> navs, Spani span) {
  return navs.slice(span.minv(), span.maxv());
}


bool goodSpan(Spani x) {
  return x.minv() < x.maxv();
}


Array<Spani> getSplitSpans(Array<Nav> navs, Duration<double> dur) {
  std::vector<int> endPoints;
  endPoints.push_back(0);
  int n = navs.size();
  for (int i = 0; i < n-1; i++) {
    if (dur < navs[i+1].time() - navs[i].time()) {
      endPoints.push_back(i+1);
    }
  }
  endPoints.push_back(navs.size());
  int spanCount = endPoints.size() - 1;
  Array<Spani> spans(spanCount);
  for (int i = 0; i < spanCount; i++) {
    spans[i] = Spani(endPoints[i], endPoints[i+1]);
    CHECK(goodSpan(spans[i]));
  }
  return spans;
}


void dispSession(Array<Nav> navs) {
  auto from = navs.first().time();
  auto to = navs.last().time();
  CHECK(from <= to);
  std::cout << "  Navs from\n";
  std::cout << "    " << from << "\n";
  std::cout << "    to " << to << "\n";
  std::cout << "    with a total duration of " << (navs.last().time() - navs.first().time()).str() << "\n";
}


bool ordered(Spani a, Spani b) {
  return a.maxv() <= b.minv();
}

void dispChoice(int i, Array<Nav> navs, Spani span) {
  CHECK(std::is_sorted(navs.begin(), navs.end()));
  CHECK(goodSpan(span));
  CHECK(0 <= span.minv());
  CHECK(span.maxv() <= navs.size());
  std::cout << "* Choice " << i+1 << ":\n";
  dispSession(slice(navs, span));
  std::cout << "\n";
}

Spani selectSpan(Array<Nav> navs, Array<Spani> spans) {
  CHECK(std::is_sorted(navs.begin(), navs.end()));
  for (int i = 0; i < spans.size()-1; i++) {
    CHECK(ordered(spans[i], spans[i+1]));
  }
  int index = 0;
  do {
    std::cout << "\nThere are " << spans.size() << " sessions:\n";
    int last = spans.size()-1;
    for (int i = 0; i < last; i++) {
      auto current = spans[i];
      auto next = spans[i+1];
      dispChoice(i, navs, current);
      std::cout << "  Gap of " << (navs[next.minv()].time() - navs[current.maxv()-1].time()).str() << "\n\n";
    }
    dispChoice(last, navs, spans.last());
    std::cout << "Select a session to look at:\n";
    std::cin >> index;
    index--;
  } while (!(0 <= index && index < spans.size()));
  return spans[index];
}

bool exploreSlice(Array<Nav> allNavs, Spani span);

bool splitData(Array<Nav> allNavs, Spani span) {
  std::cout << "Split the data from " << 0 << " to " << allNavs.size() << "\n";
  std::cout << "  by a span " << span << "\n";
  auto navs = slice(allNavs, span);
  Duration<double> duration = inputDuration("Threshold duration for splitting?");
  auto splitSpans = getSplitSpans(navs, duration);
  if (splitSpans.size() == 1) {
    std::cout << "No split was performed. The data is too dense\n";
    return true;
  }
  Spani selected = selectSpan(navs, splitSpans);
  return exploreSlice(allNavs, span.slice(selected));
}

void plotTrajectory(Array<Nav> navs) {
  GeographicReference geoRef(navs[navs.middle()].geographicPosition());
  int n = navs.size();
  Arrayd X(n), Y(n);
  for (int i = 0; i < n; i++) {
    auto p = geoRef.map(navs[i].geographicPosition());
    X[i] = p[0].meters();
    Y[i] = p[1].meters();
  }
  GnuplotExtra plot;
  plot.plot_xy(X, Y);
  plot.show();
}

bool plotData(Array<Nav> allNavs, Spani span) {
  Array<DataExtractor> extractors{
    awaExtractor,
    magHdgExtractor,
    gpsBearingExtractor,
    awsExtractor,
    watSpeedExtractor,
    gpsSpeedExtractor
  };
  Array<std::string> labels{
    "AWA (degrees)",
    "Magnetic heading (degrees)",
    "GPS bearing (degrees)",
    "AWS (knots)",
    "Water speed (knots)",
    "GPS speed (knots)",
    "Geographic trajectory"
  };
  std::cout << "What do you want to plot?\n";
  int choice = selectFromAlternatives(labels);
  auto subset = slice(allNavs, span);
  if (choice < 6) {
    plotData(labels[choice], extractors[choice], slice(allNavs, span));
  } else {
    plotTrajectory(subset);
  }
  return true;
}

bool selectSpan(Array<Nav> allNavs, Spani span) {
  Spani valid(0, allNavs.size());
  std::cout << "Span of all data: " << valid << "\n";
  std::cout << "Current span: " << span << "\n";
  std::cout << "New span?\n";
  int from = 0;
  int to = 0;
  while (true) {
    std::cout << "From? ";
    std::cin >> from;
    std::cout << "To? ";
    std::cin >> to;
    auto localSpan = Spani(from, to);
    if (valid.contains(from) && valid.contains(to) && goodSpan(localSpan)) {
      Spani newSpan = span.slice(localSpan);
      return exploreSlice(allNavs, newSpan);
    } else {
      std::cout << "Invalid span. Try again\n";
    }
  }
  return false;
}

bool exploreSlice(Array<Nav> allNavs, Spani span) {
  auto navs = slice(allNavs, span);
  while (true) {
    std::cout << "\n\n\n\n\n\nThe data you look at is from " << navs.first().time().toString() << " to "
        << navs.last().time().toString() << "\n";
    std::cout << "Its span is indices from " << span.minv()
        << " to, but not including, " << span.maxv() << "\n";
    Duration<double> totalDur = (navs.last().time() - navs.first().time());
    std::cout << "The total duration is " << (navs.last().time() - navs.first().time()).str() << "\n\n";

    std::cout << "What do you want to do?\n";
    int choice = selectFromAlternatives(Array<std::string>{
      "Split the data into sessions",
      "Select span",
      "Plot the data",
      "Return to previous data set",
      "Quit"
    });
    bool cont = false;
    switch (choice) {
      case 0:
        cont = splitData(allNavs, span);
        break;
      case 1:
        cont = selectSpan(allNavs, span);
        break;
      case 2:
        cont = plotData(allNavs, span);
        break;
      case 3:
        return true;
      case 4:
        return false;
    };
    if (!cont) {
      return false;
    }
  }
  return false;
}

int continueParsing(ArgMap &amap) {
  Array<Nav> navs = scanNmeaFolders(getPaths(amap), Nav::debuggingBoatId());
  if (navs.empty()) {
    LOG(WARNING) << "No navs loaded.";
    return 0;
  } else {
    exploreSlice(navs, Spani(0, navs.size()));
    return 0;
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  amap.registerOption("--dir", "Directory with log files")
      .alias("-d");
  auto parseResult = amap.parse(argc, argv);
  switch (parseResult) {
    case ArgMap::Continue:
      return continueParsing(amap);
    case ArgMap::Error:
      return -1;
    case ArgMap::Done:
      return 0;
  }
  return 0;
}
