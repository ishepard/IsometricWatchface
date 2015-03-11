function locationSuccess(pos) {
  var req = new XMLHttpRequest();
  var request_city = new XMLHttpRequest();
  var response;
  var response_city;
  var city;
  var countrycode;
  var street;
  console.log("lat:" + pos.coords.latitude + " long:" + pos.coords.longitude);

  var yahoo_get_city = encodeURIComponent("select countrycode, city, street from geo.placefinder where text=\"" + pos.coords.latitude + "," + pos.coords.longitude + "\" and gflags=\"R\"");
  // var yahoo_get_city = encodeURIComponent("select countrycode, city, street from geo.placefinder where text=\"40.7141667,-74.0063889 \" and gflags=\"R\"");
  var yahoo_city = "https://query.yahooapis.com/v1/public/yql?q=" + yahoo_get_city + "&format=json&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys";
  request_city.open('GET', yahoo_city , false);
  request_city.send();
  if (request_city.readyState == 4) {
      if(request_city.status == 200) {
        // console.log("request_city.RESPONSETEXT: " + request_city.responseText);
        response_city = JSON.parse(request_city.responseText);
        city = response_city.query.results.Result.city;
        countrycode = response_city.query.results.Result.countrycode;
        street = response_city.query.results.Result.street;
        
      } else {
        console.log("Error");
      }
  }
  console.log("city: " + city);
  console.log("countrycode: " + countrycode);
  console.log("street: " + street);
  var unit = "c";
  if (countrycode == "US"){
    unit = "f";
  }

  var yahoo_weather_query = encodeURIComponent("select item.condition from weather.forecast where woeid in (select woeid from geo.places(1) where text=\"" + city + "," + countrycode + "\")");
  var yahoo_weather = "https://query.yahooapis.com/v1/public/yql?q=" + yahoo_weather_query + "%20and%20u%3D%22" + unit + "%22&format=json&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys";
  
  req.open('GET', yahoo_weather, true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        // console.log("REQ.RESPONSETEXT: " + req.responseText);
        response = JSON.parse(req.responseText);
        var temp = response.query.results.channel.item.condition.temp;
        var cond = response.query.results.channel.item.condition.code;

        console.log("Temperature= " + temp);
        console.log("Conditions code= " + cond);
        var temperature = parseInt(temp);
        var conditions = parseInt(cond);
        var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_TEMP_UNIT": unit,
        "KEY_CONDITIONS": conditions,
      };
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
        
      } else {
        console.log("Error");
      }
    }
  };
  req.send(null);
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }                     
);

