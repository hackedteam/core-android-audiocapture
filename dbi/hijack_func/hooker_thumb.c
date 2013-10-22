#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include "libt.h"
#include "util.h"

#include "hooker.h"
#include "uthash.h"


static int ip_register_offset = 0xd;
static int fd = 0;
static int fd_in = 0;
static int fd_out = 0;

struct cblk_t *cblkTracks = NULL;
struct cblk_t *cblkRecordTracks = NULL;





void* (*no_proto_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w );
void* (*pause_h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
void* (*dumpTracks)(void* obj, char* buf, int len);
void* (*pbDumpTracks)(unsigned long a, int fd, unsigned long args);




/* start signaling hooks */
void* recordTrackStart_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w)  {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  

  log("\t\t\t------- enter recordTrackStart -------------\n");


  h_ptr = (void *) recordTrackStart_hook.orig;
  hook_precall(&recordTrackStart_hook);
  
  log("\t\tcall %p\n", h_ptr);
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

  log("\t\t\t------- enter recordTrackStop -------------\n");
 
  h_ptr = (void *) recordTrackStop_hook.orig;
  hook_precall(&recordTrackStop_hook);
  
  log("\t\tcall %p\n", h_ptr);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  log("\t\t\t\tresult: %d\n", (int) result);
  
  hook_postcall(&recordTrackStop_hook);

  log("\t\t\t------- exit recordTrackStop -------------\n");

  return result;
}


void* playbackTrackStart_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;

  log("\t\t\t------- enter playbackTrackStart -------------\n");
 
  h_ptr = (void *) playbackTrackStart_hook.orig;
  hook_precall(&playbackTrackStart_hook);
  
  log("\t\tcall %p\n", h_ptr);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  log("\t\t\t\tresult: %d\n", (int) result);
  
  hook_postcall(&playbackTrackStart_hook);

  log("\t\t\t------- exit playbackTrackStart -------------\n");

  return result;
}

void* playbackTrackStop_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;

  log("\t\t\t------- enter playbackTrackStop -------------\n");
 
  h_ptr = (void *) playbackTrackStop_hook.orig;
  hook_precall(&playbackTrackStop_hook);
  
  log("\t\tcall %p\n", h_ptr);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  log("\t\t\t\tresult: %d\n", (int) result);
  
  hook_postcall(&playbackTrackStop_hook);

  log("\t\t\t------- exit playbackTrackStop -------------\n");

  return result;
}



/* end signaling hooks */

/* start dumper hooks */

void* playbackTimedTrack_getNextBuffer_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*playback_h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  unsigned long* cblk_ptr;
  unsigned long cblk;
  unsigned long framesAvail;
  unsigned long framesReq;
  unsigned long bufferSize;
  unsigned long bufferStart;
  unsigned long thisStart;
  unsigned long bufferEndAddress;
  int res;
  int ii, cblkIndex;
  

  log("\t\t\t------- playback timed -------------\n");
  playback_h_ptr = (void *) playbackTimedTrack_getNextBuffer_hook.orig;
  hook_precall(&playbackTimedTrack_getNextBuffer_hook);
  log("\t\tcall %p\n", playback_h_ptr);
  result = playback_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  log("\t\t\t\tresult: %d\n", (int) result);
  hook_postcall(&playbackTimedTrack_getNextBuffer_hook);


 

  return result;
}

void* recordThread_getNextBuffer_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*record_h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  unsigned long* cblk_ptr;
  unsigned long cblk;
  unsigned long framesAvail;
  unsigned long framesReq;
  unsigned long bufferSize;
  unsigned long bufferStart;
  unsigned long thisStart;
  unsigned long bufferEndAddress;
  int res;
  

  log("\t\t\t------ record thread --------------\n");
  record_h_ptr = (void *) recordThread_getNextBuffer_hook.orig;
  hook_precall(&recordThread_getNextBuffer_hook);
  log("\t\tcall %p\n", record_h_ptr);
  result = record_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  log("\t\t\t\tresult: %d\n", (int) result);
  hook_postcall(&recordTrack_getNextBuffer_hook);
  


  return result;
}



/* 
   dumps from buffer->raw [1] to buffer->frameCount [2] * frameSize [3]
   see status_t AudioFlinger::PlaybackThread::Track::getNextBuffer(AudioBufferProvider::Buffer* buffer, int64_t pts)
   in AudioFlinger.cpp
   

*/
void* playbackTrack_getNextBuffer2_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

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

#ifdef DBG
  log("\t\t\t------- playback2 -------------\n");
#endif
  
  if( fd_out == 0) {
    fd_out = open("/data/local/tmp/log_out" , O_RDWR , S_IRUSR | S_IRGRP | S_IROTH);
  }

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
#ifdef DBG
    log("cblk_tmp %x size %d\n", cblk_tmp, sizeof(cblk_tmp));
#endif
    cblk_tmp->cblk_index = cblk;
    
    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;
    
    cblk_tmp->streamType = 0xffff; // fake value
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
    log("\t\tadded cblk from within getNextBuffer\n");
