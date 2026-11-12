#include <stdio.h>
#include <string.h>

#include <pspctrl.h>
#include <pspdebug.h>
#include <pspkernel.h>
#include <psppower.h>
#include <libpspexploit.h>

#include "installer.h"

PSP_MODULE_INFO("PRO Installer", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);

char package_path[256];
uint32_t firmware;
int install_result = -1;
char install_message[160] = "Kernel callback did not run.";

static void wait_button(unsigned mask)
{
    SceCtrlData pad;
    do {
        sceCtrlReadBufferPositive(&pad, 1);
        sceKernelDelayThread(20000);
    } while (pad.Buttons & mask);
    do {
        sceCtrlReadBufferPositive(&pad, 1);
        sceKernelDelayThread(20000);
    } while (!(pad.Buttons & mask));
}

int main(int argc, char **argv)
{
    const char *package;
    char *slash;
    int result;
    pspDebugScreenInit();
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    firmware = sceKernelDevkitVersion();
    switch (firmware) {
    case 0x06020010: package = "FLASH620.PRO"; break;
    case 0x06030910: package = "FLASH639.PRO"; break;
    case 0x06060010: package = "FLASH660.PRO"; break;
    case 0x06060110: package = "FLASH661.PRO"; break;
    default:
        pspDebugScreenPrintf("Unsupported firmware: 0x%08lX\n", (unsigned long)firmware);
        goto exit;
    }
    if (argc < 1 || !argv[0] || strlen(argv[0]) >= sizeof(package_path)) {
        pspDebugScreenPrintf("Cannot determine installer directory.\n");
        goto exit;
    }
    strcpy(package_path, argv[0]);
    slash = strrchr(package_path, '/');
    if (!slash || (size_t)(slash + 1 - package_path) + strlen(package) >= sizeof(package_path)) {
        pspDebugScreenPrintf("Installer path is invalid or too long.\n");
        goto exit;
    }
    strcpy(slash + 1, package);
    pspDebugScreenPrintf("PRO flash installer\nFirmware: 0x%08lX\nPackage: %s\n\n", (unsigned long)firmware, package);
    pspDebugScreenPrintf("Press X to install, or O to cancel.\n");
    wait_button(PSP_CTRL_CROSS | PSP_CTRL_CIRCLE);
    {
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(&pad, 1);
        if (!(pad.Buttons & PSP_CTRL_CROSS))
            goto exit;
    }
    result = scePowerLock(0);
    if (result < 0) {
        pspDebugScreenPrintf("\nCould not prevent suspend: 0x%08X\n", result);
        goto exit;
    }
    pspDebugScreenPrintf("\nInitializing kernel exploit...\n");
    result = pspXploitInitKernelExploit();
    if (result == 0)
        result = pspXploitDoKernelExploit();
    if (result == 0)
        pspXploitExecuteKernel(kernel_main);
    else
        snprintf(install_message, sizeof(install_message),
                 "Kernel exploit failed: 0x%08X", result);
    scePowerUnlock(0);
    pspDebugScreenPrintf("\n%s\n", install_message);
    if (install_result < 0)
        pspDebugScreenPrintf("If writing started, flash may be partially updated.\n");
exit:
    pspDebugScreenPrintf("\nPress X to exit.\n");
    wait_button(PSP_CTRL_CROSS);
    sceKernelExitGame();
    return 0;
}
