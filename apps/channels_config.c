

/*================================================================
 *
 *
 *   文件名称：channels_config.c
 *   创 建 者：肖飞
 *   创建日期：2021年01月18日 星期一 09时26分44秒
 *   修改日期：2021年12月07日 星期二 13时40分37秒
 *   描    述：
 *
 *================================================================*/
#include "channels_config.h"
#include "os_utils.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;
extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi3;

char *get_channel_config_channel_type_des(channel_type_t type)
{
	char *des = "unknow";

	switch(type) {
			add_des_case(CHANNEL_TYPE_NONE);
			add_des_case(CHANNEL_TYPE_NATIVE);
			add_des_case(CHANNEL_TYPE_PROXY_REMOTE);
			add_des_case(CHANNEL_TYPE_PROXY_LOCAL);

		default: {
		}
		break;
	}

	return des;
}

char *get_channel_config_charger_bms_type_des(channel_charger_bms_type_t type)
{
	char *des = "unknow";

	switch(type) {
			add_des_case(CHANNEL_CHARGER_BMS_TYPE_NONE);
			add_des_case(CHANNEL_CHARGER_BMS_TYPE_GB);
			add_des_case(CHANNEL_CHARGER_BMS_TYPE_AC);

		default: {
		}
		break;
	}

	return des;
}

char *get_channel_config_energy_meter_type_des(energy_meter_type_t type)
{
	char *des = "unknow";

	switch(type) {
			add_des_case(ENERGY_METER_TYPE_NONE);
			add_des_case(ENERGY_METER_TYPE_AC);
			add_des_case(ENERGY_METER_TYPE_AC_HLW8032);
			add_des_case(ENERGY_METER_TYPE_DC);
			add_des_case(ENERGY_METER_TYPE_AC_SDM_220);
			add_des_case(ENERGY_METER_TYPE_AC_SDM_630);
			add_des_case(ENERGY_METER_TYPE_PROXY);

		default: {
		}
		break;
	}

	return des;
}

char *get_channel_config_function_board_type_des(function_board_type_t type)
{
	char *des = "unknow";

	switch(type) {
			add_des_case(FUNCTION_BOARD_TYPE_NONE);
			add_des_case(FUNCTION_BOARD_TYPE_485);
			add_des_case(FUNCTION_BOARD_TYPE_MODBUS);

		default: {
		}
		break;
	}

	return des;
}

char *get_card_reader_type_des(card_reader_type_t type)
{
	char *des = "unknow";

	switch(type) {
			add_des_case(CARD_READER_TYPE_PSEUDO);
			add_des_case(CARD_READER_TYPE_MT_318_626);
			add_des_case(CARD_READER_TYPE_MT_318_628);
			add_des_case(CARD_READER_TYPE_ZLG);

		default: {
		}
		break;
	}

	return des;
}

char *get_power_manager_type_des(power_manager_type_t type)
{
	char *des = "unknow";

	switch(type) {
			add_des_case(POWER_MANAGER_TYPE_NATIVE);

		default: {
		}
		break;
	}

	return des;
}

static energy_meter_config_item_t energy_meter_config_item_0_0 = {
	.type = ENERGY_METER_TYPE_DC,
	.huart = &huart3,
};

static energy_meter_config_item_t *energy_meter_config_item_0_sz[] = {
	&energy_meter_config_item_0_0,
};

static function_board_config_item_t function_board_config_item_0_0 = {
	.type = FUNCTION_BOARD_TYPE_485,
	.huart = &huart3,
};

static function_board_config_item_t *function_board_config_item_0_sz[] = {
	&function_board_config_item_0_0,
};

static channel_config_t channel0_config = {
	.channel_type = CHANNEL_TYPE_NATIVE,
	.charger_config = {
		.charger_type = CHANNEL_CHARGER_BMS_TYPE_GB,
		.hcan_bms = &hcan2,
	},
	.energy_meter_config = {
		.default_type = ENERGY_METER_TYPE_NONE,
		.size = ARRAY_SIZE(energy_meter_config_item_0_sz),
		.items = energy_meter_config_item_0_sz,
	},
	.function_board_config = {
		.default_type = FUNCTION_BOARD_TYPE_NONE,
		.size = ARRAY_SIZE(function_board_config_item_0_sz),
		.items = function_board_config_item_0_sz,
	},
};

//static channel_config_t channel1_config = {
//	.channel_type = CHANNEL_TYPE_PROXY_REMOTE,
//};

//static energy_meter_config_item_t energy_meter_config_item_2_0 = {
//	.type = ENERGY_METER_TYPE_DC,
//	.huart = &huart3,
//};
//
//static energy_meter_config_item_t *energy_meter_config_item_2_sz[] = {
//	&energy_meter_config_item_2_0,
//};
//
//static function_board_config_item_t function_board_config_item_2_0 = {
//	.type = FUNCTION_BOARD_TYPE_485,
//	.huart = &huart3,
//};
//
//static function_board_config_item_t *function_board_config_item_2_sz[] = {
//	&function_board_config_item_2_0,
//};
//
//
//static channel_config_t channel2_config = {
//	.channel_type = CHANNEL_TYPE_PROXY_LOCAL,
//	.charger_config = {
//		.charger_type = CHANNEL_CHARGER_BMS_TYPE_GB,
//		.hcan_bms = &hcan2,
//	},
//	.energy_meter_config = {
//		.default_type = ENERGY_METER_TYPE_DC,
//		.size = ARRAY_SIZE(energy_meter_config_item_2_sz),
//		.items = energy_meter_config_item_2_sz,
//	},
//	.function_board_config = {
//		.default_type = FUNCTION_BOARD_TYPE_485,
//		.size = ARRAY_SIZE(function_board_config_item_2_sz),
//		.items = function_board_config_item_2_sz,
//	},
//};

