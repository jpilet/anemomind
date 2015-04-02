module.exports = [
    // Calls required for synchronization
    'setForeignDiaryNumber',
    'getFirstPacketStartingFrom',
    'handleIncomingPacket',
    'isAdmissible',
    'getForeignDiaryNumber',
    'getMailboxName',

    // Reset the contents of the mailbox
    // (useful when running a fresh test)
    'reset',

    
    // Other calls.
    'sendPacket',
    'getTotalPacketCount',
];
