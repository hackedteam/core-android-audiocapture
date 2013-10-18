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
    
    if len(sys.argv) != 2:
        print 'usage: {} dumpFile'.format(sys.argv[0])
        exit(-1)

    dump = open(sys.argv[1], 'rb').read()
    position = 0

    print '[*] Reading dump: {}'.format(sys.argv[1])
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
            tracks[cblkId].audioRaw.append(audioRaw)
        else:
            assert tracks[cblkId].streamType == streamType, '[E] streamType changed'
            assert tracks[cblkId].sampleRate == sampleRate, '[E] sampleRate changed'

            tracks[cblkId].seconds.append(seconds)
            tracks[cblkId].useconds.append(useconds)
            tracks[cblkId].audioRaw.append(audioRaw)
            
        
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
            
            # samples per sec
            header[6] =  struct.pack('<I', tracks[t].sampleRate)
            
            # bytes per sec
            header[7] =  struct.pack('<I', tracks[t].sampleRate*2)

            # size
            header[10] = struct.pack('<I', len(audioRaw))

            fh.write( ''.join(header) )


            # write raw audio
            #size = len(audioRaw) 

            #fmt = [ 'I' for i in range(0, size ) ]
            #fmt = '<' + ''.join(fmt)
            fh.write( audioRaw )

            fh.close()
            

            

        
    

    
    