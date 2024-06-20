# mini_desktop

Clone of Zephyr nrf_desktop sample with reduced memory footprint for porting

## Build Differences from Desktop

I have slightly changed the configuration structure. 

With the configuration as it is written now, the steps for getting a good build are as follows:

1. Add Build Configuration
2. BOARD = nrf52840dk_nrf52840
3. KConfig Fragments - None
4. DT Overlay - configuration\nrf52840dk_nrf52840\app.overlay
   1. (Though will select automatically)
5. Snippets - None
6. Extra Args - None
7. Optimization Level - Default
8. Build after generting - Yes
9. Sysbuild - No

### Change 1: Addition of MCUBoot Child Image

Added `/configuration/nrf52840dk_nrf52840/child_image/mcuboot/`

### Change 2: Consolidation of prj.conf into /common

Instead of the primary prj.conf existing under `configuration/(BOARD)/prj.conf`, it now lives in `configuration/common/prj_mcuboot_smp.conf`

This change was made to support both our development board (nRF52840dk) and our production board (Xiao BLE Sense) in a single prj.conf.

Further, in the binding file `mini_desktop/sample.yaml`, the mcuboot child image expects the mcuboot_CONF_FILE to be named (explicitly) `prj_mcuboot_smp.conf`. Even further, the name used for the build configuration when selecting the Configuration field of "Add Build Configuration" appears to require name to match.

## Configuration Differences from Desktop

A lot of options which are enabled in nRF Desktop have been disabled for mini_desktop. In an abundance of caution, rather than deleting those options, they have been moved to `subdirectory/temp_disable` folders, and disabled KConfigs are commented out in their respective source directory KConfigs.

## Configuration Options for FileSystem/DFU

Our "prj.conf" (acutally named `prj_mcuboot_smp.conf`) has become quite large.

I tried to come up with an elegant pair of boolean configuration options to enable and disable DFU and Filesystem, but my KConfig skills weren't up to the task.

Instead, here is how I recommend using this build to recreate the problem for this devZone post:

### Procedure to enable filesystem and DFU independently

In `configuration/common/prj_mcuboot_smp.conf`, I have added a "Custom Control Block" [Lines 7 - 61]

To enable Filesystem, select all the lines between `#### Filesystem ####` and `#### End Filesystem ####` and uncomment those Config options (Ctrl /)

To enable DFU, select all the lines between `#### DFU ####` and `#### End DFU ####` and uncomment those Config options (Ctrl /)

To disable, comment those options. 

# Error Behavior

When both DFU and Filesystem are Enabled, Flash Disk Encounters significant errors. Please see file mini_desktop/ERROR_OUTPUT.txt for trace.