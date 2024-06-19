/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef _IMU_EVENT_H_
#define _IMU_EVENT_H_


#include <app_event_manager.h>
#include <app_event_manager_profiler_tracer.h>

#ifdef __cplusplus
extern "C" {
#endif

struct imu_event {
	struct app_event_header header;

	float ax;
	float ay;
	float az;
	float gx;
	float gy;
	float gz;

};

APP_EVENT_TYPE_DECLARE(imu_event);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* _ACK_EVENT_H_ */
