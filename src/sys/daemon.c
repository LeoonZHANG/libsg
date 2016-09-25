/**
 * daemon.c
 * Start with OS and independent of terminal.
 */

#include <stdio.h>
#include <string.h>
#include <sg/sg.h>
#include <sg/sys/daemon.h>
#include <sg/sys/proc.h>
#include <sg/str/vsstr.h>
#include <sg/sys/fs.h>

#if defined(SG_OS_WINDOWS)
# include <windows.h> /* RegCreateKeyEx... */
#endif
#if defined(SG_OS_LINUX)
# include <unistd.h> /* daemon */
#endif


int sg_daemon_start_with_os(bool on)
{
#if defined(SG_OS_WINDOWS)
    HKEY key;
    HKEY root = HKEY_LOCAL_MACHINE;
    char *sub_key = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    char module[SG_PATH_MAX]; /* executable filename */
    char module_name[SG_PATH_MAX]; /* executable short filename */
    DWORD disposition = REG_OPENED_EXISTING_KEY;
    LONG retval;

    if (sg_module_path(module, SG_PATH_MAX) != 0) {
        sg_log_err("Get process path error.");
        return -1;
    }

    if (sg_module_name(module_name, SG_PATH_MAX) != 0) {
        sg_log_err("Get process name error.");
        return -1;
    }

    retval = RegCreateKeyEx(root, sub_key, 0, NULL, REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS, NULL, &key, &disposition);
    if (retval != ERROR_SUCCESS) {
        sg_log_err("failed to find.");
        return -1;
    }

    retval = RegSetValueEx(key, module_name, 0, REG_SZ, (BYTE *)module, strlen(module));
    RegCloseKey(key);
    if (retval != ERROR_SUCCESS) {
        sg_log_err("failed to reg.");
        return -1;
    }
    return 0;
#else
    const char *filename = "/etc/rc.local";
    sg_vsstr_t *file_str = sg_vsstr_alloc();
    sg_vsstr_t *proc_path = sg_vsstr_alloc(); /* executable filename */
    int retval;

    if (!sg_proc_full_path(proc_path)) {
        sg_log_err("Get process path error.");
        return -1;
    }

    if (!sg_fs_file_to_str(filename, file_str))
        return -1;

    /* already exist */
    if (strstr(sg_vsstr_raw(file_str), sg_vsstr_raw(proc_path))) {
        sg_vsstr_free(&file_str);
        return 0;
    }

    sg_vsstr_cat(file_str, "\n");
    sg_vsstr_cat(file_str, proc_path);

    retval = sg_fs_file_overwrite(filename, (uint8_t *) sg_vsstr_raw(file_str), sg_vsstr_len(file_str));
    sg_vsstr_free(&file_str);
    return retval;
#endif
}

int sg_daemon_independent_of_terminal(bool on)
{
#if defined(SG_OS_LINUX)
    return daemon(0, 0);
#else
    return 0;
#endif
}

