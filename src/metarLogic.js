//--------GeoLocation Var
var getNearest = localStorage.getItem('getNearest');
if (getNearest === 'true') {
  getNearest = true;
} else {
  getNearest = false;
}
//getNearest = true;

//--------Declare vars to be returned to the Pebble
var stationID = localStorage.getItem('stationID');
if ((stationID === null)||(stationID.length != 4)) { stationID = 'KJFK'; }
var altimeter = '';
var temperature = '';
var dewpoint = '';
var time = '';
var parserType = '';
var windDirection = '';
var windSpeed = '';
var windGust = '';
var visibility = '';
var clouds = '';
var otherWX = '';
var flightCondition = '';
var time = new Date();
var gmtOffset = time.getTimezoneOffset();
//console.log('TZOffset = ' + gmtOffset.toString());
var issueInts = [];

//--------Vars to preserve parity with other versions of the parser
var remarks = '';
var runwayVisibility = '';
var windVariableDirection = [];
var cloudList = [];
var otherList = [];
var cloudCodeList = ['FEW','SCT','BKN','OVC'];
var units = [];

//--------Station Location Identifiers
var RegionsUsingUSParser = ['C', 'K', 'M', 'P', 'T'];
var RegionsUsingInternationalParser = ['A', 'B', 'D', 'E', 'F', 'G', 'H', 'L', 'M', 'N', 'O', 'R', 'S', 'U', 'V', 'W', 'Y', 'Z'];
//The Central American region is split. Therefore we need to use the first two letters
var MStationsUsingUSParser = ['MB', 'MD', 'MK', 'MM', 'MT', 'MU', 'MW', 'MY'];
var MStationsUsingInternationalParser = ['MG', 'MH', 'MN', 'MP', 'MR', 'MS', 'MZ'];

function printList(someList) {
  var ret = '';
  for (var i=0; i<someList.length; i++) {
    ret += someList[i] + ' ';
  }
  return ret;
}

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function parseXML(xml) {
  var json = JSON.parse(xml);
  console.log(JSON.stringify(json));
  //console.log(json.weatherObservation.ICAO);
  stationID = json.weatherObservation.ICAO;
  //console.log(json.weatherObservation.observation);
  return json.weatherObservation.observation;
}

function parseHTML(html) {
  var rawMETAR = '';
  if (html.indexOf('<code>'+stationID+' ') != -1) {
    var startIndex = html.indexOf('<code>'+stationID+' ') + 6;
    var endIndex = html.substring(startIndex).indexOf('<') + startIndex;
    rawMETAR = html.substring(startIndex, endIndex);
  } else {
    console.log('Could not find start of METAR :(');
  }
  return rawMETAR;
}

function _clearVariables() {
  altimeter = '';
  temperature = '';
  dewpoint = '';
  time = '';
  parserType = '';
  windDirection = '';
  windSpeed = '';
  visibility = '';
  clouds = '';
  otherWX = '';
  flightCondition = '';
  remarks = '';
  runwayVisibility = '';
  windVariableDirection = [];
  cloudList = [];
  otherList = [];
}

function _getRemarks(rawText) {
  remarks = '';
  rawText = rawText.replace('?',' ');
  if (rawText.indexOf('BECMG') != -1) {
    remarks = rawText.substring(rawText.indexOf('BECMG'));
    return rawText.substring(0,rawText.indexOf('BECMG')-1).trim().split(' ');
  } else if (rawText.indexOf('TEMPO') != -1) {
    remarks = rawText.substring(rawText.indexOf('TEMPO'));
    return rawText.substring(0,rawText.indexOf('TEMPO')-1).trim().split(' ');
  } else if (rawText.indexOf('TEMP') != -1) {
    remarks = rawText.substring(rawText.indexOf('TEMP'));
    return rawText.substring(0,rawText.indexOf('TEMP')-1).trim().split(' ');
  } else if (rawText.indexOf('NOSIG') != -1) {
    remarks = rawText.substring(rawText.indexOf('NOSIG'));
    return rawText.substring(0,rawText.indexOf('NOSIG')-1).trim().split(' ');
  }	else if (rawText.indexOf('RMK') != -1) {
    remarks = rawText.substring(rawText.indexOf('RMK') + 4);
    return rawText.substring(0,rawText.indexOf('RMK')-1).trim().split(' ');
  }
	return rawText.trim().split(' ');
}

