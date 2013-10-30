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
#include "uthash.h"


static int ip_register_offset = 0xd;
static int fd = 0;
static int fd_in = 0;
static int fd_out = 0;

struct cblk_t *cblkTracks = NULL;
struct cblk_t *cblkRecordTracks = NULL;

char *dumpPath =  ".................____________.......................";

/* start signaling hooks */


void* recordTrackStart_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w)  {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  

  log("\t\t\t------- enter recordTrackStart -------------\n");

  h_ptr = (void *) recordTrackStart_hook.orig;
  hook_precall(&recordTrackStart_hook);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  log("\t\t\t\tresult: %d\n", (int) result);
  
  hook_postcall(&recordTrackStart_hook);

  log("\t\t\t------- exit recordTrackStart -------------\n");

  return result;

}

void* recordTrackStop_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  char *filename;
  struct cblk_t *cblk_tmp;
  unsigned long cblk;
  unsigned int uintTmp;
  time_t now;

  log("\t\t\t------- enter recordTrackStop -------------\n");
 
  h_ptr = (void *) recordTrackStop_hook.orig;
  hook_precall(&recordTrackStop_hook);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  hook_postcall(&recordTrackStop_hook);

  log("\t\t\t\tresult: %d\n", (int) result);

  cblk = *(unsigned long*) (a + 0x1c);

#ifdef DBG
  log("\tnew track cblk: %x\n", cblk);
#endif

  h_ptr = (void *) recordTrackStop_hook.orig;
  hook_precall(&recordTrackStop_hook);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  hook_postcall(&recordTrackStop_hook);
  //  log("\t\t\t\tresult: %d\n", (int) result);

  HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);

  if( cblk_tmp != NULL  ) {

    /* write fake final header */
    now = time(NULL);
    write(cblk_tmp->fd, &now, 4); // 1] epoch start
  
    uintTmp = 0xf00df00d;
    write(cblk_tmp->fd, &uintTmp, 4); // 3] streamType

    uintTmp = cblk_tmp->sampleRate;
    write(cblk_tmp->fd, &uintTmp, 4); // 4] sampleRate

    uintTmp = 0;
    write(cblk_tmp->fd, &uintTmp, 4); // 5] size of block - 0 for last block

    /* rename file to .tmp and free the cblk structure  */
#ifdef DBG
    log("Stop: should take a dump\n");
#endif	  

    /* rename file *.bin to *.tmp */
    filename = strdup(cblk_tmp->filename); // check return value
    *(filename+strlen(filename)-1) = 'p';
    *(filename+strlen(filename)-2) = 'm';
    *(filename+strlen(filename)-3) = 't';

#ifdef DBG  
    log("fname %s\n", filename);
    log("rename %d\n", rename(cblk_tmp->filename, filename) );
#endif

    if( filename != NULL) {
      free(filename);
      filename = NULL;
    }

    close(cblk_tmp->fd);

    /* free some stuff */
    if( cblk_tmp->trackId != NULL ) {
      free(cblk_tmp->trackId);
      cblk_tmp->trackId = NULL;
    }

    if( cblk_tmp->filename != NULL ) {
      free(cblk_tmp->filename);
      cblk_tmp->filename = NULL;
    }

    HASH_DEL(cblkTracks, cblk_tmp);
    free(cblk_tmp);
    

#ifdef DBG
    log("freed all the crap\n");
#endif
  }

  log("\t\t\t------- exit recordTrackStop -------------\n");

  return result;
}


void*  newTrack_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  
  void *result;
  unsigned long cblk;
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
  int streamType = (int) d;

  log("enter new track\n");
  log("\tpbthread: %x\n\tclient: %x\n\tstreamType: %x\n\tsampleRate: %d\n\tformat: %x\n\tchannelMask: %x\n\tframeCount: %x\n\tsharedBuffer: %x\n\tsessionId: %x\n\ttid: %x\n\tflags: %x\n", b, c,d, e, f, g, h, i, l, m, n);
    
  h_ptr = (void *) newTrack_hook.orig;
  hook_precall(&newTrack_hook);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  hook_postcall(&newTrack_hook);
  
  cblk = *(unsigned long*) (a + 0x1c);
  log("\tnew track cblk: %p\n", cblk);
  

  // ----------------------------------

    /* fetch the necessary fields */
