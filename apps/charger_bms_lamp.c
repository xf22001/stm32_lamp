

/*================================================================
 *
 *
 *   文件名称：charger_bms_lamp.c
 *   创 建 者：肖飞
 *   创建日期：2021年06月19日 星期六 19时12分21秒
 *   修改日期：2021年12月08日 星期三 16时28分30秒
 *   描    述：
 *
 *================================================================*/
#include "charger_bms_lamp.h"

#include "channel.h"
#include "charger_bms.h"
#include "power_manager.h"
#include "main.h"

#include "log.h"

typedef enum {
	LAMP_LED_0 = 0,
	LAMP_LED_1,
	LAMP_LED_2,
	LAMP_LED_SIZE
} lamp_led_t;

typedef struct {
	uint8_t id;
	GPIO_TypeDef *gpio_port;
	uint16_t gpio_pin;
	GPIO_TypeDef *gpio_port_fb;
	uint16_t gpio_pin_fb;
} led_relay_info_t;

led_relay_info_t led_relay_info_0 = {
	.id = LAMP_LED_0,
	.gpio_port = relay_8_GPIO_Port,
	.gpio_pin = relay_8_Pin,
	.gpio_port_fb = in_5_GPIO_Port,
	.gpio_pin_fb = in_5_Pin,
};

led_relay_info_t led_relay_info_1 = {
	.id = LAMP_LED_1,
	.gpio_port = relay_4_GPIO_Port,
	.gpio_pin = relay_4_Pin,
	.gpio_port_fb = in_5_GPIO_Port,
	.gpio_pin_fb = in_5_Pin,
};

led_relay_info_t led_relay_info_2 = {
	.id = LAMP_LED_2,
	.gpio_port = out_7_GPIO_Port,
	.gpio_pin = out_7_Pin,
	.gpio_port_fb = in_5_GPIO_Port,
	.gpio_pin_fb = in_5_Pin,
};

led_relay_info_t *led_relay_info_sz[] = {
	&led_relay_info_0,
	&led_relay_info_1,
	&led_relay_info_2,
};

typedef struct {
	uint8_t size;
	led_relay_info_t **led_relay_info_sz;
} led_relay_map_t;

led_relay_map_t led_relay_map = {
	.size = ARRAY_SIZE(led_relay_info_sz),
	.led_relay_info_sz = led_relay_info_sz,
};

static led_relay_info_t *get_led_relay_info_by_id(uint8_t id)
{
	led_relay_info_t *led_relay_info = NULL;
	int i;

	for(i = 0; i < led_relay_map.size; i++) {
		led_relay_info_t *led_relay_info_item = led_relay_map.led_relay_info_sz[i];

		if(led_relay_info_item->id == id) {
			led_relay_info = led_relay_info_item;
			break;
		}
	}

	return led_relay_info;
}

typedef struct {
	uint8_t state;
	uint32_t state_stamps;
	bitmap_t *led_config_map;
	bitmap_t *led_action_map;
} charger_bms_ctx_t;

void set_channel_led_onoff(channel_info_t *channel_info, uint8_t id, uint8_t onoff)
{
	charger_info_t *charger_info = (charger_info_t *)channel_info->charger_info;
	charger_bms_ctx_t *charger_bms_ctx = (charger_bms_ctx_t *)charger_info->charger_bms_ctx;
	channel_event_t *channel_event;
	channels_event_t *channels_event;
	channels_info_t *channels_info = (channels_info_t *)channel_info->channels_info;
	uint8_t type;
	int i;

	if(id >= LAMP_LED_SIZE) {
		debug("id:%d invalid!", id);
		return;
	}

	OS_ASSERT(charger_bms_ctx != NULL);
	set_bitmap_value(charger_bms_ctx->led_config_map, id, (onoff != 0) ? 1 : 0);

	i = get_first_value_index(charger_bms_ctx->led_config_map, 1);

	channel_event = os_calloc(1, sizeof(channel_event_t));
	channels_event = os_calloc(1, sizeof(channels_event_t));
	OS_ASSERT(channel_event != NULL);
	OS_ASSERT(channels_event != NULL);

	if(i >= charger_bms_ctx->led_config_map->size) {//关机
		type = CHANNEL_EVENT_TYPE_STOP_CHANNEL;
		channel_info->channel_event_stop.stop_reason = CHANNEL_RECORD_ITEM_STOP_REASON_MANUAL;
	} else {//开机
		type = CHANNEL_EVENT_TYPE_START_CHANNEL;
		channel_info->channel_event_start_display.charge_mode = CHANNEL_RECORD_CHARGE_MODE_UNLIMIT;
		channel_info->channel_event_start_display.start_reason = CHANNEL_RECORD_ITEM_START_REASON_BMS;
	}

	channel_event->channel_id = channel_info->channel_id;
	channel_event->type = type;
	channel_event->ctx = &channel_info->channel_event_start_display;

	channels_event->type = CHANNELS_EVENT_CHANNEL;
	channels_event->event = channel_event;

	if(send_channels_event(channels_info, channels_event, 100) != 0) {
		debug("send channel %d type %d failed!", channel_info->channel_id, type);
	} else {
		debug("send channel %d type %d successful!", channel_info->channel_id, type);
	}
}