#endif
  } 
  // otherwise dump from cblkb_tmp: buffer->raw [1] to buffer->frameCount [2] * frameSize [3]
  // and update cblk_tmp
  else {
    

    if( result == 0 && cblk_tmp->lastFrameCount != 0 ) { // aka NO_ERROR and cblk_tmp-last* contains valid pointers (see goto getNextBuffer_exit)

      // header
      headerStart = lseek(fd_out, 0, SEEK_CUR);
      //log("\t\theaderStart: %x\n", headerStart);

      uintTmp = 0xb00bb00b;
      write(fd_out, &uintTmp, 4); // 1] cblk
      write(fd_out, &uintTmp, 4); // 2] timestamp
      write(fd_out, &uintTmp, 4); // 3] timestamp
      write(fd_out, &uintTmp, 4); // 4] streamType
      write(fd_out, &uintTmp, 4); // 5] sampleRate
      write(fd_out, &uintTmp, 4); // 6] size of block

#ifdef DBG
      log("\t\t\tbuffer spans: %p -> %p\n", cblk_tmp->lastBufferRaw, (cblk_tmp->lastBufferRaw + cblk_tmp->lastFrameCount * cblk_tmp->lastFrameSize ) );
#endif
      res = write(fd_out, cblk_tmp->lastBufferRaw, cblk_tmp->lastFrameCount * cblk_tmp->lastFrameSize  );
#ifdef DBG
      log("\t\t\t\twrote: %d - expected: %d\n", res, cblk_tmp->lastFrameCount * cblk_tmp->lastFrameSize );
#endif
      positionTmp = lseek(fd_out, 0, SEEK_CUR);

#ifdef DBG
      log("\t\tfake header written: %x\n", positionTmp);
#endif
	
      // if something is written fix the header
      if( res > 0 ) {
	// go back to start of header
	lseek(fd_out, headerStart, SEEK_SET); 

	//log("\t\trolled back: %x\n", lseek(fd_out, 0, SEEK_CUR));

	// write the header
	now = time(NULL);
	tt = gettimeofday(&tv,NULL);
	write(fd_out, &cblk_tmp->cblk_index , 4); // 1] cblk
	write(fd_out, &tv.tv_sec , 4); // 2] timestamp
	write(fd_out, &tv.tv_usec , 4); // 3] timestamp
	write(fd_out, &cblk_tmp->streamType , 4); // 3] streamType
	write(fd_out, &cblk_tmp->sampleRate , 4); // 4] sampleRate
	write(fd_out, &res , 4); // 5] res
	  
#ifdef DBG
	log("\t\t\theader fixed: %x:%x:%x:%x\n", cblk_tmp->cblk_index, now, cblk_tmp->streamType, res);
#endif
	
	// reposition the fd
	lseek(fd_out, positionTmp, SEEK_SET);
	//log("\t\trolled forward: %x\n", lseek(fd_out, 0, SEEK_CUR));
      }
      // otherwise remove the header, we don't need this block
      else {
	lseek(fd_out, headerStart, SEEK_SET);
      }
    
    }

    /* in both cases update cblk_tmp for the next run  */
    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;

  }

#ifdef DBG 
  log("\t\t\t------- end playback2 -------------\n");
#endif

  
  return result;


  

}


