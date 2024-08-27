
export function assert(x) {
    if (!x) {
        console.log("Assertion failed");
        throw new Error("assertion failed");
    }
}

export function assertEqual(a, b) {
    if (a != b) {
        console.log("Assertion failed: ", a, " == ", b)
        throw new Error("assertion failed");
    }
}
