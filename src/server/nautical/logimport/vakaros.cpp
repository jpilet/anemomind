#include "vakaros.h"

namespace sail {

static_assert(sizeof(VkPosition) == 44, "bad size");
static_assert(sizeof(VkDeviceConfiguration) == 13, "bad size");
static_assert(sizeof(VkRaceTimer) == 13, "bad size");

template <typename T>
    bool read(std::istream& s, T* result) {
        s.read(reinterpret_cast<char *>(result), sizeof(T));
        return s.good() && s.gcount() == sizeof(T);
    }


template <typename T>
    T readOne(std::istream& s) {
        T r;
        read(s, &r);
        return r;
    }

void consume(std::istream& s, size_t bytes)
{
    s.ignore(bytes);
}

std::string VakarosVisitor::readStream(std::istream& s) {
    while (s.good()) {
        uint8_t key;
        if (!read<uint8_t>(s, &key)) {
            break;
        }
        //printf("Vakaros key 0x%02x at %d\n", key, s.tellg());
        switch (key) {
            case VkPageHeader::key:
                VkPageHeader header;
                read(s, &header);
                visit(header);
                break;
            case VkPageTerminator::key:
                VkPageTerminator t;
                read(s, &t);
                visit(t);
                break;
            case VkPosition::key:
                VkPosition pos;
                read(s, &pos);
                visit(pos);
                break;
            case VkDeclination::key:
                VkDeclination decl;
                read(s, &decl);
                visit(decl);
                break;
            case VkRaceTimer::key:
                VkRaceTimer timer;
                read(s, &timer);
                visit(timer);
                break;
            case VkLinePosition::key:
                VkLinePosition linePos;
                read(s, &linePos);
                visit(linePos);
                break;
            case VkShiftAngle::key:
                VkShiftAngle shiftAngle;
                read(s, &shiftAngle);
                visit(shiftAngle);
                break;
            case VkDeviceConfiguration::key:
                VkDeviceConfiguration cfg;
                read(s, &cfg);
                visit(cfg);
                break;
            case VkWind::key:
                VkWind wind;
                read(s, &wind);
                visit(wind);
                break;
            case VkSpeedThroughWater::key:
                VkSpeedThroughWater speedThroughWater;
                read(s, &speedThroughWater);
                visit(speedThroughWater);
                break;
            case VkDepth::key:
                VkDepth depth;
                read(s, &depth);
                visit(depth);
                break;
            case VkTemperature::key:
                VkTemperature temperature;
                read(s, &temperature);
                visit(temperature);
                break;
            case VkLoad::key:
                VkLoad load;
                read(s, &load);
                visit(load);
                break;

                // internal messages:
            case 0x01:
                consume(s, 32);
                break;
            case 0x07:
                consume(s, 12);
                break;
            case 0x0E:
                consume(s, 16);
                break;
            case 0x20:
                consume(s, 13);
                break;
            case 0x21:
                consume(s, 52);
                break;
            default:
                printf("Bad key: 0x%x\n", key);
                std::string error = "Bad key";
                return error;
        }
    }
    return "";
}

}  // namespace sail
