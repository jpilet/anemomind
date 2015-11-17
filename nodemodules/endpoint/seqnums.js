exports.make = function() {
    return (new Date).getTime();
};

exports.next = function(x) {
    return x + 1;
}
