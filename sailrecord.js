// Functions to parse the data in 'allnavs.js'

function isSailRecord(x) {
    if (x instanceof Array) {
	if (x.length == 23) {
	    return typeof(x[0]) == "number";   
	}
    }
    return false;
}

function isSailData(x) {
    if (x instanceof Array) { 
	if (x.length == 0) {
	    return true;
	}
	else {
	    return isSailRecord(x[0]);
	}
    }
    else {
	return false;
    }
}





// get methods
function getYear(sr) {
    return sr[0];
}

function getMonth(sr) {
    return sr[1];
}

function getDayOfMonth(sr) {
    return sr[2];
}

function getHour(sr) {
    return sr[3];
}

function getMinute(sr) {
    return sr[4];
}

function getSecond(sr) {
    return sr[5];
}

function getTimeString(sr) {
    return '' + getYear(sr) + '-' + getMonth(sr) +
	'-' + getDayOfMonth(sr) + ' ' + getHour(sr) + ':' +
	getMinute(sr) + ':' + getSecond(sr);
}


function getGpsSpeed(sr) {
    return sr[6];
}

function getAWA(sr) {
    return sr[7];    
}

function getAWS(sr) {
    return sr[8];
}

function getTWA(sr) {
    return sr[9];
}

function getTWAMax180(sr) {
    var twa = getTWA(sr);
    if (twa > 180) {
	return twa - 360;
    }
    return twa;
}


function getTWS(sr) {
    return sr[10];
}

function getMagHdg(sr) {
    return sr[11];
}

function getWatSpeed(sr) {
    return sr[12];
}

function getGpsBearing(sr) {
    return sr[13];    
}

function getPosLatDeg(sr) {
    return sr[14];
}

function getPosLatMin(sr) {
    return sr[15];
}

function getPosLatMc(sr) {
    return sr[16];    
}

function getPosLonDeg(sr) {
    return sr[17];    
}

function getPosLonMin(sr) {
    return sr[18];    
}

function getPosLonMc(sr) {
    return sr[19];    
}

function getCWD(sr) {
    return sr[20];    
}

function getWD(sr) {
    return sr[21];    
}

function getDays(sr) {
    return sr[22];    
}

function getSeconds(sr) {
    return 24*3600*sr[22];    
}






//
function getMinTWA(data) {
    var minv = 1.0e30;
    for (var i = 0; i < data.length; i++) {
	minv = Math.min(minv, getTWA(data[i]));
    }
    return minv;
}

function getMaxTWA(data) {
    var maxv = -1.0e30;
    for (var i = 0; i < data.length; i++) {
	maxv = Math.max(maxv, getTWA(data[i]));
    }
    return maxv;
}

