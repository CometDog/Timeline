#include "pebble.h"

#include "gcolor_definitions.h" // Allows the user of color

static Window *s_main_window; // Main window
static TextLayer *s_hour_label, *s_minute_label, *s_hours_label, *s_hours2_label; // Layers for labels
static Layer *s_solid_layer, *s_time_layer, *s_box_layer; // Background layers
static BitmapLayer *s_background_layer; // Create face bitmap layer
static GBitmap *s_background_bitmap; // Create bitmap of face
static GFont s_time_font, s_small_font; // Fonts

// Buffers
static char s_hour_buffer[3];
static char s_hour2_buffer[3];
static char s_minute_buffer[3];

// Update background when called
static void update_bg(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite); // Set the fill color
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone); // Fill the screen
}

// Handles updating time
static void update_time() {
  // Get time and structure
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);

  // Write time as 24h or 12h format onto the buffer
  if(clock_is_24h_style() == true) { // 24h format
    strftime(s_hour_buffer, sizeof(s_hour_buffer), "%H", t);
  } else { // 12h format
    strftime(s_hour_buffer, sizeof(s_hour_buffer), "%I", t);
  }
  strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", t); // Minutes

  // Display on Time Layer
  text_layer_set_text(s_hour_label, s_hour_buffer);
  text_layer_set_text(s_minute_label, s_minute_buffer);
  text_layer_set_text(s_hours_label, s_hour_buffer);
  
  // Do the same for the hour + 1
  temp = time(NULL) + 3600;
  t = localtime(&temp);
  
  // Write time as 24h or 12h format onto the buffer
  if(clock_is_24h_style() == true) { // 24h format
    strftime(s_hour2_buffer, sizeof(s_hour2_buffer), "%H", t);
  } else { // 12h format
    strftime(s_hour2_buffer, sizeof(s_hour2_buffer), "%I", t);
  }
  
  text_layer_set_text(s_hours2_label, s_hour2_buffer);
  
}

// Update the background fill box when called
static void update_box(Layer *layer, GContext *ctx) {
  // Get time structure and set the minute to an integer
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  int min = t->tm_min;
  
  // Make background fill increase each minute
  GRect box_frame = GRect(0, 0,((min * 2) + 12), 168);
  s_box_layer = layer_create(box_frame);
  graphics_context_set_fill_color(ctx, GColorCyan);
  graphics_fill_rect(ctx, layer_get_bounds(s_box_layer), 0, GCornerNone);
}

// Update time when called
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

// Loads the layers onto the main window
static void main_window_load(Window *window) {
  
  // Creates window_layer as root and sets its bounds
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect box_frame = GRect(0, 0, 144, 168);
  
  // Create the fonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OPEN_SANS_52));
  s_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OPEN_SANS_20));
  
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND); // Set the bitmap resource
  
  // Create background the layers
  s_solid_layer = layer_create(bounds);
  s_time_layer = layer_create(bounds);
  s_background_layer = bitmap_layer_create(bounds);
  s_box_layer = layer_create(box_frame);
  
  // Update background layers
  layer_set_update_proc(s_solid_layer, update_bg);
  layer_set_update_proc(s_box_layer, update_box);
  
  // Create the ruler image
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap); // Set the face bitmap to the background layer
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet); // Set the face layer to be a transparent png image
  
  // Create the labels
  s_hour_label = text_layer_create(GRect(0,7,144,64));
  s_minute_label = text_layer_create(GRect(0,96,144,64));
  s_hours_label = text_layer_create(GRect(1,89,144,30));
  s_hours2_label = text_layer_create(GRect(0,89,143,30));
  
  // Set background and text colors
  text_layer_set_background_color(s_hour_label, GColorClear);
  text_layer_set_text_color(s_hour_label, GColorBlack);
  text_layer_set_background_color(s_minute_label, GColorClear);
  text_layer_set_text_color(s_minute_label, GColorBlack);
  text_layer_set_background_color(s_hours_label, GColorClear);
  text_layer_set_text_color(s_hours_label, GColorBlack);
  text_layer_set_background_color(s_hours2_label, GColorClear);
  text_layer_set_text_color(s_hours2_label, GColorBlack);
  
  // Set fonts
  text_layer_set_font(s_hour_label, s_time_font);
  text_layer_set_font(s_minute_label, s_time_font);
  text_layer_set_font(s_hours_label, s_small_font);
  text_layer_set_font(s_hours2_label, s_small_font);
  
  // Avoid blank screen in case updating time fails
  text_layer_set_text(s_hour_label, "00");
  text_layer_set_text(s_minute_label, "00");
  text_layer_set_text(s_hours_label, "00");
  text_layer_set_text(s_hours2_label, "00");
  
  // Align text
  text_layer_set_text_alignment(s_hour_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_minute_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_hours_label, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_hours2_label, GTextAlignmentRight);
  
  // Apply layers to screen
  layer_add_child(window_layer, s_solid_layer); 
  layer_add_child(window_layer, s_box_layer);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  layer_add_child(window_layer, s_time_layer);
  layer_add_child(s_time_layer, text_layer_get_layer(s_hour_label));
  layer_add_child(s_time_layer, text_layer_get_layer(s_minute_label));
  layer_add_child(s_time_layer, text_layer_get_layer(s_hours_label));
  layer_add_child(s_time_layer, text_layer_get_layer(s_hours2_label));
  
  // Display the time immediately
  update_time();
}

// Unloads the layers on the main window
static void main_window_unload(Window *window) {
  // Unload the fonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_small_font);
  
  // Destroy the background layers
  layer_destroy(s_solid_layer);
  layer_destroy(s_time_layer);
  layer_destroy(s_box_layer);
  bitmap_layer_destroy(s_background_layer);
  gbitmap_destroy(s_background_bitmap);
  
  // Destroy the labels
  text_layer_destroy(s_hour_label);
  text_layer_destroy(s_minute_label);
  text_layer_destroy(s_hours_label);
  text_layer_destroy(s_hours2_label);
}
  
// Initializes the main window
static void init() {
  s_main_window = window_create(); // Create the main window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true); // Show the main window. Animations = true.
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler); // Update time every minute
}

// Deinitializes the main window
static void deinit() {
  window_destroy(s_main_window); // Destroy the main window
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}