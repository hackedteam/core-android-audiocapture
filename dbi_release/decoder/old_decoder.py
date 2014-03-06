#!/usr/bin/env python
import sys
import time
import struct
import datetime

HEADER = [ 0x52494646, 0x9ad34800, 0x57415645, 0x666d7420, 
           0x10000000, 0x01000100, 0x44ac0000, 0x88580100,
           0x02001000, 0x64617461, 0x76d3ff09 ]

class WaveHeader:
    pass

class Cblk:
    
    def __init__(self, cblkId, streamType, sampleRate):
        self.cblkId = cblkId
        self.streamType = streamType
        self.sampleRate = sampleRate
        self.seconds = []
        self.useconds = []
        self.audioRaw = []


if __name__ == '__main__':

    tracks = {}
    
    if len(sys.argv) != 3 or ( sys.argv[2] != 'in' and sys.argv[2] != 'out'):
        print 'usage: {} dumpFile in/out'.format(sys.argv[0])
        exit(-1)

    dump = open(sys.argv[1], 'rb').read()
    typeOfTrack = sys.argv[2]
    position = 0

    print '[*] Reading dump: {}'.format(sys.argv[1])
    
    suff = 0
    while position < len(dump):
        
        # header format - each field is 4 bytes le :
        # cblkId : seconds : useconds : streamType : sampleRate : blockLen
        cblkId = struct.unpack('<I', dump[position:position+4])[0]
        position += 4
        
        seconds = struct.unpack('<I', dump[position:position+4])[0]
        position += 4

        useconds = struct.unpack('<I', dump[position:position+4])[0]
        position += 4
        
        streamType = struct.unpack('<I', dump[position:position+4])[0]
        position += 4
        
        sampleRate = struct.unpack('<I', dump[position:position+4])[0]
        position += 4
        
        blockLen = struct.unpack('<I', dump[position:position+4])[0]
        position += 4
        
        audioRaw = dump[position:position+blockLen]
        position +=blockLen
        
        if cblkId not in tracks.keys():
            tracks[cblkId] = Cblk(cblkId, streamType, sampleRate)
            print '\tFound track cblk {}\t\tstreamType: {}\tsampleRate {}'.format( hex(cblkId), streamType, sampleRate)
            tracks[cblkId].seconds.append(seconds)
            tracks[cblkId].useconds.append(useconds)

            if( len(audioRaw) != 8 ):
                tracks[cblkId].audioRaw.append(audioRaw)
            else:
                print '\tShouldn\'t take place'

        else:
            if tracks[cblkId].streamType != streamType:
                print '[D] streamType changed {} -> {}'.format( streamType, tracks[cblkId].streamType )

            if tracks[cblkId].sampleRate != sampleRate:
                print '[D] sampleRate changed {} -> {}'.format( sampleRate, tracks[cblkId].sampleRate )


            tracks[cblkId].seconds.append(seconds)
            tracks[cblkId].useconds.append(useconds)


            if( len(audioRaw) > 128  ):
                tracks[cblkId].audioRaw.append(audioRaw)
            else:
                print 'wrong size {}'.format(len(audioRaw))
                #filename = 'dump_{}.bin'.format(suff)
                #print filename
                #open(filename, 'wb').write(audioRaw)
                suff +=1
                
                
        
    if len(tracks.keys()) is not 0:
        print '[*] Dumping tracks'

        for t in tracks:
            filename = 'dump_{}.wav'.format(hex(t))
            print '\tWriting {} to disk'.format(filename) 
            fh = open(filename, 'wb')

            audioRaw = tracks[t].audioRaw
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
                    
            # wechat
            if typeOfTrack == 'out':
                sampleRate = audioRawBlockLen * 21.5

            header[6] =  struct.pack('<I', sampleRate)
            print '\tsample rate: {}'.format(sampleRate)

            # bytes per sec
            header[7] =  struct.pack('<I', audioRawBlockLen * 21.5 * 2)

            # size of data
            header[10] = struct.pack('<I', len(audioRaw))

            
            fh.write( ''.join(header) )

            
            fh.write( audioRaw )

            fh.close()
            

            

        
    

    
    