function _sanitize(rawList) {
  for (var i=rawList.length-1 ; i > -1 ; i--) {
    if ((rawList[i].length > 4) && (rawList[i].charAt(0) == 'R') && ((rawList[i].charAt(3) == '/') || (rawList[i].charAt(4) == '/')) && (!isNaN(rawList[i].substring(1,3)))) {
			runwayVisibility = rawList[i];
      rawList.splice(i,1);
    } else if (((rawList[i].length == 4) || (rawList[i].length == 6)) && (rawList[i].substring(0,2) == 'RE')) {
      rawList.splice(i,1);
    } else if ((i !== 0) && ((rawList[i] == 'SM') && ((!isNaN(rawList[i-1])))) || ((rawList[i] == 'KT') && ((!isNaN(rawList[i-1].substring(0,5))))) || ((!isNaN(rawList[i])) && (cloudCodeList.indexOf(rawList[i-1]) != -1))) {
      rawList[i-1] += rawList[i];
      rawList.splice(i,1);
    } else if (['AUTO' , 'COR' , 'NSC' , 'CLR' , 'SKC' , 'NCD' , '$' , 'KT' , 'M'].indexOf(rawList[i]) != -1) { rawList.splice(i,1); }
  }
  return rawList;
}

function _getAltimeterUS(rawList) {
  if (rawList.length === 0) { return rawList; }
  if (rawList[rawList.length-1].charAt(0) == 'A') {
    altimeter = rawList.pop().substring(1);
  } else if ((rawList.length > 0) && (rawList[rawList.length-1].length == 4) && (!isNaN(rawList[rawList.length-1]))) {
    altimeter = rawList.pop();
  }
  console.log('Altimeter: ' + altimeter);
  return rawList;
}

function _getAltimeterInternational(rawList) {
  if (rawList.length === 0) { return rawList; }
  if (rawList[rawList.length-1].charAt(0) == 'Q') {
    altimeter = rawList.pop().substring(1);
  } else if ((rawList.length > 0) && (rawList[rawList.length-1].length == 4) && (!isNaN(rawList[rawList.length-1]))) {
    altimeter = rawList.pop();
  }
  console.log('Altimeter: ' + altimeter);
  return rawList;
}

function _getTemperatureAndDewpoint(rawList) {
  if (rawList.length === 0) { return rawList; }
  if (rawList[rawList.length-1].indexOf('/') != -1) {
    var tempAndDewList = rawList.pop().split('/');
    temperature = tempAndDewList[0];
    dewpoint = tempAndDewList[1];
  }
  console.log('Temperature: ' + temperature);
  console.log('Dewpoint: ' + dewpoint);
  return rawList;
}

function _getStationAndTime(rawList) {
  //station = rawList.shift();
  rawList.shift();
  if (rawList.length === 0) { return rawList; }
  if ((rawList[0].length == 7) && (rawList[0].substring(6) == 'Z') && (!isNaN(rawList[0].substring(0,6)))) {
    time = rawList.shift();
  }
  console.log('Station: ' + stationID);
  console.log('Time: ' + time);
  return rawList;
}

