#include <pebble.h>

//------------------------------------Key Definitions------------------------------------//

#define KEY_STATION 0
#define KEY_CONDITION 1
#define KEY_ISSUE_TIME 2
#define KEY_ISSUE_HOUR 3
#define KEY_ISSUE_MINUTE 4
#define KEY_WIND_DIRECTION 5
#define KEY_WIND_SPEED 6
#define KEY_TEMPERATURE 7
#define KEY_DEWPOINT 8
#define KEY_ALTIMETER 9
#define KEY_VISIBILITY 10
#define KEY_OTHER_WX 11
#define KEY_CLOUDS 12
#define KEY_OFFSET 13
#define KEY_DARKBG 14
#define KEY_SUCCESS 15

#define KEY_C_USE_DARK 0
#define KEY_C_GMT_OFFSET 1

#define UPDATE_INTERVAL 15 //minutes
#define FAIL_RETRY_INTERVAL 5 //minutes
#define FAIL_RECOG_INTERVAL 2 //minutes

//---------------------------------Pointer Declarations---------------------------------//
static Window *s_main_window;
//TextLayers
static TextLayer *s_header_time_layer;
static TextLayer *s_header_station_layer;
static TextLayer *s_header_condition_layer;
static TextLayer *s_row1_zulu_time_layer;
static TextLayer *s_row1_issue_time_layer;
static TextLayer *s_row1_difference_time_layer;
static TextLayer *s_row2_wind_direction_layer;
static TextLayer *s_row2_wind_speed_layer;
static TextLayer *s_row3_temperature_layer;
static TextLayer *s_row3_dewpoint_layer;
static TextLayer *s_row3_altimeter_layer;
static TextLayer *s_row4_visibility_layer;
static TextLayer *s_row4_other_wx_layer;
static TextLayer *s_row5_cloud_layer;
//Fonts
static GFont s_header_time_font;
static GFont s_header_small_font;
static GFont s_row_condensed_font;
//Background
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

int updateTimer;
int failedUpdateTimer;
int gmtOffset;
int issueHour;
int issueMinute;

int useDark;

//--------------------------------Pebble UI Loading Functions--------------------------------//

static void header_load(Window *window) {
  //----Current Time TextLayer----//
  #if defined(PBL_RECT)
  s_header_time_layer = text_layer_create(GRect(1, -10, 90, 50));
  #elif defined(PBL_ROUND)
  s_header_time_layer = text_layer_create(GRect(16, 58, 90, 50));
  #endif
  text_layer_set_background_color(s_header_time_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_header_time_layer, GColorWhite); }
  else { text_layer_set_text_color(s_header_time_layer, GColorBlack); }
  text_layer_set_text(s_header_time_layer, "00:00");
  text_layer_set_font(s_header_time_layer, s_header_time_font);
  text_layer_set_text_alignment(s_header_time_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_header_time_layer));
  
  //----Station TextLayer----//
  #if defined(PBL_RECT)
  s_header_station_layer = text_layer_create(GRect(94, -3, 50, 22));
  #elif defined(PBL_ROUND)
  s_header_station_layer = text_layer_create(GRect(117, 65, 50, 22));
  #endif
  text_layer_set_background_color(s_header_station_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_header_station_layer, GColorWhite); }
  else { text_layer_set_text_color(s_header_station_layer, GColorBlack); }
  text_layer_set_text(s_header_station_layer, "ICAO");
  text_layer_set_font(s_header_station_layer, s_header_small_font);
  text_layer_set_text_alignment(s_header_station_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_header_station_layer));
  
  //----Condition TextLayer----//
  #if defined(PBL_RECT)
  s_header_condition_layer = text_layer_create(GRect(93, 20, 50, 22));
  #elif defined(PBL_ROUND)
  s_header_condition_layer = text_layer_create(GRect(116, 88, 50, 22));
  #endif
  text_layer_set_background_color(s_header_condition_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_header_condition_layer, GColorWhite); }
  else { text_layer_set_text_color(s_header_condition_layer, GColorBlack); }
  text_layer_set_text(s_header_condition_layer, "....");
  text_layer_set_font(s_header_condition_layer, s_header_small_font);
  text_layer_set_text_alignment(s_header_condition_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_header_condition_layer));
}