void* playbackTrack_getNextBuffer_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);

  void* result;
  unsigned int* cblk_ptr;
  unsigned int cblk;
  unsigned long framesAvail;
  unsigned long framesReq;
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
  
  
  if( fd_out == 0) {
    fd_out = open("/data/local/tmp/log_out" , O_RDWR , S_IRUSR | S_IRGRP | S_IROTH);
  }


  log("\t\t\t------- playback -------------\n");
  h_ptr = (void *) playbackTrack_getNextBuffer_hook.orig;
  hook_precall(&playbackTrack_getNextBuffer_hook);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  log("\t\t\t\tresult: %d\n", (int) result);
  hook_postcall(&playbackTrack_getNextBuffer_hook);

  cblk_ptr = (unsigned int*) (a + 0x1c) ;
  cblk = *cblk_ptr;
  sampleRate = *(unsigned int*) (cblk + 0x30);
  log("\t\t\tcblk %p\n", cblk);
  
  
  // add the cblk to the tracks list, if
  // we don't find it, it might be because
  // the injection took place after the track
  // was created
  log("start hash find\n");
  HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);
  
  if( cblk_tmp == NULL ) {
    cblk_tmp =  malloc( sizeof(struct cblk_t) );
    log("cblk_tmp %x size %d\n", cblk_tmp, sizeof(cblk_tmp));
    cblk_tmp->cblk_index = cblk;
    cblk_tmp->playbackLastBufferStartAddress = 0;
    cblk_tmp->streamType = 0xffff; // fake value
    cblk_tmp->sampleRate = sampleRate;
    
    HASH_ADD_INT(cblkTracks, cblk_index, cblk_tmp);
    cblk_tmp = NULL;
    
    HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);
    if( cblk_tmp == NULL )
      log("uthash shit happened\n");
    log("\t\tadded cblk from within getNextBuffer\n");
  }
  // from here onwards cblk_tmp has meaningful values
  log("end hash stuff\n");



  bufferStart = *(unsigned long*) (cblk + 0x18); // fix with offset within structure
  
  // bufferSize is frameSize (0x34) * frameCount (0x1c)
  bufferSize = (*(unsigned char*) (cblk + 0x34 )) * ( *(unsigned long*) (cblk + 0x1c));
  log("\t\t\tframes: 0x%x - sizeOfFrame: %d - bufferSize: %d - sampleRate: %d\n", *(unsigned long*) (cblk + 0x1c), *(unsigned char*) (cblk + 0x34 ),  bufferSize, *(unsigned long*) (cblk + 0x30) );

  log("\t\t\tbuffer spans: %p -> %p\n", bufferStart, (bufferStart + bufferSize ) );

  thisStart = *(unsigned long*) b;
  log("\t\t\tthisStart: %p\n", thisStart);



  /* decide whether we dump or just update the offset */
  if( result  == 0 ) {
    if( cblk_tmp->playbackLastBufferStartAddress != 0 ) {

      // circular buffer wrap
      if( thisStart < cblk_tmp->playbackLastBufferStartAddress ) {
      
	// dump from playbackLastBufferStartAddress to the end of the buffer)
	log("\t\t\twrap dump: %p -> %p\n", cblk_tmp->playbackLastBufferStartAddress , bufferStart + bufferSize);

        // header
	headerStart = lseek(fd_out, 0, SEEK_CUR);
	log("headerStart: %x\n", headerStart);

	uintTmp = 0xb00bb00b;
	write(fd_out, &uintTmp, 4); // 1] cblk
	write(fd_out, &uintTmp, 4); // 2] timestamp
	write(fd_out, &uintTmp, 4); // 3] timestamp
	write(fd_out, &uintTmp, 4); // 4] streamType
	write(fd_out, &uintTmp, 4); // 5] sampleRate
	write(fd_out, &uintTmp, 4); // 6] size of block


	res = write(fd_out, cblk_tmp->playbackLastBufferStartAddress , (bufferStart + bufferSize) - cblk_tmp->playbackLastBufferStartAddress  );
	positionTmp = lseek(fd_out, 0, SEEK_CUR);

	log("header written: %x\n", positionTmp);
	
	// if something is written fix the header
	if( res > 0 ) {
	  // go back to start of header
	  lseek(fd_out, headerStart, SEEK_SET); 

	  log("rolled back: %x\n", lseek(fd_out, 0, SEEK_CUR));

	  // write the header
	  now = time(NULL);
	  tt = gettimeofday(&tv,NULL);
	  write(fd_out, &cblk_tmp->cblk_index , 4); // 1] cblk
	  write(fd_out, &tv.tv_sec , 4); // 2] timestamp
	  write(fd_out, &tv.tv_usec , 4); // 3] timestamp
	  write(fd_out, &cblk_tmp->streamType , 4); // 3] streamType
	  write(fd_out, &cblk_tmp->sampleRate , 4); // 4] sampleRate
	  write(fd_out, &res , 4); // 5] res
	  
	  log("\t\t\t-> header: %x:%x:%x:%x\n", cblk_tmp->cblk_index, now, cblk_tmp->streamType, res);

	  // reposition the fd
	  lseek(fd_out, positionTmp, SEEK_SET);
	  log("rolled forward: %x\n", lseek(fd_out, 0, SEEK_CUR));
	}
	// otherwise remove the header, we don't need this block
	else {
	  lseek(fd_out, headerStart, SEEK_SET);
	}
	log("\t\t\t\twrote: %d\n", res);

      }
      else {
      
	// dump from playbackLastBufferStartAddress to thisStart)
	log("\t\t\tdump from: %p -> %p\n", cblk_tmp->playbackLastBufferStartAddress, thisStart);

	// header
	
	headerStart = lseek(fd_out, 0, SEEK_CUR);
	log("headerStart: %x\n", headerStart);

	uintTmp = 0xcaf1caf1;
	write(fd_out, &uintTmp, 4); // 1] cblk
	write(fd_out, &uintTmp, 4); // 2] timestamp
	write(fd_out, &uintTmp, 4); // 3] timestamp
	write(fd_out, &uintTmp, 4); // 4] streamType
	write(fd_out, &uintTmp, 4); // 5] sampleRate
	write(fd_out, &uintTmp, 4); // 6] size of block


	res = write(fd_out, cblk_tmp->playbackLastBufferStartAddress , (thisStart) - cblk_tmp->playbackLastBufferStartAddress  );
	positionTmp = lseek(fd_out, 0, SEEK_CUR);
	
	log("header written: %x\n", positionTmp);

		// if something is written fix the header
	if( res > 0 ) {
	  // go back to start of header
	  lseek(fd_out, headerStart, SEEK_SET); 
	  log("rolled back: %x\n", lseek(fd_out, 0, SEEK_CUR));

	  // write the header
	  now = time(NULL);
	  tt = gettimeofday(&tv,NULL);
	  write(fd_out, &cblk_tmp->cblk_index , 4); // 1] cblk
	  write(fd_out, &tv.tv_sec , 4); // 2] timestamp
	  write(fd_out, &tv.tv_usec , 4); // 3] timestamp
	  write(fd_out, &cblk_tmp->streamType , 4); // 4] streamType
	  write(fd_out, &cblk_tmp->sampleRate , 4); // 5] sampleRate
	  write(fd_out, &res , 4); // 6] res
	  
	  log("\t\t\t-> header: %x:%x:%x:%x\n", cblk_tmp->cblk_index, now, cblk_tmp->streamType, res);

	  // reposition the fd
	  lseek(fd_out, positionTmp, SEEK_SET);
	  log("rolled forward: %x\n", lseek(fd_out, 0, SEEK_CUR));
	}
	// otherwise remove the header, we don't need this block
	else {
	  lseek(fd_out, headerStart, SEEK_SET);
	}


	log("\t\t\t\twrote: %d\n", res);
      }
    
      cblk_tmp->playbackLastBufferStartAddress = thisStart;
    } else 
      cblk_tmp->playbackLastBufferStartAddress = thisStart;
  } else {
    cblk_tmp->playbackLastBufferStartAddress = 0;
  }
  


  return result;
}