function _getWindInfo(rawList) {
  if ((rawList.length === 0) || (rawList[0].length < 5)) { return rawList; }
  var wind = "";
  if ((rawList[0].substring(rawList[0].length-2) == 'KT') || (rawList[0].substring(rawList[0].length-3) == 'KTS') || ((rawList[0].length == 5) && (!isNaN(rawList[0]))) || ((rawList[0].length > 7) && (rawList[0].indexOf('G') != -1) && (rawList[0].indexOf('/') == -1) && (rawList[0].indexOf('MPS') == -1))) {
    wind = rawList.shift();
    windDirection = wind.substring(0,3);
    windSpeed = wind.substring(3,5);
    if (wind.indexOf('G') != -1) {
      windGust = wind.substring(6,8);
    }
  } else if (rawList[0].substring(rawList[0].length-3) == 'MPS') {
    units.shift();
    units.unshift('M/S');
    wind = rawList.shift();
    windDirection = wind.substring(0,3);
    windSpeed = wind.substring(3,5);
    if (wind.indexOf('G') != -1) {
      windGust = wind.substring(6,8);
    }
  } else if ((rawList[0].length > 5) && (rawList[0].charAt(3) == '/') && (!isNaN(rawList[0].substring(0,3))) && (!isNaN(rawList[0].substring(4,6)))) {
    wind = rawList.shift();
    windDirection = wind.substring(0,3);
    windSpeed = wind.substring(4,6);
    if (wind.indexOf('G') != -1) {
      windGust = wind.substring(7,9);
    }
  }
  if ((rawList.length !== 0) && (1 < rawList[0].length < 4) && (rawList[0].substring(0,1) == 'G') && (!isNaN(rawList[0].substring(1)))) {
    windGust = rawList.shift().substring(1);
  }
  if ((rawList.length !== 0) && (rawList[0].length == 7) && (rawList[0].charAt(3) == 'V') && (!isNaN(rawList[0].substring(0,3))) && (!isNaN(rawList[0].substring(4)))) {
    windVariableDirection = rawList.shift().split('V');
  }
  console.log('Wind Direction: ' + windDirection);
  console.log('Wind Speed: ' + windSpeed);
  console.log('Wind Gust: ' + windGust);
  console.log('Wind Variable Direction: ' + printList(windVariableDirection));
  return rawList;
}

function _getVisibilityUS(rawList) {
  if (rawList.length === 0) { return rawList; }
  if (rawList[0].indexOf('SM') != -1) {
    if (rawList[0].indexOf('/') == -1) {
      visibility = parseInt(rawList[0].substring(0,rawList[0].indexOf('SM'))).toString();
    } else {
      visibility = rawList[0].substring(0,rawList[0].indexOf('SM'));
    }
    rawList.shift();
  } else if (rawList[0] == '9999') {
    visibility = '10';
    rawList.shift();
  } else if ((rawList.length > 1) && (rawList[1].indexOf('SM') != -1) && (!isNaN(rawList[0]))) {
    var vis1 = rawList.shift();
    var vis2 = rawList[0].substring(0,rawList[0].indexOf('SM')).split('/');
    rawList.shift();
    visibility = (parseInt(vis1)*parseInt(vis2[1])+parseInt(vis2[0])).toString() + '/' + vis2[1];
  }
  console.log('Visibility: ' + visibility);
  return rawList;
}

function _getVisibilityInternational(rawList) {
  if (rawList.length === 0) { return rawList; }
  if ((rawList[0].length == 4) && (!isNaN(rawList[0]))) {
    visibility = rawList.shift();
  } /*else if ((rawList[0].length == 5) && (!isNaN(rawList[0].substring(0,4))) && (isNaN(rawList[0].charAt(4)))) {
    visibility = rawList.shift().substring(0,4);
  }*/
  console.log('Visibility: ' + visibility);
  return rawList;
}

function sanitizeCloud(cloud) {
  if (cloud.length < 4) { return cloud; }
  if ((isNaN(cloud.charAt(3))) && (cloud.charAt(3) != '/')) {
    if (cloud.charAt(3) == 'O') {
      cloud = cloud.substring(0,3) + '0' + cloud.substring(4);
    } else {
      cloud = cloud.substring(0,3) + cloud.substring(4) + cloud.charAt(3);
    }
  }
  return cloud;
}

