#include <iostream>
#include <vector>
#include "../include/easy_sound.h"
#include <unistd.h>


EasyRecorder::EasyRecorder (int the_rate) {
    rate = the_rate;
    mainDev = alcOpenDevice(NULL);

    mainContext = alcCreateContext(mainDev, NULL);
    if (mainContext == NULL){
        throw "Unable to create playback context!";
    }
    // Make the playback context current
    alcMakeContextCurrent(mainContext);
    alcProcessContext(mainContext);
}

EasyRecorder::~EasyRecorder () {
    alcCaptureCloseDevice(captureDev);
    alcCloseDevice(mainDev);
    // Shut down OpenAL
    alcMakeContextCurrent(NULL);
}

void EasyRecorder::stop() {
    alcCaptureStop(captureDev);
}


void EasyRecorder::start() {
    // Open the default device
    captureDev = alcCaptureOpenDevice(NULL, rate, AL_FORMAT_MONO16, rate);
    if (captureDev == NULL){
        throw " Unable to open device!";
    }

    alcCaptureStart(captureDev);
}

std::vector<int16_t> EasyRecorder::get_data(){

    ALint samplesAvailable;

    // Get the number of samples available
    alcGetIntegerv(captureDev, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);

    ALubyte captureBuffer[samplesAvailable * 2];
    // Copy the samples to our capture buffer
    std::vector<int16_t> ret;
    if (samplesAvailable > 0){
        alcCaptureSamples(captureDev, captureBuffer, samplesAvailable);
        for (int i = 0; i < samplesAvailable * 2; ++i) {
            if (i % 2 == 0) {
                int16_t number = captureBuffer[i] | captureBuffer[i + 1] << 8;
                ret.push_back(number);
            }
        }
    }
    return ret;
}




EasyPlayer::EasyPlayer() {
    mainDev = alcOpenDevice(NULL);

    mainContext = alcCreateContext(mainDev, NULL);
    if (mainContext == NULL){
        throw "Unable to create playback context!";
    }
    // Make the playback context current
    alcMakeContextCurrent(mainContext);
    alcProcessContext(mainContext);
}

EasyPlayer::~EasyPlayer () {
    alcCaptureCloseDevice(captureDev);
    alcCloseDevice(mainDev);
    // Shut down OpenAL
    alcMakeContextCurrent(NULL);
}

void EasyPlayer::play(std::string filepath) {

    typedef unsigned int DWORD;

    FILE* fp = NULL;

    fp = fopen(filepath.c_str(), "rb");


    char type[4];
    DWORD size, chunkSize;
    short formatType, channels;
    DWORD sampleRate, avgBytesPerSec;
    short bytesPerSample, bitsPerSample;
    DWORD dataSize;

    fread(type, sizeof(char), 4, fp);
    if (type[0] != 'R' || type[1] != 'I' || type[2] != 'F' || type[3] != 'F')
        throw("OpenAL Error: No RIFF");

    fread(&size, sizeof(4), 1, fp);
    fread(type, sizeof(char), 4, fp);
    if (type[0] != 'W' || type[1] != 'A' || type[2] != 'V' || type[3] != 'E')
        throw("OpenAL Error: Not a WAVE file");

    fread(type, sizeof(char), 4, fp);
    if (type[0] != 'f' || type[1] != 'm' || type[2] != 't' || type[3] != ' ')
        throw("OpenAL Error: Not a fmt");

    fread(&chunkSize, sizeof(DWORD), 1, fp);
    fread(&formatType, sizeof(short), 1, fp);
    fread(&channels, sizeof(short), 1, fp);
    fread(&sampleRate, sizeof(DWORD), 1, fp);
    fread(&avgBytesPerSec, sizeof(DWORD), 1, fp);
    fread(&bytesPerSample, sizeof(short), 1, fp);
    fread(&bitsPerSample, sizeof(short), 1, fp);

    fread(type, sizeof(char), 4, fp);
    if (type[0] != 'd' || type[1] != 'a' || type[2] != 't' || type[3] != 'a')
        throw("OpenAL Error: Missing DATA");

    fread(&dataSize, sizeof(DWORD), 1, fp);

    unsigned char* buf = new unsigned char[dataSize];
    fread(buf, 1, dataSize, fp);

    ALuint source;
    ALuint buffer;
    ALuint frequency = sampleRate;
    ALenum format = 0;

    alGenBuffers(1, &buffer);
    alGenSources(1, &source);

    if (bitsPerSample == 8)
    {
        if (channels == 1)
            format = AL_FORMAT_MONO8;
        else if (channels == 2)
            format = AL_FORMAT_STEREO8;
    }
    else if (bitsPerSample == 16)
    {
        if (channels == 1)
            format = AL_FORMAT_MONO16;
        else if (channels == 2)
            format = AL_FORMAT_STEREO16;
    }

    alBufferData(buffer, format, buf, dataSize, frequency);

    ALfloat SourcePos[] = { 0.0f, 0.0f, 0.0f };
    ALfloat SourceVel[] = { 0.0f, 0.0f, 0.0f };
    ALfloat ListenerPos[] = { 0.0f, 0.0f, 0.0f };
    ALfloat ListenerVel[] = { 0.0f, 0.0f, 0.0f };
    ALfloat ListenerOri[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };

// Listener
    alListenerfv(AL_POSITION, ListenerPos);
    alListenerfv(AL_VELOCITY, ListenerVel);
    alListenerfv(AL_ORIENTATION, ListenerOri);

// Source
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, 1.0f);
    alSourcefv(source, AL_POSITION, SourcePos);
    alSourcefv(source, AL_VELOCITY, SourceVel);
    alSourcei(source, AL_LOOPING, AL_FALSE);

    alSourcePlay(source);

    // Wait for the source to stop playing
    ALint playState = AL_PLAYING;
    while (playState == AL_PLAYING){
        alGetSourcei(source, AL_SOURCE_STATE, &playState);
        usleep(100000);
    }
    fclose(fp);
    delete[] buf;

    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
}


void EasyPlayer::play(std::vector<int16_t> data, int rate){

    int bytesCaptured = data.size() * 2;
    ALubyte captureBuffer[bytesCaptured];

    for(int i=0; i<data.size();i++){
        int16_t el = data.at(i);
        ALubyte first = (el << 8) >> 8;
        ALubyte second = el >> 8;
        captureBuffer[i*2] = first;
        captureBuffer[(i*2)+1] = second;
    }

    ALuint buffer;
    ALuint source;
    ALint playState;

    // Generate an OpenAL buffer for the captured data
    alGenBuffers(1, &buffer);
    alGenSources(1, &source);
    alBufferData(buffer, AL_FORMAT_MONO16, captureBuffer, bytesCaptured, rate);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcePlay(source);

    // Wait for the source to stop playing
    playState = AL_PLAYING;
    while (playState == AL_PLAYING){
        alGetSourcei(source, AL_SOURCE_STATE, &playState);
        usleep(100000);
    }
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
}
