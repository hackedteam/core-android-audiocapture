/*
 *  Collin's Binary Instrumentation Tool/Framework for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *
 *  (c) 2012
 *
 *  License: GPL v2
 *
 */
#include "uthash.h"



#ifdef DEBUG
/*
 * Android log priority values, in ascending priority order.
 */
typedef enum android_LogPriority {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,    /* only for SetMinPriority() */
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT,     /* only for SetMinPriority(); must be last */
} android_LogPriority;

//memset(tag,0,256-1);
#define log(...) {\
tag[0]=tag[1]=0;\
snprintf(tag,256,"libt:%s",__FUNCTION__);\
__android_log_print(ANDROID_LOG_DEBUG, tag , __VA_ARGS__);}
#define logf(...) {FILE *f = fopen("/data/local/tmp/log", "a+");\
        if(f!=NULL){\
        fprintf(f,"%s: ",__FUNCTION__);\
        fprintf(f, __VA_ARGS__);\
        fflush(f); fclose(f); }}
 #else
 #define log(...)
 #endif

struct hook_t {
  unsigned int jump[3];		// ARM jump code
  unsigned int store[3];	// ARM stolen bytes

  unsigned char jumpt[20];	// Thumb jump code    (was 12)
  unsigned char storet[20];	// Thumb stolen bytes (was 12)

  unsigned int orig;		// Hooked function address
  unsigned int patch;		

  unsigned char thumb;		// ARM/thumb switch     
  unsigned char name[128];	// function name

  UT_hash_handle hh;
  UT_hash_handle hh_postcall;

  void *data;
};
typedef enum{
	ANDROID_V_INVALID = -1,
	ANDROID_V_4_0,
	ANDROID_V_4_1,
	ANDROID_V_4_2,
	ANDROID_V_4_3,
}android_versions;
/*
 * 4.1/4.2?/4.0?
 * (gdb) p /x (int)&(('android::AudioFlinger::PlaybackThread::Track'*)0)->mStreamType
 * $1 = 0x6c
 * (gdb) p /x (int)&(('android::AudioFlinger::PlaybackThread::Track'*)0)->mName
 * $2 = 0x70
 * (gdb) p /x (int)&(('android::audio_track_cblk_t' *)0)->mName
 * $13 = 0x35
 * (gdb)  p /x (int)&(('android::AudioFlinger::PlaybackThread::Track' *)0)->mClient <--'android::AudioFlinger::Client'
 * $1 = 0x14
 * (gdb)  p /x (int)&(('android::AudioFlinger::Client' *)0)->mPid
 * $2 = 0x10
 *
 *
 */
/*
 * >=4.3
 * (gdb) p /x (int)&(('android::AudioFlinger::PlaybackThread::Track'*)0)->mStreamType
 * $1 = 0x88
 * (gdb) p /x (int)&(('android::AudioFlinger::PlaybackThread::Track'*)0)->mName
 * $2 = 0x8c
 * (gdb) p /x (int)&(('android::audio_track_cblk_t' *)0)->mName
 * $13 = 0x35
 */
void help_precall(struct hook_t *h);
void help_postcall(struct hook_t *h);
int help(struct hook_t *h, int pid, char *libname, char *funcname, void *hookf, int by_address, unsigned int raw_address);
int help_no_hash(struct hook_t *h, int pid, char *libname, char *funcname, void *hookf, int by_address, unsigned int raw_address);
void unhelp(struct hook_t *h);

int pid_global;
unsigned long global_base_address;