/*
  dumps from buffer->raw [1] to buffer->frameCount [2] * frameSize [3]
  see AudioFlinger.cpp
  
 */
void* recordTrack_getNextBuffer2_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);

  void* result;
  unsigned long* cblk_ptr;
  unsigned long cblk;
  unsigned long framesAvail;
  unsigned long framesReq;
  unsigned long bufferSize;
  unsigned long bufferStart;
  unsigned long thisStart;
  unsigned long bufferEndAddress;
  int res;
  unsigned long lr_var;
  unsigned int sampleRate;
  struct timeval tv;

  unsigned int uintTmp;
  struct cblk_t *cblk_tmp= NULL;
  off_t headerStart = 0, positionTmp = 0;
  time_t now;
  int tt = 0xf;

  unsigned int bufferRaw = 0; 
  unsigned int framesCount = 0;
  unsigned int frameSize = 0;


  if( fd_in == 0) {
    fd_in = open("/data/local/tmp/log_in" , O_RDWR , S_IRUSR | S_IRGRP | S_IROTH);
  }

#ifdef DBG
  log("\t\t\t------ record2 --------------\n");
#endif
  
  h_ptr = (void *) recordTrack_getNextBuffer_hook.orig;
  hook_precall(&recordTrack_getNextBuffer_hook);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);

#ifdef DBG
  log("\t\t\t\tresult: %d\n", (int) result);
#endif

  hook_postcall(&recordTrack_getNextBuffer_hook);

  cblk_ptr = (unsigned long*) (a + 0x1c) ;
  cblk = *cblk_ptr;

