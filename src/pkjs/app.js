/* AVWX - app.js
 * Michael duPont
 * Displays the current flight rules for a given station
 * Ties into avwx.rest, my public aviation weather service
*/

//--------GeoLocation
var getNearest = localStorage.getItem('getNearest');
if (getNearest === 'true') {
  getNearest = true;
} else {
  getNearest = false;
}

//--------Station ID
var stationID = localStorage.getItem('stationID');
if ((stationID === null)||(stationID.length != 4)) { stationID = 'KJFK'; }

//--------Dark Background
var darkBackground = localStorage.getItem('darkBackground');
if (darkBackground === 'true') {
  darkBackground = true;
} else {
  darkBackground = false;
}

//--------GMT Offset
var datetime = new Date();
var gmtOffset = datetime.getTimezoneOffset();

//Previous Display Data
//If the phone cannot fetch new data, show the previous data and change 'KEY_SUCCESS' to false
//If no previous data, return the default display data
function getLastData() {
  var retDict = localStorage.getItem('previousdata');
  if (retDict === null) {
    retDict = {
        'KEY_STATION': stationID,
        'KEY_CONDITION': '',
        'KEY_ISSUE_TIME': ':(',
        'KEY_ISSUE_HOUR': '00',
        'KEY_ISSUE_MINUTE': '00',
        'KEY_WIND_DIRECTION': '',
        'KEY_WIND_SPEED': "I COULDN'T",
        'KEY_TEMPERATURE': '',
        'KEY_DEWPOINT': '',
        'KEY_ALTIMETER': '',
        'KEY_VISIBILITY': '',
        'KEY_OTHER_WX': 'UPDATE',
        'KEY_CLOUDS': '',
        'KEY_OFFSET': gmtOffset,
        'KEY_DARKBG': darkBackground,
        'KEY_SUCCESS': false
      };
  } else {
    retDict = JSON.parse(retDict);
    retDict.KEY_SUCCESS = false;
  }
  return retDict;
}

/******************************** Data formatting *********************************/

function createPebbleDict(wxDict) {
  var i = 0;
  //Wind
  var time = wxDict.Time;
  if (time.length > 5) { time = time.substring(2,4) + ':' + time.substring(4,6); }
  var issueInts = time.split(':');
  //Wind
  var windSpeed = wxDict['Wind-Speed'];
  var windDirection = wxDict['Wind-Direction'];
  if ((windSpeed == '00') || (windSpeed === '')) {
    windSpeed = 'CALM';
    windDirection = '---\xB0';
  } else {
    windDirection += '\xB0';
    if (wxDict['Wind-Gust'] !== ''){
      windSpeed += 'G' + wxDict['Wind-Gust'] + wxDict.Units['Wind-Speed'];
    } else {
      windSpeed += wxDict.Units['Wind-Speed'];
    }
  }
  //Altimeter
  var altimeter = wxDict.Altimeter;
  if ((altimeter.length == 4) && (wxDict.Units.Altimeter != 'hPa')) {
    altimeter = altimeter.substring(0,2) + '.' + altimeter.substring(2);
  }
  //Visibility
  var visibility = wxDict.Visibility;
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
  var cloudList = wxDict['Cloud-List'];
  var clouds = '';
  if ((cloudList.length == 1) && (cloudList[0].length == 3) && (cloudList[0][2] !== '')) {
    clouds = cloudList[0].join('');
  } else {
    for (i=0; ((i<2) && (i<cloudList.length)) ; i++) { clouds += cloudList[i][0] + cloudList[i][1] + ' '; }
  }
  clouds = clouds.trim();
  if (clouds === '') { clouds = 'SKY  CLR'; }
  //Other WX
  var otherWX = '';
  for (i=0; ((otherWX.length < 9) && (i<wxDict['Other-List'].length)) ; i++) { otherWX += wxDict['Other-List'][i] + ' '; }
  otherWX = otherWX.trim();
  //Amend Other WX
  if (otherWX === '') {
    if (('Wind-Variable-Dir' in wxDict) && (wxDict['Wind-Variable-Dir'].length == 2)) {
      otherWX = wxDict['Wind-Variable-Dir'][0] + 'V' + wxDict['Wind-Variable-Dir'][1];
    } else if ((otherWX.length < 4) && (wxDict.Remarks.indexOf('NOSIG') != -1)) {
      otherWX = 'NOSIG';
    } else {
      otherWX = 'No  WX';
    }
  } else if ((otherWX.length < 4) && (wxDict.Remarks.indexOf('NOSIG') != -1)) {
    otherWX += ' NOSIG';
  }
  //Create Dict
  console.log('Making Dict');
  var retDict = {
    'KEY_STATION':wxDict.Station,
    'KEY_CONDITION':wxDict['Flight-Rules'],
    'KEY_ISSUE_TIME':time,
    'KEY_ISSUE_HOUR':parseInt(issueInts[0]),
    'KEY_ISSUE_MINUTE':parseInt(issueInts[1]),
    'KEY_WIND_DIRECTION': windDirection,
    'KEY_WIND_SPEED': windSpeed,
    'KEY_TEMPERATURE': wxDict.Temperature.replace('M','-'),
    'KEY_DEWPOINT': wxDict.Dewpoint.replace('M','-'),
    'KEY_ALTIMETER': altimeter,
    'KEY_VISIBILITY': visibility,
    'KEY_OTHER_WX': otherWX,
    'KEY_CLOUDS': clouds,
    'KEY_OFFSET': gmtOffset,
    'KEY_DARKBG': darkBackground,
    'KEY_SUCCESS': true
  };
  //This replacement is used during design work and has the most info per field
  /*retDict = {
    'KEY_STATION':wxDict.Station,
    'KEY_CONDITION':'MVFR',
    'KEY_ISSUE_TIME':time,
    'KEY_ISSUE_HOUR':parseInt(issueInts[0]),
    'KEY_ISSUE_MINUTE':parseInt(issueInts[1]),
    'KEY_WIND_DIRECTION': '333\xB0',
    'KEY_WIND_SPEED': '12G22kt',
    'KEY_TEMPERATURE': '-04',
    'KEY_DEWPOINT': '-04',
    'KEY_ALTIMETER': '29.92',
    'KEY_VISIBILITY': '4.5',
    'KEY_OTHER_WX': 'VCTS SHRA',
    'KEY_CLOUDS': 'BKN005 OVC010',
    'KEY_OFFSET': gmtOffset,
    'KEY_DARKBG': darkBackground,
    'KEY_SUCCESS': true
  };*/
  localStorage.setItem('previousdata', JSON.stringify(retDict));
  return retDict;
}