#ifdef DBG
  log("\t\t\tcblk flags %x\n", *(signed int*) (cblk + 0x3c));
#endif

  /* [3] */
  frameSize = *(unsigned char*) (cblk + 0x34 );
#ifdef DBG
  log("\t\t\tframeSize %x\n", frameSize);
#endif

  // not really useful, info only
  sampleRate = *(unsigned int*) (cblk + 0x30);
#ifdef DBG
  log("\t\t\tsampleRate %x\n", sampleRate);
#endif

  /* [2] second field within AudioBufferProvider */
  framesCount = * (unsigned int*) (b + 4);
#ifdef DBG
  log("\t\t\tframesCount %x\n", framesCount);
#endif

  /* [1] first field (ptr) within AudioBufferProvider */
  bufferRaw = *(unsigned int*) (b);

  
  // add the cblk to the tracks list, if
  // we don't find it, it might be because
  // the injection took place after the track
  // was created
  //log("start hash find\n");
  HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);
  
  if( cblk_tmp == NULL ) {

    cblk_tmp =  malloc( sizeof(struct cblk_t) );

    cblk_tmp->cblk_index = cblk;
    cblk_tmp->streamType = streamType;

    snprintf(randString, 11, "%lu", mrand48());
    snprintf(timestamp,  11, "%d", time(NULL));

    cblk_tmp->trackId = strdup(randString);
    log("trackId: %s\n", cblk_tmp->trackId);

    //         Qi-<timestamp>-<id univoco per chiamata>-<canale>.[tmp|boh]
    // length:  2-strlen(timestamp)-strlen(randString)-1.3.1 + 3 chars for '-'
    // filenameLength including null


    if( cblk_tmp->streamType == 0 ) {
      filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(randString) + 1 + 4 + 1 + 3;

      cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
      snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-r.bin", dumpPath, timestamp, randString );
      log("filename: %s\n", cblk_tmp->filename);
        

      cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);

    }


    cblk_tmp->startOfCircularBuffer = *(unsigned int*) (cblk + 0x18);
    cblk_tmp->frameCount = *(unsigned int*) (cblk + 0x1c);


    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;


    cblk_tmp->sampleRate = sampleRate;
    
    HASH_ADD_INT(cblkTracks, cblk_index, cblk_tmp);
    cblk_tmp = NULL;
    
    HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);
    
    if( cblk_tmp == NULL ) {
#ifdef DBG
      log("uthash shit happened\n");
#endif
      return result;
    }
#ifdef DBG
    log("\tnew Track added cblk\n");
#endif

  }
  
  log("\t\t\t----------- exit new track %p %p ------------------\n", result, a)

  return result;
}


void* playbackTrackStart_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  unsigned long cblk;

  log("\t\t\t------- enter playbackTrackStart -------------\n");
 
  log("enter start track %x\n", a);
  log("\tevent: %x\n\ttriggerSession: %x\n\n", b, c);

  cblk = *(unsigned long*) (a + 0x1c);
  log("\tnew track cblk: %x\n", cblk);

  h_ptr = (void *) playbackTrackStart_hook.orig;
  hook_precall(&playbackTrackStart_hook);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  hook_postcall(&playbackTrackStart_hook);
  
  log("\t\t\t\tresult: %d\n", (int) result);
  log("\t\t\t------- exit playbackTrackStart -------------\n");

  return result;
}

void* playbackTrackStop_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  unsigned int cblk;
  char *filename;
  struct cblk_t *cblk_tmp = NULL;
  unsigned int uintTmp;
  time_t now;

#ifdef DBG
  log("\t\t\t------- enter playbackTrackStop -------------\n");
#endif

  cblk = *(unsigned long*) (a + 0x1c);

#ifdef DBG
  log("\tnew track cblk: %x\n", cblk);