#ifdef DBG
  log("\t\t\tcblk %p\n", cblk);
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
  HASH_FIND_INT( cblkRecordTracks, &cblk, cblk_tmp);
  
  if( cblk_tmp == NULL ) {
    cblk_tmp =  malloc( sizeof(struct cblk_t) );
#ifdef DBG
    log("cblk_tmp %x size %d\n", cblk_tmp, sizeof(cblk_tmp));
#endif
    cblk_tmp->cblk_index = cblk;
    
    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;
    
    cblk_tmp->streamType = 0xffff; // fake value
    cblk_tmp->sampleRate = sampleRate;
    
    HASH_ADD_INT(cblkRecordTracks, cblk_index, cblk_tmp);
    cblk_tmp = NULL;
    
    HASH_FIND_INT( cblkRecordTracks, &cblk, cblk_tmp);
    
    if( cblk_tmp == NULL ) {
#ifdef DBG
      log("uthash shit happened\n");
#endif
      return result;
    }

#ifdef DBG
    log("\t\tadded cblk from within getNextBuffer\n");
#endif
  } 
  // otherwise dump from cblkb_tmp: buffer->raw [1] to buffer->frameCount [2] * frameSize [3]
  // and update cblk_tmp
  else {
    

    if( result == 0 && cblk_tmp->lastFrameCount != 0 ) { // aka NO_ERROR and cblk_tmp-last* contains valid pointers (see goto getNextBuffer_exit)

      // header
      headerStart = lseek(fd_in, 0, SEEK_CUR);

#ifdef DBG      
      log("\t\theaderStart: %x\n", headerStart);
#endif

      uintTmp = 0xdeaddead;
      write(fd_in, &uintTmp, 4); // 1] cblk
      write(fd_in, &uintTmp, 4); // 2] timestamp
      write(fd_in, &uintTmp, 4); // 3] timestamp
      write(fd_in, &uintTmp, 4); // 4] streamType
      write(fd_in, &uintTmp, 4); // 5] sampleRate
      write(fd_in, &uintTmp, 4); // 6] size of block

#ifdef DBG
      log("\t\t\tbuffer spans: %p -> %p\n", cblk_tmp->lastBufferRaw, (cblk_tmp->lastBufferRaw + cblk_tmp->lastFrameCount * cblk_tmp->lastFrameSize ) );
#endif
      res = write(fd_in, cblk_tmp->lastBufferRaw, cblk_tmp->lastFrameCount * cblk_tmp->lastFrameSize  );
#ifdef DBG
      log("\t\t\t\twrote: %d - expected: %d\n", res, cblk_tmp->lastFrameCount * cblk_tmp->lastFrameSize );
#endif
      positionTmp = lseek(fd_in, 0, SEEK_CUR);

#ifdef DBG
      log("\t\tfake header written: %x\n", positionTmp);
#endif
	
      // if something is written fix the header
      if( res > 0 ) {
	// go back to start of header
	lseek(fd_in, headerStart, SEEK_SET); 

#ifdef DBG
	log("\t\trolled back: %x\n", lseek(fd_in, 0, SEEK_CUR));
#endif

	// write the header
	now = time(NULL);
	tt = gettimeofday(&tv,NULL);
	write(fd_in, &cblk_tmp->cblk_index , 4); // 1] cblk
	write(fd_in, &tv.tv_sec , 4); // 2] timestamp
	write(fd_in, &tv.tv_usec , 4); // 3] timestamp
	write(fd_in, &cblk_tmp->streamType , 4); // 3] streamType
	write(fd_in, &cblk_tmp->sampleRate , 4); // 4] sampleRate
	write(fd_in, &res , 4); // 5] res
	
	  
#ifdef DBG
	log("\t\t\theader fixed: %x:%x:%x:%x\n", cblk_tmp->cblk_index, now, cblk_tmp->streamType, res);
#endif
	
	// reposition the fd
	lseek(fd_in, positionTmp, SEEK_SET);
#ifdef DBG
	log("\t\trolled forward: %x\n", lseek(fd_in, 0, SEEK_CUR));
#endif
      }
      // otherwise remove the header, we don't need this block
      else {
#ifdef DBG
	log("\t\tremoving header\n");
#endif 
	lseek(fd_in, headerStart, SEEK_SET);
      }
    
    }

    /* in both cases update cblk_tmp for the next run  */
    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;

  }

#ifdef DBG 
  log("\t\t\t------- end record2 -------------\n");
#endif


  return result;
}



void* recordTrack_getNextBuffer_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* result;
  unsigned long* cblk_ptr;
  unsigned long cblk;
  unsigned long framesAvail;
  unsigned long framesReq;
  unsigned long bufferSize;
  unsigned long bufferStart;
  unsigned long thisStart;
  unsigned long bufferEndAddress;
  int res;
  unsigned long lr_var;



  if( fd_in == 0) {
    fd_in = open("/data/local/tmp/log_in" , O_RDWR | O_APPEND, S_IRUSR | S_IRGRP | S_IROTH);
  }


  log("\t\t\t------ record --------------\n");
  pause_h_ptr = (void *) recordTrack_getNextBuffer_hook.orig;
  hook_precall(&recordTrack_getNextBuffer_hook);
  //log("\t\tcall %p\n", pause_h_ptr);
  result = pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  log("\t\t\t\tresult: %d\n", (int) result);
  hook_postcall(&recordTrack_getNextBuffer_hook);

  cblk_ptr = (unsigned long*) (a + 0x1c) ;
  cblk = *cblk_ptr;
  log("\t\t\tcblk %p\n", cblk);

  //bufferStart = (cblk + 0x40); // offset not fixed, follow the data structure instead
  bufferStart = *(unsigned long*) (cblk + 0x18); 
  
  // bufferSize is frameSize (0x34) * frameCount (0x1c)
  bufferSize = (*(unsigned char*) (cblk + 0x34 )) * ( *(unsigned long*) (cblk + 0x1c));
  log("\t\t\tframes: 0x%x - sizeOfFrame: %d - bufferSize: %d - sampleRate: %d\n", *(unsigned long*) (cblk + 0x1c), *(unsigned char*) (cblk + 0x34 ),  bufferSize,  *(unsigned long*) (cblk + 0x30 ));


  log("\t\t\tbuffer spans: %p -> %p\n", bufferStart, (bufferStart + bufferSize ) );

  thisStart = *(unsigned long*) b;
  log("\t\t\tthisStart: %p\n", thisStart);

  if( result  == 0 ) {
    if( lastBufferStartAddress != 0 ) {

      // wrap
      if( thisStart < lastBufferStartAddress ) {
      
	// dump from lastBufferStartAddress to the end of the buffer)
	log("\t\t\twrap dump: %p -> %p\n", lastBufferStartAddress, bufferStart + bufferSize);
	res = write(fd_in, lastBufferStartAddress , (bufferStart + bufferSize) - lastBufferStartAddress  );
	log("\t\t\t\twrote: %d\n", res);

      }
      else {
      
	// dump from lastBufferStartAddress to thisStart)
	log("\t\t\tdump from: %p -> %p\n", lastBufferStartAddress, thisStart);
	res = write(fd_in, lastBufferStartAddress , (thisStart) - lastBufferStartAddress  );
	log("\t\t\t\twrote: %d\n", res);
      }
    
      lastBufferStartAddress = thisStart;
    } else 
      lastBufferStartAddress = thisStart;
  } else {
    lastBufferStartAddress = 0;
  }


  return result;
}


