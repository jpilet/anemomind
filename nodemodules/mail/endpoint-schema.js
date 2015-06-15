var schemautils = require('./schemautils.js');

var MethodSchema = schemautils.MethodSchema;
var EndPointSchema = schemautils.EndPointSchema;
var errorTypes = schemautils.errorTypes;
  
var methods = {};

methods.putPacket = new MethodSchema({
  httpMethod: 'post',
  input: [{packet: 'any'}],
  output: [{err: errorTypes}]
});

// Only for debugging and unit testing
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
    {lowerBound: 'hex'}
  ],
  output: [{err: errorTypes}]
});

methods.getLowerBound = new MethodSchema({
  httpMethod: 'get',
  input: [
    {src: String},
    {dst: String},
  ],
  output: [
    {err: errorTypes},
    {lowerBound: 'hex'}
  ]
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


methods.getUpperBound = new MethodSchema({
  httpMethod: 'get',
  input: [
    {src: String},
    {dst: String},
  ],
  output: [
    {err: errorTypes},
    {lowerBound: 'hex'}
  ]
});


methods.getTotalPacketCount = new MethodSchema({
    httpMethod: 'get',
    input: [],
    output: [
	{err: errorTypes},
	{count: Number}
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