#endif

  h_ptr = (void *) playbackTrackStop_hook.orig;
  hook_precall(&playbackTrackStop_hook);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  hook_postcall(&playbackTrackStop_hook);
  //  log("\t\t\t\tresult: %d\n", (int) result);

  HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);

  if( cblk_tmp != NULL && cblk_tmp->streamType == 0 ) {


    /* write fake final header */
    now = time(NULL);
    write(cblk_tmp->fd, &now, 4); // 1] epoch start
  
    uintTmp = 0xf00df00d;
    write(cblk_tmp->fd, &uintTmp, 4); // 3] streamType

    uintTmp = cblk_tmp->sampleRate;
    write(cblk_tmp->fd, &uintTmp, 4); // 4] sampleRate

    uintTmp = 0;
    write(cblk_tmp->fd, &uintTmp, 4); // 5] size of block - 0 for last block
    

    /* rename file to .tmp and free the cblk structure  */
#ifdef DBG
    log("Stop: should take a dump\n");
#endif	  


    /* rename file *.bin to *.tmp */
    filename = strdup(cblk_tmp->filename); // check return value
    *(filename+strlen(filename)-1) = 'p';
    *(filename+strlen(filename)-2) = 'm';
    *(filename+strlen(filename)-3) = 't';

#ifdef DBG  
    log("fname %s\n", filename);
    log("rename %d\n", rename(cblk_tmp->filename, filename) );
#endif

    if( filename != NULL)
      free(filename);

    close(cblk_tmp->fd);

    /* free some stuff */
    if( cblk_tmp->trackId != NULL )
      free(cblk_tmp->trackId);

    if( cblk_tmp->filename != NULL )
      free(cblk_tmp->filename);

    HASH_DEL(cblkTracks, cblk_tmp);
    free(cblk_tmp);

#ifdef DBG
    log("freed all the crap\n");
#endif
  }

  log("\t\t\t------- exit playbackTrackStop -------------\n");

  return result;
}

void* playbackTrackPause_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  unsigned long cblk;

  log("\t\t\t------- enter playbackTrackPause -------------\n");
 
  cblk = *(unsigned long*) (a + 0x1c);
  log("\tnew track cblk: %x\n", cblk);

  h_ptr = (void *) playbackTrackStart_hook.orig;
  hook_precall(&playbackTrackStart_hook);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  hook_postcall(&playbackTrackStart_hook);
  
  log("\t\t\t\tresult: %d\n", (int) result);

  log("\t\t\t------- exit playbackTrackPause -------------\n");

  return result;
}



/* end signaling hooks */

/* start dumper hooks */

/* void* playbackTimedTrack_getNextBuffer_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) { */

/*   void* (*playback_h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w); */
/*   void* result; */
/*   unsigned long* cblk_ptr; */
/*   unsigned long cblk; */
/*   unsigned long framesAvail; */
/*   unsigned long framesReq; */
/*   unsigned long bufferSize; */
/*   unsigned long bufferStart; */
/*   unsigned long thisStart; */
/*   unsigned long bufferEndAddress; */
/*   int res; */
/*   int ii, cblkIndex; */
  

/*   log("\t\t\t------- playback timed -------------\n"); */
/*   playback_h_ptr = (void *) playbackTimedTrack_getNextBuffer_hook.orig; */
/*   hook_precall(&playbackTimedTrack_getNextBuffer_hook); */
/*   log("\t\tcall %p\n", playback_h_ptr); */
/*   result = playback_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w); */
/*   log("\t\t\t\tresult: %d\n", (int) result); */
/*   hook_postcall(&playbackTimedTrack_getNextBuffer_hook); */


/*   return result; */
/* } */

/* void* recordThread_getNextBuffer_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) { */

/*   void* (*record_h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w); */
/*   void* result; */
/*   unsigned long* cblk_ptr; */
/*   unsigned long cblk; */
/*   unsigned long framesAvail; */
/*   unsigned long framesReq; */
/*   unsigned long bufferSize; */
/*   unsigned long bufferStart; */
/*   unsigned long thisStart; */
/*   unsigned long bufferEndAddress; */
/*   int res; */

/*   log("\t\t\t------ record thread --------------\n"); */
/*   record_h_ptr = (void *) recordThread_getNextBuffer_hook.orig; */
/*   hook_precall(&recordThread_getNextBuffer_hook); */
/*   result = record_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w); */
/*   log("\t\t\t\tresult: %d\n", (int) result); */
/*   hook_postcall(&recordTrack_getNextBuffer_hook); */

/*   return result; */
/* } */




void* playbackTrack_getNextBuffer3_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);

  void* result;
  unsigned int* cblk_ptr;
  unsigned int cblk;
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

