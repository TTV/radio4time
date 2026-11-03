/* --------------------------------------
Radio4time Pebble watchface
Version: 3.0

http://www.cleggo.co.uk

(C)TTV 2014
-------------------------------------- */

#include "pebble.h"
#include "num2words.h"
#include "oblique.h"

#define BUFFER_SIZE 86

// sent keys
enum {
  TTVNEWS_METHOD = 0x0
};

// receive keys
enum {
  TTVNEWS_TITLE = 0x1
};

bool refetch_story = false;

static struct CommonWordsData {
  TextLayer *label_1, *label_2, *label_3, *news_layer;
  BitmapLayer *connection_layer, *battery_layer;
  Window *window;
  char hBuffer[BUFFER_SIZE];
  char buffer[BUFFER_SIZE];
  char dBuffer[BUFFER_SIZE];
  char newsBuffer[256];
  GBitmap *image_con, *image_dis, *image_battery_1, *image_battery_2, *image_battery_3, *image_battery_4, *image_battery_5, *image_battery_c;
  bool connected;
} s_data;

static void fetch_story(void) {
  refetch_story = false;

  Tuplet method_tuple = TupletCString(TTVNEWS_METHOD, "story");
	
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &method_tuple);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *method_tuple = dict_find(iter, TTVNEWS_METHOD);

  if (!method_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Missing method");
    return;
  }

  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Method : %s", method_tuple->value->cstring);	
	
  if (strcmp(method_tuple->value->cstring, "ready") == 0)
	  refetch_story = true; // first msg fails so re-fetch on the fail handler
  else if (strcmp(method_tuple->value->cstring, "story") == 0) {
	  Tuple *title_tuple = dict_find(iter, TTVNEWS_TITLE);
	  if (title_tuple)
		strncpy(s_data.newsBuffer, title_tuple->value->cstring, 255);
	  else
	    snprintf(s_data.newsBuffer, 255, "BBC News");
	  text_layer_set_text(s_data.news_layer, s_data.newsBuffer);
  }
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  if (refetch_story)
    fetch_story(); // first call which triggers ready always fails
  else
  	APP_LOG(APP_LOG_LEVEL_DEBUG, "out_failed_handler %d", reason);
}
	
static void handle_battery(BatteryChargeState charge_state) {
  if (charge_state.is_charging)
	bitmap_layer_set_bitmap(s_data.battery_layer, s_data.image_battery_c );
  else if (charge_state.charge_percent > 80)
	bitmap_layer_set_bitmap(s_data.battery_layer, s_data.image_battery_1 );
  else if (charge_state.charge_percent > 60)
	bitmap_layer_set_bitmap(s_data.battery_layer, s_data.image_battery_2 );
  else if (charge_state.charge_percent > 40)
	bitmap_layer_set_bitmap(s_data.battery_layer, s_data.image_battery_3 );
  else if (charge_state.charge_percent > 20)
	bitmap_layer_set_bitmap(s_data.battery_layer, s_data.image_battery_4 );
  else
	bitmap_layer_set_bitmap(s_data.battery_layer, s_data.image_battery_5 );
}

static void handle_bluetooth(bool connected) {
  if (!connected && s_data.connected)
    vibes_short_pulse();
  s_data.connected = connected;
  bitmap_layer_set_bitmap(s_data.connection_layer, connected ? s_data.image_con : s_data.image_dis );
}