static void row1_load(Window *window) {  
  //----Zulu Time Layer----//
  #if defined(PBL_RECT)
  s_row1_zulu_time_layer = text_layer_create(GRect(3, 43, 42, 25));
  #elif defined(PBL_ROUND)
  s_row1_zulu_time_layer = text_layer_create(GRect(23, 108, 42, 25));
  #endif
  text_layer_set_background_color(s_row1_zulu_time_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_row1_zulu_time_layer, GColorWhite); }
  else { text_layer_set_text_color(s_row1_zulu_time_layer, GColorBlack); }
  text_layer_set_text(s_row1_zulu_time_layer, "00:00");
  text_layer_set_font(s_row1_zulu_time_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row1_zulu_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row1_zulu_time_layer));
  
  //----Issue Time Layer----//
  #if defined(PBL_RECT)
  s_row1_issue_time_layer = text_layer_create(GRect(74, 43, 42, 25));
  #elif defined(PBL_ROUND)
  s_row1_issue_time_layer = text_layer_create(GRect(92, 108, 42, 25));
  #endif
  text_layer_set_background_color(s_row1_issue_time_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_row1_issue_time_layer, GColorWhite); }
  else { text_layer_set_text_color(s_row1_issue_time_layer, GColorBlack); }
  text_layer_set_text(s_row1_issue_time_layer, "UPDT");
  text_layer_set_font(s_row1_issue_time_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row1_issue_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row1_issue_time_layer));
  
  //----Difference Time Layer----//
  #if defined(PBL_RECT)
  s_row1_difference_time_layer = text_layer_create(GRect(120, 43, 22, 25));
  #elif defined(PBL_ROUND)
  s_row1_difference_time_layer = text_layer_create(GRect(138, 108, 22, 25));
  #endif
  text_layer_set_background_color(s_row1_difference_time_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_row1_difference_time_layer, GColorWhite); }
  else { text_layer_set_text_color(s_row1_difference_time_layer, GColorBlack); }
  text_layer_set_text(s_row1_difference_time_layer, "--");
  text_layer_set_font(s_row1_difference_time_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row1_difference_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row1_difference_time_layer));
}

static void row2_load(Window *window) {
  //----Wind Direction Layer----//
  #if defined(PBL_RECT)
  s_row2_wind_direction_layer = text_layer_create(GRect(21, 67, 40, 23));
  #elif defined(PBL_ROUND)
  s_row2_wind_direction_layer = text_layer_create(GRect(31, 21, 40, 23));
  #endif
  text_layer_set_background_color(s_row2_wind_direction_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_row2_wind_direction_layer, GColorWhite); }
  else { text_layer_set_text_color(s_row2_wind_direction_layer, GColorBlack); }
  text_layer_set_text(s_row2_wind_direction_layer, "");
  text_layer_set_font(s_row2_wind_direction_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row2_wind_direction_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row2_wind_direction_layer));
  
  //----Wind Speed Layer----//
  #if defined(PBL_RECT)
  s_row2_wind_speed_layer = text_layer_create(GRect(57, 67, 86, 23));
  #elif defined(PBL_ROUND)
  s_row2_wind_speed_layer = text_layer_create(GRect(78, 21, 86, 23));
  #endif
  text_layer_set_background_color(s_row2_wind_speed_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_row2_wind_speed_layer, GColorWhite); }
  else { text_layer_set_text_color(s_row2_wind_speed_layer, GColorBlack); }
  text_layer_set_text(s_row2_wind_speed_layer, "");
  text_layer_set_font(s_row2_wind_speed_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row2_wind_speed_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row2_wind_speed_layer));
}

