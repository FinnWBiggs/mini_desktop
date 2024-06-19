#include <zephyr/kernel.h>

#include "imu_event.h"
#include "motion_event.h"
#include "hid_event.h"

#define MODULE motion_imu
#include <caf/events/module_state_event.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, LOG_LEVEL_INF);


static enum state {
	STATE_IDLE,
	STATE_CONNECTED,
	STATE_PENDING
} state;

static bool handle_hid_report_subscription_event(const struct hid_report_subscription_event *event)
{
	if ((event->report_id == REPORT_ID_MOUSE) ||
	    (event->report_id == REPORT_ID_BOOT_MOUSE)) {
		static uint8_t peer_count;

		if (event->enabled) {
			__ASSERT_NO_MSG(peer_count < UCHAR_MAX);
			peer_count++;
		} else {
			__ASSERT_NO_MSG(peer_count > 0);
			peer_count--;
		}

		bool is_connected = (peer_count != 0);

		if ((state == STATE_IDLE) && is_connected) {
            state = STATE_CONNECTED;
			// if (is_motion_active()) {
			// 	clear_accumulated_motion();
			// 	send_motion();
			// 	state = STATE_PENDING;
			// } else {
			// 	state = STATE_CONNECTED;
			// }
			return false;
		}
		if ((state != STATE_IDLE) && !is_connected) {
			state = STATE_IDLE;
			return false;
		}
	}

	return false;
}

static bool handle_hid_report_sent_event(const struct hid_report_sent_event *event)
{
	if((event->report_id == REPORT_ID_MOUSE)||(event->report_id == REPORT_ID_BOOT_MOUSE))
    {
		if(state == STATE_PENDING)
        {state = STATE_CONNECTED;}
	}
	return false;
}

static bool imu_event_handler(struct imu_event *event)
{
    if (state == STATE_CONNECTED)
	{
        struct motion_event *mevent = new_motion_event();

        mevent->dx = (int16_t)(event->gx);
        mevent->dy = (int16_t)(event->gy);

        APP_EVENT_SUBMIT(mevent);
    }
    return false;
}

static bool app_event_handler(const struct app_event_header *aeh)
{    
    if(is_imu_event(aeh))
    {return imu_event_handler(cast_imu_event(aeh));}

	if (is_hid_report_sent_event(aeh))
    {return handle_hid_report_sent_event(cast_hid_report_sent_event(aeh));}

	if (is_hid_report_subscription_event(aeh))
    {return handle_hid_report_subscription_event(cast_hid_report_subscription_event(aeh));}

	/* If event is unhandled, unsubscribe. */
	__ASSERT_NO_MSG(false);

    return false;
}

APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, imu_event);
APP_EVENT_SUBSCRIBE(MODULE, hid_report_sent_event);
APP_EVENT_SUBSCRIBE(MODULE, hid_report_subscription_event);