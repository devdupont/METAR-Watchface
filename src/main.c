#include <pebble.h>

//------------------------------------Key Definitions------------------------------------//

#define KEY_STATION 0
#define KEY_CONDITION 1
#define KEY_ISSUE_TIME 2
#define KEY_WIND_DIRECTION 3
#define KEY_WIND_SPEED 4
#define KEY_TEMPERATURE 5
#define KEY_DEWPOINT 6
#define KEY_ALTIMETER 7
#define KEY_VISIBILITY 8
#define KEY_OTHER_WX 9
#define KEY_CLOUDS 10

#define UPDATE_INTERVAL 15 //minutes

//---------------------------------Pointer Declarations---------------------------------//
static Window *s_main_window;
//TextLayers
static TextLayer *s_header_time_layer;
static TextLayer *s_header_station_layer;
static TextLayer *s_header_condition_layer;
static TextLayer *s_row1_issue_time_layer;
static TextLayer *s_row2_wind_direction_layer;
static TextLayer *s_row2_wind_speed_layer;
static TextLayer *s_row3_temperature_layer;
static TextLayer *s_row3_dewpoint_layer;
static TextLayer *s_row3_altimeter_layer;
static TextLayer *s_row4_visibility_layer;
static TextLayer *s_row4_other_wx_layer;
static TextLayer *s_row5_cloud_layer;
//Fonts
static GFont *s_header_time_font;
static GFont *s_header_small_font;
static GFont *s_row_regular_font;
static GFont *s_row_condensed_font;
//Background
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

int i;

//--------------------------------Pebble UI Loading Functions--------------------------------//

static void header_load(Window *window) {
  //----Current Time TextLayer----//
  s_header_time_layer = text_layer_create(GRect(1, -10, 90, 50));
  text_layer_set_background_color(s_header_time_layer, GColorClear);
  text_layer_set_text_color(s_header_time_layer, GColorBlack);
  text_layer_set_text(s_header_time_layer, "00:00");
  text_layer_set_font(s_header_time_layer, s_header_time_font);
  text_layer_set_text_alignment(s_header_time_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_header_time_layer));
  
  //----Station TextLayer----//
  s_header_station_layer = text_layer_create(GRect(94, -3, 50, 22));
  text_layer_set_background_color(s_header_station_layer, GColorClear);
  text_layer_set_text_color(s_header_station_layer, GColorBlack);
  text_layer_set_text(s_header_station_layer, "ICAO");
  text_layer_set_font(s_header_station_layer, s_header_small_font);
  text_layer_set_text_alignment(s_header_station_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_header_station_layer));
  
  //----Condition TextLayer----//
  s_header_condition_layer = text_layer_create(GRect(93, 20, 50, 22));
  text_layer_set_background_color(s_header_condition_layer, GColorClear);
  text_layer_set_text_color(s_header_condition_layer, GColorBlack);
  text_layer_set_text(s_header_condition_layer, "....");
  text_layer_set_font(s_header_condition_layer, s_header_small_font);
  text_layer_set_text_alignment(s_header_condition_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_header_condition_layer));
}

static void row1_load(Window *window) {
  //----Issue Time Layer----//
  s_row1_issue_time_layer = text_layer_create(GRect(40, 42, 90, 25));
  text_layer_set_background_color(s_row1_issue_time_layer, GColorClear);
  text_layer_set_text_color(s_row1_issue_time_layer, GColorBlack);
  text_layer_set_text(s_row1_issue_time_layer, "UPDT...");
  text_layer_set_font(s_row1_issue_time_layer, s_row_regular_font);
  text_layer_set_text_alignment(s_row1_issue_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row1_issue_time_layer));
}

static void row2_load(Window *window) {
  //----Wind Direction Layer----//
  s_row2_wind_direction_layer = text_layer_create(GRect(21, 67, 40, 23));
  text_layer_set_background_color(s_row2_wind_direction_layer, GColorClear);
  text_layer_set_text_color(s_row2_wind_direction_layer, GColorBlack);
  text_layer_set_text(s_row2_wind_direction_layer, "....");
  text_layer_set_font(s_row2_wind_direction_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row2_wind_direction_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row2_wind_direction_layer));
  
  //----Wind Speed Layer----//
  s_row2_wind_speed_layer = text_layer_create(GRect(60, 67, 80, 23));
  text_layer_set_background_color(s_row2_wind_speed_layer, GColorClear);
  text_layer_set_text_color(s_row2_wind_speed_layer, GColorBlack);
  text_layer_set_text(s_row2_wind_speed_layer, "..KT");
  text_layer_set_font(s_row2_wind_speed_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row2_wind_speed_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row2_wind_speed_layer));
}

