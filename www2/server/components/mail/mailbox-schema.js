/*

  This file specifies what
  operations a mailbox interface should support.
  It applies to the direct interaction with a mailbox
  object, a remote mailbox over HTTP, or a remote mailbox
  over bluetooth.

  The specification in this file serves two purposes:
   * It can be used to generate a mailbox API
   * It can be used to check that implementations
     conform with this specification.
  
  Types used for:
    * 'hex' : An integer represented as a hexadecimal number
    * 'binary' : A buffer. With JSON, it will be converted
      to a string of hexadecimal numbers. With msgpack,
      it will be encoded to a buffer.
    * 'any' : A general object. With JSON it will be encoded
      as a JSON object using the 'json-buffer' library.
      
      With msgpack, it will be encoded
      as a binary buffer.
      
    * The usual ones used in MongoDB schemas:
      String, Boolean, Number, etc.

    How to get the names of function parameters:
    
      http://stackoverflow.com/questions/1007981/how-to-get-function-parameter-names-values-dynamically-from-javascript

    Can be used to validate that the methods of a mailbox implements
    this specification.
*/  

var methods = {};

function Schema(spec) {
    this.spec = spec;
}


// Below follows specifications of what methods every
// method of a mailbox object should support:
methods.setForeignDiaryNumber = new Schema({
    input: [
	{otherMailbox: 'hex'},
	{newValue: 'hex'}
    ],
    output: [
	{err: 'any'},
    ]
});

methods.getFirstPacketStartingFrom = new Schema({
    input: [
	{diaryNumber: 'hex'},
	{lightWeight: Boolean},
    ],
    output: [
	{err: 'any'},
	{packet: 'any'}
    ]
});

methods.handleIncomingPacket = new Schema({
    input: [
	{packet: 'any'}
    ],
    output: [
	{err: 'any'}
    ]
});

methods.isAdmissible = new Schema({
    input: [
	{src: 'hex'},
	{dst: 'hex'},
	{seqNumber: 'hex'}
    ],
    output: [
	{err: 'any'},
	{p: Boolean}
    ]
});

methods.getForeignDiaryNumber = new Schema({
    input: [
	{otherMailbox: 'hex'}
    ],
    output: [
	{err: 'any'},
	{diaryNumber: 'hex'}
    ]
});

methods.getForeignStartNumber = new Schema({
    input: [
	{otherMailbox: 'hex'}
    ],
    output: [
	{err: 'any'},
	{diaryNumber: 'hex'}
    ]
});

methods.getMailboxName = new Schema({
    input: [],
    output: [
	{mailboxName: 'hex'}
    ]
});

methods.reset = new Schema({
    input: [],
    output: [
	{err: 'any'}
    ]
});

methods.sendPacket = new Schema({
    input: [
	{dst: 'hex'},
	{label: Number},
	{data: 'binary'}
    ],
    output: [
	{err: 'any'}
    ]
});

methods.getTotalPacketCount = new Schema({
    input: [],
    output: [
	{err: 'any'},
	{count: Number}
    ]
});


function MailboxSchema(methods) {
    this.methods = methods;
}

module.exports = new MailboxSchema(
    methods
);