function splitCloud(cloud, beginsWithVV) {
  var newSplitCloud = [];
  cloud = sanitizeCloud(cloud);
  if (beginsWithVV) {
    newSplitCloud.push(cloud.substring(0,2));
    cloud = cloud.substring(2);
  }
  while (cloud.length >= 3) {
    newSplitCloud.push(cloud.substring(0,3));
    cloud = cloud.substring(3);
  }
  if (cloud.length !== 0) { newSplitCloud.push(cloud); }
  return newSplitCloud;
}

function _getClouds(rawList) {
  if (rawList.length === 0) { return rawList; }
  for (var i=rawList.length-1 ; i > -1 ; i--) {
    if ((rawList[i].length > 5) && (cloudCodeList.indexOf(rawList[i].substring(0,3)) != -1)) {
      cloudList.push(splitCloud(rawList[i], false));
      rawList.splice(i,1);
    } else if ((rawList[i].length > 4) && (rawList[i].substring(0,2) == 'VV')) {
      cloudList.push(splitCloud(rawList[i], true));
      rawList.splice(i,1);
    }
  }
  cloudList.reverse();
  console.log('Cloud List: ' + printList(cloudList));
  return rawList;
}

function parseUSVariant(rawText) {
  console.log('Using American Parser');
  _clearVariables();
  parserType = 'N';
  units = ['KT','C','SM','FT','inHg'];
  var rawList = _getRemarks(rawText);
  console.log('Remarks: ' + remarks);
  console.log('List before sanitization: ' + printList(rawList));
  rawList = _sanitize(rawList);
  console.log('List after sanitization:  ' + printList(rawList));
  rawList = _getAltimeterUS(rawList);
  rawList = _getTemperatureAndDewpoint(rawList);
  rawList = _getStationAndTime(rawList);
  rawList = _getWindInfo(rawList);
  rawList = _getVisibilityUS(rawList);
  otherList = _getClouds(rawList);
  console.log('Other List: ' + printList(otherList));
}

function parseInternationalVariant(rawText) {
  console.log('Using International Parser');
  _clearVariables();
  parserType = 'I';
  units = ['KT','C','M','FT','hPa'];
  var rawList = _getRemarks(rawText);
  console.log('Remarks: ' + remarks);
  console.log('List before sanitization: ' + printList(rawList));
  rawList = _sanitize(rawList);
  console.log('List after sanitization:  ' + printList(rawList));
  rawList = _getAltimeterInternational(rawList);
  rawList = _getTemperatureAndDewpoint(rawList);
  rawList = _getStationAndTime(rawList);
  rawList = _getWindInfo(rawList);
  if ((rawList.length > 0) && (rawList[0] == 'CAVOK')) {
    visibility = '9999';
    rawList.shift();
  } else {
    rawList = _getVisibilityInternational(rawList);
    rawList = _getClouds(rawList);
  }
  otherList = rawList;
  console.log('Other List: ' + printList(otherList));
}

function parseMETAR(rawText) {
  if (rawText.length < 2) { return null; }
  if (RegionsUsingUSParser.indexOf(rawText.charAt(0)) != -1) { return parseUSVariant(rawText); }
  else if (RegionsUsingInternationalParser.indexOf(rawText.charAt(0)) != -1) { return parseInternationalVariant(rawText); }
  else if (MStationsUsingUSParser.indexOf(rawText.substring(0,2)) != -1) { return parseUSVariant(rawText); }
  else if (MStationsUsingInternationalParser.indexOf(rawText.substring(0,2)) != -1) { return parseInternationalVariant(rawText); }
}

function getCeiling(clouds) {
  for (var i=0 ; i<clouds.length ; i++) {
    if ((!isNaN(clouds[i][1])) && (['OVC','BKN','VV'].indexOf(clouds[i][0]) != -1)) {
      return clouds[i];
    }
  }
  return null;
}

