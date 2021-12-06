

/*================================================================
 *   
 *   
 *   文件名称：channels_notify_voice.h
 *   创 建 者：肖飞
 *   创建日期：2021年08月03日 星期二 11时23分01秒
 *   修改日期：2021年08月06日 星期五 22时18分23秒
 *   描    述：
 *
 *================================================================*/
#ifndef _CHANNELS_NOTIFY_VOICE_H
#define _CHANNELS_NOTIFY_VOICE_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "app_platform.h"
#include "cmsis_os.h"

#include "channels.h"

#ifdef __cplusplus
}
#endif

int init_channels_notify_voice(channels_info_t *channels_info);

#endif //_CHANNELS_NOTIFY_VOICE_H
