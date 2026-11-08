#include "decoderProcess.h"

#include <iostream>
#include <vector>

namespace {
std::wstring environment(const wchar_t* name) {
    const DWORD size = GetEnvironmentVariableW(name, nullptr, 0);
    if (!size) return {};
    std::vector<wchar_t> value(size);
    const DWORD length = GetEnvironmentVariableW(name, value.data(), size);
    return length && length < size ? std::wstring(value.data(), length) : std::wstring();
}

std::wstring fullPath(const std::wstring& path) {
    const DWORD size = GetFullPathNameW(path.c_str(), 0, nullptr, nullptr);
    if (!size) return {};
    std::vector<wchar_t> value(size);
    const DWORD length = GetFullPathNameW(path.c_str(), size, value.data(), nullptr);
    return length && length < size ? std::wstring(value.data(), length) : std::wstring();
}

bool isFile(const std::wstring& path) {
    const DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

// Quote an argument using the Windows C runtime's command-line rules.
std::wstring quoted(const std::wstring& value) {
    std::wstring result = L"\"";
    size_t slashes = 0;
    for (const wchar_t c : value) {
        if (c == L'\\') { ++slashes; continue; }
        result.append(c == L'"' ? 2 * slashes + 1 : slashes, L'\\');
        result += c;
        slashes = 0;
    }
    result.append(2 * slashes, L'\\');
    return result + L'"';
}
}

DecoderProcess::~DecoderProcess() { stop(); }

bool DecoderProcess::running() const {
    return process_ && WaitForSingleObject(process_, 0) == WAIT_TIMEOUT;
}

const char* DecoderProcess::status() const {
    if (failure_) return failure_;
    if (running()) return "Decoder starting";
    return process_ ? "Decoder exited (see launch log)" : nullptr;
}

void DecoderProcess::stop() {
    // Closing the writer notifies the decoder to finish its session.
    if (input_) CloseHandle(input_);
    input_ = nullptr;
    if (process_) {
        WaitForSingleObject(process_, 4000);
        CloseHandle(process_);
    }
    process_ = nullptr;
}

bool DecoderProcess::start() {
    if (running()) return true;
    stop();
    failure_ = nullptr;
    const auto folder = fullPath(L"..\\ssvep-decoding");
    const auto profile = L"ssvep_decoding/profiles/decoder_4s_3h.json";
    if (folder.empty() || !isFile(folder + L"\\ssvep_decoding\\__main__.py") ||
        !isFile(folder + L"\\" + profile)) {
        failure_ = "Decoder folder or profile missing";
        return false;
    }
    auto python = environment(L"SSVEP_DECODER_PYTHON");
    if (!python.empty()) {
        python = fullPath(python);
    } else {
        python = folder + L"\\.venv\\Scripts\\python.exe";
        const auto user = environment(L"USERPROFILE");
        if (!isFile(python) && !user.empty()) python = user + L"\\.conda\\envs\\venv\\python.exe";
    }
    if (!isFile(python)) {
        failure_ = "Set SSVEP_DECODER_PYTHON";
        return false;
    }
    auto port = environment(L"SSVEP_DECODER_PORT");
    if (port.empty()) port = L"COM3";

    const auto logs = folder + L"\\sessions";
    if (!CreateDirectoryW(logs.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        failure_ = "Cannot create decoder sessions folder";
        return false;
    }
    const auto logPath = logs + L"\\maze-launch-" + std::to_wstring(GetCurrentProcessId()) + L".log";
    SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
    HANDLE log = CreateFileW(logPath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ,
                             &security, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (log == INVALID_HANDLE_VALUE) {
        failure_ = "Cannot open decoder launch log";
        return false;
    }
    HANDLE reader = nullptr, writer = nullptr;
    if (!CreatePipe(&reader, &writer, &security, 0)) {
        CloseHandle(log);
        failure_ = "Cannot open decoder stop signal";
        return false;
    }

    SIZE_T size = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
    std::vector<unsigned char> attributes(size);
    STARTUPINFOEXW startup{};
    startup.StartupInfo.cb = sizeof(startup);
    startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    startup.StartupInfo.hStdInput = reader;
    startup.StartupInfo.hStdOutput = startup.StartupInfo.hStdError = log;
    startup.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributes.data());
    HANDLE inherited[] = {reader, log};
    const bool initialized = size && InitializeProcThreadAttributeList(startup.lpAttributeList, 1, 0, &size);
    PROCESS_INFORMATION child{};
    BOOL launched = FALSE;
    if (initialized && UpdateProcThreadAttribute(startup.lpAttributeList, 0,
            PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inherited, sizeof(inherited), nullptr, nullptr)) {
        const auto command = quoted(python) + L" -u -m ssvep_decoding live --port " + quoted(port)
            + L" --seconds 3600 --decoder " + quoted(profile) + L" --maze-control --stop-on-stdin";
        std::vector<wchar_t> args(command.begin(), command.end());
        args.push_back(0);
        launched = CreateProcessW(python.c_str(), args.data(), nullptr, nullptr, TRUE,
            EXTENDED_STARTUPINFO_PRESENT | CREATE_NO_WINDOW, nullptr, folder.c_str(),
            &startup.StartupInfo, &child);
    }
    const DWORD launchError = launched ? 0 : GetLastError();
    if (initialized) DeleteProcThreadAttributeList(startup.lpAttributeList);
    CloseHandle(reader);
    CloseHandle(log);
    if (!launched) {
        CloseHandle(writer);
        failure_ = "Cannot launch decoder Python";
        std::cerr << "Decoder launch failed (Win32 error " << launchError << ").\n";
        return false;
    }
    CloseHandle(child.hThread);
    process_ = child.hProcess;
    input_ = writer;
    std::wcout << L"Decoder launch log: " << logPath << L'\n';
    return true;
}
