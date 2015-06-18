var schemautils = require('./schemautils.js');

var MethodSchema = schemautils.MethodSchema;
var EndPointSchema = schemautils.EndPointSchema;
var errorTypes = schemautils.errorTypes;

var methods = {};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////  Essential methods for the interface
/////////////////////////////////////////////////////////////////////////////
methods.putPacket = new MethodSchema({
  httpMethod: 'post',
  input: [{packet: 'any'}],
  output: [{err: errorTypes}]
});

methods.getPacket = new MethodSchema({
  httpMethod: 'get',
  input: [
    {src: String},
    {dst: String},
    {seqNumber: 'hex'}
  ],
  output: [
    {err: errorTypes},
    {packet: 'any'}
  ]
});

methods.getSrcDstPairs = new MethodSchema({
  httpMethod:'get',
  input: [],
  output: [
    {err: errorTypes},
    {pairs: 'any'}
  ]
});

methods.setLowerBound = new MethodSchema({
  httpMethod: 'get',
  input: [
    {src: String},
    {dst: String},
    {lb: 'hex'}
  ],
  output: [{err: errorTypes},
           {lb: 'hex'}]
});

methods.getLowerBounds = new MethodSchema({
  httpMethod: 'post',
  input: [
    {pairs: 'any'}
  ],
  output: [
    {err: errorTypes},
    {lbs: 'any'}
  ]
});

methods.getUpperBounds = new MethodSchema({
  httpMethod: 'post',
  input: [
    {pairs: 'any'}
  ],
  output: [
    {err: errorTypes},
    {ubs: 'any'}
  ]
});

/////////////////////////////////////////////////////////////////////////////
/////////////////////////  Only for debugging and unit testing
/////////////////////////////////////////////////////////////////////////////
methods.getTotalPacketCount = new MethodSchema({
  httpMethod: 'get',
  input: [],
  output: [
    {err: errorTypes},
    {count: Number}
  ]
});

methods.sendPacket = new MethodSchema({
  httpMethod: 'post',
  input: [
    {dst: String},
    {label: Number},
    {data: 'buffer'}
  ],
  output: [
    {err: errorTypes}
  ]
});


methods.reset = new MethodSchema({
  httpMethod:'get',
  input: [],
  output: [
    {err: errorTypes}
  ]
});

module.exports = new EndPointSchema(methods);
