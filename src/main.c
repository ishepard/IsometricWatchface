#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_TEMP_UNIT 1
#define KEY_CONDITIONS 2

static Window *s_main_window;
static BitmapLayer *s_background_layer;
static BitmapLayer *weather_layer_white;
static BitmapLayer *weather_layer_black;
static BitmapLayer *battery_layer;
static GBitmap *s_background_bitmap;
static GBitmap *battery_bitmap;
static GBitmap *weather_img_white;
static GBitmap *weather_img_black;
static TextLayer *battery_percentage;

GBitmap *gbitmap_number[4];
BitmapLayer *bitmap_layer[4];

typedef enum { 
  LEFT, RIGHT 
} side_t;

const int position[8] = {
  13, 58,
  43, 73,
  73, 73,
  103, 58
};

const int leftNumbers[] = {
  RESOURCE_ID_IMAGE_LEFT0,
  RESOURCE_ID_IMAGE_LEFT1,
  RESOURCE_ID_IMAGE_LEFT2,
  RESOURCE_ID_IMAGE_LEFT3,
  RESOURCE_ID_IMAGE_LEFT4,
  RESOURCE_ID_IMAGE_LEFT5,
  RESOURCE_ID_IMAGE_LEFT6,
  RESOURCE_ID_IMAGE_LEFT7,
  RESOURCE_ID_IMAGE_LEFT8,
  RESOURCE_ID_IMAGE_LEFT9
};

const int rightNumbers[] = {
  RESOURCE_ID_IMAGE_RIGHT0,
  RESOURCE_ID_IMAGE_RIGHT1,
  RESOURCE_ID_IMAGE_RIGHT2,
  RESOURCE_ID_IMAGE_RIGHT3,
  RESOURCE_ID_IMAGE_RIGHT4,
  RESOURCE_ID_IMAGE_RIGHT5,
  RESOURCE_ID_IMAGE_RIGHT6,
  RESOURCE_ID_IMAGE_RIGHT7,
  RESOURCE_ID_IMAGE_RIGHT8,
  RESOURCE_ID_IMAGE_RIGHT9
};

static uint32_t choose_png(char hour_n, side_t side){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Choose png called with %c", hour_n);
  return (side == LEFT) ? leftNumbers[(hour_n - '0')] : rightNumbers[(hour_n - '0')];
}


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  for (int i = 0; i <= 3; i++){
    if (gbitmap_number[i]){
      gbitmap_destroy(gbitmap_number[i]);
      gbitmap_number[i] = NULL;
      bitmap_layer_set_bitmap(bitmap_layer[i], NULL);
    }
    if (i <= 1){
      gbitmap_number[i] = gbitmap_create_with_resource(choose_png(buffer[i], LEFT));
    } else {
      gbitmap_number[i] = gbitmap_create_with_resource(choose_png(buffer[i+1], RIGHT));
    }

    bitmap_layer_set_bitmap(bitmap_layer[i], gbitmap_number[i]);

  }
  
}

static void main_window_load(Window *window) { 
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_background_color(s_background_layer, GColorClear); 
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

  battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  battery_layer = bitmap_layer_create(GRect(119, 5, 25, 10));
  bitmap_layer_set_bitmap(battery_layer, battery_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(battery_layer));

  // Create time TextLayer
  battery_percentage = text_layer_create(GRect(88, 0, 30, 20));
  text_layer_set_background_color(battery_percentage, GColorClear);
  text_layer_set_text_color(battery_percentage, GColorWhite);
  text_layer_set_text(battery_percentage, "100%");

  // Improve the layout to be more like a watchface
  text_layer_set_font(battery_percentage, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(battery_percentage, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(battery_percentage));

  
  // weather_img_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WIND_WHITE);
  weather_layer_white = bitmap_layer_create(GRect(15, 42, 55, 28));
  // bitmap_layer_set_bitmap(weather_layer_white, weather_img_white);
  bitmap_layer_set_compositing_mode(weather_layer_white, GCompOpOr); 
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(weather_layer_white));

  // weather_img_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WIND_BLACK);
  weather_layer_black = bitmap_layer_create(GRect(15, 42, 55, 28));
  // bitmap_layer_set_bitmap(weather_layer_black, weather_img_black);
  bitmap_layer_set_compositing_mode(weather_layer_black, GCompOpClear); 
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(weather_layer_black));


  for (int i = 0; i <= 3; i++){
    bitmap_layer[i] = bitmap_layer_create(GRect(position[2*i], position[2*i +1], 29, 63));
    bitmap_layer_set_background_color(bitmap_layer[i], GColorClear);
    bitmap_layer_set_compositing_mode(bitmap_layer[i], GCompOpOr);
    if (i <= 1){
      bitmap_layer_set_alignment(bitmap_layer[i], GAlignBottomRight);
    } else {
      bitmap_layer_set_alignment(bitmap_layer[i], GAlignTopRight);
    }

    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bitmap_layer[i]));
  }
  update_time();
}

