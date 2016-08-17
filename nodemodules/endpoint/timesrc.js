module.exports.reset = function() {
    module.exports.now = function() {
	return new Date();
    };
};

module.exports.reset();
