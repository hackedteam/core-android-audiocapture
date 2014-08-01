#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "../uthash.h"

#define ROUNDUP(x, y) ((((x)+((y)-1))/(y))*(y))
#define DBG 1

#define FILESIZE 1048576 // ~5 seconds

struct hook_t *help_hash1;
struct hook_t *postcall_hash;

extern char *dumpPath;

/* From audio.h Audio stream types */
typedef enum {
    AUDIO_STREAM_DEFAULT          = -1,
    AUDIO_STREAM_VOICE_CALL       = 0,
    AUDIO_STREAM_SYSTEM           = 1,
    AUDIO_STREAM_RING             = 2,
    AUDIO_STREAM_MUSIC            = 3,
    AUDIO_STREAM_ALARM            = 4,
    AUDIO_STREAM_NOTIFICATION     = 5,
    AUDIO_STREAM_BLUETOOTH_SCO    = 6,
    AUDIO_STREAM_ENFORCED_AUDIBLE = 7, /* Sounds that cannot be muted by user and must be routed to speaker */
    AUDIO_STREAM_DTMF             = 8,
    AUDIO_STREAM_TTS              = 9,
    AUDIO_STREAM_CNT,
    AUDIO_STREAM_MAX              = AUDIO_STREAM_CNT - 1,
} audio_stream_type_t;
typedef enum  {
    IDLE,
    TERMINATED,
    FLUSHED,
    STOPPED,
// next 2 states are currently used for fast tracks only
    STOPPING_1,     // waiting for first underrun
    STOPPING_2,     // waiting for presentation complete
    RESUMING,
    ACTIVE,
    PAUSING,
    PAUSED
}track_state;
typedef enum {
  STATUS_NEW   = 0,
  STATUS_STOP  = 1,
  STATUS_START = 2,
} cblk_status; 

struct cblk_t {

  unsigned int cblk_index; // address of cblk struct in memory, used as the key
  unsigned int sampleRate;
  int streamType;
  

  
  // Qi-<timestamp>-<id univoco per chiamata>-<canale>.[tmp|boh]
  // canale=l or r
  //  2-10-10-1-3-1(null) 
  char* trackId; 
  char *filename;
  int fd;

  /* getNextBuffer hook  */
  unsigned int playbackLastBufferStartAddress; // last buffer start address returned for this cblk 

 
  /* getNextBuffer2 hook */
  unsigned int lastBufferRaw;
  unsigned int lastFrameCount;
  unsigned int lastFrameSize;
  
  /* getNextBuffer3 hook */
  unsigned int startOfCircularBuffer;
  unsigned int frameCount;
  
  unsigned long fileOffset; // global offset, needed to recompose the tracks

  int stopDump; // for record
 
  // todo: usato per fissare il problema del riutilizzo dei buffer (da verificare)
  cblk_status lastStatus;
  
  UT_hash_handle hh;
  

};


unsigned int base_address;

void* no_proto(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;

// 1] function prototype

void* recordTrack_getNextBuffer3_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;
void* playbackTrack_getNextBuffer3_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;


void* recordThread_getNextBuffer_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;
void* playbackTimedTrack_getNextBuffer_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;

void* newRecordTrack_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;

/* signaling */
void* recordTrackStart_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;
void* playbackTrackStart_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;
void* playbackTrackPause_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;
void* newTrack_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;

void* recordTrackStop_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;
void* playbackTrackStop_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) ;

// 2] hook

struct hook_t newTrack_helper;


struct hook_t getBuffer_helper;


struct hook_t recordTrack_getNextBuffer_helper;
struct hook_t playbackTrack_getNextBuffer_helper;

struct hook_t recordThread_getNextBuffer_helper;
struct hook_t playbackTimedTrack_getNextBuffer_helper;

/* signaling */
struct hook_t playbackTrackStart_helper;
struct hook_t playbackTrackStop_helper;
struct hook_t playbackTrackPause_helper;

struct hook_t recordTrackStart_helper;
struct hook_t recordTrackStop_helper;


struct hook_t newRecordTrack_helper;


unsigned long audioFlingerInstance;
unsigned long pbInstance;

// getBuffer globals
unsigned long bufferStart;
unsigned long lastSize;
unsigned long lastOffset;

unsigned long lastFrameSize;
unsigned long lastBufferStart;
unsigned long lastBufferEnd;

// recordTrack_getNextBuffer globals
unsigned long lastBufferStartAddress;
 

// 3] insert the hook

/* Hooks for Android 4.1,4.2 */

/* dumpers */
//android::AudioFlinger::RecordThread::RecordTrack::getNextBuffer(android::AudioBufferProvider::Buffer*, long long)
#define HOOK_coverage_11 help_no_hash(&recordTrack_getNextBuffer_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger12RecordThread11RecordTrack13getNextBufferEPNS_19AudioBufferProvider6BufferEx", recordTrack_getNextBuffer3_h, 1,  0);//0x35275);
//android::AudioFlinger::PlaybackThread::Track::getNextBuffer(android::AudioBufferProvider::Buffer*, long long)
#define HOOK_coverage_12 help_no_hash(&playbackTrack_getNextBuffer_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track13getNextBufferEPNS_19AudioBufferProvider6BufferEx", playbackTrack_getNextBuffer3_h, 1, 0);// 0x352d1);