static void row3_load(Window *window) {
  //----Temperature Layer----//
  #if defined(PBL_RECT)
  s_row3_temperature_layer = text_layer_create(GRect(11, 92, 28, 23));
  #elif defined(PBL_ROUND)
  s_row3_temperature_layer = text_layer_create(GRect(62, 0, 28, 23));
  #endif
  text_layer_set_background_color(s_row3_temperature_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_row3_temperature_layer, GColorWhite); }
  else { text_layer_set_text_color(s_row3_temperature_layer, GColorBlack); }
  text_layer_set_text(s_row3_temperature_layer, "");
  text_layer_set_font(s_row3_temperature_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row3_temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row3_temperature_layer));
  
  //----Dewpoint Layer----//
  #if defined(PBL_RECT)
  s_row3_dewpoint_layer = text_layer_create(GRect(49, 92, 28, 23));
  #elif defined(PBL_ROUND)
  s_row3_dewpoint_layer = text_layer_create(GRect(90, 0, 28, 23));
  #endif
  text_layer_set_background_color(s_row3_dewpoint_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_row3_dewpoint_layer, GColorWhite); }
  else { text_layer_set_text_color(s_row3_dewpoint_layer, GColorBlack); }
  text_layer_set_text(s_row3_dewpoint_layer, "");
  text_layer_set_font(s_row3_dewpoint_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row3_dewpoint_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row3_dewpoint_layer));
  
  //----Altimeter Layer----//
  #if defined(PBL_RECT)
  s_row3_altimeter_layer = text_layer_create(GRect(96, 92, 46, 23));
  #elif defined(PBL_ROUND)
  s_row3_altimeter_layer = text_layer_create(GRect(70, 152, 46, 23));
  #endif
  text_layer_set_background_color(s_row3_altimeter_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_row3_altimeter_layer, GColorWhite); }
  else { text_layer_set_text_color(s_row3_altimeter_layer, GColorBlack); }
  text_layer_set_text(s_row3_altimeter_layer, "");
  text_layer_set_font(s_row3_altimeter_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row3_altimeter_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row3_altimeter_layer));
}

static void row4_load(Window *window) {
  //----Visibility Layer----//
  #if defined(PBL_RECT)
  s_row4_visibility_layer = text_layer_create(GRect(23, 117, 28, 23));
  #elif defined(PBL_ROUND)
  s_row4_visibility_layer = text_layer_create(GRect(39, 131, 28, 23));
  #endif
  text_layer_set_background_color(s_row4_visibility_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_row4_visibility_layer, GColorWhite); }
  else { text_layer_set_text_color(s_row4_visibility_layer, GColorBlack); }
  text_layer_set_text(s_row4_visibility_layer, "");
  text_layer_set_font(s_row4_visibility_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row4_visibility_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row4_visibility_layer));
  
  //----Other WX Layer----//
  #if defined(PBL_RECT)
  s_row4_other_wx_layer = text_layer_create(GRect(50, 117, 92, 26));
  #elif defined(PBL_ROUND)
  s_row4_other_wx_layer = text_layer_create(GRect(64, 131, 92, 26));
  #endif
  text_layer_set_background_color(s_row4_other_wx_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_row4_other_wx_layer, GColorWhite); }
  else { text_layer_set_text_color(s_row4_other_wx_layer, GColorBlack); }
  text_layer_set_text(s_row4_other_wx_layer, "");
  text_layer_set_font(s_row4_other_wx_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row4_other_wx_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row4_other_wx_layer));
}

static void row5_load(Window *window) {
  //----Cloud Layer----//
  #if defined(PBL_RECT)
  s_row5_cloud_layer = text_layer_create(GRect(20, 142, 122, 23));
  #elif defined(PBL_ROUND)
  s_row5_cloud_layer = text_layer_create(GRect(40, 44, 130, 23));
  #endif
  text_layer_set_background_color(s_row5_cloud_layer, GColorClear);
  if (useDark) { text_layer_set_text_color(s_row5_cloud_layer, GColorWhite); }
  else { text_layer_set_text_color(s_row5_cloud_layer, GColorBlack); }
  text_layer_set_text(s_row5_cloud_layer, "");
  text_layer_set_font(s_row5_cloud_layer, s_row_condensed_font);
  text_layer_set_text_alignment(s_row5_cloud_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_row5_cloud_layer));
}

