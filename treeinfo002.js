function makeTreeInfo002(sm) {
    var stateCount = sm.getStateCount();
    var labels = new Array(41);
    for (var i = 0; i < stateCount; i++) {
	labels[i] = sm.getShortStateLabel(i);
    }
    labels[25] = 'Top';
    labels[26] = 'Sailing';
    labels[27] = 'In race';
    labels[28] = 'Upwind leg';
    labels[29] = 'Starboard tack';
    labels[30] = 'Port tack';
    labels[31] = 'Downwind leg';
    labels[32] = 'Starboard tack';
    labels[33] = 'Port tack';
    labels[34] = 'Not in race';
    labels[35] = 'Idle';
    labels[36] = 'Starboard tack';
    labels[37] = 'Port tack';
    labels[38] = 'Just before race';
    labels[39] = 'Starboard tack';
    labels[40] = 'Port tack';
    return new TreeInfo(stateCount, labels);
}

