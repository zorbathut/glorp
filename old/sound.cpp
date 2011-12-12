
#include "sound.h"

#include "debug.h"
#include "util.h"

#include <vorbis/vorbisfile.h>
#include <AL/al.h>

bool loadAsOgg(const char *prefix, int *buffer) {
  CHECK(alGetError() == AL_NO_ERROR);
  
  string fname = StringPrintf("data/%s.ogg", prefix);
    
  OggVorbis_File ov;
  if (ov_fopen(fname.c_str(), &ov))
    return false;
  
  vorbis_info *inf = ov_info(&ov, -1);
  CHECK(inf);
  CHECK(inf->channels == 1 || inf->channels == 2);
  CHECK(inf->rate == 44100);

  vector<short> data;

  while(1) {
    char buf[4096];
    int bitstream = 0;
    int rv = ov_read(&ov, buf, sizeof(buf), 0, 2, 1, &bitstream);
    CHECK(bitstream == 0);
    CHECK(rv >= 0);
    CHECK(rv <= sizeof(buf));
    CHECK(rv % (2 * inf->channels) == 0);
    if(rv == 0)
      break;
    short *pt = reinterpret_cast<short*>(buf);
    rv /= 2;
    
    data.insert(data.end(), pt, pt + rv);
  }

  alGenBuffers(1, (ALuint*)buffer);  
  alBufferData(*buffer, (inf->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, &data[0], data.size() * 2, inf->rate);
  
  CHECK(alGetError() == AL_NO_ERROR);
  
  CHECK(!ov_clear(&ov));

  return true;
}

struct WaveHeader {
  long riff;
  long size;
  long type;
};

struct WaveChunkHeader {
  long type;
  long size;
}; 

struct WaveFormat {
  short format;
  short channels;
  long rate;
  long bps;
  short align;
  short depth;
} ;

struct WaveFile {
  WaveHeader header;
  WaveChunkHeader format_header;
  WaveFormat format;
  WaveChunkHeader data_header;
};

unsigned long endian_swap(unsigned long in) {
  return ((in & 0xFF000000) >> 24) | ((in & 0x00FF0000) >>  8) | ((in & 0x0000FF00) <<  8) | ((in & 0x000000FF) << 24);
}

bool loadAsWav(const char *prefix, int *buffer) {
  CHECK(alGetError() == AL_NO_ERROR);
  
  string fname = StringPrintf("data/%s.wav", prefix);
  FILE *fil = fopen(fname.c_str(), "rb");
  
  if(!fil)
    return false;
  
  WaveFile header;
  CHECK(fread(&header, sizeof(header), 1, fil) == 1);
  
  CHECK(header.header.riff == endian_swap('RIFF'));
  CHECK(header.header.type == endian_swap('WAVE'));
  CHECK(header.format_header.type == endian_swap('fmt '));
  CHECK(header.format_header.size == 16);
  CHECK(header.format.format = 1);
  CHECK(header.format.channels == 1 || header.format.channels == 2);
  CHECK(header.format.depth == 16);
  CHECK(header.data_header.type == endian_swap('data'));
  
  vector<short> data(header.data_header.size / 2);
  
  CHECK(fread(&data[0], header.data_header.size, 1, fil) == 1);
  
  fclose(fil);
  
  alGenBuffers(1, (ALuint*)buffer);  
  alBufferData(*buffer, (header.format.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, &data[0], data.size() * 2, header.format.rate);
  
  CHECK(alGetError() == AL_NO_ERROR);
  
  return true;
};

bool loadAsFlac(const char *prefix, int *buffer) {
  return false;
};

int loadSound(const char *prefix) {
  int buffer = 0;
  if(loadAsOgg(prefix, &buffer)) { CHECK(buffer); return buffer; }
  if(loadAsWav(prefix, &buffer)) { CHECK(buffer); return buffer; }
  if(loadAsFlac(prefix, &buffer)) { CHECK(buffer); return buffer; }
  return 0;
}