#ifdef DBG
  log("\t\t\t------- playback3 -------------\n");
#endif
  
  /* if( cblk_tmp->fd == 0) { */
  /*   cblk_tmp->fd = open("/data/local/tmp/log_out" , O_RDWR , S_IRUSR | S_IRGRP | S_IROTH); */
  /* } */


  /* call the original function */
  h_ptr = (void *) playbackTrack_getNextBuffer_hook.orig;
  hook_precall(&playbackTrack_getNextBuffer_hook);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  hook_postcall(&playbackTrack_getNextBuffer_hook);
   
#ifdef DBG
  log("\t\t\t\tresult: %d\n", (int) result);
#endif

  
  /* fetch the necessary fields */
  cblk_ptr = (unsigned int*) (a + 0x1c) ;
  cblk = *cblk_ptr;
#ifdef DBG
  log("\t\t\tcblk %p\n", cblk);
#endif

#ifdef DBG
  log("\t\t\tcblk flags %x\n", *(signed int*) (cblk + 0x3c));
#endif

  /* [3] */
  frameSize = *(unsigned char*) (cblk + 0x34 );
#ifdef DBG
  log("\t\t\tframeSize %x\n", frameSize);
#endif

  // not really useful, info only
  sampleRate = *(unsigned int*) (cblk + 0x30);
#ifdef DBG
  log("\t\t\tsampleRate %x\n", sampleRate);
#endif

  /* [2] second field within AudioBufferProvider */
  framesCount = * (unsigned int*) (b + 4);
#ifdef DBG
  log("\t\t\tframesCount %x\n", framesCount);
#endif

  /* [1] first field (ptr) within AudioBufferProvider */
  bufferRaw = *(unsigned int*) (b);

  
  // add the cblk to the tracks list, if
  // we don't find it, it might be because
  // the injection took place after the track
  // was created
  //log("start hash find\n");
  HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);
  
  
/*     cblk_tmp =  malloc( sizeof(struct cblk_t) ); */

/* #ifdef DBG */
/*     log("cblk_tmp %x size %d\n", cblk_tmp, sizeof(cblk_tmp)); */
/* #endif */

/*     cblk_tmp->cblk_index = cblk; */
/*     cblk_tmp->streamType = -3; // dummy value, we can't tell from within getNextBuffer the exact streamType */
    
/*     snprintf(randString, 11, "%lu", mrand48()); */
/*     snprintf(timestamp,  11, "%d", time(NULL)); */

/*     cblk_tmp->trackId = strdup(randString); */
/*     log("trackId: %s\n", cblk_tmp->trackId); */

/*     //         Qi-<timestamp>-<id univoco per chiamata>-<canale>.[tmp|boh] */
/*     // length:  2-strlen(timestamp)-strlen(randString)-1.3.1 + 3 chars for '-' */
/*     // filenameLength including null */

/*     filenameLength = 21 + 2 + strlen(timestamp) + strlen(randString) + 1 + 4 + 1 + 3; */

/*     cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  ); */
/*     snprintf(cblk_tmp->filename, filenameLength, "/data/local/tmp/dump/Qi-%s-%s-r.bin",timestamp, randString ); */
/*     log("filename: %s\n", cblk_tmp->filename); */
        

/*     cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH); */
/*     log("%s fd: %d\n", cblk_tmp->filename, cblk_tmp->fd); */


/*     cblk_tmp->startOfCircularBuffer = *(unsigned int*) (cblk + 0x18); */
/*     cblk_tmp->frameCount = *(unsigned int*) (cblk + 0x1c); */

/* #ifdef DBG */
/*     log("cblk start of circular buffer: %x\nframe count: %x\nframe size %d\nbuffer end: %x\n", cblk_tmp->startOfCircularBuffer, cblk_tmp->frameCount, frameSize, cblk_tmp->startOfCircularBuffer + (cblk_tmp->frameCount * frameSize)); */
/* #endif */

/*     cblk_tmp->lastBufferRaw  = bufferRaw; */
/*     cblk_tmp->lastFrameCount = framesCount; */
/*     cblk_tmp->lastFrameSize  = frameSize; */
    
/*     cblk_tmp->sampleRate = sampleRate; */
    
