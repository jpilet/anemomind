#ifndef LOGIMPORT_VAKAROS_H
#define LOGIMPORT_VAKAROS_H

#include <istream>

namespace sail {

struct __attribute__((packed)) VkPageHeader {
    static const uint8_t key = 0xFF;
    uint8_t revision;
    uint8_t logState[6];
};

struct __attribute__((packed)) VkPageTerminator {
    static const uint8_t key = 0xFE;
    uint16_t lengthOfPreviousPage;
};

struct __attribute__((packed)) VkPosition {
    static const uint8_t key = 0x02;

    uint64_t timestamp; // ms since 1970
    int32_t latitude; // in 1e-7 degrees
    int32_t longitude; // in 1e-7 degrees
    float sog; // in meters/sec
    float cog; // in radians
    float altitude; // in meters
    float quat_w;
    float quat_x;
    float quat_y;
    float quat_z;
};

struct __attribute__((packed)) VkDeclination {
    static const uint8_t key = 0x03;

    uint64_t timestamp; // ms since 1970
    float declination_rad;
    int32_t latitude;
    int32_t longitude;
};

struct __attribute__((packed)) VkRaceTimer {
    static const uint8_t key = 0x04;

    uint64_t timestamp; // ms since 1970
    uint8_t type;
    int32_t timer;

    enum {
        RESET = 0,
        START = 1,
        SYNC = 2,
        RACE_START = 3,
        RACE_END = 4
    };
};

struct __attribute__((packed)) VkLinePosition {
    static const uint8_t key = 0x05;

    uint64_t timestamp; // ms since 1970
    uint8_t lineEndType;
    int32_t latitude;
    int32_t longitude;

    enum {
        PIN_LEFT = 0,
        BOAT_RIGHT = 1
    };
};

struct __attribute__((packed)) VkShiftAngle {
    static const uint8_t key = 0x06;

    uint64_t timestamp; // ms since 1970
    uint8_t tackId; // 0 = starboard, 1 = port
    uint8_t manualOrAuto;
    float trueHeadingDeg;
    float sog;
};

struct __attribute__((packed)) VkDeviceConfiguration {
    static const uint8_t key = 0x08;

    uint64_t unused;
    uint32_t config;
    uint8_t loggingRateHz;
};

struct __attribute__((packed)) VkWind {
    static const uint8_t key = 0x0A;

    uint64_t timestamp; // ms since 1970
    float awa;
    float aws; // in meters per sec
};

struct __attribute__((packed)) VkSpeedThroughWater {
    static const uint8_t key = 0x0B;

    uint64_t timestamp; // ms since 1970
                        // both speeds in m/s
    float speedThroughWaterForward;
    float speedThroughWaterHorizontal;
};


struct __attribute__((packed)) VkDepth {
    static const uint8_t key = 0x0C;

    uint64_t timestamp; // ms since 1970
    float depth; // in meters
};


struct __attribute__((packed)) VkTemperature {
    static const uint8_t key = 0x10;

    uint64_t timestamp; // ms since 1970
    float temperature; // in deg celsius
};

struct __attribute__((packed)) VkLoad {
    static const uint8_t key = 0x0F;

    uint64_t timestamp; // ms since 1970
    uint8_t shortName[4];
    float loadAmount;
};

class VakarosVisitor {
  public:

    // Return empty string if OK, otherwise an error.
    std::string readStream(std::istream& s);
  protected:
    virtual void visit(const VkPageHeader& header) {};
    virtual void visit(const VkPageTerminator& terminator) {};
    virtual void visit(const VkPosition& pos) {};
    virtual void visit(const VkDeclination& decl) {};
    virtual void visit(const VkRaceTimer& raceTimer) {};
    virtual void visit(const VkLinePosition& linePos) {};
    virtual void visit(const VkShiftAngle& shift) {};
    virtual void visit(const VkDeviceConfiguration& cfg) {};
    virtual void visit(const VkWind& wind) {};
    virtual void visit(const VkSpeedThroughWater& stw) {};
    virtual void visit(const VkDepth& depth) {};
    virtual void visit(const VkTemperature& temp) {};
    virtual void visit(const VkLoad& load) {};
};

} // namespace sail
#endif // LOGIMPORT_VAKAROS_H
