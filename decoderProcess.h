#pragma once

#include <windows.h>
#include <string>

// Owns only the decoder process started by this game.
class DecoderProcess {
public:
    DecoderProcess() = default;
    ~DecoderProcess();
    DecoderProcess(const DecoderProcess&) = delete;
    DecoderProcess& operator=(const DecoderProcess&) = delete;
    bool start();
    bool running() const;
    void stop();
    const char* status() const;

private:
    HANDLE process_ = nullptr;
    HANDLE input_ = nullptr;
    const char* failure_ = nullptr;
};
