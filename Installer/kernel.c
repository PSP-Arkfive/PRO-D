#include <stdio.h>
#include <string.h>

#include <libpspexploit.h>
#include <pspiofilemgr.h>
#include <systemctrl.h>
#include <rebootexconfig.h>
#include <psprebootex.h>
#include <rebootex.h>

#include "installer.h"
#include "package.h"

static KernelFunctions k;
static ProFile files[PRO_FILE_COUNT];
static unsigned char buffer[8192], verify_buffer[8192];

static int seek_file(int fd, int offset, int whence)
{
    return k.KernelIOLSeek(fd, offset, whence);
}

static int copy_file(int archive, const ProFile *file)
{
    char path[80];
    unsigned remaining = file->size;
    int output, verify, result = -1;
    snprintf(path, sizeof(path), "flash0:%.63s", file->name);
    if (k.KernelIOLSeek(archive, file->offset, PSP_SEEK_SET) != file->offset)
        return -1;
    output = k.KernelIOOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (output < 0)
        return output;
    while (remaining) {
        unsigned chunk = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
        int n = k.KernelIORead(archive, buffer, chunk);
        if (n <= 0 || (unsigned)n > chunk)
            goto close_output;
        int written = 0;
        while (written < n) {
            int part = k.KernelIOWrite(output, buffer + written, n - written);
            if (part <= 0 || part > n - written)
                goto close_output;
            written += part;
        }
        remaining -= n;
    }
    result = 0;
close_output:
    if (k.KernelIOClose(output) < 0)
        result = -1;
    if (result < 0)
        return result;

    if (k.KernelIOLSeek(archive, file->offset, PSP_SEEK_SET) != file->offset)
        return -1;
    verify = k.KernelIOOpen(path, PSP_O_RDONLY, 0);
    if (verify < 0)
        return verify;
    remaining = file->size;
    while (remaining) {
        unsigned chunk = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
        int n = k.KernelIORead(archive, buffer, chunk);
        int m = n > 0 ? k.KernelIORead(verify, verify_buffer, n) : -1;
        if (n <= 0 || (unsigned)n > chunk || m != n || memcmp(buffer, verify_buffer, n)) {
            result = -1;
            break;
        }
        remaining -= n;
    }
    if (result == 0 && k.KernelIORead(verify, buffer, 1) != 0)
        result = -1;
    if (k.KernelIOClose(verify) < 0)
        result = -1;
    return result;
}

static SceModule* loadexec = NULL;

static int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4)
{
    // Copy Rebootex into Memory
    memset((char *)REBOOTEX_TEXT, 0, REBOOTEX_MAX_SIZE);
    if (rebootex[0] == 0x1F && rebootex[1] == 0x8B) // gzip packed rebootex
        k.KernelGzipDecompress((unsigned char *)REBOOTEX_TEXT, REBOOTEX_MAX_SIZE, rebootex, NULL);
    else // plain payload
        memcpy((void*)REBOOTEX_TEXT, rebootex, size_rebootex);
    
    // Initialize PRO configuration
    RebootexConfigPRO* conf = (RebootexConfigPRO*)(REBOOTEX_CONFIG);
    memset((char *)REBOOTEX_CONFIG, 0, sizeof(RebootexConfigPRO));
    conf->magic = REBOOTEX_CONFIG_MAGIC_PRO;
    
    // Load Sony Reboot Buffer
    int (*sceLoadReboot)(void *, unsigned int, void *, unsigned int) = (void*)loadexec->text_addr;
    return sceLoadReboot(arg1, arg2, arg3, arg4);
}

static void cfwReboot(void)
{
    loadexec = (SceModule*)pspXploitFindModuleByName("sceLoadExec");

    u32 addr = 0;
    u32 text_addr = loadexec->text_addr;
    u32 topaddr = text_addr + loadexec->text_size;
    u32 rebootcall = JAL(text_addr);

    // patch rebootex into loadexec
    int patches = 2;
    for (addr = text_addr; addr < topaddr && patches; addr += 4) {
        u32 data = _lw(addr);
        if (data == 0x3C018860) {
            _sb(0xFC, addr); // Patch Reboot Buffer Entry Point
            patches--;
        } else if (data == rebootcall) {
            _sw(JAL(LoadReboot), addr); // Patch Reboot Buffer Loader
            patches--;
        }
    }

    // Invalidate Cache
    k.KernelDcacheWritebackInvalidateAll();
    k.KernelIcacheInvalidateAll();
}