static void clean_led_relay_config(charger_info_t *charger_info)
{
	charger_bms_ctx_t *charger_bms_ctx = (charger_bms_ctx_t *)charger_info->charger_bms_ctx;
	memset(charger_bms_ctx->led_action_map->data, 0, charger_bms_ctx->led_action_map->cell_size);
}

static void action_led_relay(charger_info_t *charger_info)
{
	charger_bms_ctx_t *charger_bms_ctx = (charger_bms_ctx_t *)charger_info->charger_bms_ctx;
	channel_info_t *channel_info = charger_info->channel_info;
	int i;

	if(memcmp(charger_bms_ctx->led_config_map->data, charger_bms_ctx->led_action_map->data, charger_bms_ctx->led_action_map->cell_size) == 0) {
		return;
	}

	memcpy(charger_bms_ctx->led_action_map->data, charger_bms_ctx->led_config_map->data, charger_bms_ctx->led_action_map->cell_size);

	for(i = 0; i < charger_bms_ctx->led_action_map->size; i++) {
		led_relay_info_t *led_relay_info = get_led_relay_info_by_id(i);
		GPIO_PinState state = (get_bitmap_value(charger_bms_ctx->led_action_map, i) == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET;

		OS_ASSERT(led_relay_info != NULL);

		HAL_GPIO_WritePin(led_relay_info->gpio_port, led_relay_info->gpio_pin, state);
		debug("channel %d set led %d %s", channel_info->channel_id, i, (state == GPIO_PIN_SET) ? "on" : "off");
	}
}

typedef enum {
	CHARGER_BMS_STATE_IDLE = 0,
	CHARGER_BMS_STATE_STARTING,
	CHARGER_BMS_STATE_CHARGING,
	CHARGER_BMS_STATE_STOPPING,
} charger_bms_state_t;

static int prepare_bms_state_idle(void *_charger_info)
{
	int ret = 0;
	//charger_info_t *charger_info = (charger_info_t *)_charger_info;

	return ret;
}

static int handle_request_bms_state_idle(void *_charger_info)
{
	int ret = 0;

	charger_info_t *charger_info = (charger_info_t *)_charger_info;
	//channel_info_t *channel_info = charger_info->channel_info;

	if(charger_info->charger_bms_request_action == CHARGER_BMS_REQUEST_ACTION_START) {
		charger_info->charger_bms_request_action = CHARGER_BMS_REQUEST_ACTION_NONE;

		set_charger_bms_request_state(charger_info, CHARGER_BMS_STATE_STARTING);
	}

	return ret;
}

static int prepare_bms_state_starting(void *_charger_info)
{
	int ret = 0;
	charger_info_t *charger_info = (charger_info_t *)_charger_info;
	charger_bms_ctx_t *charger_bms_ctx = (charger_bms_ctx_t *)charger_info->charger_bms_ctx;

	charger_bms_ctx->state = 0;

	return ret;
}

static int handle_request_bms_state_starting(void *_charger_info)
{
	int ret = 0;

	charger_info_t *charger_info = (charger_info_t *)_charger_info;
	//channel_info_t *channel_info = charger_info->channel_info;
	//charger_bms_ctx_t *charger_bms_ctx = (charger_bms_ctx_t *)charger_info->charger_bms_ctx;
	//uint32_t ticks = osKernelSysTick();

	if(charger_info->charger_bms_request_action == CHARGER_BMS_REQUEST_ACTION_STOP) {
		charger_info->charger_bms_request_action = CHARGER_BMS_REQUEST_ACTION_NONE;

		set_charger_bms_request_state(charger_info, CHARGER_BMS_STATE_STOPPING);
		return ret;
	}

	set_charger_bms_request_state(charger_info, CHARGER_BMS_STATE_CHARGING);

	return ret;
}

static int prepare_bms_state_charging(void *_charger_info)
{
	int ret = 0;
	charger_info_t *charger_info = (charger_info_t *)_charger_info;
	channel_info_t *channel_info = charger_info->channel_info;
	channels_info_t *channels_info = (channels_info_t *)channel_info->channels_info;
	power_manager_info_t *power_manager_info = (power_manager_info_t *)channels_info->power_manager_info;

	channel_request_charging(channel_info);
	set_power_manager_channel_request_state(power_manager_info, channel_info->channel_id, POWER_MANAGER_CHANNEL_REQUEST_STATE_START);

	return ret;
}

static int handle_request_bms_state_charging(void *_charger_info)
{
	int ret = 0;
	charger_info_t *charger_info = (charger_info_t *)_charger_info;
	channel_info_t *channel_info = charger_info->channel_info;
	channels_info_t *channels_info = (channels_info_t *)channel_info->channels_info;
	power_manager_info_t *power_manager_info = (power_manager_info_t *)channels_info->power_manager_info;
	//charger_bms_ctx_t *charger_bms_ctx = (charger_bms_ctx_t *)charger_info->charger_bms_ctx;

	if(charger_info->charger_bms_request_action == CHARGER_BMS_REQUEST_ACTION_STOP) {
		charger_info->charger_bms_request_action = CHARGER_BMS_REQUEST_ACTION_NONE;

		set_power_manager_channel_request_state(power_manager_info, channel_info->channel_id, POWER_MANAGER_CHANNEL_REQUEST_STATE_STOP);
		set_charger_bms_request_state(charger_info, CHARGER_BMS_STATE_STOPPING);
	}

	action_led_relay(charger_info);

	return ret;
}

static int prepare_bms_state_stopping(void *_charger_info)
{
	int ret = 0;
	charger_info_t *charger_info = (charger_info_t *)_charger_info;
	channel_info_t *channel_info = charger_info->channel_info;
	charger_bms_ctx_t *charger_bms_ctx = (charger_bms_ctx_t *)charger_info->charger_bms_ctx;
	channel_request_end(channel_info);
	clean_led_relay_config(charger_info);
	action_led_relay(charger_info);

	charger_bms_ctx->state = 0;

	return ret;
}

static int handle_request_bms_state_stopping(void *_charger_info)
{
	int ret = 0;

	charger_info_t *charger_info = (charger_info_t *)_charger_info;
	channel_info_t *channel_info = charger_info->channel_info;
	//charger_bms_ctx_t *charger_bms_ctx = (charger_bms_ctx_t *)charger_info->charger_bms_ctx;
	//uint32_t ticks = osKernelSysTick();

	channel_request_idle(channel_info);
	set_charger_bms_request_state(charger_info, CHARGER_BMS_STATE_IDLE);

	return ret;
}

static charger_bms_state_handler_t state_handler_sz[] = {
	{
		.bms_state = CHARGER_BMS_STATE_IDLE,
		.prepare = prepare_bms_state_idle,
		.handle_request = handle_request_bms_state_idle,
	},
	{
		.bms_state = CHARGER_BMS_STATE_STARTING,
		.prepare = prepare_bms_state_starting,
		.handle_request = handle_request_bms_state_starting,
	},
	{
		.bms_state = CHARGER_BMS_STATE_CHARGING,
		.prepare = prepare_bms_state_charging,
		.handle_request = handle_request_bms_state_charging,
	},
	{
		.bms_state = CHARGER_BMS_STATE_STOPPING,
		.prepare = prepare_bms_state_stopping,
		.handle_request = handle_request_bms_state_stopping,
	},
};

static charger_bms_state_handler_t *get_charger_bms_state_handler(uint8_t bms_state)
{
	int i;
	charger_bms_state_handler_t *charger_bms_state_handler = NULL;

	for(i = 0; i < ARRAY_SIZE(state_handler_sz); i++) {
		charger_bms_state_handler_t *charger_bms_state_handler_item = &state_handler_sz[i];

		if(charger_bms_state_handler_item->bms_state == bms_state) {
			charger_bms_state_handler = charger_bms_state_handler_item;
		}
	}

	return charger_bms_state_handler;
}

static char *get_charger_bms_state_des(uint8_t state)
{
	char *des = NULL;

	switch(state) {
			add_des_case(CHARGER_BMS_STATE_IDLE);
			add_des_case(CHARGER_BMS_STATE_STARTING);
			add_des_case(CHARGER_BMS_STATE_CHARGING);
			add_des_case(CHARGER_BMS_STATE_STOPPING);

		default: {
			des = "unknow state";
		}
		break;
	}

	return des;
}

static void update_charger_bms_state(charger_info_t *charger_info)
{
	channel_info_t *channel_info = charger_info->channel_info;
	charger_bms_state_handler_t *charger_bms_state_handler = NULL;
	uint8_t request_state = charger_info->request_state;

	if((charger_info->state == request_state) && (charger_info->charger_bms_state_handler != NULL)) {
		return;
	}

	charger_bms_state_handler = get_charger_bms_state_handler(request_state);
	OS_ASSERT(charger_bms_state_handler != NULL);

	debug("charger %d change state: %s -> %s!", channel_info->channel_id, get_charger_bms_state_des(charger_info->state), get_charger_bms_state_des(request_state));

	charger_info->state = request_state;

	if(charger_bms_state_handler->prepare != NULL) {
		charger_bms_state_handler->prepare(charger_info);
	}

	charger_info->charger_bms_state_handler = charger_bms_state_handler;
}

static int handle_request(charger_info_t *charger_info)
{
	int ret = 0;

	if(charger_info->charger_bms_state_handler == NULL) {
		debug("");
		return ret;
	}

	ret = charger_info->charger_bms_state_handler->handle_request(charger_info);

	return ret;
}

static void charger_handle_request(charger_info_t *charger_info)
{
	int ret;

	mutex_lock(charger_info->handle_mutex);

	ret = handle_request(charger_info);

	if(ret != 0) {
	}

	mutex_unlock(charger_info->handle_mutex);
}

static void charger_bms_periodic(void *_charger_info, void *_channels_info)
{
	charger_info_t *charger_info = (charger_info_t *)_charger_info;

	update_charger_bms_state(charger_info);
	charger_handle_request(charger_info);
}

static void modify_valid_time(void)
{
	struct tm tm;
	time_t ts;

	tm.tm_year = 2021 - 1900;
	tm.tm_mon = 1 - 1;
	tm.tm_mon = 1 - 1;
	tm.tm_mday = 1;
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;
	ts = mktime(&tm);
	set_time(ts);
}

static int init_lamp(void *_charger_info)
{
	int ret = 0;
	charger_info_t *charger_info = (charger_info_t *)_charger_info;
	channel_info_t *channel_info = charger_info->channel_info;
	channels_info_t *channels_info = (channels_info_t *)channel_info->channels_info;
	charger_bms_ctx_t *charger_bms_ctx = os_calloc(1, sizeof(charger_bms_ctx_t));

	OS_ASSERT(charger_bms_ctx != NULL);
	OS_ASSERT(charger_info->charger_bms_ctx == NULL);
	charger_info->charger_bms_ctx = charger_bms_ctx;

	charger_bms_ctx->led_config_map = alloc_bitmap(LAMP_LED_SIZE);
	OS_ASSERT(charger_bms_ctx->led_config_map != NULL);
	charger_bms_ctx->led_action_map = alloc_bitmap(LAMP_LED_SIZE);
	OS_ASSERT(charger_bms_ctx->led_action_map != NULL);

	charger_info->charger_bms_state_handler = NULL;
	set_charger_bms_request_state(charger_info, CHARGER_BMS_STATE_IDLE);
	update_charger_bms_state(charger_info);

	charger_info->periodic_request_cb.fn = charger_bms_periodic;
	charger_info->periodic_request_cb.fn_ctx = charger_info;
	OS_ASSERT(register_callback(channels_info->common_periodic_chain, &charger_info->periodic_request_cb) == 0);

	modify_valid_time();

	return ret;
}

static int deinit_lamp(void *_charger_info)
{
	int ret = 0;
	charger_info_t *charger_info = (charger_info_t *)_charger_info;
	channel_info_t *channel_info = charger_info->channel_info;
	channels_info_t *channels_info = (channels_info_t *)channel_info->channels_info;
	charger_bms_ctx_t *charger_bms_ctx = (charger_bms_ctx_t *)charger_info->charger_bms_ctx;

	OS_ASSERT(remove_callback(channels_info->common_periodic_chain, &charger_info->periodic_request_cb) == 0);

	charger_info->charger_bms_ctx = NULL;

	OS_ASSERT(charger_bms_ctx != NULL);
	os_free(charger_bms_ctx);

	return ret;
}

charger_bms_handler_t charger_bms_handler_lamp = {
	.charger_type = CHANNEL_CHARGER_BMS_TYPE_LAMP,
	.init = init_lamp,
	.deinit = deinit_lamp,
};
