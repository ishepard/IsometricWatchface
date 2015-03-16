var battery = localStorage.getItem("battery");
var date = localStorage.getItem("date");
var temperature_unit = localStorage.getItem("temperature_unit");
var weather = JSON.parse(localStorage.getItem("weather"));
var temperature;
var conditions;

function locationSuccess(pos) {
  var req = new XMLHttpRequest();
  var request_city = new XMLHttpRequest();
  var response;
  var response_city;
  var city;
  var countrycode;
  var street;

  var yahoo_get_city = encodeURIComponent("select countrycode, city, street from geo.placefinder where text=\"" + pos.coords.latitude + "," + pos.coords.longitude + "\" and gflags=\"R\"");
  var yahoo_city = "https://query.yahooapis.com/v1/public/yql?q=" + yahoo_get_city + "&format=json&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys";
  request_city.open('GET', yahoo_city , false);
  request_city.send();
  if (request_city.readyState == 4) {
      if(request_city.status == 200) {
        response_city = JSON.parse(request_city.responseText);
        city = response_city.query.results.Result.city;
        countrycode = response_city.query.results.Result.countrycode;
        street = response_city.query.results.Result.street;
        
      } else {
        console.log("Error");
      }
  }

  if (weather){
    var unit = 'c';
    if (temperature_unit == 'f'){
      unit = 'f';
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

          // console.log("Temperature= " + temp);
          // console.log("Conditions code= " + cond);
          temperature = parseInt(temp);
          conditions = parseInt(cond);
          var dictionary = {
          "KEY_TEMPERATURE": temperature,
          "DISP_TEMPERATURE_UNIT": temperature_unit,
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

function set_form(){
  output_form = {};
  if (battery == 'none'){
    output_form['battery_none'] = true;
  } else if (battery == 'icon'){
    output_form['battery_icon'] = true;
  } else if (battery == 'percentage'){
    output_form['battery_percentage'] = true;
  } else if (battery == 'both'){
    output_form['battery_both'] = true;
  }
  if (date == 'none'){
    output_form['date_none'] = true;
  } else if (date == 'number_weekday'){
    output_form['number_weekday'] = true;
  } else if (date == 'number_month'){
    output_form['number_month'] = true;
  } else if (date == 'all_date'){
    output_form['all_date'] = true;
  }
  if (weather){
    output_form['weather'] = 'Yes';
    if (temperature_unit == 'c'){
      output_form['celsius'] = true;
    } else if (temperature_unit == 'f'){
      output_form['fahrenheit'] = true;
    } else if (temperature_unit == 'k'){
      output_form['kelvin'] = true;
    }
  } else {
    output_form['weather'] = 'No';
  }
  return output_form;
}

Pebble.addEventListener('showConfiguration', function(e) {
  // console.log(encodeURIComponent(JSON.stringify(set_form())));
  var result = set_form();
  Pebble.openURL('http://www.davidespadini.it/form_isometric.html?' + encodeURIComponent(JSON.stringify(result)));
});

Pebble.addEventListener('webviewclosed',
  function(e) {
    var configuration = JSON.parse(decodeURIComponent(e.response));

    if (configuration.battery_none == true){
      battery = 'none';
    } else if (configuration.battery_icon == true){
      battery = 'icon';
    } else if (configuration.battery_percentage == true){
      battery = 'percentage';
    } else if (configuration.battery_both == true){
      battery = 'both';
    }
    if (configuration.date_none == true){
      date = 'none';
    } else if (configuration.number_weekday == true){
      date = 'number_weekday';
    } else if (configuration.number_month == true){
      date = 'number_month';
    } else if (configuration.all_date == true){
      date = 'all_date';
    }
    if (configuration.weather == 'yes'){
      weather = true;
      if (configuration.celsius == true){
        temperature_unit = 'c';
      } else if (configuration.fahrenheit == true){
        temperature_unit = 'f';
      } else if (configuration.kelvin == true){
        temperature_unit = 'k';
      }
    } else {
      weather = false;
    }
    localStorage.setItem("battery", battery);
    localStorage.setItem("date", date);
    localStorage.setItem("weather", weather);
    localStorage.setItem("temperature_unit", temperature_unit);
    getWeather();
    var dictionary = {
      "DISP_BATTERY": battery,
      "DISP_DATE": date,
      "DISP_WEATHER": weather
      };
    
    // Send to Pebble
    Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log("Display information sent to Pebble successfully!");
      },
      function(e) {
        console.log("Error sending display info to Pebble!");
      }
    );
  }
);