static void row3_load(Window *window) {
  //----Temperature Layer----//
  s_row3_temperature_layer = text_layer_create(GRect(11, 92, 28, 23));
  text_layer_set_background_color(s_row3_temperature_layer, GColorClear);
  text_layer_set_text_color(s_row3_temperature_layer, GColorBlack);
  text_layer_set_text(s_row3_temperature_layer, "...");
  text_layer_set_font(s_row3_temperature_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row3_temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row3_temperature_layer));
  
  //----Dewpoint Layer----//
  s_row3_dewpoint_layer = text_layer_create(GRect(49, 92, 28, 23));
  text_layer_set_background_color(s_row3_dewpoint_layer, GColorClear);
  text_layer_set_text_color(s_row3_dewpoint_layer, GColorBlack);
  text_layer_set_text(s_row3_dewpoint_layer, "...");
  text_layer_set_font(s_row3_dewpoint_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row3_dewpoint_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row3_dewpoint_layer));
  
  //----Altimeter Layer----//
  s_row3_altimeter_layer = text_layer_create(GRect(96, 92, 46, 23));
  text_layer_set_background_color(s_row3_altimeter_layer, GColorClear);
  text_layer_set_text_color(s_row3_altimeter_layer, GColorBlack);
  text_layer_set_text(s_row3_altimeter_layer, ".....");
  text_layer_set_font(s_row3_altimeter_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row3_altimeter_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row3_altimeter_layer));
}

static void row4_load(Window *window) {
  //----Visibility Layer----//
  s_row4_visibility_layer = text_layer_create(GRect(23, 117, 28, 23));
  text_layer_set_background_color(s_row4_visibility_layer, GColorClear);
  text_layer_set_text_color(s_row4_visibility_layer, GColorBlack);
  text_layer_set_text(s_row4_visibility_layer, "...");
  text_layer_set_font(s_row4_visibility_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row4_visibility_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row4_visibility_layer));
  
  //----Other WX Layer----//
  s_row4_other_wx_layer = text_layer_create(GRect(50, 117, 92, 23));
  text_layer_set_background_color(s_row4_other_wx_layer, GColorClear);
  text_layer_set_text_color(s_row4_other_wx_layer, GColorBlack);
  text_layer_set_text(s_row4_other_wx_layer, "...");
  text_layer_set_font(s_row4_other_wx_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row4_other_wx_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row4_other_wx_layer));
}

static void row5_load(Window *window) {
  //----Cloud Layer----//
  s_row5_cloud_layer = text_layer_create(GRect(20, 142, 122, 23));
  text_layer_set_background_color(s_row5_cloud_layer, GColorClear);
  text_layer_set_text_color(s_row5_cloud_layer, GColorBlack);
  text_layer_set_text(s_row5_cloud_layer, "...");
  text_layer_set_font(s_row5_cloud_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row5_cloud_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row5_cloud_layer));
}

static void main_window_load(Window *window) {
  //----Load Background----//
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  //----Set Fonts----//
  s_header_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OPEN_SANS_CONDENSED_LIGHT_50));
  s_header_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_COUSINE_BOLD_20));
  s_row_regular_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_MEDIUM_22));
  s_row_condensed_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OPEN_SANS_CONDENSED_LIGHT_22));
  
  //----Load Content Layers----//
  header_load(window);
  row1_load(window);
  row2_load(window);
  row3_load(window);
  row4_load(window);
  row5_load(window);
}

//-------------------------------Pebble UI Unloading Functions-------------------------------//

