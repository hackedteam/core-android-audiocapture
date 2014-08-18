#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "libt.h"
#include "util.h"
#include <errno.h>
#include "hooker.h"
// below is addressed by -I$PLATFORM_NDK
#include "include/asm/jazzdma.h"

//#include "../../../../androidSources/prebuilts/ndk/8/platforms/android-9/arch-mips/usr/include/asm/jazzdma.h"
#include "uthash.h"
#include <signal.h> // or csignal if C++ code

static int ip_register_offset = 0xd;
static int fd = 0;
static int fd_in = 0;
static int fd_out = 0;

struct cblk_t *cblkTracks = NULL;
struct cblk_t *cblkRecordTracks = NULL;

char *dumpPath =  ".................____________.......................";

/* start signaling hooks */
char * printTrackstate(track_state t){
  switch (t){
    case IDLE:
      return "IDLE";
      break;
    case TERMINATED:
      return "TERMINATED";
      break;
    case FLUSHED:
      return "FLUSHED";
      break;
    case STOPPED:
      return "STOPPED";
      break;
      // next 2 states are currently used for fast tracks only
    case STOPPING_1:     // waiting for first underrun
      return "STOPPING_1";
      break;
    case STOPPING_2:     // waiting for presentation complete
      return "STOPPING_2";
      break;
    case RESUMING:
      return "RESUMING:";
      break;
    case ACTIVE:
      return "ACTIVE";
      break;
    case PAUSING:
      return "PAUSING";
      break;
    case PAUSED:
      return "PAUSED";
      break;

    default:
      break;
  }
#ifdef DEBUG_HOOKFNC
  log("state=%d",t);
#endif
  return "UNKNOWN";
}
void* localTrackStart_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w)  {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  unsigned long cblk;
// this = 'android::AudioFlinger::RecordThread'
// (gdb) p /x (int)&(('android::AudioFlinger::RecordThread' *)0)->mName 
// $29 = 0xa8
// (gdb) print (int)&((android::AudioFlinger::RecordThread::RecordTrack *)0)->mId
// $3 = 108 (0x6c)
// (gdb) print &this->mId
// $6 = (const int *) 0xb854e94c
  track_state mState;
  mState = *(int*) (a + 0x2c );


  h_ptr = (void *) recordTrackStart_helper.orig;
  helper_precall(&recordTrackStart_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&recordTrackStart_helper);
  cblk = *(unsigned long*) (a + 0x1c);
  int mId = 0;
  mId = *(int*) (a + 0x6c );
  unsigned char mName = 0;
  mName = *(unsigned char*) (cblk + 0x35 );
#ifdef DEBUG_HOOKFNC
  //log("[a]trackId %x  cblk: %x\n",a, cblk);
  log("b=%x c=%x d=%x e=%x f=%x g=%x h=%x i=%x j=%x k=%x l=%x m=%x n=%x o=%x  cblk %p mId=%x mState=%s , mName=%x\n"
      ,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,cblk,mId,printTrackstate(mState), mName);
  #endif
  return result;

}

void* localTrackStop_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  char *filename;
  struct cblk_t *cblk_tmp;
  unsigned long cblk,trackId;
  unsigned int uintTmp;
  time_t now;
 

  h_ptr = (void *) recordTrackStop_helper.orig;
  helper_precall(&recordTrackStop_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&recordTrackStop_helper);

  cblk = *(unsigned long*) (a + 0x1c);

  int mId = 0;
  mId = *(int*) (a + 0x6c );
  unsigned char mName = 0;
  mName = *(unsigned char*) (cblk + 0x35 );
  track_state mState;
  mState = *(int*) (a + 0x2c );



  h_ptr = (void *) recordTrackStop_helper.orig;
  helper_precall(&recordTrackStop_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&recordTrackStop_helper);
#ifdef DEBUG_HOOKFNC
  //log("[a]trackId %x  cblk: %x\n",a, cblk);
  //log("b=%x c=%x d=%x e=%x f=%x g=%x h=%x i=%x j=%x k=%x l=%x m=%x n=%x o=%x  cblk %p mId=%x\n",  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,cblk,mId);
