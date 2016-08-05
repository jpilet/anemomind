module.exports.reset = function() {
    module.exports.get = function() {
	return new Date();
    };
};

module.exports.get = function() {
    return new Date();
}