/* end dumper hooks */

void*  stepServer_h(void *a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void* result;
  unsigned long cblk_t;
  unsigned long tmp;
  unsigned long serverAtEntryPoint; 
  unsigned long serverAtEpilogue; 
  unsigned char frameSize;
  unsigned long numberOfFrames;
  unsigned long frameCount; // cblk->frameCount
  unsigned long rawBufferStart;
  unsigned long offsetWithinRawBuffer;
  int res;

  log("enter stepServer - cblk instance: %x, number of frames: %d - %x\n", a, b, b);

  numberOfFrames = (unsigned long) b;

  if( fd == 0) {
    fd = open("/data/local/tmp/log2" , O_RDWR | O_APPEND, S_IRUSR | S_IRGRP | S_IROTH);
  }
  log("\t\tlog2 fd open: %d\n", fd);

 
  cblk_t = (unsigned long) a ; // start of cblk 

  /* if( rawBufferStart == 0) { */
  rawBufferStart = cblk_t + 0x40; // FIXME
  log("\t\trawBufferStart is at %p\n", rawBufferStart);
  /* } */

  serverAtEntryPoint = *(unsigned long*) (cblk_t + 0xc);

  
  frameSize = *(unsigned char *)  (cblk_t + 0x34); // size of each frame
  frameCount = *(unsigned long*)  (cblk_t + 0x1c); // buffer frame capacity
  log("\t\tframe size: %d\n", *(unsigned char *)  (cblk_t + 0x34) );
  log("\t\tframe size: %d\n", frameCount );
  log("\t\tsample rate: %d\n", *(unsigned long *)( cblk_t + 0x30) );

  
  /* real call */
  pause_h_ptr = (void *) stepServer_hook.orig;
  hook_precall(&stepServer_hook);
  //log("\t\t\tcall %p\n", pause_h_ptr);
  result = pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  //log("\t\tpostcall buffer: %p\n", result);
  hook_postcall(&stepServer_hook);


  serverAtEpilogue = *(unsigned long*) (cblk_t + 0xc);


  offsetWithinRawBuffer = (serverAtEntryPoint * frameSize) % (  frameCount * frameSize );

  // dump from offsetWithinRawBuffer to (numberOfFrames (arg0 i.e b) * frameSize)
  res = write(fd, rawBufferStart + offsetWithinRawBuffer, numberOfFrames * frameSize );
  log("\t\t\tdumping from %p size %d\n", rawBufferStart + offsetWithinRawBuffer, numberOfFrames * frameSize);
  log("\t\t\tbytes written %d\n", res);



  log("exit stepServer\n");

  return result;
}


void*  stepUser_h(void *a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void* result;
  unsigned long cblk_t;
  unsigned long tmp;
  unsigned long user; // cblk->serverbase
  unsigned char frameSize;
  unsigned long numberOfFrames;
  unsigned long frameCount; // cblk->frameCount
  unsigned long rawBufferStart;
  unsigned long offsetWithinRawBuffer;
  int res;

  log("enter stepUser - cblk instance: %x, number of frames: %d - %x\n", a, b, b);

  numberOfFrames = (unsigned long) b;

  if( fd == 0) {
    fd = open("/data/local/tmp/log2" , O_RDWR | O_APPEND, S_IRUSR | S_IRGRP | S_IROTH);
  }
  log("\t\tlog2 fd open: %d\n", fd);

 
  cblk_t = (unsigned long) a ; // start of cblk 

  /* if( rawBufferStart == 0) { */
  rawBufferStart = cblk_t + 0x40; //FIXME
  log("\t\trawBufferStart is at %p\n", rawBufferStart);
  /* } */

  user = *(unsigned long*) (cblk_t + 0x8);

  
  frameSize = *(unsigned char *)  (cblk_t + 0x34); // size of each frame
  frameCount = *(unsigned long*)  (cblk_t + 0x1c); // buffer frame capacity
  log("\t\tframe size: %d\n", *(unsigned char *)  (cblk_t + 0x34) );
  log("\t\tframe size: %d\n", frameCount );
  log("\t\tsample rate: %d\n", *(unsigned long *)( cblk_t + 0x30) );

  offsetWithinRawBuffer = (user * frameSize) % (  frameCount * frameSize );

  // dump from offsetWithinRawBuffer to (numberOfFrames (arg0 i.e b) * frameSize)
  res = write(fd, rawBufferStart + offsetWithinRawBuffer, numberOfFrames * frameSize );
  log("\t\t\tdumping from %p size %d\n", rawBufferStart + offsetWithinRawBuffer, numberOfFrames * frameSize);
  log("\t\t\tbytes written %d\n", res);
  
  /* real call */
  pause_h_ptr = (void *) stepUser_hook.orig;
  hook_precall(&stepUser_hook);
  //log("\t\t\tcall %p\n", pause_h_ptr);
  result = pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  //log("\t\tpostcall buffer: %p\n", result);
  hook_postcall(&stepUser_hook);




  log("exit stepUser\n");

  return result;
}