/***************************** AVWX fetch functions *******************************/

//Retrieve and parse JSON object for a given url
//Calls handleRequest with fetched object
//@param url The url to fetch
var updateReport = function(url) {
  //url = 'http://195.264.426.231';
  var request = new XMLHttpRequest();
  request.onreadystatechange = function(e) {
    console.log(request.readyState , request.statusText);
    if (request.readyState == 4) {
      if (request.status == 200) {
        console.log(request.responseText);
        if ((request.responseText !== null) && (request.responseText.charAt(0) == '{')) {
          var resp = JSON.parse(request.responseText);
          if (('Error' in resp) || (!('Flight-Rules' in resp))) {
            sendDictionaryToPebble(getLastData());
          } else {
            sendDictionaryToPebble(createPebbleDict(resp));
          }
        } else {
          sendDictionaryToPebble(getLastData());
        }
      } else {
        sendDictionaryToPebble(getLastData());
      }
    }
  };
  console.log('Now Fetching: ' + url);
  request.open('GET', url, true);
  request.send();
};

//Called when position lookup is succesful
//@param pos A Pebble position object
function locationSuccess(pos) {
  var latitude = pos.coords.latitude;
  var longitude = pos.coords.longitude;
  console.log('Latitude = ' + latitude.toString());
  console.log('Longitude = ' + longitude.toString());
  var url = 'http://avwx.rest/api/metar.php?lat=' + latitude.toString() + '&lon=' + longitude.toString() + '&format=JSON';
  updateReport(url);
}

function locationError(err) {
  console.log('Error requesting location! ' + err.toString());
  var dictionary = {
    'KEY_STATION': stationID,
    'KEY_CONDITION': '',
    'KEY_ISSUE_TIME': ':(',
    'KEY_ISSUE_HOUR': '00',
    'KEY_ISSUE_MINUTE': '00',
    'KEY_WIND_DIRECTION': '',
    'KEY_WIND_SPEED': 'I NEED',
    'KEY_TEMPERATURE': '',
    'KEY_DEWPOINT': '',
    'KEY_ALTIMETER': '',
    'KEY_VISIBILITY': '',
    'KEY_OTHER_WX': 'LOCATION',
    'KEY_CLOUDS': 'PERMISION',
    'KEY_OFFSET': gmtOffset,
    'KEY_DARKBG': darkBackground,
    'KEY_SUCCESS': false
  };
  sendDictionaryToPebble(dictionary);
}

//Called when getNearest is true
function useGeoURL() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

/**************************** Pebble comm/listeners ******************************/

//Send a dictionary to the Pebble
function sendDictionaryToPebble(dictionary) {
  console.log(JSON.stringify(dictionary));
  console.log('Sending Dict');
  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Status sent to Pebble successfully!');
    },
    function(e) {
      console.log('Error sending status to Pebble!', e);
    }
  );
}

//Handler for update request
function handleUpdate() {
  console.log('Now updating');
  if (getNearest === true) {
    useGeoURL();
  } else {
    var url = 'http://avwx.rest/api/metar/' + stationID;
    updateReport(url);
  }
}

//Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    handleUpdate();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    handleUpdate();
  }
);

//Listen for when user opens config page
Pebble.addEventListener('showConfiguration', function(e) {
  //Show config page
  console.log('Now showing config page');
  Pebble.openURL('http://mdupont.com/Pebble-Config/pebble-metar-watchface-setup-3-3.html?station=' + stationID + '&near=' + getNearest.toString() + '&back=' + darkBackground.toString());
});

//Listen for when user closes config page
Pebble.addEventListener('webviewclosed',
  function(e) {
    console.log('Configuration window returned: ' + e.response);
    if (e.response.length !== 0) {
      var options = JSON.parse(decodeURIComponent(e.response));
      console.log('Options = ' + JSON.stringify(options));
      if (options.stationID !== '') { localStorage.setItem('stationID', options.stationID); }
      localStorage.setItem('getNearest', options.getNearest);
      localStorage.setItem('darkBackground', options.darkBackground);
    }
  }
);