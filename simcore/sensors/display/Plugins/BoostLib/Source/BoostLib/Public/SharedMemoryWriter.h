#pragma once

#include "CoreMinimal.h"
#include <string>
#include <vector>
#include <cstdint>

// #include "SharedMemoryWriter.generated.h"

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef __GNUC__
#define DLL_EXPORT __attribute__ ((dllexport))
#else
#define DLL_EXPORT __declspec(dllexport)
#endif
#else
#if __GNUC__ >= 4
#define DLL_EXPORT __attribute__ ((visibility ("default")))
#else
#define DLL_EXPORT
#endif
#endif

class DLL_EXPORT SharedMemoryReader
{
public:
    SharedMemoryReader();
    ~SharedMemoryReader();

    bool init(const std::string& key, const int microsecond);
    bool reset();
    bool read(std::vector<uint8_t>& buffer, int64_t& timestamp);

private:
    std::string key_;
    std::string mtx_key_;
    TSharedPtr<struct SharedSegment> shared_segment_ = 0;

    int max_lock_wait_microsecond_ = 0;
};

class DLL_EXPORT SharedMemoryWriter
{
public:
    SharedMemoryWriter();
    ~SharedMemoryWriter();

    // microsecond is not ready
    bool init(const std::string& key, const int microsecond = 0);
    bool reset();
    bool write(const std::vector<uint8_t>& buffer, const int64_t timestamp);
    bool remove();

private:
    std::string key_;
    std::string mtx_key_;
    TSharedPtr<struct SharedSegment> shared_segment_ = 0;

    int max_lock_wait_microsecond_ = 0;
};