void*  getBuffer_h(void *a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  unsigned long result;
  unsigned long r3;
  unsigned long tmp;
  unsigned long mBuffer; // ThreadBase
  unsigned long r2; // cblk->serverbase
  unsigned char frameSize;
  int res;
  int lr_var;
  

  log("enter getBuffer - instance: %x, offset: %d frames: %d\n", a, b, c);

  if( fd == 0) {
   fd = open("/data/local/tmp/log2" , O_RDWR | O_APPEND, S_IRUSR | S_IRGRP | S_IROTH);
  }
 
  /* asm("mov %[lr], lr": [lr] "=r" (lr_var) ); */
  /* log("\t\tlr: %x\n", lr_var); */


  /* mBuffer = ((unsigned long)a + 0x20) ; */
  /* mBuffer = *(unsigned long*)mBuffer; */
  /* log("\t\tmBuffer: %x\n", mBuffer); */

  // a is r0
  tmp = ((unsigned long)a + 0x1c) ;
  //log("\t\tr0 + 0x1c: %x\n", tmp);
  r3 = *(unsigned long*) tmp ; // cblk is r3

  if( bufferStart == 0)
    bufferStart = r3 + 0x40; // from dbg -> FIXME

  r2 = *(unsigned long*) (r3 + 0x14);
  /* log("\t\tlast size , aka offset - cblk->serverbase: %d", b - r2); */
  
  
  //log("\t\tr3 = [r0 + 0x1c] %x\n",  r3);

  /* cblk = *(unsigned long *) cblk; */
  /* log("\tcblk struct 2 at %p\n",  cblk); */

  //tmp = (r3 + 0x34 );
  //log("\t\tr3 + 0x34: %x\n", tmp);
  
  frameSize = *(unsigned char *)  (r3 + 0x34);
  log("\t\tframe size: %d\n", *(unsigned char *)  (r3 + 0x34) );
  log("\t\tsample rate: %d\n", *(unsigned long *)( r3 + 0x30) );

  
  if( lastOffset != 0 ) {
   res = write(fd, lastOffset, (unsigned long) c * lastFrameSize );
   log("\t\t\tdumping from %x size %d\n", lastOffset,  (unsigned long) c * lastFrameSize);
   log("\t\t\tres: %d\n", res);
  }



  pause_h_ptr = (void *) getBuffer_hook.orig;
  hook_precall(&getBuffer_hook);

  log("\t\t\tcall %p\n", pause_h_ptr);
  result = (unsigned long)  pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);

  log("\t\tpostcall buffer: %p\n", result);
  hook_postcall(&getBuffer_hook);


  // dump
  /* if( bufferStart == result ) { */
  /*   res = write(fd,bufferStart, (lastOffset + lastSize)  - bufferStart ); */
  /*   log("\t\t\tdumping from %p size %d\n", bufferStart,  (lastOffset + lastSize)  - bufferStart); */
  /*   log("\t\t\tres: %d\n", res); */
  /* } */



  lastFrameSize = frameSize;
  lastSize = (unsigned long) b - r2;
  lastOffset = result;
  log("exit getBuffer\n");

  return (void*)result;
}




void*  pause_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void *result;
  
  log("enter start_output\n")
    //log("\tstream: %x\n\tsamplingRate: %x\n\tformat: %x\n\tchannels: %x\n\tflags: %x\n",b,c,d,e,*f)
    log("\toutput: %x\n\tstream: %x\n\tsession: %x\n",b,c,d)
    

  pause_h_ptr = (void *) pause_plt_hook.orig;

  log("precall\n");
  hook_precall(&pause_plt_hook);

  log("call %p\n", pause_h_ptr);
  result = pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);

  log("postcall %x\n", result);
  hook_postcall(&pause_plt_hook);
  
  log("exit start_output\n")

  return result;
}

void*  setStreamVolume_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void *result;
  
  log("enter stream volume\n")
    log("\tstream: %x\n\tindex: %x\n\tdevice: %x\n",b,c,d)
    

  pause_h_ptr = (void *) pause_plt_hook.orig;

  log("precall\n");
  hook_precall(&pause_plt_hook);

  log("call %p\n", pause_h_ptr);
  result = pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);

  log("postcall %x\n", result);
  hook_postcall(&pause_plt_hook);
  
  log("exit stream volume\n")

  return result;
}