#endif
  trackId=mName;
  HASH_FIND_INT( cblkRecordTracks, &trackId, cblk_tmp);
  log("Looking for [mName]%x  cblk=%p !!! mState=%s\n", trackId,cblk,printTrackstate(mState));
  if( cblk_tmp != NULL){
    HASH_DEL(cblkRecordTracks, cblk_tmp);
    free(cblk_tmp);
  }else{
#ifdef DEBUG_HOOKFNC
    log("[mId] %x  mName=%x cblk%p not found !!! mState=%s \n",mId,mName, cblk,printTrackstate(mState));
#endif
  }

  return result;
}


void*  remoteNewTrack_h(void* a,void* thread, void * client, void* _streamType, void* _sampleRate, void* format, void* channelMask, void* frameCount, void* sharedBuffer,void* sessionId, void* flags, void* l, void* m,  void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
 /* Hooks for Android 4.1,4.2 
        AudioFlinger::PlaybackThread::Track::Track(
            PlaybackThread *thread,
            const sp<Client>& client,
            audio_stream_type_t streamType,
            uint32_t sampleRate,
            audio_format_t format,
            audio_channel_mask_t channelMask,
            int frameCount,
            const sp<IMemory>& sharedBuffer,
            int sessionId,
            IAudioFlinger::track_flags_t flags)
        :   TrackBase(thread, client, sampleRate, format, channelMask, frameCount, sharedBuffer, sessionId),
        */
/* Hooks for Android 4.0
  AudioFlinger::PlaybackThread::Track::Track(
            const wp<ThreadBase>& thread,
            const sp<Client>& client,
            int streamType,
            uint32_t sampleRate,
            uint32_t format,
            uint32_t channelMask,
            int frameCount,
            const sp<IMemory>& sharedBuffer,
            int sessionId)
    :   TrackBase(thread, client, sampleRate, format, channelMask, frameCount, 0, sharedBuffer, sessionId),
    mMute(false), mSharedBuffer(sharedBuffer), mName(-1), mMainBuffer(NULL), mAuxBuffer(NULL),
    mAuxEffectId(0), mHasVolumeController(false)
 */ 
  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  
  void *result;
  unsigned long cblk,trackId;
  unsigned int sampleRate;
  ssize_t res;
  /* int ii, cblkIndex; */
  unsigned int uintTmp;
  struct cblk_t *cblk_tmp= NULL;
  off_t headerStart = 0, positionTmp = 0;
  time_t now;
  int tt = 0xf;

  unsigned int bufferRaw = 0;
  unsigned int framesCount = 0;
  unsigned int frameSize = 0;

  char randString[11]; // rand %d 10 + null;
  char timestamp[11];  // epoch %ld 10 + null;
  int filenameLength = 0;
  char *filename;
  char* tmp;
  
  int written;
  int streamType = (int) _sampleRate;
     track_state mState;
  mState = *(int*) (a + 0x2c );
  h_ptr = (void *) newTrack_helper.orig;
  helper_precall(&newTrack_helper);
  result = h_ptr( a,  thread, client, _streamType,  _sampleRate,  format,  channelMask,  frameCount ,  sharedBuffer,  sessionId,  flags, l, m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&newTrack_helper);


  cblk = *(unsigned long*) (a + 0x1c);
  unsigned char mName = 0;
  mName = *(unsigned char*) (cblk + 0x35 );
  int mId = 0;
  mId = *(int*) (a + 0x6c );
 

  trackId=mName;
  #ifdef DEBUG_HOOKFNC
    log("cblk %p mName=%x mId=%x mState=%s thread=%x client=%x _streamType=%x _sampleRate=%x format=%x channelMask=%x frameCount=%x  sharedBuffer=%x sessionId=%x flags=%x \n"
	,cblk,mName,mId,printTrackstate(mState),thread,  client, _streamType,  _sampleRate,  format,  channelMask,  frameCount,  sharedBuffer,  sessionId,  flags);
  #endif
/*
  HASH_FIND_INT( cblkTracks, &trackId, cblk_tmp);
   if( cblk_tmp == NULL ) {
    cblk_tmp =  malloc( sizeof(struct cblk_t) );
    cblk_tmp->cblk_index = trackId;
    cblk_tmp->lastStatus=0;
    HASH_ADD_INT(cblkTracks, cblk_index, cblk_tmp); // new track
    cblk_tmp = NULL;
    HASH_FIND_INT( cblkTracks, &trackId, cblk_tmp);
    if( cblk_tmp == NULL ) {
#ifdef DEBUG_HOOKFNC
  log(" trackId:%x  cblk %p INSERTION FAILED \n",trackId,cblk);
#endif
    }else{
#ifdef DEBUG_HOOKFNC
    log("thread=%x client=%x _streamType=%x _sampleRate=%x format=%x channelMask=%x frameCount=%x  sharedBuffer=%x sessionId=%x flags=%x cblk %p mName=%x \n",
	thread,  client, _streamType,  _sampleRate,  format,  channelMask,  frameCount,  sharedBuffer,  sessionId,  flags,cblk,mName);
  #endif
    }
   }else{
     
#ifdef DEBUG_HOOKFNC
  log("double entry for trackId: %x  streamType: %x cblk: %p\n",
  trackId, _streamType,cblk);
  //log("\tpbthread: %x\n\tclient: %x\n\tstreamType: %x\n\tsampleRate: %d\n\tformat: %x\n\tchannelMask: %x\n\tframeCount: %x\n\tsharedBuffer: %x\n\tsessionId: %x\n\ttid: %x\n\tflags: %x\n"
  //, b, c,d, e, f, g, h, i, l, m, n);
#endif
   }
   */
  return result;
}

