#include <pebble.h>

#define TIMEOUT_MS 1000
#define MAX_READ_SIZE 100
#define UPDATE_SPEED 200
#define FILTER_SIZE 3
#define GESTURE 0
#define JOYSTICK 1

static Window *s_main_window;
static TextLayer *s_status_layer;
static TextLayer *s_mode_text_layer;
static TextLayer *s_raw_text_layer;
static char s_text_buffer1[20];
static char s_text_buffer2[20];
static SmartstrapAttribute *s_raw_attribute;
static SmartstrapAttribute *s_attr_attribute;
static int16_t accel_data[3];
static int16_t xfilter[FILTER_SIZE];
static int16_t yfilter[FILTER_SIZE];
static int16_t zfilter[FILTER_SIZE];
static int filterindex = 0;
static uint8_t mode = 0;

static int16_t calc_nominal(int16_t array[FILTER_SIZE]){
    int nom = 0;
    int i = 0;
    for(i = 0; i < FILTER_SIZE; i++){
        nom += array[i];
    }
    return (int16_t)(nom/FILTER_SIZE);
}

static void data_handler(AccelData *data, uint32_t num_samples) {

    uint32_t i;
    int xavg = 0;
    int yavg = 0;
    int zavg = 0;
    int xnom = 0;
    int ynom = 0;
    int znom = 0;

    for(i = 0; i<num_samples; i++){
        xavg += (int16_t)data[i].x;
        yavg += (int16_t)data[i].y;
        zavg += (int16_t)data[i].z;
    }

    xavg /= 10;
    yavg /= 10;
    zavg /= 10;

    xfilter[filterindex] = xavg;
    yfilter[filterindex] = yavg;
    zfilter[filterindex] = zavg;

    xnom = calc_nominal(xfilter);
    ynom = calc_nominal(yfilter);
    znom = calc_nominal(zfilter);

    accel_data[0] = xavg-xnom;
    accel_data[1] = yavg-ynom;
    accel_data[2] = zavg-znom;
    filterindex = (filterindex+1)%FILTER_SIZE;


}
static void prv_update_mode(void) {
    SmartstrapResult result;
    if (!smartstrap_service_is_available(smartstrap_attribute_get_service_id(s_attr_attribute))) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "s_attr_attribute is not available");
        return;
    }

    // get the write buffer
    uint8_t *buffer = NULL;
    size_t length = 0;
    result = smartstrap_attribute_begin_write(s_attr_attribute, &buffer, &length);
    if (result != SmartstrapResultOk) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Write of s_attr_attribute failed with result %d", result);
        return;
    }

    // write the data into the buffer
    memcpy(buffer, &mode, 1);

    // send it off
    result = smartstrap_attribute_end_write(s_attr_attribute, sizeof(mode), false);
    if (result != SmartstrapResultOk) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Write of s_attr_attribute failed with result %d", result);
    }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    mode = GESTURE;
    text_layer_set_text(s_mode_text_layer, "Mode: Gesture");
    prv_update_mode();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    mode = JOYSTICK;
    text_layer_set_text(s_mode_text_layer, "Mode: Joystick");
    prv_update_mode();
}
static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}
static void prv_update_text(void) {
    if (smartstrap_service_is_available(SMARTSTRAP_RAW_DATA_SERVICE_ID)) {
        text_layer_set_text(s_status_layer, "Connected");
        text_layer_set_background_color(s_status_layer, GColorGreen);
    } else {
        text_layer_set_text(s_status_layer, "Disconnected");
        text_layer_set_background_color(s_status_layer, GColorRed);
    }
}

static void prv_did_read(SmartstrapAttribute *attr, SmartstrapResult result,
        const uint8_t *data, size_t length) {
    if (attr == s_attr_attribute) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "did_read(s_attr_attribute, %d, %d)", result, length);
        if (result == SmartstrapResultOk && length == 4) {
            uint32_t num;
            memcpy(&num, data, 4);
            snprintf(s_text_buffer1, 20, "%u", (unsigned int)num);
            text_layer_set_text(s_mode_text_layer, s_text_buffer1);
        }
    } else if (attr == s_raw_attribute) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "did_read(s_raw_attribute, %d, %d)", result, length);
        if (result == SmartstrapResultOk && length == 4) {
            uint32_t time;
            memcpy(&time, data, 4);
            snprintf(s_text_buffer2, 20, "%u", (unsigned int)time);
            text_layer_set_text(s_raw_text_layer, s_text_buffer2);
        }
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "did_read(<%p>, %d)", attr, result);
    }
}

static void prv_did_write(SmartstrapAttribute *attr, SmartstrapResult result) {
    if (attr == s_attr_attribute) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "did_write(s_attr_attribute, %d)", result);
    } else if (attr == s_raw_attribute) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "did_write(s_raw_attribute, %d)", result);
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "did_write(<%p>, %d)", attr, result);
    }
}

