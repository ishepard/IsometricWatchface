#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_TEMP_UNIT 1
#define KEY_CONDITIONS 2

static Window *s_main_window;
static BitmapLayer *s_background_layer;
static BitmapLayer *weather_layer;
static BitmapLayer *battery_layer;
static GBitmap *s_background_bitmap;
static GBitmap *battery_bitmap;
static GBitmap *weather_img;
static TextLayer *battery_percentage;
static TextLayer *date_layer;
static TextLayer *temperature_layer;

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
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Choose png called with %c", hour_n);
  return (side == LEFT) ? leftNumbers[(hour_n - '0')] : rightNumbers[(hour_n - '0')];
}

static void set_battery_bitmap(int percentage){
  APP_LOG(APP_LOG_LEVEL_INFO, "percentage: %d", percentage);
  if (battery_bitmap){
    gbitmap_destroy(battery_bitmap);
    battery_bitmap = NULL;
    bitmap_layer_set_bitmap(battery_layer, NULL);
  }
  if (percentage == -1){
    battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGING);
  } else if (percentage > 80){
    battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_100);
  } else if (percentage > 60 && percentage <=80){
    battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_80);
  } else if (percentage > 40 && percentage <=60){
    battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_60);
  } else if (percentage > 20 && percentage <=40){
    battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_40);
  } else if (percentage > 5 && percentage <=20){
    battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_20);
  } else if (percentage <=5){
    battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_5);
  }
  bitmap_layer_set_bitmap(battery_layer, battery_bitmap);
}

static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display
  static char battery_buffer[32];
  if (new_state.is_charging) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Battery is charging");
    set_battery_bitmap(-1);
  } else {
    set_battery_bitmap(new_state.charge_percent);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Current battery level: %d/100", new_state.charge_percent);
  snprintf(battery_buffer, sizeof(battery_buffer), "%d%%", new_state.charge_percent);
  text_layer_set_text(battery_percentage, battery_buffer);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char buffer_date[] = "00 aaa";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  strftime(buffer_date, sizeof(buffer_date), "%d %a", tick_time);
  text_layer_set_text(date_layer, buffer_date);

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
  // Set background
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_background_color(s_background_layer, GColorClear); 
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

  // Set battery default bitmap
  battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_100);
  battery_layer = bitmap_layer_create(GRect(119, 6, 25, 10));
  bitmap_layer_set_bitmap(battery_layer, battery_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(battery_layer));

  // Set Battery Layer
  battery_percentage = text_layer_create(GRect(80, 0, 40, 30));
  text_layer_set_background_color(battery_percentage, GColorClear);
  text_layer_set_text_color(battery_percentage, GColorWhite);
  text_layer_set_text(battery_percentage, "100%");
  // text_layer_set_font(battery_percentage, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_font(battery_percentage, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ALPACA_16)));
  text_layer_set_text_alignment(battery_percentage, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(battery_percentage));

  // Set Date Layer
  date_layer = text_layer_create(GRect(0, 0, 70, 50));
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_text(date_layer, "00 aaa");
  text_layer_set_font(date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ALPACA_16)));
  text_layer_set_text_alignment(date_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

  // Set Date Layer
  temperature_layer = text_layer_create(GRect(73, 130, 70, 40));
  text_layer_set_background_color(temperature_layer, GColorClear);
  text_layer_set_text_color(temperature_layer, GColorWhite);
  //text_layer_set_text(temperature_layer, "9°C");
  text_layer_set_font(temperature_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ALPACA_30)));
  text_layer_set_text_alignment(temperature_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(temperature_layer));

  // Set weather bitmap
  // weather_layer_white = bitmap_layer_create(GRect(15, 42, 55, 28));
  // bitmap_layer_set_compositing_mode(weather_layer_white, GCompOpOr); 
  // layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(weather_layer_white));
  // weather_layer_black = bitmap_layer_create(GRect(15, 42, 55, 28));
  // bitmap_layer_set_compositing_mode(weather_layer_black, GCompOpClear); 
  // layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(weather_layer_black));
  weather_layer = bitmap_layer_create(GRect(0, 127, 40, 40));
  bitmap_layer_set_alignment(weather_layer, GAlignBottomLeft);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(weather_layer));

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
  battery_handler(battery_state_service_peek());
  update_time();
}

static void main_window_unload(Window *window) {
  for (int i = 0; i <= 3; i++){
    bitmap_layer_destroy(bitmap_layer[i]);
    gbitmap_destroy(gbitmap_number[i]);
  }
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(weather_layer);
  bitmap_layer_destroy(battery_layer);
  gbitmap_destroy(s_background_bitmap);
  gbitmap_destroy(battery_bitmap);
  gbitmap_destroy(weather_img);
  text_layer_destroy(date_layer);
  text_layer_destroy(temperature_layer);
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
static uint32_t get_bitmap_from_condition(int condition_code){
  uint32_t resource_id = 0;
  switch(condition_code){
    case 30:
      resource_id = RESOURCE_ID_IMAGE_PARTLY_CLOUDY_DAY;
      break;
    case 29:
      resource_id = RESOURCE_ID_IMAGE_PARTLY_CLOUDY_NIGHT;
      break;
    case 26:
    case 27:
    case 28:
    case 44:
      resource_id = RESOURCE_ID_IMAGE_CLOUDY;
      break;
    case 31:
    case 33:
      resource_id = RESOURCE_ID_IMAGE_CLEAR_NIGHT;
      break;
    case 32:
    case 34:
    case 36:
      resource_id = RESOURCE_ID_IMAGE_CLEAR_DAY;
      break;
    case 13:
    case 15:
    case 16:
    case 25:
    case 41:
    case 42:
    case 43:
    case 46:
      resource_id = RESOURCE_ID_IMAGE_SNOW;
      break;
    case 19:
    case 20:
    case 21:
    case 22:
      resource_id = RESOURCE_ID_IMAGE_FOG;
      break;
    case 3:
    case 4:
    case 9:
    case 11:
    case 12:
    case 40:
    case 45:
    case 47:
      resource_id = RESOURCE_ID_IMAGE_RAIN;
      break;
    case 0:
    case 1:
    case 2:
    case 23:
    case 24:
    case 37:
    case 38:
    case 39:
      resource_id = RESOURCE_ID_IMAGE_WIND;
      break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 10:
    case 17:
    case 18:
    case 35:
      resource_id = RESOURCE_ID_IMAGE_SLEET;
      break;
    default:
      return 0;
  }
  return resource_id;
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static char temperature_buffer[8];
  int condition_code = 3200;  
  
  Tuple *t = dict_read_first(iterator);

  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°C", (int)t->value->int32);
      break;
    case KEY_TEMP_UNIT:
      //snprintf(unit_buffer, sizeof(unit_buffer), "%s", t->value->cstring);
      break;
    case KEY_CONDITIONS:
      condition_code = (int)t->value->int32;
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    t = dict_read_next(iterator);
  }
  if (weather_img){
    gbitmap_destroy(weather_img);
    weather_img = NULL;
    bitmap_layer_set_bitmap(weather_layer, NULL);
  }
  weather_img = gbitmap_create_with_resource(get_bitmap_from_condition(condition_code));

  bitmap_layer_set_bitmap(weather_layer, weather_img);
  text_layer_set_text(temperature_layer, temperature_buffer);
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
  
  battery_state_service_subscribe(battery_handler);
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