//status_t AudioFlinger::PlaybackThread::Track::start(AudioSystem::sync_event_t event,
  //                                                  int triggerSession)
void* remoteTrackStart_h(void* a, void* event, void* triggerSession, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  unsigned long cblk;

  struct cblk_t *cblk_tmp= NULL;
  unsigned long trackId;
  char randString[11]; // rand %d 10 + null;
  char timestamp[11];  // epoch %ld 10 + null;
  int filenameLength = 0;
  char *filename;

  cblk = *(unsigned long*) (a + 0x1c);
  track_state mState;
  mState = *(int*) (a + 0x2c );
  unsigned char mName = 0;

  
  h_ptr = (void *) playbackTrackStart_helper.orig;

  helper_precall(&playbackTrackStart_helper);
  result = h_ptr( a,  event,  triggerSession,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&playbackTrackStart_helper);
  int mId = 0;
  mId = *(int*) (a + 0x6c );
 
    mName = *(unsigned char*) (cblk + 0x35 );
    trackId = mName;
    /*
  HASH_FIND_INT( cblkTracks, &trackId, cblk_tmp);
   if( cblk_tmp == NULL ) {
    cblk_tmp =  malloc( sizeof(struct cblk_t) );
    cblk_tmp->cblk_index = trackId;
    cblk_tmp->lastStatus=0;
    HASH_ADD_INT(cblkTracks, cblk_index, cblk_tmp); // new track
    cblk_tmp = NULL;
    HASH_FIND_INT( cblkTracks, &trackId, cblk_tmp);
    if( cblk_tmp == NULL ) {
#ifdef DEBUG_HOOKFNC
  log(" trackId:%x  cblk %p INSERTION FAILED \n",trackId,cblk);
#endif
    }else{
#ifdef DEBUG_HOOKFNC
      log("event=%x triggerSession=%x cblk %p mName=%x \n",  event,  triggerSession,cblk,mName);
  #endif
    }
   }else{
     
#ifdef DEBUG_HOOKFNC
  log("trackId: %x added by newTrack cblk: %p\n",
  trackId,cblk);
  //log("\tpbthread: %x\n\tclient: %x\n\tstreamType: %x\n\tsampleRate: %d\n\tformat: %x\n\tchannelMask: %x\n\tframeCount: %x\n\tsharedBuffer: %x\n\tsessionId: %x\n\ttid: %x\n\tflags: %x\n"
  //, b, c,d, e, f, g, h, i, l, m, n);
#endif
   }
*/
    #ifdef DEBUG_HOOKFNC
  log("trackId: tId%x mId=%x cblk: %p ,mState=%s\n",
  trackId,mId,cblk,printTrackstate(mState));
#endif
  return result;
}

