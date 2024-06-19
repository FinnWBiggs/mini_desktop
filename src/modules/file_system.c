#include <zephyr/kernel.h>

#define MODULE file_system

#include <caf/events/module_state_event.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, LOG_LEVEL_INF);

#if CONFIG_DISK_DRIVER_FLASH
	#include <zephyr/storage/flash_map.h> 
#endif
#if CONFIG_FAT_FILESYSTEM_ELM
	#include <ff.h>
#endif

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usbd_msc.h>
#include <zephyr/fs/fs.h>

#define STORAGE_PARTITION		storage_partition
#define STORAGE_PARTITION_ID		FIXED_PARTITION_ID(STORAGE_PARTITION)

static struct fs_mount_t fs_mnt;


static int setup_flash(struct fs_mount_t *mnt)
{
	int rc = 0;
	#if CONFIG_DISK_DRIVER_FLASH
		unsigned int id;
		const struct flash_area *pfa;

		mnt->storage_dev = (void *)STORAGE_PARTITION_ID;
		id = STORAGE_PARTITION_ID;

		rc = flash_area_open(id, &pfa);
		printk("Area %u at 0x%x on %s for %u bytes\n",
			id, (unsigned int)pfa->fa_off, pfa->fa_dev->name,
			(unsigned int)pfa->fa_size);

		if (rc < 0 && IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
			printk("Erasing flash area ... ");
			rc = flash_area_erase(pfa, 0, pfa->fa_size);
			printk("%d\n", rc);
		}

		if (rc < 0) {
			flash_area_close(pfa);
		}
	#endif
	return rc;
}

static int mount_app_fs(struct fs_mount_t *mnt)
{
	int rc;

	#if CONFIG_FAT_FILESYSTEM_ELM
		static FATFS fat_fs;

		mnt->type = FS_FATFS;
		mnt->fs_data = &fat_fs;
		if (IS_ENABLED(CONFIG_DISK_DRIVER_RAM)) {
			mnt->mnt_point = "/RAM:";
		} else if (IS_ENABLED(CONFIG_DISK_DRIVER_SDMMC)) {
			mnt->mnt_point = "/SD:";
		} else {
			mnt->mnt_point = "/NAND:";
		}

	#elif CONFIG_FILE_SYSTEM_LITTLEFS
		mnt->type = FS_LITTLEFS;
		mnt->mnt_point = "/lfs";
		mnt->fs_data = &storage;
	#endif
	rc = fs_mount(mnt);

	return rc;
}

static void setup_disk(void)
{
	struct fs_mount_t *mp = &fs_mnt;
	struct fs_dir_t dir;
	struct fs_statvfs sbuf;
	int rc;

	fs_dir_t_init(&dir);

	if (IS_ENABLED(CONFIG_DISK_DRIVER_FLASH)) {
		rc = setup_flash(mp);
		if (rc < 0) {
			LOG_ERR("Failed to setup flash area");
			return;
		}
	}

	if (!IS_ENABLED(CONFIG_FILE_SYSTEM_LITTLEFS) &&
	    !IS_ENABLED(CONFIG_FAT_FILESYSTEM_ELM)) {
		LOG_INF("No file system selected");
		return;
	}

	rc = mount_app_fs(mp);
	if (rc < 0) {
		LOG_ERR("Failed to mount filesystem");
		return;
	}

	// Allow log messages to flush to avoid interleaved output
	k_sleep(K_MSEC(50));

	printk("Mount %s: %d\n", fs_mnt.mnt_point, rc);

	rc = fs_statvfs(mp->mnt_point, &sbuf);
	if (rc < 0) {
		printk("FAIL: statvfs: %d\n", rc);
		return;
	}

	printk("%s: bsize = %lu ; frsize = %lu ;"
	       " blocks = %lu ; bfree = %lu\n",
	       mp->mnt_point,
	       sbuf.f_bsize, sbuf.f_frsize,
	       sbuf.f_blocks, sbuf.f_bfree);

	rc = fs_opendir(&dir, mp->mnt_point);
	printk("%s opendir: %d\n", mp->mnt_point, rc);

	if (rc < 0) {
		LOG_ERR("Failed to open directory");
	}

	while (rc >= 0) {
		struct fs_dirent ent = { 0 };

		rc = fs_readdir(&dir, &ent);
		if (rc < 0) {
			LOG_ERR("Failed to read directory entries");
			break;
		}
		if (ent.name[0] == 0) {
			printk("End of files\n");
			break;
		}
		printk("  %c %u %s\n",
		       (ent.type == FS_DIR_ENTRY_FILE) ? 'F' : 'D',
		       ent.size,
		       ent.name);
	}

	(void)fs_closedir(&dir);

	return;
}

static void read_config(void)
{
	#define READ_PATH "/NAND:/pointer.json"
	struct fs_file_t file;
	fs_open(&file, READ_PATH, FS_O_CREATE | FS_O_READ);
	struct fs_dirent ent;
	fs_stat(READ_PATH, &ent);
	printk("%s\t%d\n",ent.name,ent.size);
	char rstr[ent.size];

	size_t readSize = 50;
	char rbuf[readSize];
	LOG_INF("Reading %s\n",READ_PATH);
	for(int i = 0; readSize == sizeof(rbuf); i += readSize)
	{
		readSize = fs_read(&file, rbuf, sizeof(rbuf));
		printk("looping: %d\trbuf =\n%s\n",i+readSize,rbuf);
		// memcpy(rstr+i, rbuf, readSize);
	}
	rstr[ent.size] = '\0';
	fs_close(&file);
	// sleep_time_ms[0] = strtol(rstr,NULL,10);
	// printk("%d\n",sleep_time_ms[0]);

	k_msleep(1000);
	printk(rstr);
	printk("\n%d\n",sizeof(rstr));
}

static bool app_event_handler(const struct app_event_header *aeh)
{
    if(is_module_state_event(aeh))
    {
        struct module_state_event *event = cast_module_state_event(aeh);

		if(check_state(event, MODULE_ID(main), MODULE_STATE_READY))
        {
            int ret;
            setup_disk();

            ret = usb_enable(NULL);
            if(ret != 0)
            {
                LOG_ERR("Failed to enable USB");
                return 0;
            }

            LOG_INF("The device is put in USB mass storage mode.\n");

            struct fs_file_t file;
            char str[100] = "val";
            printk("file ptr created\n");

            fs_file_t_init(&file);
            fs_open(&file,"/NAND:/file.txt", FS_O_CREATE | FS_O_WRITE);
            fs_truncate(&file,0);
            printk("&file open for write\n");
            fs_write(&file, str, strlen(str));
            printk("&file written\n");
            fs_close(&file);
            printk("&file closed\n");

            // read_config();
		}

		return false;
    }
    __ASSERT_NO_MSG(false);
    return false;
}

APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);
