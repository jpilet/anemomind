import SQLite
import Foundation

func putAngle(dict: [String:String], key: String) {
  if let val = dict[key],
    var dbl = Double(val.components(separatedBy:" ")[0]) {

    if (val.hasSuffix("P")) {
      dbl *= -1;
    }
    print("\(dbl)", terminator: ",")
  } else {
    print("x", terminator: ",")
  }
}
func putSpeed(dict: [String:String], key: String) {
  if let val = dict[key],
    let dbl = Double(val.components(separatedBy:" ")[0]) {
    print("\(dbl)", terminator: ",")
  } else {
    print("x", terminator: ",")
  }
}

let db = try Connection(CommandLine.arguments[1])

// CREATE TABLE IF NOT EXISTS "Records" (
//    "timeStamp" TEXT PRIMARY KEY NOT NULL,
//    "level" INTEGER NOT NULL,
//    "lat" REAL NOT NULL,
//    "lon" REAL NOT NULL,
//    "data" BLOB NOT NULL);
let records = Table("Records")

let timeStamp = Expression<String?>("timeStamp")
let lat = Expression<Double?>("lat")
let lon = Expression<Double?>("lon")
let data = Expression<Blob>("data")

print("Lat.,Long.,AWS,AWA,TWS,TWA,COG,SOG,HDG,Speed Through Water,DATE/TIME(UTC)")
for rec in try db.prepare(records) {
  if let ts = rec[timeStamp],
    let _lat = rec[lat],
    let _lon = rec[lon] {

    let blob = rec[data].bytes;
    let nsdata = NSData(bytes: blob, length: blob.count) as Data
    if let unarchived = NSKeyedUnarchiver.unarchiveObject(with: nsdata) as? [String:String] {

      print("\(_lat),\(_lon)", terminator: ",")
      putSpeed(dict: unarchived, key: "AWS")
      putAngle(dict: unarchived, key: "AWA")
      putSpeed(dict: unarchived, key: "TWS")
      putAngle(dict: unarchived, key: "TWA")
      putSpeed(dict: unarchived, key: "COG")
      putSpeed(dict: unarchived, key: "SOG")
      putSpeed(dict: unarchived, key: "HDG")
      putSpeed(dict: unarchived, key: "STW")
      print("\(ts)Z")
    }
  }
}