static void prv_write_raw(void) {
    SmartstrapResult result;
    if (!smartstrap_service_is_available(smartstrap_attribute_get_service_id(s_raw_attribute))) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "s_raw_attribute is not available");
        return;
    }

    // get the write buffer
    uint8_t *buffer = NULL;
    size_t length = 0;
    result = smartstrap_attribute_begin_write(s_raw_attribute, &buffer, &length);
    if (result != SmartstrapResultOk) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Write of s_raw_attribute failed with result %d", result);
        return;
    }

    // write the data into the buffer
    memcpy(buffer, &accel_data, sizeof(accel_data));

    // send it off
    result = smartstrap_attribute_end_write(s_raw_attribute, sizeof(accel_data), false);
    if (result != SmartstrapResultOk) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Write of s_attr_attribute failed with result %d", result);
    }
}

/*static void prv_read_raw(void) {
  if (!smartstrap_service_is_available(smartstrap_attribute_get_service_id(s_raw_attribute))) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "s_raw_attribute is not available");
  return;
  }
  SmartstrapResult result = smartstrap_attribute_read(s_raw_attribute);
  if (result != SmartstrapResultOk) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Read of s_raw_attribute failed with result: %d", result);
  }
  }*/

static void prv_send_request(void *context) {
    prv_write_raw();
    /*prv_read_raw();*/
    app_timer_register(UPDATE_SPEED, prv_send_request, NULL);
}

static void prv_availablility_status_changed(SmartstrapServiceId service_id, bool is_available) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Availability for 0x%x is %d", service_id, is_available);
    prv_update_text();
}

static void prv_notified(SmartstrapAttribute *attr) {
    if (attr == s_attr_attribute) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "notified(s_attr_attribute)");
    } else if (attr == s_raw_attribute) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "notified(s_raw_attribute)");
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "notified(<%p>)", attr);
    }
}

static void prv_main_window_load(Window *window) {
    s_status_layer = text_layer_create(GRect(0, 50, 144, 40));
    text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    prv_update_text();
    text_layer_set_text_color(s_status_layer, GColorBlack);
    text_layer_set_background_color(s_status_layer, GColorRed);
    text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(s_status_layer, GTextOverflowModeWordWrap);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_status_layer));

    s_mode_text_layer = text_layer_create(GRect(0, 100, 144, 40));
    text_layer_set_font(s_mode_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text(s_mode_text_layer, "Mode: Gesture");
    text_layer_set_text_color(s_mode_text_layer, GColorBlack);
    text_layer_set_background_color(s_mode_text_layer, GColorClear);
    text_layer_set_text_alignment(s_mode_text_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(s_mode_text_layer, GTextOverflowModeWordWrap);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_mode_text_layer));

    /*s_raw_text_layer = text_layer_create(GRect(0, 100, 144, 40));
      text_layer_set_font(s_raw_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
      text_layer_set_text(s_raw_text_layer, "-");
      text_layer_set_text_color(s_raw_text_layer, GColorBlack);
      text_layer_set_background_color(s_raw_text_layer, GColorClear);
      text_layer_set_text_alignment(s_raw_text_layer, GTextAlignmentCenter);
      text_layer_set_overflow_mode(s_raw_text_layer, GTextOverflowModeWordWrap);
      layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_raw_text_layer));*/
}

static void prv_main_window_unload(Window *window) {
    text_layer_destroy(s_status_layer);
}

static void prv_init(void) {
    s_main_window = window_create();
    window_set_click_config_provider(s_main_window, click_config_provider);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
            .load = prv_main_window_load,
            .unload = prv_main_window_unload
            });
    window_stack_push(s_main_window, true);
    SmartstrapHandlers handlers = (SmartstrapHandlers) {
        .availability_did_change = prv_availablility_status_changed,
            .did_write = prv_did_write,
            .did_read = prv_did_read,
            .notified = prv_notified
    };
    smartstrap_subscribe(handlers);

    /********** Accel addon ************/
    // Subscribe to the accelerometer data service
    int num_samples = 10;
    accel_data_service_subscribe(num_samples, data_handler);
    // Choose update rate
    accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
    /******** End Accel addon **********/

    smartstrap_set_timeout(50);
    s_raw_attribute = smartstrap_attribute_create(0, 0, 2000);
    s_attr_attribute = smartstrap_attribute_create(0x1001, 0x1001, 20);
    app_timer_register(UPDATE_SPEED, prv_send_request, NULL);
}

static void prv_deinit(void) {
    window_destroy(s_main_window);
    smartstrap_unsubscribe();
}

int main(void) {
    prv_init();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "STARTING APP");
    if (s_attr_attribute && s_raw_attribute) {
        app_event_loop();
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "ENDING APP");
    prv_deinit();
}

