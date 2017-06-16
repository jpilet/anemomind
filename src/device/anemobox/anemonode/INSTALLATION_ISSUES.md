If we get this error when calling "./run.sh":
```
Anemobox firmware version 1.7
module.js:338
    throw err;
          ^
Error: Cannot find module 'debug'
    at Function.Module._resolveFilename (module.js:336:15)
    at Function.Module._load (module.js:278:25)
    at Module.require (module.js:365:17)
    at require (module.js:384:17)
    at Object.<anonymous> (/home/anemonode/node_modules/express/node_modules/finalhandler/index.js:14:13)
    at Module._compile (module.js:460:26)
    at Object.Module._extensions..js (module.js:478:10)
    at Module.load (module.js:355:32)
    at Function.Module._load (module.js:310:12)
    at Module.require (module.js:365:17)
```
Then
```
cd node_modules/express/node_modules/finalhandler
npm install
```
