const Enum = require('enum');

const StatusEnum = new Enum({
    OPEN: "open",
    ACTIVE: "active",
    EXPIRED: "expired",
    INACTIVE: "inactive"
});

module.exports.statusEnum = StatusEnum;