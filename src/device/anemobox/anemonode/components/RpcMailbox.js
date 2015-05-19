schema = require('mail/mailbox-schema.js');



// Prefix all mailbox-related calls with mb
// to avoid naming collisions for common names (such as "reset")
function makeRpcFuncName(methodName) {
    return "mb_" + methodName;
}

for (var methodName in schema.methods) {
    var rpcFuncName = makeRpcFuncName(methodName);
    console.log("name: %j", rpcFuncName);
}
