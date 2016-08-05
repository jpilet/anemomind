#include <iostream>
#include <server/nautical/logimport/LogLoader.h>

using namespace sail;

int main() {
  

  std::cout << "Loading logs..." << std::endl;
  LogLoader loader;
  loader.load("/Users/jonas/data/boat55ca03d49b16d6f47e5dbfc0");
  //auto ds = loader.makeNavDataset().fitBounds();

  return 0;
}
