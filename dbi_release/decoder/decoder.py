#!/usr/bin/env python
import os
import sys
import time
import struct
import datetime

HEADER = [ 0x52494646, 0x9ad34800, 0x57415645, 0x666d7420,
           0x10000000, 0x01000100, 0x44ac0000, 0x88580100,
           0x02001000, 0x64617461, 0x76d3ff09 ]

class WaveHeader:
    pass

class Track:

    def __init__(self, trackId, trackType, sampleRate):
        self.trackId = trackId
        self.trackType = trackType
        self.streamType = None
        self.sampleRate = sampleRate
        self.epoch = []
        self.audioRaw = []


if __name__ == '__main__':


    # trackId -> files with the same trackId
    tracksFilename = {}

    # trackId -> Track object
    tracks = {}

    if len(sys.argv) != 2 :
        print 'usage: {} dumpFolder'.format(sys.argv[0])
        exit(-1)


    for f in os.listdir(sys.argv[1]):
        fname = f.split('.')[0].split('-')
        #assert fname[0] == 'Qi', '[E] wrong filename{}'.format( fname[0] )
        if not fname[0] == 'Qi':
            continue
        epoch = fname[1]
        trackId = fname[2]
        trackType = fname[3]

        if not trackId in tracksFilename.keys():
            tracksFilename[trackId] = [f]
        else:
            tracksFilename[trackId].append(f)


    for trackId in tracksFilename:
        print '[*] Track id: {}'.format(trackId)

        for tf in sorted(tracksFilename[trackId]):
            fname = os.path.join(sys.argv[1], tf)
            print '[*]\tReading dump: {}'.format(fname)

            dump = open(fname, 'rb').read()
            trackType = fname.split('.')[0].split('-')[-1]

            position = 0
            suff = 0
            while position < len(dump):

                # header format - each field is 4 bytes le :
                # epoch : streamType : sampleRate : blockLen

                epoch = struct.unpack('<I', dump[position:position+4])[0]
                position += 4

                streamType = struct.unpack('<I', dump[position:position+4])[0]
                position += 4

                sampleRate = struct.unpack('<I', dump[position:position+4])[0]
                position += 4

                blockLen = struct.unpack('<I', dump[position:position+4])[0]
                position += 4

                audioRaw = dump[position:position+blockLen]
                position +=blockLen

                if trackId not in tracks.keys():
                    tracks[trackId] = Track(trackId, trackType, sampleRate)
                    print '[*]\t\tCreating trackId {}\t\ttrackType: {}\tsampleRate: {}'.format(trackId, trackType, sampleRate)
                    tracks[trackId].epoch.append(epoch)

                    if( len(audioRaw) != 8 ):
                        tracks[trackId].audioRaw.append(audioRaw)
                    else:
                        print '[W]\t\tshouldn\'t take place'

                else:
                    #print '[*]\t\tFound existing trackId {}\t\ttrackType: {}'.format(trackId, trackType)
                    tracks[trackId].epoch.append(epoch)

                    if( len(audioRaw) > 128  ):
                        tracks[trackId].audioRaw.append(audioRaw)
                    else:
                        print '[W]\t\twrong size {}'.format(len(audioRaw))

                    #filename = 'dump_{}.bin'.format(suff)
                    #print filename
                    #open(filename, 'wb').write(audioRaw)
                    suff +=1


    if len(tracks.keys()) is not 0:
        print '[*] Dumping tracks'

        for t in tracks:
            filename = 'dump_{}_{}.wav'.format(t, tracks[t].trackType)
            print '\tWriting {} to disk'.format(filename)
            fh = open(filename, 'wb')

            audioRaw = tracks[t].audioRaw
            print '[D] number of blocks {} - size first {}'.format(len(audioRaw), len(audioRaw[0]) )
            audioRaw = ''.join(audioRaw)


            # write header
            header = HEADER

            header = [struct.pack('>I',x) for x in header ]

            # size of file = fsize - 8
            header[1] = struct.pack('<I',len(''.join(header)) + len(audioRaw) )

            # samples per sec
            # sample rate test: size of block * 21.5
            audioRawBlockLen = len(tracks[t].audioRaw[0])

            for r in tracks[t].audioRaw:
                if audioRawBlockLen != len(r):
                    print '\texpected: {} found: {}\t position {}'.format(audioRawBlockLen, len(r), tracks[t].audioRaw.index(r))


            #if tracks[t].trackType == 'r':
            #sampleRate = audioRawBlockLen * 21.5
            sampleRate = tracks[t].sampleRate

            header[6] =  struct.pack('<I', sampleRate)
            print '\tsample rate: {}'.format(sampleRate)

            # bytes per sec
            header[7] =  struct.pack('<I', audioRawBlockLen * 21.5 * 2)

            # size of data
            header[10] = struct.pack('<I', len(audioRaw))


            fh.write( ''.join(header) )


            fh.write( audioRaw )

            fh.close()









