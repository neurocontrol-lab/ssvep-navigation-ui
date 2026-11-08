#include <winsock2.h>
#include <windows.h>
#include "decoderLink.h"

#include <cmath>
#include <locale>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")

namespace {
constexpr unsigned short port = 46081;
constexpr std::int64_t staleMs = 1000;

bool parseAction(const std::string& token, MazeAction& action) {
    if (token == "-") action = MazeAction::None;
    else if (token == "forward") action = MazeAction::Forward;
    else if (token == "backward") action = MazeAction::Backward;
    else if (token == "turn_left") action = MazeAction::Left;
    else if (token == "turn_right") action = MazeAction::Right;
    else return false;
    return true;
}

bool parse(const char* bytes, int length, DecoderSample& out) {
    std::istringstream in(std::string(bytes, length));
    in.imbue(std::locale::classic());
    std::string version, state, winner, accepted, extra;
    int control, emit;
    if (!(in >> version >> out.sentMs >> state >> winner >> out.score >> out.margin
          >> accepted >> control >> emit) || version != "SSVEP1") return false;
    if (state == "warm") out.state = DecoderState::Warming;
    else if (state == "score") out.state = DecoderState::Scored;
    else if (state == "invalid") out.state = DecoderState::Invalid;
    else if (state == "stop") out.state = DecoderState::Stopped;
    else return false;
    if (!parseAction(winner, out.winner) || !parseAction(accepted, out.accepted)
        || (control != 0 && control != 1) || (emit != 0 && emit != 1)
        || !std::isfinite(out.score) || out.score < 0 || out.score > 1
        || !std::isfinite(out.margin) || out.margin < 0 || out.margin > out.score) return false;
    out.control = control != 0;
    out.emit = emit != 0;
    for (auto& frequency : out.frequencies)
        if (!(in >> frequency) || !std::isfinite(frequency) || frequency <= 0) return false;
    if (in >> extra) return false;
    if (out.state != DecoderState::Scored)
        return out.winner == MazeAction::None && out.accepted == MazeAction::None && !out.emit;
    if (out.winner == MazeAction::None) return false;
    if (out.accepted != MazeAction::None && out.accepted != out.winner) return false;
    return !out.emit || out.accepted != MazeAction::None;
}
}

DecoderLink::~DecoderLink() {
    if (socket_ != INVALID_SOCKET) closesocket(socket_);
    if (started_) WSACleanup();
}

bool DecoderLink::open() {
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return false;
    started_ = true;
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) return false;
    BOOL exclusive = TRUE;
    u_long nonblocking = 1;
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    if (setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                   reinterpret_cast<const char*>(&exclusive), sizeof(exclusive)) == SOCKET_ERROR
        || bind(sock, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR
        || ioctlsocket(sock, FIONBIO, &nonblocking) == SOCKET_ERROR) {
        closesocket(sock);
        return false;
    }
    socket_ = sock;
    return true;
}

bool DecoderLink::poll(std::int64_t nowMs) {
    if (socket_ == INVALID_SOCKET) return false;
    bool updated = false;
    // Bound work per frame. Only the newest valid message can drive a pulse.
    for (int i = 0; i < 16; ++i) {
        char packet[256];
        const int size = recv(socket_, packet, sizeof(packet), 0);
        if (size == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEMSGSIZE) continue;
            break;
        }
        DecoderSample candidate;
        if (size > 0 && parse(packet, size, candidate)
            && candidate.sentMs > sample_.sentMs
            && candidate.sentMs >= nowMs - staleMs && candidate.sentMs <= nowMs + 50) {
            sample_ = candidate;
            updated = true;
        }
    }
    return updated;
}

DecoderState DecoderLink::state(std::int64_t nowMs) const {
    if (socket_ == INVALID_SOCKET) return DecoderState::Unavailable;
    if (sample_.sentMs && nowMs - sample_.sentMs > staleMs) return DecoderState::Stale;
    return sample_.state;
}

std::int64_t decoderClockMs() {
    static const auto frequency = [] { LARGE_INTEGER f{}; QueryPerformanceFrequency(&f); return f.QuadPart; }();
    LARGE_INTEGER value{};
    QueryPerformanceCounter(&value);
    return value.QuadPart / frequency * 1000 + value.QuadPart % frequency * 1000 / frequency;
}

const char* actionLabel(MazeAction action) {
    switch (action) {
    case MazeAction::Forward: return "Forward";
    case MazeAction::Backward: return "Backward";
    case MazeAction::Left: return "Turn left";
    case MazeAction::Right: return "Turn right";
    default: return "None";
    }
}
