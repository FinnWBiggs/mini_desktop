/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app_event_manager.h>

#define MODULE main
#include <caf/events/module_state_event.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE);

int main(void)
{
	#ifdef CONFIG_APP_MSC_STORAGE_FLASH_FATFS
	// int ret;
	// setup_disk();

	// ret = usb_enable(NULL);

	// if (ret != 0) {
	// 	LOG_ERR("Failed to enable USB");
	// 	return 0;
	// }

	// LOG_INF("The device is put in USB mass storage mode.\n");

	// struct fs_file_t file;
	// char str[100] = "val";
	// printk("file ptr created\n");

	// fs_file_t_init(&file);
	// fs_open(&file,"/NAND:/file.txt", FS_O_CREATE | FS_O_WRITE);
	// fs_truncate(&file,0);
	// printk("&file open for write\n");
	// fs_write(&file, str, strlen(str));
	// printk("&file written\n");
	// fs_close(&file);
	// printk("&file closed\n");
	
	#endif //CONFIG_APP_MSC_STORAGE_FLASH_FATFS

	if (app_event_manager_init()) {
		LOG_ERR("Application Event Manager not initialized");
	} else {
		module_set_state(MODULE_STATE_READY);
	}
	return 0;
}
