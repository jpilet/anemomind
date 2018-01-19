// To execute this file, run from the build directory:
// make root -j8
// and then, at root prompt, enter:
//   .x simple_twa_histo.C
//
//  or:
//
//  .L simple_twa_histo.C+
//
//  And then copy/paste the lines of simple_twa_histo() below.

#include <server/common/Env.h>
#include <server/nautical/NavDataset.h>
#include <server/nautical/logimport/LogLoader.h>

#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>

using namespace sail;

NavDataset load(std::string source) {
  if (source.size() == 0) {
    source = std::string(Env::SOURCE_DIR) + "/datasets/psaros33_Banque_Sturdza/2014/20140614 Bol d'or/";
  }
  NavDataset ds = LogLoader::loadNavDataset(source);
  std::set<DataCode> awaCode{AWA, TWS, TWA, GPS_SPEED};
  return ds.createMergedChannels(awaCode);
}

void awaHisto(NavDataset& ds, TH1D* histo) {
  auto awa = ds.samples<AWA>();

  for (auto x: awa) {
    histo->Fill(x.value.normalizedAt0().degrees());
  }
}

void polarHisto(NavDataset& ds, TH3D* polar_histo) {
  auto twa = ds.samples<TWA>();
  auto tws = ds.samples<TWS>();
  TimedSampleRange<Velocity<double>> speed = ds.samples<GPS_SPEED>();

  long count = 0;
  for (auto twa_it : twa) {
    TimeStamp t = twa_it.time;
    auto speed_now = speed.nearest(t);
    if (!speed_now.defined()) {
      continue;
    }
    auto tws_now = tws.nearest(t);
    if (!tws_now.defined()) {
      continue;
    }
    polar_histo->Fill(twa_it.value.normalizedAt0().degrees(),
                     tws_now.get().value.knots(),
                     speed_now.get().value.knots());
    ++count;
  }
  std::cout << "Count: " << count << std::endl;
  polar_histo->Draw();
}

void slice(TH3D* polar, double tws, TH2D* slice) {
  TAxis* xaxis = polar->GetXaxis();
  TAxis* yaxis = polar->GetYaxis();
  TAxis* zaxis = polar->GetZaxis();

  int biny = yaxis->FindBin(tws);
  int lastbin_x = xaxis->GetNbins();
  int lastbin_z = zaxis->GetNbins();

  slice->Reset();

  for (int binx = 1; binx <= lastbin_x; ++binx) {
    for (int binz = 1; binz <= lastbin_z; ++binz) {
      int bin = polar->GetBin(binx, biny, binz);
      double val = polar->GetBinContent(bin);
      slice->Fill(xaxis->GetBinCenter(binx), yaxis->GetBinCenter(binz), val);
    }
  }
}

void sliceNormalized(TH3D* polar, double tws, TH2D* slice) {
  TAxis* xaxis = polar->GetXaxis();
  TAxis* yaxis = polar->GetYaxis();
  TAxis* zaxis = polar->GetZaxis();

  int biny = yaxis->FindBin(tws);
  int lastbin_x = xaxis->GetNbins();
  int lastbin_z = zaxis->GetNbins();

  slice->Reset();

  for (int binx = 1; binx <= lastbin_x; ++binx) { // for all angles...
    double total = 0;
    for (int binz = 1; binz <= lastbin_z; ++binz) {  // for all boat speeds...
      int bin = polar->GetBin(binx, biny, binz);
      double val = polar->GetBinContent(bin);
      total += val;
    }
    if (total == 0) {
      continue;
    }

    double norm_factor = 1.0 / total;

    for (int binz = 1; binz <= lastbin_z; ++binz) {  // for all boat speeds...
      int bin = polar->GetBin(binx, biny, binz);
      double val = polar->GetBinContent(bin);
      slice->Fill(xaxis->GetBinCenter(binx), zaxis->GetBinCenter(binz), val * norm_factor);
    }
  }
}
      
TH3D* createPolar3DHisto() {
  return new TH3D("polar", "Polar histogram",
    90, -180, 180, // x axis is TWA in degrees
    12, 0, 24, // y axis is TWS in knots
    20, 0, 16  // z axis is boat speed in knots
    );
}

TH2D* createPolarSlice() {
  TH2D* h = new TH2D("polar18", "Polar at 18 knots",
    90, -180, 180, // x axis is TWA in degrees
    20, 0, 16  // y axis is boat speed in knots
    );
  h->SetContour(10);
  return h;
}

NavDataset simple_twa_histo() {
  NavDataset ds = load("");
  TH3D* polar_histo = createPolar3DHisto();
  polarHisto(ds, polar_histo);
  TH2D* h = createPolarSlice();
  sliceNormalized(polar_histo, 18, h);
  h->Draw("col cont3");
  return ds;
}

/*
TH1D histo("awa_histo", "AWA histogram for Irene", 180, -180, 180);

awaHisto(ds, &histo);
*/