static void update_time(struct tm *t) {
  /*
  t->tm_hour = 8;
  t->tm_min = 25;
  */
  fuzzy_time_to_words(t->tm_hour, t->tm_min, s_data.hBuffer, BUFFER_SIZE, s_data.buffer, BUFFER_SIZE);
  text_layer_set_text(s_data.label_1, s_data.hBuffer);
  text_layer_set_text(s_data.label_2, s_data.buffer);

  char *date = s_data.dBuffer;
  size_t remaining = BUFFER_SIZE;
  size_t sz;
  sz = strftime(date, remaining, "%e", t);
  remaining -= sz;
  date += sz;

  if ((t->tm_mday == 1) || (t->tm_mday == 21) || (t->tm_mday == 31))
	  sz = snprintf(date, remaining, "st");
  else if ((t->tm_mday == 2) || (t->tm_mday == 22))
	  sz = snprintf(date, remaining, "nd");
  else if ((t->tm_mday == 3) || (t->tm_mday == 23))
	  sz = snprintf(date, remaining, "rd");
  else
	  sz = snprintf(date, remaining, "th");
  remaining -= sz;
  date += sz;
	
  sz = strftime(date, remaining, " %B", t);
	
  text_layer_set_text(s_data.label_3, s_data.dBuffer);

  text_layer_set_text(s_data.news_layer, s_data.newsBuffer);
	
  handle_battery(battery_state_service_peek());
  handle_bluetooth(bluetooth_connection_service_peek());

  if (t->tm_min == 0)
    fetch_story();
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void do_init(void) {
  s_data.connected = false;
  s_data.window = window_create();
  const bool animated = true;
  window_stack_push(s_data.window, animated);

  window_set_background_color(s_data.window, GColorBlack);
  GFont font_s = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  GFont font_l = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
  GFont font_t = fonts_get_system_font(FONT_KEY_GOTHIC_14);

  s_data.image_con = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECTED);
  s_data.image_dis = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DISCONNECTED);
  s_data.image_battery_1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_1);
  s_data.image_battery_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_2);
  s_data.image_battery_3 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_3);
  s_data.image_battery_4 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_4);
  s_data.image_battery_5 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_5);
  s_data.image_battery_c = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_C);
	
  Layer *root_layer = window_get_root_layer(s_data.window);
  GRect frame = layer_get_frame(root_layer);

  s_data.label_1 = text_layer_create(GRect(0, 0, frame.size.w, 28));
  text_layer_set_background_color(s_data.label_1, GColorBlack);
  text_layer_set_text_color(s_data.label_1, GColorWhite);
  text_layer_set_font(s_data.label_1, font_s);
  layer_add_child(root_layer, text_layer_get_layer(s_data.label_1));
	
  s_data.label_2 = text_layer_create(GRect(0, 28, frame.size.w, 95));
  text_layer_set_background_color(s_data.label_2, GColorBlack);
  text_layer_set_text_color(s_data.label_2, GColorWhite);
  text_layer_set_font(s_data.label_2, font_l);
  layer_add_child(root_layer, text_layer_get_layer(s_data.label_2));

  s_data.label_3 = text_layer_create(GRect(0, 123, frame.size.w, 28));
  text_layer_set_background_color(s_data.label_3, GColorBlack);
  text_layer_set_text_color(s_data.label_3, GColorWhite);
  text_layer_set_font(s_data.label_3, font_s);
  layer_add_child(root_layer, text_layer_get_layer(s_data.label_3));

  s_data.battery_layer = bitmap_layer_create(GRect(frame.size.w - 16, 14, 16, 9));
  bitmap_layer_set_bitmap(s_data.battery_layer, s_data.image_battery_c);
  bitmap_layer_set_alignment(s_data.battery_layer, GAlignTopRight);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_data.battery_layer));

  s_data.connection_layer = bitmap_layer_create(GRect(frame.size.w - 13, 130, 13, 21));
  bitmap_layer_set_bitmap(s_data.connection_layer, s_data.image_dis);
  bitmap_layer_set_alignment(s_data.connection_layer, GAlignBottomRight);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_data.connection_layer));

  s_data.news_layer = text_layer_create(GRect(0, 153, frame.size.w, frame.size.h - 153));
  text_layer_set_background_color(s_data.news_layer, GColorBlack);
  text_layer_set_text_color(s_data.news_layer, GColorWhite);
  text_layer_set_font(s_data.news_layer, font_t);
  layer_add_child(root_layer, text_layer_get_layer(s_data.news_layer));	

  snprintf(s_data.newsBuffer, 255, "BBC News");	
	
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);

  app_message_register_inbox_received(in_received_handler);
  app_message_register_outbox_failed(out_failed_handler);

  app_message_open(64, 64);

  fetch_story();
}

static void do_deinit(void) {
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  gbitmap_destroy(s_data.image_con);
  gbitmap_destroy(s_data.image_dis);
  gbitmap_destroy(s_data.image_battery_1);
  gbitmap_destroy(s_data.image_battery_2);
  gbitmap_destroy(s_data.image_battery_3);
  gbitmap_destroy(s_data.image_battery_4);
  gbitmap_destroy(s_data.image_battery_5);
  gbitmap_destroy(s_data.image_battery_c);
  text_layer_destroy(s_data.label_1);
  text_layer_destroy(s_data.label_2);
  text_layer_destroy(s_data.label_3);
  text_layer_destroy(s_data.news_layer);
  bitmap_layer_destroy(s_data.battery_layer);
  bitmap_layer_destroy(s_data.connection_layer);
  window_destroy(s_data.window);
}

int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}