static void main_window_load(Window *window) {
  //----Load Background----//
  if (useDark) { s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_ALT); }
  else { s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND); }
  //Create a background layer regardless of screen size
  s_background_layer = bitmap_layer_create(layer_get_bounds(window_get_root_layer(window)));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  //----Set Fonts----//
  s_header_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OPEN_SANS_CONDENSED_LIGHT_50));
  s_header_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_COUSINE_BOLD_20));
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
  text_layer_destroy(s_row1_zulu_time_layer);
  text_layer_destroy(s_row1_issue_time_layer);
  text_layer_destroy(s_row1_difference_time_layer);
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
  fonts_unload_custom_font(s_row_condensed_font);
  //----Destroy Background----//
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
}

//--------------------------------Data Updating Functions--------------------------------//

/*char *itoa(int num)
{
static char buff[20] = {};
int i = 0, temp_num = num, length = 0;
char *string = buff;
if(num >= 0) {
// count how many characters in the number
while(temp_num) {
temp_num /= 10;
length++;
}
// assign the number to the buffer starting at the end of the 
// number and going to the begining since we are doing the
// integer to character conversion on the last number in the
// sequence
for(i = 0; i < length; i++) {
buff[(length-1)-i] = '0' + (num % 10);
num /= 10;
}
buff[i] = '\0'; // can't forget the null byte to properly end our string
}
else
return "Unsupported Number";
return string;
}*/

static void update_time() {
  //----Update Main Time Display----//
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
  
  //Thank you to Ben Koch for helping with the code below
  
  //----Update Zulu Time Display----//
  #ifdef PBL_SDK_2
    time_t temp_gmt = time(NULL) + (gmtOffset*60);
  #else
    time_t temp_gmt = time(NULL);
  #endif
  struct tm *zulu_time = gmtime(&temp_gmt);
  static char bufferZulu[] = "00:00";
  strftime(bufferZulu, sizeof("00:00"), "%H:%M", zulu_time);
  text_layer_set_text(s_row1_zulu_time_layer, bufferZulu);
  
  //----Update Difference Time Display----//
  //Find minutes since last update
  static char bufferHour[] = "00";
  strftime(bufferHour, sizeof(bufferHour), "%H", zulu_time);
  int gmtMinutes = (atoi(bufferHour))*60;
  static char bufferMinute[] = "00";
  strftime(bufferMinute, sizeof(bufferMinute), "%M", zulu_time);
  gmtMinutes += atoi(bufferMinute);
  int differenceMinutes = gmtMinutes - (issueHour*60) - issueMinute;
  
  //Correct errors, format, and display text
  //Add (24*60) minutes if current time is past midnight and issued time is before midnight
  if (differenceMinutes < 0) { differenceMinutes += 1440; }
  /*APP_LOG(APP_LOG_LEVEL_INFO, itoa(gmtMinutes));
  APP_LOG(APP_LOG_LEVEL_INFO, itoa(issueHour));
  APP_LOG(APP_LOG_LEVEL_INFO, itoa(issueMinute));
  APP_LOG(APP_LOG_LEVEL_INFO, itoa(differenceMinutes));*/
  if (((issueHour < 1) && (issueMinute < 1)) || (differenceMinutes < 0)) { //If connection issue or watch start-up
    text_layer_set_text(s_row1_difference_time_layer, "--");
  } else if (differenceMinutes > 99) { //If report is older than 99 minutes
    text_layer_set_text(s_row1_difference_time_layer, "++");
  } else { //Else set the difference layer
    static char bufferDifference[] = "00";
    snprintf(bufferDifference, sizeof(bufferDifference), "%d", differenceMinutes);
    text_layer_set_text(s_row1_difference_time_layer, bufferDifference);
  }
}