// audio.h
void*  newRecordTrack_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void *result;
  unsigned long cblk;
  void* (*func_h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);

  log("enter start  record track\n");
  //log("\tpbthread: %x\n\tclient: %x\n\tsampleRate: %d\n", b, c,d);
    
  //cblk = *(unsigned long*) (a + 0x1c);
  //log("\tcblk: %p\n", cblk);

  func_h_ptr = (void *) newRecordTrack_hook.orig;

  log("precall\n");
  hook_precall(&newRecordTrack_hook);

  log("call %p\n", func_h_ptr);
  result = func_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);

  log("postcall %x\n", result);
  hook_postcall(&newRecordTrack_hook);
  
  log("exit start record track %p %p\n", result, a)

  return result;
}


void*  newTrack_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void *result;
  unsigned long cblk;

  log("enter new track\n");
  log("\tpbthread: %x\n\tclient: %x\n\tstreamType: %x\n\tsampleRate: %d\n\tformat: %x\n\tchannelMask: %x\n\tframeCount: %x\n\tsharedBuffer: %x\n\tsessionId: %x\n\ttid: %x\n\tflags: %x\n", b, c,d, e, f, g, h, i, l, m, n);
    
  cblk = *(unsigned long*) (a + 0x1c);
  log("\tcblk: %p\n", cblk);

  pause_h_ptr = (void *) newTrack_hook.orig;

  log("precall\n");
  hook_precall(&newTrack_hook);

  log("call %p\n", pause_h_ptr);
  result = pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);

  log("postcall %x\n", result);
  hook_postcall(&newTrack_hook);
  
  log("exit new track %p %p\n", result, a)

  return result;
}



void*  startTrack_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void *result;
  
  log("enter start track\n")
    log("\tevent: %x\n\ttriggerSession: %x\n\n", b, c)
    

  pause_h_ptr = (void *) startTrack_hook.orig;

  log("precall\n");
  hook_precall(&startTrack_hook);

  log("call %p\n", pause_h_ptr);
  result = pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);

  log("postcall %x\n", result);
  hook_postcall(&startTrack_hook);
  
  log("exit start track %p %p\n", result, a)

  return result;
}



void*  stopTrack_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void *result;
  //int fd = open("/data/local/tmp/log2", O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
  //char *args = (char *) malloc(2048);
  int ii;
  int res;
  

  /* pbDumpTracks = (void*) ( global_base_address +  0x3a841 ); */

  /* log("pbDumpTracks: %p audioflinger instance: %p\n", pbDumpTracks, pbInstance); */

  /* if( pbInstance != 0 ) { */

  /*   log("\tfd: %d\n", fd); */
  /*   pbDumpTracks(pbInstance, fd, 0xbadf00d); */

  /* } */
  
  /* dumpTracks = (void*) ( global_base_address + 0x35349);// 0x3a841 ); */
  /* log("\tpid: %d\n", pid_global); */
  /* log("\tdumpTracks %p\n", dumpTracks); */
  /* dumpTracks(a, args, 2047); */

  /* for(ii=0; ii < 2048; ii++) { */
  /*   log("%x ",args[ii]); */
  /*   if( ii % 30 == 0) */
  /*     log("\n"); */
  /* } */
  /* log("--------------------------------------\n"); */

  /* log("%s\n", args)   */

  log("enter stop track\n");
  

  pause_h_ptr = (void *) stopTrack_hook.orig;

  log("precall\n");
  hook_precall(&stopTrack_hook);

  log("call %p\n", pause_h_ptr);
  result = pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);


  //

  log("postcall\n");
  hook_postcall(&stopTrack_hook);
  
  log("exit stop track \n");
  
  //free(args);
  return result;
}


void*  audioFlingerC2_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void *result;
  
  log("enter PBCreateTrack: %x %d %x\n", a, b, c)

  pause_h_ptr = (void *) audioFlingerC2_hook.orig;

  log("\tprecall\n");
  hook_precall(&audioFlingerC2_hook);

  log("\t\tcall %p\n", pause_h_ptr);
  result = pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);

  log("\tpostcall\n");
  hook_postcall(&audioFlingerC2_hook);
  
  pbInstance = (unsigned long) a;
  log("exit PBCreateTrack %p\n", pbInstance)


  return result;
}



void*  audioFlingerC1_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void *result;
  
  log("enter audioFlinger create track\n");

  audioFlingerInstance = (unsigned long) a;

  pause_h_ptr = (void *) audioFlingerC1_hook.orig;

  log("\tprecall\n");
  hook_precall(&audioFlingerC1_hook);

  log("\t\tcall %p\n", pause_h_ptr);
  result = pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);

  // don't fix the hook
  //log("\tpostcall\n");
  //hook_postcall(&audioFlingerC1_hook);
  
  log("exit audioFlinger create track\n")

  return result;
}





void*  threadLoop_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
  
  void *result;
  
  log("enter thread loop\n")

  pause_h_ptr = (void *) threadLoop_hook.orig;

  log("\tprecall\n");
  hook_precall(&threadLoop_hook);

  log("\t\tcall %p\n", pause_h_ptr);
  result = pause_h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);

  log("\tpostcall\n");
  hook_postcall(&threadLoop_hook);
  
  log("exit thread loop \n")

  return result;
}