void* remoteTrackStop_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  unsigned int cblk;
  unsigned int trackId;
  char *filename;
  struct cblk_t *cblk_tmp = NULL;
  unsigned int uintTmp;
  time_t now;

  cblk = *(unsigned long*) (a + 0x1c);


  h_ptr = (void *) playbackTrackStop_helper.orig;
  helper_precall(&playbackTrackStop_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&playbackTrackStop_helper);
  track_state mState;
  mState = *(int*) (a + 0x2c );
  unsigned char mName = 0;
  mName = *(unsigned char*) (cblk + 0x35 );
  int mId = 0;
  mId = *(int*) (a + 0x6c );
 #ifdef DEBUG_HOOKFNC
  //log("[a]trackId %x  cblk: %x\n",a, cblk);
  //log("b=%x c=%x d=%x e=%x f=%x g=%x h=%x i=%x j=%x k=%x l=%x m=%x n=%x o=%x  cblk %p mName=%x\n",  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,cblk,mName);
  #endif
  trackId=mName;
  HASH_FIND_INT( cblkTracks, &trackId, cblk_tmp);
  if( cblk_tmp != NULL){
    HASH_DEL(cblkTracks, cblk_tmp);
    log("trackId %x mId=%x  %x REMOVED mState=%s\n",mName,mId, cblk,printTrackstate(mState));
    free(cblk_tmp);
  }else{
    #ifdef DEBUG_HOOKFNC
  log("[a]trackId %x mId=%x %x not found mState=%s!!!\n",mName,mId, cblk,printTrackstate(mState));
  #endif
  }
  return result;
}



void* remoteTrackPause_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  unsigned long cblk;

  cblk = *(unsigned long*) (a + 0x1c);
  int mId = 0;
  mId = *(int*) (a + 0x6c );
 

  h_ptr = (void *) playbackTrackStart_helper.orig;
  helper_precall(&playbackTrackStart_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&playbackTrackStart_helper);

  //raise(SIGABRT);  /* To continue from here in GDB: "signal 0". */
  unsigned char mName = 0;
    mName = *(unsigned char*) (cblk + 0x35 );
    track_state mState;
   mState = *(int*) (a + 0x2c );
   #ifdef DEBUG_HOOKFNC
  //log("[a]trackId %x  cblk: %x\n",a, cblk);
  log("cblk %p mId=%x mName=%x mState=%s b=%x c=%x d=%x e=%x f=%x g=%x h=%x i=%x j=%x k=%x l=%x m=%x n=%x o=%x \n",cblk,mId,mName,printTrackstate(mState), b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o);
  #endif
  return result;
}





void* remoteTrack_getNextBuffer3_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);

  void* result;
  unsigned int* cblk_ptr;
  unsigned int cblk;
  unsigned int trackId;
  unsigned long framesAvail;
  unsigned long bufferSize;
  unsigned long bufferStart;
  unsigned long thisStart;
  unsigned long bufferEndAddress;
  unsigned int sampleRate;
  struct timeval tv;
  ssize_t res;
  /* int ii, cblkIndex; */
  unsigned int uintTmp;
  struct cblk_t *cblk_tmp= NULL;
  off_t headerStart = 0, positionTmp = 0;
  time_t now;
  int tt = 0xf;
  
  unsigned int bufferRaw = 0;
  unsigned int framesCount = 0;
  unsigned int frameSize = 0;

  char randString[11]; // rand %d 10 + null;
  char timestamp[11];  // epoch %ld 10 + null;
  int filenameLength = 0;
  char *filename;
  char* tmp;
  
  int written;

  cblk = *(unsigned long*) (a + 0x1c);
  
  /* if( cblk_tmp->fd == 0) { */
  /*   cblk_tmp->fd = open("/data/local/tmp/log_out" , O_RDWR , S_IRUSR | S_IRGRP | S_IROTH); */
  /* } */

  // tmp
  framesCount = * (unsigned int*) (b + 4);
  track_state mState;
  mState = *(int*) (a + 0x2c );
  int mId = 0;
  mId = *(int*) (a + 0x6c );
 

  /* call the original function */
  h_ptr = (void *) playbackTrack_getNextBuffer_helper.orig;
  helper_precall(&playbackTrack_getNextBuffer_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&playbackTrack_getNextBuffer_helper);
  unsigned char mName = 0;
  mName = *(unsigned char*) (cblk + 0x35 );
   #ifdef DEBUG_HOOKFNC
  //log("[a]trackId %x  cblk: %x\n",a, cblk);