static void main_window_unload(Window *window) {
  bitmap_layer_destroy(s_background_layer);
  for (int i = 0; i <= 3; i++){
    bitmap_layer_destroy(bitmap_layer[i]);
    gbitmap_destroy(gbitmap_number[i]);
  }
  text_layer_destroy(battery_percentage);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 15 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}
static void set_bitmap_from_condition(int condition_code){
  if (weather_img_white || weather_img_black){
    gbitmap_destroy(weather_img_white);
    gbitmap_destroy(weather_img_black);
    weather_img_white = NULL;
    weather_img_black = NULL;
    bitmap_layer_set_bitmap(weather_layer_white, NULL);
    bitmap_layer_set_bitmap(weather_layer_black, NULL);
  }
  if (condition_code == 30){
      weather_img_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PARTLY_CLOUDY_DAY_WHITE);
      weather_img_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PARTLY_CLOUDY_DAY_BLACK);
  } else if (condition_code == 29){
      weather_img_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PARTLY_CLOUDY_NIGHT_WHITE);
      weather_img_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PARTLY_CLOUDY_NIGHT_BLACK);
  } else if ((condition_code >= 26 && condition_code <= 28) || condition_code == 44) {
      weather_img_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLOUDY_WHITE);
      weather_img_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLOUDY_BLACK);
  } else if (condition_code == 31 || condition_code == 33) {
      weather_img_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLEAR_NIGHT_WHITE);
      weather_img_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLEAR_NIGHT_BLACK);
  } else if (condition_code == 32 || condition_code == 34 || condition_code == 36) {
      weather_img_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLEAR_DAY_WHITE);
      weather_img_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLEAR_DAY_BLACK);
  } else if (
        (condition_code >= 41 && condition_code <= 43) || condition_code == 16 ||
        condition_code == 13 || condition_code == 15 || condition_code == 25 || 
        condition_code == 46
      ){
      weather_img_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SNOW_WHITE);
      weather_img_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SNOW_BLACK);
  } else if (condition_code >= 19 && condition_code <= 22){
      weather_img_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FOG_WHITE);
      weather_img_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FOG_BLACK);
  } else if ( 
        condition_code == 3 || condition_code == 4 || condition_code == 9 ||
        condition_code == 11 || condition_code == 12 || condition_code == 40 ||
        condition_code == 45 || condition_code == 47
      ){
      weather_img_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RAIN_WHITE);
      weather_img_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RAIN_BLACK);
  } else if (
        condition_code == 23 || condition_code == 24 || (condition_code >= 0 && condition_code <= 2) ||
        (condition_code >= 37 && condition_code <= 39)
      ){
      weather_img_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WIND_WHITE);
      weather_img_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WIND_BLACK);
  } else if ( 
        (condition_code >= 5 && condition_code <= 8) || condition_code == 10 || 
        (condition_code >= 14 && condition_code <= 17) || condition_code == 18 || 
        condition_code == 35
      ){
      weather_img_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SLEET_WHITE);
      weather_img_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SLEET_BLACK);
  } else {
    return;
  }
  bitmap_layer_set_bitmap(weather_layer_white, weather_img_white);
  bitmap_layer_set_bitmap(weather_layer_black, weather_img_black);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[8];
  static char unit_buffer[8];
  int condition_code = 3200;  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      // snprintf(temperature_buffer, sizeof(temperature_buffer), "%d", (int)t->value->int32);
      break;
    case KEY_TEMP_UNIT:
      //snprintf(unit_buffer, sizeof(unit_buffer), "%s", t->value->cstring);
      break;
    case KEY_CONDITIONS:
      condition_code = (int)t->value->int32;
      // snprintf(conditions_buffer, sizeof(conditions_buffer), "%d", (int)t->value->int32);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    t = dict_read_next(iterator);
  }
  set_bitmap_from_condition(condition_code);
  
  // Assemble full string and display
  // snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
  // text_layer_set_text(s_weather_layer, weather_layer_buffer);
  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });


  window_set_background_color(s_main_window, GColorBlack);
  window_set_fullscreen(s_main_window, true);
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}