/*     HASH_ADD_INT(cblkTracks, cblk_index, cblk_tmp); */
/*     cblk_tmp = NULL; */
    
/*     HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp); */
    
/*     if( cblk_tmp == NULL ) { */
/* #ifdef DBG */
/*       log("uthash shit happened\n"); */
/* #endif */
/*       return result; */
/*     } */
/* #ifdef DBG */
/*     log("\t\tadded cblk from within getNextBuffer\n"); */
/* #endif */

/*   } */

/*   else { */

  if( cblk_tmp != NULL ) {    

    if( result == 0 && framesCount != 0 && cblk_tmp->streamType == 0 ) {

      // header
      headerStart = lseek(cblk_tmp->fd, 0, SEEK_CUR);
      //log("\t\theaderStart: %x\n", headerStart);

      // epoch : streamType : sampleRate : blockLen

      uintTmp = 0xb00bb00b;
      write(cblk_tmp->fd, &uintTmp, 4); // 1] epoch start
      //write(cblk_tmp->fd, &uintTmp, 4); // 2] epoch end
      write(cblk_tmp->fd, &uintTmp, 4); // 3] streamType
      write(cblk_tmp->fd, &uintTmp, 4); // 4] sampleRate
      write(cblk_tmp->fd, &uintTmp, 4); // 5] size of block

#ifdef DBG
      log("\t\t\tbuffer spans: %p -> %p\n", bufferRaw, (bufferRaw + framesCount * frameSize ) );
#endif
      res = write(cblk_tmp->fd, bufferRaw, framesCount * frameSize  );
#ifdef DBG
      log("\t\t\t\twrote: %d - expected: %d\n", res, framesCount * frameSize );
#endif
      positionTmp = lseek(cblk_tmp->fd, 0, SEEK_CUR);

#ifdef DBG
      log("\t\tfake header written: %x\n", positionTmp);
#endif
	
      // if something is written fix the header
      if( res > 0 ) {
	// go back to start of header
	lseek(cblk_tmp->fd, headerStart, SEEK_SET);

	log("\t\trolled back: %x\n", lseek(cblk_tmp->fd, 0, SEEK_CUR));

	// write the header
	now = time(NULL);
	//tt = gettimeofday(&tv,NULL);
	written = write(cblk_tmp->fd, &now , 4); // 1] timestamp start
	//written = write(cblk_tmp->fd, &now , 4); // 2] timestamp end - fake value
	written = write(cblk_tmp->fd, &cblk_tmp->streamType , 4); // 3] streamType
	written = write(cblk_tmp->fd, &cblk_tmp->sampleRate , 4); // 4] sampleRate
	written = write(cblk_tmp->fd, &res , 4); // 5] res
	  
#ifdef DBG
	log("\t\t\theader fixed %x -> %x:%x:%x:%x\n", cblk_tmp->cblk_index, now, cblk_tmp->streamType, cblk_tmp->sampleRate, res);
#endif
	
	// reposition the fd
	lseek(cblk_tmp->fd, positionTmp, SEEK_SET);
	log("\t\trolled forward: %x\n", lseek(cblk_tmp->fd, 0, SEEK_CUR));


	/* dump when fd is at position >= 5242880 */
	if( lseek(cblk_tmp->fd, 0, SEEK_CUR) >= 5242880 ) {

	  log("should take a dump\n");
	  
	  /* rename file *.bin to *.tmp */
	  filename = strdup(cblk_tmp->filename); // check return value
	  *(filename+strlen(filename)-1) = 'p';
	  *(filename+strlen(filename)-2) = 'm';
	  *(filename+strlen(filename)-3) = 't';
	  
	  log("fname %s\n", filename);
	  log("rename %d\n", rename(cblk_tmp->filename, filename) );

	  if( filename != NULL)  {
	    log("freeing filename\n");
	    free(filename);
	    log("after filename free\n");
	  }
	  
	  close(cblk_tmp->fd);

	  /* generate a new filename for the track */
	  snprintf(timestamp,  11, "%d", time(NULL));
	  
	  filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(cblk_tmp->trackId) + 1 + 4 + 1 + 3;

	  cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
	  snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-r.bin", dumpPath, timestamp, cblk_tmp->trackId );
	  log("new filename: %s\n", cblk_tmp->filename);
        
	  cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);
	  log("%s fd: %d\n", cblk_tmp->filename, cblk_tmp->fd);

	}

      }
      // otherwise remove the header, we don't need this block
      else {
	lseek(cblk_tmp->fd, headerStart, SEEK_SET);
      }
    
    }

    /* in both cases update cblk_tmp for the next run  */
    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;

  }

