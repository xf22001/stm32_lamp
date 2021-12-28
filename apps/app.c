

/*================================================================
 *
 *
 *   文件名称：app.c
 *   创 建 者：肖飞
 *   创建日期：2019年10月11日 星期五 16时54分03秒
 *   修改日期：2021年12月28日 星期二 16时40分51秒
 *   描    述：
 *
 *================================================================*/
#include "app.h"

#include <string.h>

#include "iwdg.h"

#include "os_utils.h"
#include "config_layout.h"

#include "test_serial.h"
#include "test_event.h"
#include "file_log.h"
#include "uart_debug.h"
#include "probe_tool.h"
#include "net_client.h"
#include "ftp_client.h"
#include "ftpd/ftpd.h"
#include "usb_upgrade.h"
#include "usbh_user_callback.h"
#include "vfs.h"

#include "channels_config.h"
#include "channels.h"
#include "duty_cycle_pattern.h"
#include "display.h"
#include "sal_hook.h"
#include "channels_notify_voice.h"

#include "log.h"

extern IWDG_HandleTypeDef hiwdg;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart1;
extern SPI_HandleTypeDef hspi3;

static app_info_t *app_info = NULL;
static os_signal_t app_event = NULL;

app_info_t *get_app_info(void)
{
	return app_info;
}

int app_load_config(void)
{
	config_layout_t *config_layout = get_config_layout();
	size_t offset = (size_t)&config_layout->mechine_info_seg.storage_mechine_info.mechine_info;
	debug("offset:%d", offset);
	return load_config_item(app_info->storage_info, "eva", &app_info->mechine_info, sizeof(mechine_info_t), offset);
}

int app_save_config(void)
{
	config_layout_t *config_layout = get_config_layout();
	size_t offset = (size_t)&config_layout->mechine_info_seg.storage_mechine_info.mechine_info;
	debug("offset:%d", offset);
	return save_config_item(app_info->storage_info, "eva", &app_info->mechine_info, sizeof(mechine_info_t), offset);
}

int app_event_init(size_t size)
{
	int ret = -1;

	if(app_event != NULL) {
		return ret;
	}

	app_event = signal_create(size);
	OS_ASSERT(app_event != NULL);
	ret = 0;
	return ret;
}

void app_init(void)
{
	mem_info_init();
	mt_file_init();
}

void send_app_event(app_event_t event, uint32_t timeout)
{
	signal_send(app_event, event, timeout);
}

static void app_mechine_info_invalid(void *fn_ctx, void *chain_ctx)
{
	app_info_t *app_info = (app_info_t *)fn_ctx;
	modbus_data_ctx_t *modbus_data_ctx = (modbus_data_ctx_t *)chain_ctx;

	if(modbus_data_ctx->influence < (void *)&app_info->mechine_info) {
		return;
	}

	if(modbus_data_ctx->influence >= (void *)(&app_info->mechine_info + 1)) {
		return;
	}

	debug("[%p, %p, %p]", &app_info->mechine_info, modbus_data_ctx->influence, &app_info->mechine_info + 1);

	app_info->mechine_info_invalid = 1;
}

__weak void load_app_display_cache(app_info_t *app_info)
{
}

__weak void sync_app_display_cache(app_info_t *app_info)
{
}

static void app_mechine_info_changed(void *fn_ctx, void *chain_ctx)
{
	app_info_t *app_info = (app_info_t *)fn_ctx;

	sync_app_display_cache(app_info);

	if(app_info->mechine_info_invalid != 0) {
		app_info->mechine_info_invalid = 0;
		app_save_config();
	}
}

void update_network_ip_config(app_info_t *app_info)
{
	int exit = 0;

	while(exit == 0) {
		if(set_dhcp_enable(app_info->mechine_info.dhcp_enable) != 0) {
			debug("");
			osDelay(1000);
		} else {
			if(app_info->mechine_info.dhcp_enable == 0) {
				ip_addr_t ip;
				ip_addr_t sn;
				ip_addr_t gw;

				if(ipaddr_aton(app_info->mechine_info.ip, &ip) == 0) {
					debug("ip:%s", app_info->mechine_info.ip);
				}

				if(ipaddr_aton(app_info->mechine_info.sn, &sn) == 0) {
					debug("sn:%s", app_info->mechine_info.sn);
				}

				if(ipaddr_aton(app_info->mechine_info.gw, &gw) == 0) {
					debug("gw:%s", app_info->mechine_info.gw);
				}

				if(set_default_ipaddr(&ip) != 0) {
					debug("");
					osDelay(1000);
					continue;
				}

				if(set_default_netmask(&sn) != 0) {
					debug("");
					osDelay(1000);
					continue;
				}

				if(set_default_gw(&gw) != 0) {
					debug("");
					osDelay(1000);
					continue;
				}

				if(set_default_dns_server(&gw) != 0) {
					debug("");
					osDelay(1000);
					continue;
				}
			}

			exit = 1;
		}
	}
}

static uint8_t reset_config = 0;

void app_set_reset_config(void)
{
	OS_ASSERT(app_info != NULL);
	app_info->mechine_info.reset_config = 1;
}

uint8_t app_get_reset_config(void)
{
	return reset_config;
}

