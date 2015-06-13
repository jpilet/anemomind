var schemautils = require('./schemautils.js');

var MethodSchema = schemautils.MethodSchema;
var EndPointSchema = schemautils.EndPointSchema;
var errorTypes = schemautils.errorTypes;
  
var methods = {};

// Below follows specifications of what methods every
// method of a mailbox object should support:
methods.setForeignDiaryNumber = new MethodSchema({
    httpMethod:'get',
    input: [
	{otherMailbox: String},
	{newValue: 'hex'}
    ],
    output: [
	{err: errorTypes},
    ]
});

methods.getFirstPacketStartingFrom = new MethodSchema({
    httpMethod:'get',
    input: [
	{diaryNumber: 'hex'},
	{lightWeight: Boolean},
    ],
    output: [
	{err: errorTypes},
	{packet: 'any'}
    ]
});

methods.handleIncomingPacket = new MethodSchema({
    httpMethod:'post',
    input: [
	{packet: 'any'}
    ],
    output: [
	{err: errorTypes}
    ]
});

methods.isAdmissible = new MethodSchema({
    httpMethod:'get',
    input: [
	{src: String},
	{dst: String},
	{seqNumber: 'hex'}
    ],
    output: [
	{err: errorTypes},
	{p: Boolean}
    ]
});

methods.getForeignDiaryNumber = new MethodSchema({
    httpMethod:'get',
    input: [
	{otherMailbox: String}
    ],
    output: [
	{err: errorTypes},
	{diaryNumber: 'hex'}
    ]
});

methods.getForeignStartNumber = new MethodSchema({
    httpMethod:'get',
    input: [
	{otherMailbox: String}
    ],
    output: [
	{err: errorTypes},
	{diaryNumber: 'hex'}
    ]
});

methods.reset = new MethodSchema({
    httpMethod:'get',
    input: [],
    output: [
	{err: errorTypes}
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

methods.getTotalPacketCount = new MethodSchema({
    httpMethod: 'get',
    input: [],
    output: [
	{err: errorTypes},
	{count: Number}
    ]
});

module.exports = new EndPointSchema(methods);
