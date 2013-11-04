function makeSailStateMachine001() {
    var sm = new StateMachine();
    
    return sm;
}

var sm = makeSailStateMachine001();

document.writeln('allnavs are saildata? ' + isSailData(allnavs));
document.writeln('allnavs are sailrecord? ' + isSailRecord(allnavs[0]));
document.writeln('State machine created: ' + sm);
DOUT('getMaxTWA(allnavs)');
DOUT('getMinTWA(allnavs)');

// for (var i = 0; i < 30; i++) {
//     var sr = allnavs[i];
//     var twa = getTWAMax180(sr);
//     DOUT('i');
//     DOUT('twa');
//     DOUT('isUpwind(sr)');
//     DOUT('isDownwind(sr)');
//     DOUT('isBeamReach(sr)');
//     DOUT('getMinorStateLabel(sr)');
// }

var mat = makeMatrix(3, 4);
mat.disp();


sm = new SailModel001(allnavs);
sm.disp();

makeMajorStateConnections().disp();
makeMinorStateConnections().disp();

sm.connectivity.disp();
sm.costs.disp();
document.writeln(sm.minorStateCounts);

document.writeln('SUCCESS');
