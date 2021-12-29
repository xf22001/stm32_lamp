

/*================================================================
 *
 *
 *   文件名称：channels_addr_handler.c
 *   创 建 者：肖飞
 *   创建日期：2021年07月16日 星期五 14时03分28秒
 *   修改日期：2021年12月29日 星期三 11时12分32秒
 *   描    述：
 *
 *================================================================*/
#include "modbus_data_value.h"
#include "app.h"
#include "channels.h"
#include "display.h"
#include "power_manager.h"

#include "log.h"

int get_channel_led_state(channel_info_t *channel_info, uint8_t id);
void set_channel_led_onoff(channel_info_t *channel_info, uint8_t id, uint8_t onoff);

void channels_modbus_data_action(void *fn_ctx, void *chain_ctx)
{
	channels_info_t *channels_info = (channels_info_t *)fn_ctx;
	modbus_data_ctx_t *modbus_data_ctx = (modbus_data_ctx_t *)chain_ctx;

	switch(modbus_data_ctx->addr) {
		case 0: {//软件版本 major
			modbus_data_value_r(modbus_data_ctx, VER_MAJOR);
		}
		break;

		case 1: {//软件版本 minor
			modbus_data_value_r(modbus_data_ctx, VER_MINOR);
		}
		break;

		case 2: {//软件版本 rev
			modbus_data_value_r(modbus_data_ctx, VER_REV);
		}
		break;

		case 100: {//需求电压 0.1v
			uint8_t channel_id = 0;
			channel_info_t *channel_info = channels_info->channel_info + channel_id;
			modbus_data_value_rw(modbus_data_ctx, channel_info->require_voltage);
		}
		break;

		case 101: {//需求电流 0.1a
			uint8_t channel_id = 0;
			channel_info_t *channel_info = channels_info->channel_info + channel_id;
			modbus_data_value_rw(modbus_data_ctx, channel_info->require_current);
		}
		break;

		case 102: {//输出电压 0.1v
			uint8_t channel_id = 0;
			power_manager_info_t *power_manager_info = channels_info->power_manager_info;
			power_manager_channel_info_t *power_manager_channel_info = power_manager_info->power_manager_channel_info + channel_id;
			modbus_data_value_r(modbus_data_ctx, power_manager_channel_info->status.charge_output_voltage_module);
		}
		break;

		case 103: {//输出电流 0.1a
			uint8_t channel_id = 0;
			power_manager_info_t *power_manager_info = channels_info->power_manager_info;
			power_manager_channel_info_t *power_manager_channel_info = power_manager_info->power_manager_channel_info + channel_id;
			modbus_data_value_r(modbus_data_ctx, power_manager_channel_info->status.charge_output_current_module);
		}
		break;

		case 104: {//1灯开关 0-off 1-on
			uint8_t channel_id = 0;
			uint8_t index = 0;
			channel_info_t *channel_info = channels_info->channel_info + channel_id;

			if(modbus_data_ctx->action == MODBUS_DATA_ACTION_GET) {
				modbus_data_ctx->value = get_channel_led_state(channel_info, index);
			} else if(modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) {
				set_channel_led_onoff(channel_info, index, modbus_data_ctx->value);
			}
		}
		break;

		case 105: {//2灯开关 0-off 1-on
			uint8_t channel_id = 0;
			uint8_t index = 1;
			channel_info_t *channel_info = channels_info->channel_info + channel_id;

			if(modbus_data_ctx->action == MODBUS_DATA_ACTION_GET) {
				modbus_data_ctx->value = get_channel_led_state(channel_info, index);
			} else if(modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) {
				set_channel_led_onoff(channel_info, index, modbus_data_ctx->value);
			}
		}
		break;

		case 106: {//3灯开关 0-off 1-on
			uint8_t channel_id = 0;
			uint8_t index = 2;
			channel_info_t *channel_info = channels_info->channel_info + channel_id;

			if(modbus_data_ctx->action == MODBUS_DATA_ACTION_GET) {
				modbus_data_ctx->value = get_channel_led_state(channel_info, index);
			} else if(modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) {
				set_channel_led_onoff(channel_info, index, modbus_data_ctx->value);
			}
		}
		break;

		case 107: {//总灯开关 1-on
			uint8_t channel_id = 0;
			int i;
			channel_info_t *channel_info = channels_info->channel_info + channel_id;

			if(modbus_data_ctx->action == MODBUS_DATA_ACTION_GET) {
				uint8_t value = 1;

				for(i = 0; i < 3; i++) {
					if(get_channel_led_state(channel_info, i) == 0) {
						value = 0;
						break;
					}
				}

				//所有灯状态1, 返回1
				modbus_data_ctx->value = value;
			} else if(modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) {
				if(modbus_data_ctx->value == 1) {
					for(i = 0; i < 3; i++) {
						set_channel_led_onoff(channel_info, i, 1);
					}
				}
			}
		}
		break;

		case 108: {//总灯开关 1-off
			uint8_t channel_id = 0;
			int i;
			channel_info_t *channel_info = channels_info->channel_info + channel_id;

			if(modbus_data_ctx->action == MODBUS_DATA_ACTION_GET) {
				uint8_t value = 1;

				for(i = 0; i < 3; i++) {
					if(get_channel_led_state(channel_info, i) == 1) {
						value = 0;
						break;
					}
				}

				//所有灯状态0, 返回1
				modbus_data_ctx->value = value;
			} else if(modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) {
				if(modbus_data_ctx->value == 1) {
					for(i = 0; i < 3; i++) {
						set_channel_led_onoff(channel_info, i, 0);
					}
				}
			}
		}
		break;

		case 200: {//门禁开关 0-正常 1故障
			modbus_data_value_r(modbus_data_ctx, get_fault(channels_info->faults, CHANNELS_FAULT_DOOR));
		}
		break;

		case 201: {//急停开关 0-正常 1故障
			modbus_data_value_r(modbus_data_ctx, get_fault(channels_info->faults, CHANNELS_FAULT_FORCE_STOP));
		}
		break;

		case 202: {//p粘连 0-正常 1故障
			uint8_t channel_id = 0;
			channel_info_t *channel_info = channels_info->channel_info + channel_id;

			modbus_data_value_r(modbus_data_ctx, get_fault(channel_info->faults, CHANNEL_FAULT_ADHESION_P));
		}
		break;

		case 203: {//n粘连 0-正常 1故障
			uint8_t channel_id = 0;
			channel_info_t *channel_info = channels_info->channel_info + channel_id;

			modbus_data_value_r(modbus_data_ctx, get_fault(channel_info->faults, CHANNEL_FAULT_ADHESION_N));
		}
		break;

		default:
			debug("error! op:%s, addr:%d",
			      (modbus_data_ctx->action == MODBUS_DATA_ACTION_GET) ? "get" :
			      (modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) ? "set" :
			      "unknow",
			      modbus_data_ctx->addr);
			break;
	}
}