#ifdef DBG
  log("\t\t\t------- end playback3 -------------\n");
#endif

  
  return result;

}



void* recordTrack_getNextBuffer3_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);

  void* result;
  unsigned int* cblk_ptr;
  unsigned int cblk;
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

#ifdef DBG
  log("\t\t\t------- record3 -------------\n");
#endif
  

  /* call the original function */
  h_ptr = (void *) recordTrack_getNextBuffer_hook.orig;
  hook_precall(&recordTrack_getNextBuffer_hook);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  hook_postcall(&recordTrack_getNextBuffer_hook);
   
#ifdef DBG
  log("\t\t\t\tresult: %d\n", (int) result);
#endif
  

  /* fetch the necessary fields */
  cblk_ptr = (unsigned int*) (a + 0x1c) ;
  cblk = *cblk_ptr;
#ifdef DBG
  log("\t\t\tcblk %p\n", cblk);
#endif

#ifdef DBG
  log("\t\t\tcblk flags %x\n", *(signed int*) (cblk + 0x3c));
#endif

  /* [3] */
  frameSize = *(unsigned char*) (cblk + 0x34 );
#ifdef DBG
  log("\t\t\tframeSize %x\n", frameSize);
#endif

  // not really useful, info only
  sampleRate = *(unsigned int*) (cblk + 0x30);
#ifdef DBG
  log("\t\t\tsampleRate %x\n", sampleRate);
#endif

  /* [2] second field within AudioBufferProvider */
  framesCount = * (unsigned int*) (b + 4);
#ifdef DBG
  log("\t\t\tframesCount %x\n", framesCount);
#endif

  /* [1] first field (ptr) within AudioBufferProvider */
  bufferRaw = *(unsigned int*) (b);

  
  // add the cblk to the tracks list, if
  // we don't find it, it might be because
  // the injection took place after the track
  // was created
  HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);

  if( cblk_tmp == NULL ) {

    /* old code: insert from getBuffer  if( cblk_tmp != NULL ) { */

    cblk_tmp =  malloc( sizeof(struct cblk_t) );

#ifdef DBG
    log("cblk_tmp %x size %d\n", cblk_tmp, sizeof(cblk_tmp));
#endif

    cblk_tmp->cblk_index = cblk;
    
    snprintf(randString, 11, "%lu", mrand48());
    snprintf(timestamp,  11, "%d", time(NULL));

    cblk_tmp->trackId = strdup(randString);
    log("trackId: %s\n", cblk_tmp->trackId);

    //         Qi-<timestamp>-<id univoco per chiamata>-<canale>.[tmp|boh]
    // length:  2-strlen(timestamp)-strlen(randString)-1.3.1 + 3 chars for '-'
    // filenameLength including null

    filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(randString) + 1 + 4 + 1 + 3;

    cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
    snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-l.bin",dumpPath, timestamp, randString );
    log("filename: %s\n", cblk_tmp->filename);
        

    cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);
    log("%s fd: %d\n", cblk_tmp->filename, cblk_tmp->fd);


    cblk_tmp->startOfCircularBuffer = *(unsigned int*) (cblk + 0x18);
    cblk_tmp->frameCount = *(unsigned int*) (cblk + 0x1c);

#ifdef DBG
    log("cblk start of circular buffer: %x\nframe count: %x\nframe size %d\nbuffer end: %x\n", cblk_tmp->startOfCircularBuffer, cblk_tmp->frameCount, frameSize, cblk_tmp->startOfCircularBuffer + (cblk_tmp->frameCount * frameSize));
#endif

    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;
    
    cblk_tmp->streamType = -2; // dummy value, we can't tell from within getNextBuffer the exact streamType
    cblk_tmp->sampleRate = sampleRate;
    
    HASH_ADD_INT(cblkTracks, cblk_index, cblk_tmp);
    cblk_tmp = NULL;
    
    HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);
    
    if( cblk_tmp == NULL ) {
#ifdef DBG
      log("uthash shit happened\n");
#endif
      return result;
    }
