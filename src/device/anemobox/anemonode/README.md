AnemoNode: a javascript interface to Dispatcher
===============================================

Compilation and test
--------------------

To compile, run

```
npm build .
```

Test with: ```node run.js```


Usage
-----

This node module declares an object with:
```
{
  dispatcher: {
    shortDescription (String) : {
      description (String),
      unit (String)
      type (String: 'angle' or 'velocity')
      value(index) (returns a Number. index defaults to 0: the last measure)
      timestamp(index) (returns a Date. index defaults to 0, the last measure)
      length() (returns the number of stored measures)
      setValue(source, x) (source: string, x: Number)
      subscribe(function(value))  // adds a listener, returns an index
      unsubscribe(index)  // the argument is the returned value of subscribe()
      dataCode // an int uniquely identifying the data.
    }
  },
  Nmea0183Source() // a class. use with: new Nmea0183Source();
}
```

For example, to get the apparent wind angle:
```
var awa = dispatcher.awa.value();
```