function getFlightRules(vis , splitCloud) {
  if (vis === '') {
    return 'IFR';
  } else if (vis.indexOf('/') != -1) {
    if (vis.charAt(0) == 'M') {
      vis = 0;
    } else {
      vis = parseInt(vis.split('/')[0]) / parseInt(vis.split('/')[1]);
    }
  } else if ((vis.length == 4) && (!isNaN(vis))) {
    vis = parseInt(vis) * 0.000621371;
  } else {
    vis = parseInt(vis);
  }
  var cld = 99;
  if ((splitCloud) && (splitCloud.length > 1)) {
    cld = parseInt(splitCloud[1]);
  }
  console.log("Getting rules with '" + vis.toString() + "' and '" + cld.toString() + "'");
  if ((vis < 5) || (cld < 30)) {
    if ((vis < 3) || (cld < 10)) {
      if ((vis < 1) || (cld < 5)) {
        return 'LIFR';
      }
      return 'IFR';
    }
    return 'MVFR';
  }
  return 'VFR';
}

function prepareDataForPebble() {
  var i;
  //Time
  time = time.substring(2,4) + ':' + time.substring(4,6);
  issueInts = time.split(':');
  console.log('Time: ' + time);
  console.log('Issue Ints: ' + issueInts.toString());
  //Wind
  if ((windSpeed == '00') || (windSpeed === '')) {
    windSpeed = 'CALM';
    windDirection = '---\xB0';
  } else {
    windDirection += '\xB0';
    if (windGust !== ''){
      windSpeed += 'G' + windGust + units[0];
    } else {
      windSpeed += units[0];
    }
  }
  //Temperature and Dewpoint
  temperature = temperature.replace('M','-');
  dewpoint = dewpoint.replace('M','-');
  //Altimeter
  if (altimeter.length == 4) { altimeter = altimeter.substring(0,2) + '.' + altimeter.substring(2); }
  //Visibility
  if (visibility == '9999') { visibility = '10'; }
  if (visibility.indexOf('/') != -1) {   // 5/4 -> 1.25
    var visList = visibility.split('/');
    visibility = (parseInt(visList[0]) / parseInt(visList[1])).toString();
  } else if ((visibility.length == 4) && (!isNaN(visibility))) {   // 4500 -> 4.500 -> 4.5
    visibility = parseFloat(visibility.charAt(0) + '.' + visibility.substring(1)).toString();
  }
  if (visibility.length > 3) {
    if (visibility[0] == '0') {   // 0.75 -> .75
      visibility = visibility.substring(1,4);
    } else {   // 1.75 -> 1.7
      visibility = visibility.substring(0,3);
    }
  }
  //Clouds
  if ((cloudList.length == 1) && (cloudList[0].length == 3) && (cloudList[0][2] !== '')) {
    clouds = cloudList[0].join('');
  } else {
    for (i=0; ((i<2) && (i<cloudList.length)) ; i++) { clouds += cloudList[i][0] + cloudList[i][1] + ' '; }
  }
  clouds = clouds.trim();
  if (clouds === '') { clouds = 'SKY  CLR'; }
  //Other WX
  for (i=0; ((otherWX.length < 9) && (i<otherList.length)) ; i++) { otherWX += otherList[i] + ' '; }
  otherWX = otherWX.trim();
  //Amend Other WX
  if (otherWX === '') {
    if (windVariableDirection.length == 2) {
      otherWX = windVariableDirection[0] + 'V' + windVariableDirection[1];
    } else if ((otherWX.length < 4) && (remarks.indexOf('NOSIG') != -1)) {
      otherWX = 'NOSIG';
    } else {
      otherWX = 'No  WX';
    }
  } else if ((otherWX.length < 4) && (remarks.indexOf('NOSIG') != -1)) {
    otherWX += ' NOSIG';
  }
}

function sendDictionaryToPebble(dictionary) {
  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('METAR info sent to Pebble successfully!');
    },
    function(e) {
      console.log('Error sending METAR info to Pebble!');
    }
  );
}