static channel_config_t *channel_config_sz[] = {
	&channel0_config,
	//&channel1_config,
	//&channel2_config,
};

//static card_reader_config_item_t card_reader_config_item_0 = {
//	.type = CARD_READER_TYPE_ZLG,
//	.huart = &huart2,
//};

static card_reader_config_item_t card_reader_config_item_0 = {
	.type = CARD_READER_TYPE_PSEUDO,
	.huart = NULL,
};

static card_reader_config_item_t *card_reader_config_item_sz[] = {
	&card_reader_config_item_0,
};

power_manager_group_config_t power_manager_group_config_item_0 = {
	.channel_number = ARRAY_SIZE(channel_config_sz),
	.power_module_group_number = 8,
	.power_module_number_per_power_module_group = 1,
};

static power_manager_group_config_t *power_manager_group_config_sz[] = {
	&power_manager_group_config_item_0,
};

static channels_config_t channels_config_0 = {
	.id = 0,
	.channel_number = ARRAY_SIZE(channel_config_sz),
	.channel_config = channel_config_sz,
	.power_module_config = {
		.channels_power_module_number = 8,
		.hcan = &hcan1,
		.channels_power_module_type = CHANNELS_POWER_MODULE_TYPE_NATIVE,
	},
	.voice_config = {

	},
	.card_reader_config = {
		.default_type = CARD_READER_TYPE_PSEUDO,
		.size = ARRAY_SIZE(card_reader_config_item_sz),
		.items = card_reader_config_item_sz,
	},
	.display_config = {
		//.huart = &huart6,
	},
	.proxy_channel_info = {
		.hcan = &hcan1,
	},
	.power_manager_config = {
		.type = POWER_MANAGER_TYPE_NATIVE,
		.power_manager_group_number = ARRAY_SIZE(power_manager_group_config_sz),
		.power_manager_group_config = power_manager_group_config_sz,
	},
	.board_temperature_adc = &hadc1,
	.board_temperature_adc_rank = 0,
};

static channels_config_t *channels_config_sz[] = {
	&channels_config_0,
};

channels_config_t *get_channels_config(uint8_t id)
{
	int i;
	channels_config_t *channels_config = NULL;
	channels_config_t *channels_config_item = NULL;

	for(i = 0; i < ARRAY_SIZE(channels_config_sz); i++) {
		channels_config_item = channels_config_sz[i];

		if(channels_config_item->id == id) {
			channels_config = channels_config_item;
			break;
		}
	}

	return channels_config;
}

energy_meter_config_item_t *get_energy_meter_config_item(channel_config_t *channel_config, energy_meter_type_t type)
{
	int i;
	energy_meter_config_t *energy_meter_config = &channel_config->energy_meter_config;
	energy_meter_config_item_t *energy_meter_config_item = NULL;

	for(i = 0; i < energy_meter_config->size; i++) {
		energy_meter_config_item_t *_energy_meter_config_item = energy_meter_config->items[i];

		if(_energy_meter_config_item->type == type) {
			energy_meter_config_item = _energy_meter_config_item;
			break;
		}
	}

	return energy_meter_config_item;
}

function_board_config_item_t *get_function_board_config_item(channel_config_t *channel_config, function_board_type_t type)
{
	int i;
	function_board_config_t *function_board_config = &channel_config->function_board_config;
	function_board_config_item_t *function_board_config_item = NULL;

	for(i = 0; i < function_board_config->size; i++) {
		function_board_config_item_t *_function_board_config_item = function_board_config->items[i];

		if(_function_board_config_item->type == type) {
			function_board_config_item = _function_board_config_item;
			break;
		}
	}

	return function_board_config_item;
}

card_reader_config_item_t *get_card_reader_config_item(channels_config_t *channels_config, card_reader_type_t type)
{
	int i;
	card_reader_config_t *card_reader_config = &channels_config->card_reader_config;
	card_reader_config_item_t *card_reader_config_item = NULL;

	for(i = 0; i < card_reader_config->size; i++) {
		card_reader_config_item_t *_card_reader_config_item = card_reader_config->items[i];

		if(_card_reader_config_item->type == type) {
			card_reader_config_item = _card_reader_config_item;
			break;
		}
	}

	return card_reader_config_item;
}

proxy_channel_item_t *get_proxy_channel_item_by_proxy_channel_index(proxy_channel_info_t *proxy_channel_info, uint8_t proxy_channel_index)
{
	proxy_channel_item_t *item = NULL;
	int i;

	if(proxy_channel_info->proxy_channel_number == 0) {
		return item;
	}

	if(proxy_channel_info->items == NULL) {
		return item;
	}

	for(i = 0; i < proxy_channel_info->proxy_channel_number; i++) {
		proxy_channel_item_t *_item = proxy_channel_info->items + i;

		if(_item->proxy_channel_index == proxy_channel_index) {
			item = _item;
			break;
		}
	}

	return item;
}

proxy_channel_item_t *get_proxy_channel_item_by_channel_id(proxy_channel_info_t *proxy_channel_info, uint8_t channel_id)
{
	proxy_channel_item_t *item = NULL;
	int i;

	if(proxy_channel_info->proxy_channel_number == 0) {
		return item;
	}

	if(proxy_channel_info->items == NULL) {
		return item;
	}

	for(i = 0; i < proxy_channel_info->proxy_channel_number; i++) {
		proxy_channel_item_t *_item = proxy_channel_info->items + i;

		if(_item->channel_id == channel_id) {
			item = _item;
			break;
		}
	}

	return item;
}
