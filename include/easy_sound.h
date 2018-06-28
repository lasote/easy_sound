#include <AL/al.h>
#include <AL/alc.h>

class EasyRecorder {
public:
    EasyRecorder(int);
    void start();
    void stop();
    std::vector<int16_t> get_data();
    void end();
    ALuint rate;
    ~EasyRecorder();
private:
    ALCdevice *mainDev;
    ALCcontext *mainContext;
    ALCdevice *captureDev;
};

class EasyPlayer {
public:
    EasyPlayer();
    void play(std::string);
    void play(std::vector<int16_t>, int rate);
    ~EasyPlayer();
private:
    ALCdevice *mainDev;
    ALCcontext *mainContext;
    ALCdevice *captureDev;
};


