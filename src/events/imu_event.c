
#include <stdio.h>

#include "imu_event.h"

LOG_MODULE_REGISTER(LOG_MODULE_NAME,LOG_LEVEL_ERR);

static void log_imu_event(const struct app_event_header *aeh){
	struct imu_event *event = cast_imu_event(aeh);

	APP_EVENT_MANAGER_LOG(aeh, 
		"IMU:\tgx=%5.2f, gy=%5.2f, gz=%5.2f\tax=%5.2f, ay=%5.2f, az=%5.2f",
		event->gx, event->gy, event->gz, event->ax, event->ay, event->az);
}

static void profile_imu_event(struct log_event_buf *buf,
			      const struct app_event_header *aeh)
{
	struct imu_event *event = cast_imu_event(aeh);
	ARG_UNUSED(event);
	
	// nrf_profiler_log_encode_float16(buf, event->gx);
	// nrf_profiler_log_encode_float16(buf, event->gy);
	// nrf_profiler_log_encode_float16(buf, event->gz);

	// nrf_profiler_log_encode_float16(buf, event->ax);
	// nrf_profiler_log_encode_float16(buf, event->ay);
	// nrf_profiler_log_encode_float16(buf, event->az);

}

APP_EVENT_INFO_DEFINE(imu_event,
		  ENCODE(),
		  ENCODE(),
		  profile_imu_event);

#ifdef CONFIG_ENABLE_IMU_LOG
APP_EVENT_TYPE_DEFINE(imu_event,
		  log_imu_event,
		  &imu_event_info,
		  APP_EVENT_FLAGS_CREATE(APP_EVENT_TYPE_FLAGS_INIT_LOG_ENABLE)
	);
#else
APP_EVENT_TYPE_DEFINE(imu_event,
		  log_imu_event,
		  &imu_event_info,
		  APP_EVENT_FLAGS_CREATE()
	);
#endif