//  log("b=%x c=%x d=%x e=%x f=%x g=%x h=%x i=%x j=%x k=%x l=%x m=%x n=%x o=%x  cblk %p mName=%x\n",  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,cblk,mName);
  #endif

  trackId=mName;
  HASH_FIND_INT( cblkTracks, &trackId, cblk_tmp);
  if( cblk_tmp == NULL ) {
    cblk_tmp =  malloc( sizeof(struct cblk_t) );
    cblk_tmp->cblk_index = trackId;
    cblk_tmp->lastStatus=0;
    HASH_ADD_INT(cblkTracks, cblk_index, cblk_tmp); // new track
    cblk_tmp = NULL;
    HASH_FIND_INT( cblkTracks, &trackId, cblk_tmp);
    if( cblk_tmp == NULL ) {
#ifdef DEBUG_HOOKFNC
      log(" mid=0%x trackId:%x  cblk %p INSERTION FAILED \n",mId,trackId,cblk);
#endif
    }else{
#ifdef DEBUG_HOOKFNC
      log(" cblk %p mId=%x mName=%x mState=%s\n",cblk,mId,mName,printTrackstate(mState));
#endif
    }
  }else{
    if(cblk_tmp->lastStatus==0) {
#ifdef DEBUG_HOOKFNC
      log(" trackId:%x mId=%x - desiredFrames: %x cblk %p mState=%s\n", mName,mId, framesCount,cblk,printTrackstate(mState));
#endif
      cblk_tmp->lastStatus=1;
    }
  }


  /*
  HASH_FIND_INT( cblkTracks, &trackId, cblk_tmp);
   if( cblk_tmp != NULL &&  cblk_tmp->lastStatus==0) {
#ifdef DEBUG_HOOKFNC
  log(" trackId:%x  - desiredFrames: %x cblk %p \n", mName, framesCount,cblk);
#endif
  cblk_tmp->lastStatus=1;
   }else if(cblk_tmp == NULL){
     log(" trackId:%x  - NOT FOUND: %x cblk %p \n", mName, framesCount,cblk);
   }
   */
  return result;

}



void* localTrack_getNextBuffer3_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);

  void* result;
  unsigned int* cblk_ptr;
  unsigned int cblk,trackId;
  unsigned long framesAvail;
  unsigned long bufferSize;
  unsigned long bufferStart;
  unsigned long thisStart;
  unsigned long bufferEndAddress;
  unsigned int sampleRate;
  struct timeval tv;
  ssize_t res;
  /* int ii, cblkIndex; */
  unsigned int uintTmp;
  struct cblk_t *cblk_tmp= NULL;
  off_t headerStart = 0, positionTmp = 0;
  time_t now;
  int tt = 0xf;
  
  unsigned int bufferRaw = 0;
  unsigned int framesCount = 0;
  unsigned int frameSize = 0;

  char randString[11]; // rand %d 10 + null;
  char timestamp[11];  // epoch %ld 10 + null;
  int filenameLength = 0;
  char *filename;
  char* tmp;
  
  int written;


  

  
  

  /* call the original function */
  h_ptr = (void *) recordTrack_getNextBuffer_helper.orig;
  helper_precall(&recordTrack_getNextBuffer_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&recordTrack_getNextBuffer_helper);

  cblk_ptr = (unsigned int*) (a + 0x1c) ;

  cblk = *cblk_ptr;
  int mId = 0;
  mId = *(int*) (a + 0x6c );
  unsigned char mName = 0;
  mName = *(unsigned char*) (cblk + 0x35 );
#ifdef DEBUG_HOOKFNC
  //log("[a]trackId %x  cblk: %x\n",a, cblk);
  //log("b=%x c=%x d=%x e=%x f=%x g=%x h=%x i=%x j=%x k=%x l=%x m=%x n=%x o=%x  cblk %p mId=%x\n",  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,cblk,mId);
#endif
  track_state mState;
  mState = *(int*) (a + 0x2c );
  //(gdb)  p /x (int)&(('android::AudioFlinger::PlaybackThread::Track' *)0)->mState
  //$10 = 0x2c
  trackId=mName;
  HASH_FIND_INT( cblkRecordTracks, &trackId, cblk_tmp);
   if( cblk_tmp == NULL ) {
    cblk_tmp =  malloc( sizeof(struct cblk_t) );
    cblk_tmp->cblk_index = trackId;
#ifdef DEBUG_HOOKFNC
  log(" mId:%x mName=%x - desiredFrames: %x cblk %p mState=%s\n", mId, mName,framesCount,cblk,printTrackstate(mState));
#endif
   HASH_ADD_INT(cblkRecordTracks, cblk_index, cblk_tmp); // new track
    cblk_tmp = NULL;
    
    HASH_FIND_INT( cblkRecordTracks, &trackId, cblk_tmp);
    
    if( cblk_tmp == NULL ) {
#ifdef DEBUG_HOOKFNC
      log("uthash shit happened\n");
#endif
      return result;
    }
   }
  return result;

}
