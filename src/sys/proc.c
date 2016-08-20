/*
 * proc.c
 * Author: wangwei.
 * Process management.
 */

/* windows create process, find process filename by ID:http://blog.csdn.net/seulww/article/details/21737295 */

#include <stdio.h>
#include <string.h>
#include <sg/sys/os.h>
#if defined(OS_WIN)
# include <Windows.h> /* QueryFullProcessImageName / ZeroMemory / CloseHandle ... */
# include <tlhelp32.h> /* Process32First / CreateToolhelp32Snapshot ... */
# include <process.h> /* getpid */
# include <Psapi.h>
# pragma comment(lib,"psapi.lib")
#else
# include <unistd.h> /* getpid */
# include <signal.h> /* kill */
#endif
#include <sg/util/log.h>
#include <sg/util/assert.h>
#include <sg/sys/proc.h>
#include <sg/sys/shell.h>
#include <sg/str/string.h>

struct sg_proc_shell_context {
    void *cb;
    void *context;
    int count;
};

#if defined(OS_WIN)

/* Is a process run as common admin (not super admin) or not. */
static int proc_is_run_as_admin(uint32_t pid)
{
    int elevated = -1;
    HANDLE token = NULL;
    HANDLE proc;
    TOKEN_ELEVATION token_ele;
    DWORD ret_len = 0;

    proc = OpenProcess(PROCESS_QUERY_INFORMATION /* PROCESS_ALL_ACCESS */, 0, (DWORD)pid);
    if (!proc) {
        printf( "GetLastError: %d\n",  GetLastError());
        return -1;
    }

    /* Get target process token. */
    if (!OpenProcessToken(proc, TOKEN_QUERY, &token)) {
        printf( "GetLastError 2: %d\n",  GetLastError());
        return -1;
    }

    /* Retrieve token elevation information. */
    if (!GetTokenInformation(token, TokenElevation, &token_ele, sizeof(token_ele), &ret_len ) ||
        ret_len != sizeof(token_ele)) {
        CloseHandle(token);
        printf( "GetLastError 3: %d\n",  GetLastError());
        return -1;
    }

    elevated = token_ele.TokenIsElevated;

    CloseHandle(token);
    return elevated ? 1 : 0;
}

static char *proc_filename(DWORD pid)
{
    HANDLE proc;
    DWORD s = 1000;
    char* filename;

    proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (proc == INVALID_HANDLE_VALUE) {
        printf("err:%d.", GetLastError());
        return NULL;
    }

    filename = (char*)calloc(1000, sizeof(char));

    /*
    For the best results use the following table to convert paths.
    Windows 2000 = GetModuleFileNameEx()
    Windows XP x32 = GetProcessImageFileName()
    Windows XP x64 = GetProcessImageFileName()
    Windows Vista = QueryFullProcessImageName()
    Windows 7 = QueryFullProcessImageName()
    */
    /* GetModuleFileNameEx(proc, NULL, (LPSTR)filename, 1000); */
    if (!QueryFullProcessImageName(proc, 0, filename, &s))
        printf("err:%d.", GetLastError());

    return filename;
}

/* Up privilege to debug level before open or close process, to avoid "access denied". */
int proc_current_enable_debug_privil(void)
{
	int ret = -1;
	HANDLE token = INVALID_HANDLE_VALUE;
	TOKEN_PRIVILEGES tp;

	/* Open current process token. */
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES /* | TOKEN_QUERY */, &token)) {
		//GetLastError()
		goto end;
	}

	/* Lookup privilege of current process. */
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME /* SE_SHUTDOWN_NAME */, &tp.Privileges[0].Luid)) {
		//GetLastError()
		goto end;
	}

	/* Give privileges. */
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	/* Notify Windows to change privilege for current process to debug level. */
	if (!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), (PTOKEN_PRIVILEGES)NULL, NULL)) {
		//GetLastError()
		goto end;
	}

end:
	if (token != INVALID_HANDLE_VALUE)
		CloseHandle(token);
	return ret;
}

