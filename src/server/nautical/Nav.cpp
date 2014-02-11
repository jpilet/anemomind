/*
 * Nav.cpp
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#include "Nav.h"
#include <server/common/ArrayIO.h>
#include <algorithm>
#include <server/plot/gnuplot_i.hpp>
#include <server/common/LineKM.h>
#include <server/common/Duration.h>
#include <server/plot/extra.h>
#include <server/nautical/Ecef.h>
#include <ctime>

namespace sail
{

Nav::Nav()
{
	_year = -1;
	_month = -1;
	_dayOfTheMonth = -1;
	_hour = -1;
	_minute = -1;
	_second = -1;
	_gpsSpeed = -1;
	_awa = -1;
	_aws = -1;

	_twaFromFile = -1;
	_twsFromFile = -1;

	_magHdg = -1;
	_watSpeed = -1;
	_gpsBearing = -1;
	_posLatDeg = -1;
	_posLatMin = -1;
	_posLatMc = -1;
	_posLonDeg = -1;
	_posLonMin = -1;
	_posLonMc = -1;
	_cwd = -1;
	_wd = -1;
	_timeDays = -1;
}


//tm_sec	int	seconds after the minute	0-60*
//tm_min	int	minutes after the hour	0-59
//tm_hour	int	hours since midnight	0-23
//tm_mday	int	day of the month	1-31
//tm_mon	int	months since January	0-11
//tm_year	int	years since 1900
//tm_wday	int	days since Sunday	0-6
//tm_yday	int	days since January 1	0-365
//tm_isdst	int	Daylight Saving Time flag

Nav::Nav(MDArray2d row)
{
	assert(row.rows() == 1);
	assert(row.cols() == 23 || row.cols() == 22);

	_year = row(0, 0);
	_month = row(0, 1);
	_dayOfTheMonth = row(0, 2);
	_hour = row(0, 3);
	_minute = row(0, 4);
	_second = row(0, 5);
	_gpsSpeed = row(0, 6);
	_awa = row(0, 7);
	_aws = row(0, 8);

	_twaFromFile = row(0, 9);
	_twsFromFile = row(0, 10);

	_magHdg = row(0, 11);
	_watSpeed = row(0, 12);
	_gpsBearing = row(0, 13);
	_posLatDeg = row(0, 14);
	_posLatMin = row(0, 15);
	_posLatMc = row(0, 16);
	_posLonDeg = row(0, 17);
	_posLonMin = row(0, 18);
	_posLonMc = row(0, 19);
	_cwd = row(0, 20);
	_wd = row(0, 21);

	const bool timeFromFile = false;

	if (timeFromFile)
	{
		assert(row.cols() == 23);
		_timeDays = row(0, 22);
	}
	else
	{
		struct tm time;
		time.tm_gmtoff = 0;
		time.tm_isdst = 0; // daylight saving. What to put here???
		time.tm_sec = int(_second);
		time.tm_min = _minute;
		time.tm_hour = _hour;
		time.tm_mon = _month - 1;
		time.tm_year = (_year + 2000) - 1900;
		time.tm_mday = _dayOfTheMonth;

		// Ignored
		time.tm_yday = -1;
		time.tm_wday = -1;

		time_t x = mktime(&time);
		_timeDays = (1.0/(24*60*60))*x;
		//_time = (2000 + _year);

	}
}

Nav::~Nav()
{
	// TODO Auto-generated destructor stub
}

double degMinMc2Radians(double deg, double min, double mc)
{
	return deg2rad(deg + (1.0/60)*(min + 0.001*mc));
}

double Nav::getLonRadians() const
{
	return degMinMc2Radians(_posLonDeg, _posLonMin, _posLonMc);
}

double Nav::getLatRadians() const
{
	return degMinMc2Radians(_posLatDeg, _posLatMin, _posLatMc);
}

void Nav::getEcef3dPos(double &xOut, double &yOut, double &zOut) const
{
	lla2ecef(getLonRadians(), getLatRadians(), 0.0, xOut, yOut, zOut);
}

const char Nav::AllNavsPath[] = "../../../../datasets/allnavs.txt";

// From load_data.m
//year = 1;
//month = 2;
//dayOfTheMonth = 3;
//hour = 4;
//minute = 5;
//second = 6;
//gpsSpeed = 7;
//awa = 8;
//aws = 9;
//twa = 10;
//tws = 11;
//magHdg = 12;
//watSpeed = 13;
//gpsBearing = 14;
//pos_lat_deg = 15;
//pos_lat_min = 16;
//pos_lat_mc = 17;
//pos_lon_deg = 18;
//pos_lon_min = 19;
//pos_lon_mc = 20;
//cwd = 21;
//wd = 22;
//days = 23;
//days_data = datenum(unsorted_data(:,year) + 2000, unsorted_data(:,month), unsorted_data(:, dayOfTheMonth), unsorted_data(:,hour), unsorted_data(:,minute), unsorted_data(:,second));



Array<Nav> loadNavsFromText(std::string filename, bool sort)
{
	MDArray2d data = loadMatrixText<double>(filename);
	int count = data.rows();

	std::vector<Nav> navs(count);
	for (int i = 0; i < count; i++)
	{
		navs[i] = Nav(data.sliceRow(i));
	}

	if (sort)
	{
		std::sort(navs.begin(), navs.end());
	}

	return Array<Nav>::referToVector(navs).dup();
}

bool areSortedNavs(Array<Nav> navs)
{
	int count = navs.size();
	for (int i = 0; i < count-1; i++)
	{
		if (navs[i].getTimeDays() > navs[i+1].getTimeDays())
		{
			return false;
		}
	}
	return true;
}

void plotNavTimeVsIndex(Array<Nav> navs)
{
	Gnuplot plot;
	int count = navs.size();

	std::vector<double> X(count), Y(count);
	for (int i = 0; i < count; i++)
	{
		X[i] = i;
		Y[i] = navs[i].getTimeDays();
	}

	plot.set_style("lines");
	plot.plot_xy(X, Y);
	plot.set_xautoscale();
	plot.set_yautoscale();
	plot.set_xlabel("Index");
	plot.set_ylabel("Time");
	plot.showonscreen();
	sleepForever();
}

double getNavsMaxInterval(Array<Nav> navs)
{
	int count = navs.size();
	double m = 0.0;
	for (int i = 0; i < count-1; i++)
	{
		m = std::max(m, navs[i+1].getTimeSeconds() - navs[i].getTimeSeconds());
	}
	return m;
}

void dispNavTimeIntervals(Array<Nav> navs)
{
	double mintime = navs.reduce<double>(navs[0].getTimeDays(), [&] (double a, Nav b) {return std::min(a, b.getTimeSeconds());});
	double maxtime = navs.reduce<double>(navs[0].getTimeDays(), [&] (double a, Nav b) {return std::max(a, b.getTimeSeconds());});
	double m = getNavsMaxInterval(navs);
	std::cout << "Max interval (seconds): " << m << std::endl;
	std::cout << "Span (seconds): " << maxtime - mintime << std::endl;
	int navCount = navs.size();
	int binCount = 30;
	LineKM line(log(2.0), log(m+1), 1, binCount);

	Arrayi bins(binCount);
	bins.setTo(0);
	for (int i = 0; i < navCount-1; i++)
	{
		double span = navs[i+1].getTimeSeconds() - navs[i].getTimeSeconds();
		int index = std::max(0, int(floor(line(log(span)))));
		bins[index]++;
	}
	for (int i = 0; i < binCount; i++)
	{
		cout << "Bin " << i+1 << "/" << binCount << ": " << bins[i] << " intervals longer than the previous but shorter than " << Duration(exp(line.inv(i+1))).str() << endl;
	}
}

int countNavSplitsByDuration(Array<Nav> navs, double durSeconds)
{
	int count = navs.size();
	int counter = 0;
	for (int i = 0; i < count-1; i++)
	{
		if (navs[i+1].getTimeSeconds() - navs[i].getTimeSeconds() > durSeconds)
		{
			counter++;
		}
	}
	return counter;
}

Array<Array<Nav> > splitNavsByDuration(Array<Nav> navs, double durSeconds)
{
	int count = 1 + countNavSplitsByDuration(navs, durSeconds);
	Array<Array<Nav> > dst(count);
	int navCount = navs.size();
	int from = 0;
	int counter = 0;
	for (int i = 0; i < navCount-1; i++)
	{
		if (navs[i+1].getTimeSeconds() - navs[i].getTimeSeconds() > durSeconds)
		{
			dst[counter] = navs.slice(from, i+1);
			counter++;
			from = i+1;
		}
	}
	dst.last() = navs.sliceFrom(from);
	assert(counter + 1 == count);
	return dst;
}

MDArray2d calcNavsEcefTrajectory(Array<Nav> navs)
{
	int count = navs.size();
	MDArray2d data(count, 3);
	for (int i = 0; i < count; i++)
	{
		Nav &nav = navs[i];
		lla2ecef(nav.getLonRadians(), nav.getLatRadians(), 0.0, data(i, 0), data(i, 1), data(i, 2));
	}
	return data;
}

Array<MDArray2d> calcNavsEcefTrajectories(Array<Array<Nav> > navs)
{
	int count = navs.size();
	Array<MDArray2d> dst(count);
	for (int i = 0; i < count; i++)
	{
		dst[i] = calcNavsEcefTrajectory(navs[i]);
	}
	return dst;
}

void plotNavsEcefTrajectory(Array<Nav> navs)
{
	assert(areSortedNavs(navs));

	GnuplotExtra plot;
	plot.set_style("lines");
	plot.plot(calcNavsEcefTrajectory(navs));
	//plot.cmd("set view equal xyz");
	plot.show();
}

void plotNavsEcefTrajectories(Array<Array<Nav> > navs)
{
	int count = navs.size();
	LineKM hue(0, count, 0.0, 360.0);

	GnuplotExtra plot;
	plot.set_style("lines");
	for (int i = 0; i < count; i++)
	{
		plot.plot(calcNavsEcefTrajectory(navs[i]));
	}
	plot.cmd("set view equal xyz");
	plot.show();
}

int countNavs(Array<Array<Nav> > navs)
{
	int counter = 0;
	int count = navs.size();
	for (int i = 0; i < count; i++)
	{
		counter += navs[i].size();
	}
	return counter;
}


} /* namespace sail */
