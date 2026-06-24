#include <fstream>

#include <server/common/TimeStamp.h>
#include <server/nautical/logimport/SourceGroup.h>
#include <server/nautical/logimport/vakaros.h>

#include <Eigen/Geometry>

namespace sail {

class VakarosImporter : public VakarosVisitor {
  public:
    VakarosImporter(LogAccumulator *dst);
  protected:
    virtual void visit(const VkPosition& pos);
    virtual void visit(const VkDeclination& decl);
    virtual void visit(const VkRaceTimer& raceTimer);
    virtual void visit(const VkLinePosition& linePos);
    virtual void visit(const VkShiftAngle& shift);
    virtual void visit(const VkDeviceConfiguration& cfg);
    virtual void visit(const VkWind& wind);
    virtual void visit(const VkSpeedThroughWater& stw);
    virtual void visit(const VkDepth& depth);
    virtual void visit(const VkTemperature& temp);
    virtual void visit(const VkLoad& load);
  private:
    SourceGroup _sourceGroup;
};

bool readVakaros(const std::string& path, LogAccumulator *dst)
{
    std::ifstream file(path);
    if (!file.good()) {
        std::cerr << path << ": can't open file" << std::endl;
        return false;
    }
    VakarosImporter importer(dst);
    std::string result = importer.readStream(file);
    if (!result.empty()) {
        std::cerr << path << ": " << result << std::endl;
        return false;
    }
    return true;
}

VakarosImporter::VakarosImporter(LogAccumulator *dst) : _sourceGroup("Vakaros", dst) { }

static GeographicPosition<double> posFromLatLon(int32_t lat, int32_t lon) {
    return GeographicPosition<double>(
        Angle<double>::degrees(1e-7 * double(lon)),
        Angle<double>::degrees(1e-7 * double(lat)));
}

void VakarosImporter::visit(const VkPosition& pos) {
    TimeStamp time = TimeStamp::fromMilliSecondsSince1970(pos.timestamp);

    pushBack(time, Velocity<double>::metersPerSecond(pos.sog), _sourceGroup.GPS_SPEED);
    pushBack(time, Angle<double>::radians(pos.cog), _sourceGroup.GPS_BEARING);
    pushBack(time, posFromLatLon(pos.latitude, pos.longitude), _sourceGroup.GPS_POS);

    Eigen::Quaternion<double> q( pos.quat_w, pos.quat_x, pos.quat_y, pos.quat_z);
    Eigen::Vector3d p = q * Eigen::Vector3d::Unit(0);

    double heading = atan2(p.y(), p.x());
    pushBack(time, Angle<double>::radians(heading), _sourceGroup.MAG_HEADING);

    // Not sure about those.
    double pitch = atan2(p.z(), p.y());
    double roll = atan2(p.z(), p.x());

    pushBack(time, Angle<double>::radians(pitch), _sourceGroup.PITCH);
    pushBack(time, Angle<double>::radians(roll), _sourceGroup.ROLL);
}

void VakarosImporter::visit(const VkDeclination& decl) {}
void VakarosImporter::visit(const VkRaceTimer& raceTimer) {}
void VakarosImporter::visit(const VkLinePosition& linePos) {}
void VakarosImporter::visit(const VkShiftAngle& shift) {}
void VakarosImporter::visit(const VkDeviceConfiguration& cfg) {}

void VakarosImporter::visit(const VkWind& wind) {
    TimeStamp time = TimeStamp::fromMilliSecondsSince1970(wind.timestamp);
    pushBack(time, Angle<double>::degrees(wind.awa), _sourceGroup.AWA);
    pushBack(time, Velocity<double>::metersPerSecond(wind.aws), _sourceGroup.AWS);
}

void VakarosImporter::visit(const VkSpeedThroughWater& stw) {
    TimeStamp time = TimeStamp::fromMilliSecondsSince1970(stw.timestamp);
    pushBack(time, Velocity<double>::metersPerSecond(stw.speedThroughWaterForward), _sourceGroup.WAT_SPEED);
}
void VakarosImporter::visit(const VkDepth& depth) {}
void VakarosImporter::visit(const VkTemperature& temp) {}
void VakarosImporter::visit(const VkLoad& load) {}

}  // namespace sail