DWORD getppid()
{
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe32;
    DWORD ppid = 0, pid = GetCurrentProcessId();

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    __try {
        if (hSnapshot == INVALID_HANDLE_VALUE) __leave;

        ZeroMemory(&pe32, sizeof(pe32));
        pe32.dwSize = sizeof(pe32);
        if (!Process32First(hSnapshot, &pe32)) __leave;

        do {
            if (pe32.th32ProcessID == pid) {
                ppid = pe32.th32ParentProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));

    }
    __finally {
        if (hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(hSnapshot);
    }
    return ppid;
}

uid_t getuid()
{
    const int UNLEN = 256;
    PSID sid = NULL;
    TCHAR* domain = NULL;
    DWORD sid_size = 0;
    DWORD domain_size = 0;
    SID_NAME_USE use = SidTypeUnknown;

    TCHAR buffer[257];
    DWORD buffer_len = _countof(buffer);

    if (!GetUserName(buffer, &buffer_len)) {
        return 0;
    }

    //Called once to set the size of the sid pointer and domain  
    LookupAccountName(NULL, buffer, NULL, &sid_size, NULL, &domain_size, &use);

    //Allocate memory based on sid and domain size
    sid = (PSID)LocalAlloc(LMEM_FIXED, sid_size);
    domain = (TCHAR*)malloc(domain_size * sizeof(TCHAR));

    //Initialize sid and domain
    if (!LookupAccountName(NULL, buffer, sid, &sid_size, domain, &domain_size, &use)) {
        return 0;
    }
    if (!IsValidSid(sid)) {
        return 0;
    }
    return (uid_t)sid;
}
#else

void ps_e_shell_callback(enum sg_shell_event evt, const char *line, void *context)
{
#define PROC_ID_MAX_LEN 32
    int sscanf_ret;
    char pid[PROC_ID_MAX_LEN] = {0};
    struct sg_proc_shell_context *ctx = (struct sg_proc_shell_context *)context;
    sg_proc_found_callback proc_found;

    if (evt == SGSHELLEVENT_OVER || !line || strlen(line) == 0 || !ctx || !ctx->cb)
        return;

    proc_found = (sg_proc_found_callback)ctx->cb;

    if (line[0] == ' ')
        sscanf_ret = sscanf(line, "%*[ ]%[0-9]", pid); /* like: "  162 ?        00:00:00 kpsmoused" */
    else
        sscanf_ret = sscanf(line, "%[0-9]", pid); /* like: "12296 pts/10   00:00:00 sh" */

    if (strlen(pid) > 0)
        proc_found(pid, ctx->context);
}

int sg_proc_list_all(sg_proc_found_callback cb, void *context)
{
    int ret = 0;
    struct sg_proc_shell_context ctx;

    assert(cb);

    ctx.cb = (void *)cb;
    ctx.context = context;
    ctx.count = 0;

#if defined(WIN32)
    STARTUPINFO st;
    PROCESS_INFORMATION pi;
    PROCESSENTRY32 ps;
    HANDLE snapshot;
    DWORD pid;

    ZeroMemory(&st, sizeof(STARTUPINFO));
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    st.cb = sizeof(STARTUPINFO);
    ZeroMemory(&ps, sizeof(PROCESSENTRY32));
    ps.dwSize = sizeof(PROCESSENTRY32);

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        sg_log_err("CreateToolhelp32Snapshot error.");
        return NULL;
    }

    if (!Process32First(snapshot, &ps)) {
        sg_log_err("Process32First error.");
        CloseHandle(snapshot);
        return 0;
    }

    do {
        cb(context);
        ++ctx.count;
    } while (Process32Next(snapshot, &ps));

    CloseHandle(snapshot);
    return proc_list;
#else
    ret = sg_shell_exec("ps -e", ps_e_shell_callback, &ctx);
#endif

    return (ret == 0) ? ctx.count : -1;
}

void ls_l_shell_callback(enum sg_shell_event evt, const char *line, void *context)
{
    sg_vlstr *filename = (sg_vlstr *)context;
    char *exe_str;
    const char *exe_tag = "exe -> /"; /* Use '/' for avoiding 'exe -> ' string in filename. */

    if (evt == SGSHELLEVENT_OVER || !line || strlen(line) == 0)
        return;

    /* !Wierd! */
    exe_str = sg_str_r_str((char *)line, (char *)exe_tag);
    if (exe_str)
        sg_vlstrcat(filename, exe_str + strlen(exe_tag) - 1 /* '/' char in the end */);
}

sg_vlstr *sg_proc_filename(pid_t pid)
{
#define PROC_LIST_CMD_LEN 256
    int ret;
    struct sg_proc_shell_context ctx;
    char list_cmd[PROC_LIST_CMD_LEN];
    sg_vlstr *filename;

    filename = sg_vlstralloc();
    if (!filename)
        return NULL;
    snprintf(list_cmd, PROC_LIST_CMD_LEN, "ls -l /proc/%d/exe", pid);

    ret = sg_shell_exec(list_cmd, ls_l_shell_callback, filename);
    //printf("ret:%d\n", ret);
    assert(ret == 0);

    if (sg_vlstrlen(filename) > 0)
        return filename;

    sg_vlstrfree(&filename);
    return NULL;
}

#endif

pid_t sg_proc_id_current(void)
{
    return getpid();
}

pid_t sg_proc_id_parent(void)
{
    return getppid();
}

uid_t sg_proc_user_id_current(void)
{
    return getuid();
}

int sg_proc_kill(pid_t pid)
{
    int ret;

    assert(pid > 0);

#if defined(OS_WIN)
    /* TerminateProcess */
#else
    ret = kill(pid, SIGKILL);
    if (ret != 0) {
        //errno
        return -1;
    }
#endif

    return 0;
}

pid_t sg_proc_open(const char *cmd)
{
    sg_shell_open(cmd, NULL, NULL);
}