void app(void const *argument)
{
	poll_loop_t *poll_loop;
	channels_info_t *channels_info = NULL;
	display_info_t *display_info = NULL;
	int ret;

	app_init();

	app_info = (app_info_t *)os_calloc(1, sizeof(app_info_t));

	OS_ASSERT(app_info != NULL);

	app_info->storage_info = get_or_alloc_storage_info(&hspi3);
	OS_ASSERT(app_info->storage_info != NULL);

	ret = app_load_config();

	if(ret == 0) {
		debug("app_load_config successful!");
		reset_config = app_info->mechine_info.reset_config;

		if(app_get_reset_config() != 0) {
			ret = -1;
		}
	} else {
		debug("app_load_config failed!");
	}

	if(ret == 0) {
		debug("device id:\'%s\', server uri:\'%s\'!", app_info->mechine_info.device_id, app_info->mechine_info.uri);
	} else {
		snprintf(app_info->mechine_info.device_id, sizeof(app_info->mechine_info.device_id), "%s", "0000000000");
		snprintf(app_info->mechine_info.uri, sizeof(app_info->mechine_info.uri), "%s", "tcp://112.74.40.227:12345");
		debug("device id:\'%s\', server uri:\'%s\'!", app_info->mechine_info.device_id, app_info->mechine_info.uri);
		snprintf(app_info->mechine_info.ip, sizeof(app_info->mechine_info.ip), "%d.%d.%d.%d", 10, 42, 0, 122);
		snprintf(app_info->mechine_info.sn, sizeof(app_info->mechine_info.sn), "%d.%d.%d.%d", 255, 255, 255, 0);
		snprintf(app_info->mechine_info.gw, sizeof(app_info->mechine_info.gw), "%d.%d.%d.%d", 10, 42, 0, 1);
		app_info->mechine_info.dhcp_enable = 1;
		app_info->mechine_info.request_type = REQUEST_TYPE_SSE;
		app_info->mechine_info.reset_config = 0;
		app_save_config();
	}

	load_app_display_cache(app_info);

	update_network_ip_config(app_info);

	poll_loop = get_or_alloc_poll_loop(0);
	OS_ASSERT(poll_loop != NULL);

	probe_broadcast_add_poll_loop(poll_loop);
	probe_server_add_poll_loop(poll_loop);

	while(is_log_server_valid() == 0) {
		osDelay(1);
	}

	//get_or_alloc_uart_debug_info(&huart1);
	//add_log_handler((log_fn_t)log_uart_data);

	add_log_handler((log_fn_t)log_udp_data);
	//add_log_handler((log_fn_t)log_file_data);

	debug("===========================================start app============================================");

	//ftpd_init();

	//test_event();

	channels_info = start_channels();
	OS_ASSERT(channels_info != NULL);

	//net_client_add_poll_loop(poll_loop);
	//ftp_client_add_poll_loop(poll_loop);

	display_info = (display_info_t *)channels_info->display_info;
	OS_ASSERT(display_info != NULL);

	if(display_info->modbus_slave_info != NULL) {
		app_info->display_data_invalid_callback_item.fn = app_mechine_info_invalid;
		app_info->display_data_invalid_callback_item.fn_ctx = app_info;
		OS_ASSERT(register_callback(display_info->modbus_slave_info->data_invalid_chain, &app_info->display_data_invalid_callback_item) == 0);

		app_info->display_data_changed_callback_item.fn = app_mechine_info_changed;
		app_info->display_data_changed_callback_item.fn_ctx = app_info;
		OS_ASSERT(register_callback(display_info->modbus_slave_info->data_changed_chain, &app_info->display_data_changed_callback_item) == 0);
	}

	if(init_channels_notify_voice(channels_info) != 0) {
		debug("");
	}

	while(1) {
		uint32_t event;
		ret = signal_wait(app_event, &event, 1000);

		if(ret == 0) {
			switch(event) {
				case APP_EVENT_HOST_USER_CLASS_ACTIVE: {
					if(mt_f_mount(get_vfs_fs(), "", 0) == FR_OK) {
						start_usb_upgrade();
					}
				}
				break;

				case APP_EVENT_HOST_USER_CONNECTION: {
				}
				break;

				case APP_EVENT_HOST_USER_DISCONNECTION: {
					try_to_close_log();

					if(mt_f_mount(0, "", 0) != FR_OK) {
					}
				}
				break;

				default: {
					debug("unhandled event %x", event);
				}
				break;
			}
		}

		handle_open_log();
		handle_usb_upgrade();
	}
}

static pattern_state_t work_pattern_state = {
	.type = PWM_COMPARE_COUNT_UP,
	.duty_cycle = 0,
};

static void update_work_led(void)
{
	//计数值小于duty_cycle,输出1;大于duty_cycle输出0
	uint16_t duty_cycle = get_duty_cycle_pattern(&work_pattern_state, 1000, 0, 20);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, duty_cycle);
}

void idle(void const *argument)
{
	MX_IWDG_Init();
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);

	while(1) {
		HAL_IWDG_Refresh(&hiwdg);
		update_work_led();
		osDelay(10);
	}
}

int force_bootloader(void)
{
	int ret = -1;
	u_uint8_bits_t u_uint8_bits;
	u_uint8_bits.v = 0;

	HAL_Init();
	MX_GPIO_Init();

	//u_uint8_bits.s.bit0 = (HAL_GPIO_ReadPin(d1_GPIO_Port, d1_Pin) == GPIO_PIN_SET) ? 1 : 0;
	//u_uint8_bits.s.bit1 = (HAL_GPIO_ReadPin(d2_GPIO_Port, d2_Pin) == GPIO_PIN_SET) ? 1 : 0;
	//u_uint8_bits.s.bit2 = (HAL_GPIO_ReadPin(d3_GPIO_Port, d3_Pin) == GPIO_PIN_SET) ? 1 : 0;

	if(u_uint8_bits.v == 0x07) {
		ret = 0;
	}

	HAL_DeInit();

	return ret;
}