static void main_window_unload(Window *window) {
  //----Destroy TextLayers----//
  text_layer_destroy(s_header_time_layer);
  text_layer_destroy(s_header_station_layer);
  text_layer_destroy(s_header_condition_layer);
  text_layer_destroy(s_row1_issue_time_layer);
  text_layer_destroy(s_row2_wind_direction_layer);
  text_layer_destroy(s_row2_wind_speed_layer);
  text_layer_destroy(s_row3_temperature_layer);
  text_layer_destroy(s_row3_dewpoint_layer);
  text_layer_destroy(s_row3_altimeter_layer);
  text_layer_destroy(s_row4_visibility_layer);
  text_layer_destroy(s_row4_other_wx_layer);
  text_layer_destroy(s_row5_cloud_layer);
  //----Unload GFonts----//
  fonts_unload_custom_font(s_header_time_font);
  fonts_unload_custom_font(s_header_small_font);
  fonts_unload_custom_font(s_row_regular_font);
  fonts_unload_custom_font(s_row_condensed_font);
  //----Destroy Background----//
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
}

//--------------------------------Data Updating Functions--------------------------------//

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%l:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_header_time_layer, buffer);
}

static void auto_update_handler() {
  //Call for updated data if reached UPDATE_INTERVAL
  if (i > UPDATE_INTERVAL) {
    i = 0;
    app_message_outbox_send(); //Sending empty outbox requests new updated inbox
  } else {
    i++;
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  auto_update_handler();
}

static void update_station(Tuple *t) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%s", t->value->cstring);
  text_layer_set_text(s_header_station_layer, buffer);
  layer_mark_dirty(text_layer_get_layer(s_header_station_layer));
}

static void update_condition(Tuple *t) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%s", t->value->cstring);
  text_layer_set_text(s_header_condition_layer, buffer);
}

static void update_issue_time(Tuple *t) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%s", t->value->cstring);
  text_layer_set_text(s_row1_issue_time_layer, buffer);
  layer_mark_dirty(text_layer_get_layer(s_row1_issue_time_layer));
}

static void update_wind_direction(Tuple *t) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%s", t->value->cstring);
  text_layer_set_text(s_row2_wind_direction_layer, buffer);
}

static void update_wind_speed(Tuple *t) {
  static char buffer[16];
  snprintf(buffer, sizeof(buffer), "%s", t->value->cstring);
  text_layer_set_text(s_row2_wind_speed_layer, buffer);
}

static void update_temperature(Tuple *t) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%s", t->value->cstring);
  text_layer_set_text(s_row3_temperature_layer, buffer);
}

static void update_dewpoint(Tuple *t) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%s", t->value->cstring);
  text_layer_set_text(s_row3_dewpoint_layer, buffer);
}

static void update_altimeter(Tuple *t) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%s", t->value->cstring);
  text_layer_set_text(s_row3_altimeter_layer, buffer);
}

static void update_visibility(Tuple *t) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%s", t->value->cstring);
  text_layer_set_text(s_row4_visibility_layer, buffer);
}

static void update_other_wx(Tuple *t) {
  static char buffer[32];
  snprintf(buffer, sizeof(buffer), "%s", t->value->cstring);
  text_layer_set_text(s_row4_other_wx_layer, buffer);
}

static void update_clouds(Tuple *t) {
  static char buffer[32];
  snprintf(buffer, sizeof(buffer), "%s", t->value->cstring);
  text_layer_set_text(s_row5_cloud_layer, buffer);
}

//---------------------------------App Messege Callbacks---------------------------------//

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Updating Watch Display Elements");
  
   // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_STATION:
      update_station(t);
      break;
    case KEY_CONDITION:
      update_condition(t);
      break;
    case KEY_ISSUE_TIME:
      update_issue_time(t);
      break;
    case KEY_WIND_DIRECTION:
      update_wind_direction(t);
      break;
    case KEY_WIND_SPEED:
      update_wind_speed(t);
      break;
    case KEY_TEMPERATURE:
      update_temperature(t);
      break;
    case KEY_DEWPOINT:
      update_dewpoint(t);
      break;
    case KEY_ALTIMETER:
      update_altimeter(t);
      break;
    case KEY_VISIBILITY:
      update_visibility(t);
      break;
    case KEY_OTHER_WX:
      update_other_wx(t);
      break;
    case KEY_CLOUDS:
      update_clouds(t);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
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

//-----------------------------------Init Deinit and Main-----------------------------------//

static void init() {
  i = 0;
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  //Update the clock time before window load
  update_time();
  
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