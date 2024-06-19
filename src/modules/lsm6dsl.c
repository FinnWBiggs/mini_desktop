
#include <zephyr/kernel.h>

/* IMU imports */
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/sys/util.h>

/* AEM imports */
#include "imu_event.h"

#define MODULE lsm6dsl 
#include <caf/events/module_state_event.h>

/* IMU Setup */

static inline float out_ev(struct sensor_value *val){
    return (val->val1 + (float)val->val2 / 1000000);
}

static struct sensor_value accel_x_out, accel_y_out, accel_z_out;
static struct sensor_value gyro_x_out, gyro_y_out, gyro_z_out;

#ifdef CONFIG_LSM6DSL_TRIGGER

// Method to be called on trigger
static void lsm6dsl_trigger_handler(const struct device *dev, const struct sensor_trigger *trig){
    static struct sensor_value accel_x, accel_y, accel_z;
	static struct sensor_value gyro_x, gyro_y, gyro_z;

    /* Macros for collecting info from sensor */
    // Accel
    sensor_sample_fetch_chan(dev, SENSOR_CHAN_ACCEL_XYZ);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_X, &accel_x);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Y, &accel_y);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Z, &accel_z);

	// Gyro
	sensor_sample_fetch_chan(dev, SENSOR_CHAN_GYRO_XYZ);
	sensor_channel_get(dev, SENSOR_CHAN_GYRO_X, &gyro_x);
	sensor_channel_get(dev, SENSOR_CHAN_GYRO_Y, &gyro_y);
	sensor_channel_get(dev, SENSOR_CHAN_GYRO_Z, &gyro_z);

    // Generate event to submit (TODO: using this data above as fields)
    struct imu_event *event = new_imu_event();
    event->ax = out_ev(&accel_x);
    event->ay = out_ev(&accel_y);
    event->az = out_ev(&accel_z);
    event->gx = out_ev(&gyro_x);
    event->gy = out_ev(&gyro_y);
    event->gz = out_ev(&gyro_z);

	APP_EVENT_SUBMIT(event);
}

int init_sensor(void){
    char out_str[64]; 
    struct sensor_value odr_attr; // "output data rate"

    /* set accel/gyro sampling frequency to 104 Hz */
	odr_attr.val1 = 52; // 104hz is finiky
	odr_attr.val2 = 000000; // and 0 millionths of a hz

    // heavy lifting done here
    const struct device *const lsm6dsl_dev = DEVICE_DT_GET_ONE(st_lsm6dsl);

    /* readiness checking */
    if (!device_is_ready(lsm6dsl_dev)) {
		printk("sensor: device not ready.\n");
		return 1;
	}

    /* configuration checking */
    if (sensor_attr_set(lsm6dsl_dev, SENSOR_CHAN_ACCEL_XYZ,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
		printk("Cannot set sampling frequency for accelerometer.\n");
		return 2;
	}

	if (sensor_attr_set(lsm6dsl_dev, SENSOR_CHAN_GYRO_XYZ,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
		printk("Cannot set sampling frequency for gyro.\n");
		return 3;
	}

    /* Trigger setup */
    struct sensor_trigger trig;

    // Options for trigger - dev note CAN I HAVE MULTIPLE TRIG ON ONE DEV?
    trig.type = SENSOR_TRIG_DATA_READY;
	trig.chan = SENSOR_CHAN_ACCEL_XYZ;

    // set
    if (sensor_trigger_set(lsm6dsl_dev, &trig, lsm6dsl_trigger_handler) != 0) {
		printk("Could not set sensor type and channel\n");
		return 4;
	}

    // // validate sensor functioning
    // if (sensor_sample_fetch(lsm6dsl_dev) < 0) {
	// 	printk("Sensor sample update error\n");
	// 	return 5;
	// }

    printk("IMU set up successfully \n");

    return 0;

}

static bool app_event_handler(const struct app_event_header *aeh)
{    
    if(is_module_state_event(aeh)){
        struct module_state_event *event = cast_module_state_event(aeh);

		if (check_state(event, MODULE_ID(main), MODULE_STATE_READY)) {
            int ret = init_sensor();
            if (ret != 0){
                printk("Error condn %d in imu setup", ret);
            }
		}
		return false;
    }

    return false;
}

#endif /* CONFIG_LSM6DSL_TRIGGER */

APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);

