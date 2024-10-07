#include "vakaros.h"

#include <fstream>
#include <iostream>

using namespace sail;

class VakarosPrinter : public sail::VakarosVisitor {
  public:
    VakarosPrinter() {}

  protected:
    virtual void visit(const VkPosition& pos) {
        std::cout << "VkPosition\n";
    };
    virtual void visit(const VkDeclination& decl) {
        std::cout << "VkDeclination\n";
    };
    virtual void visit(const VkRaceTimer& raceTimer) {
        std::cout << "VkRaceTimer\n";
    };
    virtual void visit(const VkLinePosition& linePos) {
        std::cout << "VkLinePosition\n";
    };
    virtual void visit(const VkShiftAngle& shift) {
        std::cout << "VkShiftAngle\n";
    };
    virtual void visit(const VkDeviceConfiguration& cfg) {
        std::cout << "VkDeviceConfiguration\n";
    };
    virtual void visit(const VkWind& wind) {
        std::cout << "VkWind\n";
    };
    virtual void visit(const VkSpeedThroughWater& stw) {
        std::cout << "VkSpeedThroughWater\n";
    };
    virtual void visit(const VkDepth& depth) {
        std::cout << "VkDepth\n";
    };
    virtual void visit(const VkTemperature& temp) {
        std::cout << "VkTemperature\n";
    };
    virtual void visit(const VkLoad& load) {
        std::cout << "VkLoad\n";
    };
};

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.vkx>\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file.good()) {
        std::cerr << argv[1] << ": failed to open\n";
        return 1;
    }

    VakarosPrinter printer;
    std::string err = printer.readStream(file);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return 1;
    }
    return 0;
}

