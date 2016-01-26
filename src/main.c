#include <pebble.h>
#include <time.h>
  
static Window *s_main_window;
static TextLayer *s_syd_time_layer,*s_lon_time_layer,*s_san_time_layer,*s_epoch_layer,*s_date_layer;
static int s_battery_level;
static bool s_bt_connected;
static Layer *s_battery_layer,*s_blue_conn_layer;

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  // Update meter
  layer_mark_dirty(s_battery_layer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 144.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorGreen);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

static void bt_conn_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  if (s_bt_connected){
    graphics_context_set_fill_color(ctx, GColorBlue);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  } else {
    graphics_context_set_fill_color(ctx, GColorRed);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  }
}

static void bluetooth_callback(bool connected) {
  s_bt_connected = connected;
  layer_mark_dirty(s_blue_conn_layer);

  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  // struct tm *tick_time2 = gmtime(&temp);
  time_t seconds_past_epoch = time(0);

  // Create a long-lived buffer
  static char tm_buffer[] = "00:00";
  //static char tm_buffer2[] = "00:00";
  //static char tm_buffer3[] = "00:00";
  static char ep_buffer[] = "124567890";
  static char dt_buffer[16] = "May 10th";

  // Generate Epoch
  snprintf(ep_buffer, 10, "%d", (int) seconds_past_epoch);
  
  // Generate Date
  strftime(dt_buffer, sizeof(dt_buffer), "%a, %d %b", tick_time);

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(tm_buffer, sizeof("00:00"), "%H:%M", tick_time);
    // strftime(tm_buffer2, sizeof("00:00"), "%H:%M", tick_time2);
  } else {
    //Use 12 hour format
    strftime(tm_buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  // Display this time on the TextLayer
  text_layer_set_text(s_syd_time_layer, tm_buffer);
  text_layer_set_text(s_lon_time_layer, tm_buffer);
  text_layer_set_text(s_san_time_layer, tm_buffer);
  text_layer_set_text(s_epoch_layer, ep_buffer);
  text_layer_set_text(s_date_layer, dt_buffer);
}

static void main_window_load(Window *window) {
  // Create Sydney time TextLayer
  s_syd_time_layer = text_layer_create(GRect(0, 18, 144, 50));
  text_layer_set_background_color(s_syd_time_layer, GColorClear);
  text_layer_set_text_color(s_syd_time_layer, GColorBlack);
  text_layer_set_text(s_syd_time_layer, "00:00");

  // Create London time TextLayer
  s_lon_time_layer = text_layer_create(GRect(0, 60, 144, 30));
  text_layer_set_background_color(s_lon_time_layer, GColorClear);
  text_layer_set_text_color(s_lon_time_layer, GColorBlack);
  text_layer_set_text(s_lon_time_layer, "00:00");

  // Create San Diego time TextLayer
  s_san_time_layer = text_layer_create(GRect(0, 90, 144, 30));
  text_layer_set_background_color(s_san_time_layer, GColorClear);
  text_layer_set_text_color(s_san_time_layer, GColorBlack);
  text_layer_set_text(s_san_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_syd_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_syd_time_layer, GTextAlignmentCenter);
  
  // London and San Deigo are smaller
  text_layer_set_font(s_lon_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_lon_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_san_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_san_time_layer, GTextAlignmentCenter);

  // Create epoch TextLayer
  s_epoch_layer = text_layer_create(GRect(0, 130, 144, 39));
  text_layer_set_background_color(s_epoch_layer, GColorClear);
  text_layer_set_text_color(s_epoch_layer, GColorBlack);
  text_layer_set_text(s_epoch_layer, "NOTHING");

  // Improve the epoch layer
  text_layer_set_font(s_epoch_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_epoch_layer, GTextAlignmentCenter);
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 3, 144, 20));
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // Improve the date layer
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_syd_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_lon_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_san_time_layer));  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_epoch_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

  // Create battery meter Layer
  s_battery_layer = layer_create(GRect(0, 163, 144, 5));
  layer_set_update_proc(s_battery_layer, battery_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  // Create conn meter Layer
  s_blue_conn_layer = layer_create(GRect(0, 0, 144, 5));
  layer_set_update_proc(s_blue_conn_layer, bt_conn_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_blue_conn_layer);
  
  // Make sure the time is displayed from the start
  update_time();

  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Ensure bluetooth layer is displayed from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_syd_time_layer);
  text_layer_destroy(s_lon_time_layer);
  text_layer_destroy(s_san_time_layer);  
  text_layer_destroy(s_epoch_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_battery_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

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