/* signaling hooks */

// PlaybackThread::Track
// android::AudioFlinger::PlaybackThread::Track::Track(android::AudioFlinger::PlaybackThread*, android::sp<android::AudioFlinger::Client> const&, audio_stream_type_t, unsigned int, audio_format_t, unsigned int, int, android::sp<android::IMemory> const&, int, unsigned int)
#define HOOK_coverage_2 help_no_hash(&newTrack_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5TrackC2EPS1_RKNS_2spINS0_6ClientEEE19audio_stream_type_tj14audio_format_tjiRKNS4_INS_7IMemoryEEEij", newTrack_h, 1,  0);
//android::AudioFlinger::PlaybackThread::Track::Track(android::AudioFlinger::PlaybackThread*, android::sp<android::AudioFlinger::Client> const&, audio_stream_type_t, unsigned int, audio_format_t, unsigned int, unsigned int, android::sp<android::IMemory> const&, int, unsigned int)
#define HOOK_coverage_2_fall help_no_hash(&newTrack_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5TrackC1EPS1_RKNS_2spINS0_6ClientEEE19audio_stream_type_tj14audio_format_tjjRKNS4_INS_7IMemoryEEEij", newTrack_h, 1,  0);

// android::AudioFlinger::PlaybackThread::Track::start(android::AudioSystem::sync_event_t, int)
#define HOOK_coverage_17 help_no_hash(&playbackTrackStart_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track5startENS_11AudioSystem12sync_event_tEi", playbackTrackStart_h, 1, 0);

//android::AudioFlinger::PlaybackThread::Track::stop()
#define HOOK_coverage_19 help_no_hash(&playbackTrackStop_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track4stopEv", playbackTrackStop_h, 1, 0);
//android::AudioFlinger::PlaybackThread::Track::pause()
#define HOOK_coverage_20 help_no_hash(&playbackTrackPause_helper,  pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track5pauseEv", playbackTrackPause_h, 1,  0);

// RecordThread::RecordTrack
//android::AudioFlinger::RecordThread::RecordTrack::start(android::AudioSystem::sync_event_t, int)
#define HOOK_coverage_16 help_no_hash(&recordTrackStart_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger12RecordThread11RecordTrack5startENS_11AudioSystem12sync_event_tEi", recordTrackStart_h, 1, 0);
//android::AudioFlinger::RecordThread::RecordTrack::stop()
#define HOOK_coverage_18 help_no_hash(&recordTrackStop_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger12RecordThread11RecordTrack4stopEv", recordTrackStop_h, 1, 0);


/* Hooks for android 4.0 
 * 
 * hooks 18, 19, 20, are ok */

/* dumpers 4.0 */
//android::AudioFlinger::RecordThread::RecordTrack::getNextBuffer(android::AudioBufferProvider::Buffer*)
#define HOOK_coverage_40_11 help_no_hash(&recordTrack_getNextBuffer_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger12RecordThread11RecordTrack13getNextBufferEPNS_19AudioBufferProvider6BufferE", recordTrack_getNextBuffer3_h, 1,  0);
//android::AudioFlinger::PlaybackThread::Track::getNextBuffer(android::AudioBufferProvider::Buffer*)
#define HOOK_coverage_40_12 help_no_hash(&playbackTrack_getNextBuffer_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track13getNextBufferEPNS_19AudioBufferProvider6BufferE", playbackTrack_getNextBuffer3_h, 1, 0);


/* signaling 4.0 */

// PlaybackThread::Track
///device/include/server/AudioFlinger/AudioFlinger.cpp
//android::AudioFlinger::PlaybackThread::Track::Track(android::wp<android::AudioFlinger::ThreadBase> const&, android::sp<android::AudioFlinger::Client> const&, int, unsigned int, unsigned int, unsigned int, int, android::sp<android::IMemory> const&, int)
#define HOOK_coverage_40_2 help_no_hash(&newTrack_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5TrackC2ERKNS_2wpINS0_10ThreadBaseEEERKNS_2spINS0_6ClientEEEijjjiRKNS8_INS_7IMemoryEEEi", newTrack_h, 1,  0);
//android::AudioFlinger::PlaybackThread::Track::start()
#define HOOK_coverage_40_17 help_no_hash(&playbackTrackStart_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track5startEv", playbackTrackStart_h, 1, 0);

// RecordThread::RecordTrack
//android::AudioFlinger::RecordThread::RecordTrack::start()
#define HOOK_coverage_40_16 help_no_hash(&recordTrackStart_helper, pid, "libaudioflinger", "_ZN7android12AudioFlinger12RecordThread11RecordTrack5startEv", recordTrackStart_h, 1, 0);