static void auto_update_handler() {
  //Call for updated data if reached UPDATE_INTERVAL
  if (updateTimer > UPDATE_INTERVAL + FAIL_RECOG_INTERVAL) {
    updateTimer = 0;
    APP_LOG(APP_LOG_LEVEL_INFO, "Reseting counter to 0");
    failedUpdateTimer = 0;
    
    #ifdef PBL_SDK_3
    //The phone won't read it, but we need a dict to execute the outbox call
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    #endif
    app_message_outbox_send();
    APP_LOG(APP_LOG_LEVEL_INFO, "Requesting update");
  } else {
    updateTimer++;
    failedUpdateTimer++;
    APP_LOG(APP_LOG_LEVEL_INFO, "Incremented counter");
  }
  if (failedUpdateTimer == FAIL_RECOG_INTERVAL) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Set bad screen and counter to fail_recog+1");
    failedUpdateTimer = FAIL_RECOG_INTERVAL + 1;
    updateTimer = UPDATE_INTERVAL - FAIL_RETRY_INTERVAL;
    text_layer_set_text(s_row1_issue_time_layer, ":(");
    text_layer_set_text(s_row1_difference_time_layer, "");
    text_layer_set_text(s_row2_wind_speed_layer, "Sorry I");
    #if defined(PBL_RECT)
    text_layer_set_text(s_row4_other_wx_layer, "could not");
    text_layer_set_text(s_row5_cloud_layer, "connect");
    #elif defined(PBL_ROUND)
    text_layer_set_text(s_row4_other_wx_layer, "connect");
    text_layer_set_text(s_row5_cloud_layer, "could not");
    #endif
    text_layer_set_text(s_header_condition_layer, "");
    text_layer_set_text(s_row2_wind_direction_layer, "");
    text_layer_set_text(s_row3_temperature_layer, "");
    text_layer_set_text(s_row3_dewpoint_layer, "");
    text_layer_set_text(s_row3_altimeter_layer, "");
    text_layer_set_text(s_row4_visibility_layer, "");
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
  #ifdef PBL_COLOR
  if (strcmp(buffer, "VFR") == 0) { text_layer_set_text_color(s_header_condition_layer, GColorIslamicGreen); }
  else if (strcmp(buffer, "MVFR") == 0) { text_layer_set_text_color(s_header_condition_layer, GColorBlue); }
  else if (strcmp(buffer, "IFR") == 0) { text_layer_set_text_color(s_header_condition_layer, GColorRed); }
  else if (strcmp(buffer, "LIFR") == 0) { text_layer_set_text_color(s_header_condition_layer, GColorPurple); }
  else { text_layer_set_text_color(s_header_condition_layer, GColorBlack); }
  #endif
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

static void update_difference_styling(Tuple *t) {
  bool success = t->value->uint8;
  if (success) {
    text_layer_set_background_color(s_row1_difference_time_layer, GColorClear);
    if (useDark) { text_layer_set_text_color(s_row1_difference_time_layer, GColorWhite); }
    else { text_layer_set_text_color(s_row1_difference_time_layer, GColorBlack); }
  } else {
    #ifdef PBL_COLOR
    text_layer_set_background_color(s_row1_difference_time_layer, GColorRed);
    text_layer_set_text_color(s_row1_difference_time_layer, GColorWhite);
    #else
    if (useDark) { 
      text_layer_set_background_color(s_row1_difference_time_layer, GColorWhite);
      text_layer_set_text_color(s_row1_difference_time_layer, GColorBlack);
    } else {
      text_layer_set_background_color(s_row1_difference_time_layer, GColorBlack);
      text_layer_set_text_color(s_row1_difference_time_layer, GColorWhite);
    }
    #endif
  }
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
    case KEY_OFFSET:
      gmtOffset = t->value->int16;
      persist_write_int(KEY_C_GMT_OFFSET, gmtOffset);
      break;
    case KEY_ISSUE_HOUR:
      issueHour = t->value->int8;
      break;
    case KEY_ISSUE_MINUTE:
      issueMinute = t->value->int8;
      break;
    case KEY_DARKBG:
      persist_write_int(KEY_C_USE_DARK, t->value->int8);
      break;
    case KEY_SUCCESS:
      update_difference_styling(t);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Setting to 3 after successful update");
  failedUpdateTimer = FAIL_RECOG_INTERVAL + 1;
  update_time();
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
  updateTimer = 0;
  failedUpdateTimer = 0;
  gmtOffset = persist_read_int(KEY_C_GMT_OFFSET);
  issueHour = 0;
  issueMinute = 0;
  useDark = persist_read_int(KEY_C_USE_DARK);
  
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
  //app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_open(1080, 64);
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