#ifdef DBG
    log("\t\tgetNextBuffer added cblk\n");
#endif
    
  }

  else {

#ifdef DBG
    log("\t\tstreamType: %d\n", cblk_tmp->streamType);
#endif

    if( result == 0 && framesCount != 0 ) {

      // header
      headerStart = lseek(cblk_tmp->fd, 0, SEEK_CUR);
      //log("\t\theaderStart: %x\n", headerStart);

      // epoch : streamType : sampleRate : blockLen

      uintTmp = 0xc00cc00c;
      write(cblk_tmp->fd, &uintTmp, 4); // 1] epoch
      write(cblk_tmp->fd, &uintTmp, 4); // 2] streamType
      write(cblk_tmp->fd, &uintTmp, 4); // 3] sampleRate
      write(cblk_tmp->fd, &uintTmp, 4); // 4] size of block

#ifdef DBG
      log("\t\t\tbuffer spans: %p -> %p\n", bufferRaw, (bufferRaw + framesCount * frameSize ) );
#endif
      res = write(cblk_tmp->fd, bufferRaw, framesCount * frameSize  );
#ifdef DBG
      log("\t\t\t\twrote: %d - expected: %d\n", res, framesCount * frameSize );
#endif
      positionTmp = lseek(cblk_tmp->fd, 0, SEEK_CUR);

#ifdef DBG
      log("\t\tfake header written: %x\n", positionTmp);
#endif
	
      // if something is written fix the header
      if( res > 0 ) {
	// go back to start of header
	lseek(cblk_tmp->fd, headerStart, SEEK_SET);

	log("\t\trolled back: %x\n", lseek(cblk_tmp->fd, 0, SEEK_CUR));

	// write the header
	now = time(NULL);
	//tt = gettimeofday(&tv,NULL);
	written = write(cblk_tmp->fd, &now , 4); // 1] timestamp
	written = write(cblk_tmp->fd, &cblk_tmp->streamType , 4); // 2] streamType
	written = write(cblk_tmp->fd, &cblk_tmp->sampleRate , 4); // 3] sampleRate
	written = write(cblk_tmp->fd, &res , 4); // 4] res

	  
#ifdef DBG
	log("\t\t\theader fixed %x -> %x:%x:%x:%x\n", cblk_tmp->cblk_index, now, cblk_tmp->streamType, cblk_tmp->sampleRate, res);
#endif
	
	// reposition the fd
	lseek(cblk_tmp->fd, positionTmp, SEEK_SET);
	//log("\t\trolled forward: %x\n", lseek(cblk_tmp->fd, 0, SEEK_CUR));
	

	/* dump when fd is at position >= 5242880 */
	if( lseek(cblk_tmp->fd, 0, SEEK_CUR) >= 5242880 ) {

	  log("should take a dump\n");
	  
	  /* rename file *.bin to *.tmp */
	  filename = strdup(cblk_tmp->filename); // check return value
	  *(filename+strlen(filename)-1) = 'p';
	  *(filename+strlen(filename)-2) = 'm';
	  *(filename+strlen(filename)-3) = 't';
	  
	  log("fname %s\n", filename);
	  log("rename %d\n", rename(cblk_tmp->filename, filename) );

	  if( filename != NULL)  {
	    log("freeing filename\n");
	    free(filename);
	    log("after filename free\n");
	  }
	  
	  close(cblk_tmp->fd);

	  /* generate a new filename for the track */
	  snprintf(timestamp,  11, "%d", time(NULL));
	  
	  filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(cblk_tmp->trackId) + 1 + 4 + 1 + 3;

	  cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
	  snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-l.bin", dumpPath, timestamp, cblk_tmp->trackId );
	  log("new filename: %s\n", cblk_tmp->filename);
        
	  cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);
	  log("%s fd: %d\n", cblk_tmp->filename, cblk_tmp->fd);

	}

      }
      // otherwise remove the header, we don't need this block
      else {
	lseek(cblk_tmp->fd, headerStart, SEEK_SET);
      }
    
    }

    /* in both cases update cblk_tmp for the next run  */
    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;

  }

#ifdef DBG
  log("\t\t\t------- end record3 -------------\n");
#endif

  
  return result;

}