/*
   This function takes the raw html either from aviationweather.gov or geonames.org ,
   extracts the METAR report, parses it, and returns the value dictionary to the Pebble
*/
function parseMetarAndSendToPebble(responseText) {
  console.log('Recieved Response');
  var metar = '';
  if (getNearest) {
    metar = parseXML(responseText);
  } else {
    metar = parseHTML(responseText);
  }
  //var metar = '';
  console.log(metar);
  if (metar === '') {
    time = ':(';
    windSpeed = "I COULDN'T" ;
    otherWX = 'GET YOUR';
    clouds = 'STATION';
  } else {
    parseMETAR(metar);
    flightCondition = getFlightRules(visibility , getCeiling(cloudList));
    console.log('Flight Rules = ' + flightCondition);
    prepareDataForPebble();
  }
  
  var dictionary = {
    'KEY_STATION': stationID,
    'KEY_CONDITION': flightCondition,
    'KEY_ISSUE_TIME': time,
    'KEY_ISSUE_HOUR': parseInt(issueInts[0]),
    'KEY_ISSUE_MINUTE': parseInt(issueInts[1]),
    'KEY_WIND_DIRECTION': windDirection,
    'KEY_WIND_SPEED': windSpeed,
    'KEY_TEMPERATURE': temperature,
    'KEY_DEWPOINT': dewpoint,
    'KEY_ALTIMETER': altimeter,
    'KEY_VISIBILITY': visibility,
    'KEY_OTHER_WX': otherWX,
    'KEY_CLOUDS': clouds,
    'KEY_OFFSET': gmtOffset
  };
  //console.log('Dictionary being returned: ' + JSON.stringify(dictionary));

  // Send to Pebble
  sendDictionaryToPebble(dictionary);
}

function locationSuccess(pos) {
  var latitude = pos.coords.latitude;
  var longitude = pos.coords.longitude;
  console.log('Latitude = ' + latitude.toString());
  console.log('Longitude = ' + longitude.toString());
  var url = 'http://api.geonames.org/findNearByWeatherJSON?lat=' + latitude + '&lng=' + longitude + '&username=flyinactor91';
  xhrRequest(url, 'GET', 
    function(responseText) {
      parseMetarAndSendToPebble(responseText);
    }      
  );
}

function locationError(err) {
  console.log('Error requesting location!');
  var dictionary = {
    'KEY_STATION': stationID,
    'KEY_CONDITION': flightCondition,
    'KEY_ISSUE_TIME': ':(',
    'KEY_ISSUE_HOUR': '00',
    'KEY_ISSUE_MINUTE': '00',
    'KEY_WIND_DIRECTION': windDirection,
    'KEY_WIND_SPEED': 'I NEED',
    'KEY_TEMPERATURE': temperature,
    'KEY_DEWPOINT': dewpoint,
    'KEY_ALTIMETER': altimeter,
    'KEY_VISIBILITY': visibility,
    'KEY_OTHER_WX': 'LOCATION',
    'KEY_CLOUDS': 'PERMISION',
    'KEY_OFFSET': gmtOffset
  };
  sendDictionaryToPebble(dictionary);
}

function useGeoURL() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

function updateMETAR() {
  console.log('getNearest = ' + getNearest.toString());
  if (getNearest) {
    console.log('Getting nearest METAR station');
    useGeoURL();
  } else {
    var url = 'http://www.aviationweather.gov/metar/data?ids='+stationID+'&format=raw&date=0&hours=0';
    xhrRequest(url, 'GET', 
      function(responseText) {
        console.log('Getting set METAR station');
        parseMetarAndSendToPebble(responseText);
      }      
    );
  }
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
    updateMETAR();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    updateMETAR();
  }                     
);

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  console.log('Now showing config page');
  Pebble.openURL('http://mdupont.com/Pebble-Config/pebble-metar-watchface-setup-2-1.html');
});

Pebble.addEventListener('webviewclosed',
  function(e) {
    console.log(e.response.length);
    console.log('Configuration window returned: ' + e.response);
    if (e.response.length !== 0) {
      var options = JSON.parse(decodeURIComponent(e.response));
      console.log('Options = ' + JSON.stringify(options));
      if (options.stationID !== '') { localStorage.setItem('stationID', options.stationID); }
      localStorage.setItem('getNearest', options.getNearest);
      //updateMETAR();
    }
  }
);