static int install_thread(SceSize args, void *argp)
{
    int (*get_model)(void);
    unsigned nid;
    int model, archive, result, mounted = 0;
    ProReader reader = { k.KernelIORead, seek_file };
    (void)args;
    (void)argp;

    nid = firmware == 0x06020010 ? 0x864EBFD7 :
          firmware == 0x06030910 ? 0x458A70B5 : 0x07C586A1;
    get_model = (void *)pspXploitFindFunction("sceSystemMemoryManager", "SysMemForKernel", nid);
    if (!get_model) {
        snprintf(install_message, sizeof(install_message), "Cannot resolve PSP model.");
        return -1;
    }
    model = get_model();
    if (pro_select_files(firmware, model, files) < 0) {
        snprintf(install_message, sizeof(install_message), "Unsupported firmware/model combination: %d.", model);
        return -1;
    }
    archive = k.KernelIOOpen(package_path, PSP_O_RDONLY, 0);
    if (archive < 0) {
        snprintf(install_message, sizeof(install_message), "Cannot open firmware package: 0x%08X", archive);
        return -1;
    }
    if (pro_validate_package(&reader, archive, files) < 0) {
        snprintf(install_message, sizeof(install_message), "Invalid, incomplete, or truncated firmware package.");
        goto close_archive;
    }
    result = k.IoUnassign("flash0:");
    if (result < 0) {
        snprintf(install_message, sizeof(install_message), "Cannot unassign flash0: 0x%08X", result);
        goto close_archive;
    }
    result = k.IoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0);
    if (result < 0) {
        snprintf(install_message, sizeof(install_message), "Cannot mount flash0 writable: 0x%08X", result);
        goto restore_mount;
    }
    mounted = 1;
    for (unsigned i = 0; i < PRO_FILE_COUNT; ++i) {
        unsigned index = (i + 1) % PRO_FILE_COUNT;
        result = copy_file(archive, &files[index]);
        if (result < 0) {
            snprintf(install_message, sizeof(install_message), "Write/verification failed: %.63s (0x%08X)", files[index].name, result);
            goto restore_mount;
        }
    }
    install_result = 0;
    snprintf(install_message, sizeof(install_message), "Installed and verified PRO files for model %02dg. Rebooting...", model + 1);
    cfwReboot();
restore_mount:
    if (mounted && k.IoUnassign("flash0:") < 0) {
        install_result = -1;
        snprintf(install_message, sizeof(install_message), "Cannot unmount writable flash0; reboot with care.");
        goto close_archive;
    }
    result = k.IoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDONLY, NULL, 0);
    if (result < 0) {
        install_result = -1;
        snprintf(install_message, sizeof(install_message), "Cannot restore read-only flash0: 0x%08X", result);
    }
close_archive:
    if (k.KernelIOClose(archive) < 0) {
        install_result = -1;
        snprintf(install_message, sizeof(install_message), "Cannot close firmware package.");
    }
    return install_result;
}

void kernel_main(void)
{
    int k1 = pspSdkSetK1(0);
    SceUID thread;
    pspXploitScanKernelFunctions(&k);
    pspXploitRepairKernel();
    if (!k.KernelIOOpen || !k.KernelIORead || !k.KernelIOWrite ||
        !k.KernelIOLSeek || !k.KernelIOClose || !k.IoAssign ||
        !k.IoUnassign || !k.KernelCreateThread || !k.KernelStartThread ||
        !k.KernelWaitThreadEnd || !k.KernelDeleteThread) {
        snprintf(install_message, sizeof(install_message), "Missing kernel functions.");
        goto done;
    }
    thread = k.KernelCreateThread("pro_installer", (void *)KERNELIFY(&install_thread), 0x20, 0x10000, 0, NULL);
    if (thread < 0) {
        snprintf(install_message, sizeof(install_message), "Cannot create kernel thread: 0x%08X", thread);
        goto done;
    }
    {
        int result = k.KernelStartThread(thread, 0, NULL);
        if (result < 0)
            snprintf(install_message, sizeof(install_message), "Cannot start kernel thread: 0x%08X", result);
        else
            k.KernelWaitThreadEnd(thread, NULL);
    }
    k.KernelDeleteThread(thread);
done:
    pspSdkSetK1(k1);
}
