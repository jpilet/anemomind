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
#include <server/common/string.h>
#include <server/common/ArrayIO.h>
#include <server/common/Functional.h>

using namespace sail;
using namespace sail::NavCompat;

/*
 * Code taken from Plot.cpp. Don't know why we need it.
 */
Duration<double> getRawTime(const Nav &n) {
  return Duration<double>::seconds(double(n.time().toMilliSecondsSince1970()/int64_t(1000)));
}

Arrayd getSeconds(NavDataset navs) {
  return toArray(sail::map(Range(navs), [&](const Nav &x) {
    return getRawTime(x).seconds();
  }));
}

namespace {

  typedef std::function<Arrayd(const NavDataset &x)> DataExtractor;

  DataExtractor
    makeAngleExtractor(std::function<Angle<double>(Nav)> f) {
    return [=](NavDataset navs) {
      return toArray(sail::map(cleanContinuousAngles(toArray(sail::map(Range(navs), f))),
       [](Angle<double> x) {
        return x.degrees();
      }));
    };
  }

  DataExtractor
    makeSpeedExtractor(std::function<Velocity<double>(Nav)> f) {
    return [=](NavDataset navs) {
      return toArray(sail::map(Range(navs), [=](const Nav &x) {
        return f(x).knots();
      }));
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

  void plotData(std::string label, std::function<Arrayd(NavDataset)> extractor,
    NavDataset navs) {
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
  return toArray(sail::map(amap.optionArgs("--dir"), [=](ArgMap::Arg* arg) {
    return PathBuilder::makeDirectory(arg->value()).get();
  }));
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

std::map<std::string, Duration<double> > makeUnitMap(
    Array<Array<std::string> > names,
    Array<Duration<double> >  durations) {
  std::map<std::string, Duration<double> > m;
  assert(names.size() == durations.size());
  int count = durations.size();
  for (int i = 0; i < count; i++) {
    auto namesSub = names[i];
    auto dur = durations[i];
    for (auto name: namesSub) {
      m[name] = dur;
    }
  }
  return m;
}

Duration<double> inputDuration(std::string ask) {
  while (true) {
    auto unitMap = makeUnitMap(
        Array<Array<std::string> >{
           Array<std::string>{"s", "seconds", "second", "sec", "secs"},
           Array<std::string>{"min", "minutes", "minute"},
           Array<std::string>{"h", "hour", "hours"},
           Array<std::string>{"day", "days"},
           Array<std::string>{"w", "week", "weeks"},
           Array<std::string>{"month", "months"}
        },
        Array<Duration<double> >{
          Duration<double>::seconds(1.0),
          Duration<double>::minutes(1.0),
          Duration<double>::hours(1.0),
          Duration<double>::days(1.0),
          Duration<double>::weeks(1.0),
          Duration<double>::days(30.0)
        }
    );

    std::cout << ask << std::endl;
    std::string response;
    std::getline(std::cin, response);
    auto parts = split(response, ' ');
    if (parts.size() == 2) {
      double amount = 0;
      if (tryParseDouble(parts[0], &amount)) {
        auto unit = unitMap.find(parts[1]);
        if (unit != unitMap.end()) {
          return amount*unit->second;
        } else {
          std::cout << "No such unit: " << parts[1] << "\n";
        }
      } else {
        std::cout << "Cannot parse as number: " << parts[0] << "\n";
      }
    } else {
      std::cout << "Please provide the amount and the unit, separated by a space.\n";
    }
  }
}

NavDataset slice(NavDataset navs, Spani span) {
  return slice(navs, span.minv(), span.maxv());
}


bool goodSpan(Spani x) {
  return x.minv() < x.maxv();
}


Array<Spani> getSplitSpans(NavDataset navs, Duration<double> dur) {
  std::vector<int> endPoints;
  endPoints.push_back(0);
  int n = getNavSize(navs);
  for (int i = 0; i < n-1; i++) {
    if (dur < getNav(navs, i+1).time() - getNav(navs, i).time()) {
      endPoints.push_back(i+1);
    }
  }
  endPoints.push_back(getNavSize(navs));
  int spanCount = endPoints.size() - 1;
  Array<Spani> spans(spanCount);
  for (int i = 0; i < spanCount; i++) {
    spans[i] = Spani(endPoints[i], endPoints[i+1]);
    CHECK(goodSpan(spans[i]));
  }
  return spans;
}


void dispSession(NavDataset navs) {
  auto from = getFirst(navs).time();
  auto to = getLast(navs).time();
  CHECK(from <= to);
  std::cout << "  Navs from\n";
  std::cout << "    " << from << "\n";
  std::cout << "    to " << to << "\n";
  std::cout << "    with a total duration of " << (getLast(navs).time() - getFirst(navs).time()).str() << "\n";
}


bool ordered(Spani a, Spani b) {
  return a.maxv() <= b.minv();
}

void dispChoice(int i, NavDataset navs, Spani span) {
  CHECK(std::is_sorted(getBegin(navs), getEnd(navs)));
  CHECK(goodSpan(span));
  CHECK(0 <= span.minv());
  CHECK(span.maxv() <= getNavSize(navs));
  std::cout << "* Choice " << i+1 << ":\n";
  dispSession(slice(navs, span));
  std::cout << "\n";
}

Spani selectSpan(NavDataset navs, Array<Spani> spans) {
  CHECK(std::is_sorted(getBegin(navs), getEnd(navs)));
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
      std::cout << "  Gap of " << (getNav(navs, next.minv()).time()
      - getNav(navs, current.maxv()-1).time()).str() << "\n\n";
    }
    dispChoice(last, navs, spans.last());
    std::cout << "Select a session to look at:\n";
    std::cin >> index;
    index--;
  } while (!(0 <= index && index < spans.size()));
  return spans[index];
}

bool exploreSlice(NavDataset allNavs, Spani span);

bool splitData(NavDataset allNavs, Spani span) {
  std::cout << "Split the data from " << 0 << " to " << getNavSize(allNavs) << "\n";
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

void plotTrajectory(NavDataset navs) {
  GeographicReference geoRef(getNav(navs, getMiddleIndex(navs)).geographicPosition());
  int n = getNavSize(navs);
  Arrayd X(n), Y(n);
  for (int i = 0; i < n; i++) {
    auto p = geoRef.map(getNav(navs, i).geographicPosition());
    X[i] = p[0].meters();
    Y[i] = p[1].meters();
  }
  GnuplotExtra plot;
  plot.plot_xy(X, Y);
  plot.show();
}

bool plotData(NavDataset allNavs, Spani span) {
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

bool selectSpan(NavDataset allNavs, Spani span) {
  Spani valid(0, getNavSize(allNavs));
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

bool exploreSlice(NavDataset allNavs, Spani span) {
  auto navs = slice(allNavs, span);
  while (true) {
    std::cout << "\n\n\n\n\n\nThe data you look at is from " << getFirst(navs).time().toString() << " to "
        << getLast(navs).time().toString() << "\n";
    std::cout << "Its span is indices from " << span.minv()
        << " to, but not including, " << span.maxv() << "\n";
    Duration<double> totalDur = (getLast(navs).time() - getFirst(navs).time());
    std::cout << "The total duration is " << (getLast(navs).time() - getFirst(navs).time()).str() << "\n\n";

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
  NavDataset navs = scanNmeaFolders(getPaths(amap), Nav::debuggingBoatId());
  if (isEmpty(navs)) {
    LOG(WARNING) << "No navs loaded.";
    return 0;
  } else {
    exploreSlice(navs, Spani(0, getNavSize(navs)));
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
