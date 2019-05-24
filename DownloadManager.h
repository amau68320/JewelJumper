#pragma once
#include <mgpcl/String.h>
#include <mgpcl/Thread.h>
#include <mgpcl/Atomic.h>
#include <mgpcl/Mutex.h>
#include <mgpcl/Singleton.h>

#define dlMgr DownloadManager::instance()

class DownloadManager : protected m::Thread, public m::Singleton<DownloadManager>
{
    friend class m::Singleton<DownloadManager>;

public:
    void startDownloadManager();
    void onQuit();
    void queueDownload(const m::String &fname);
    int currentFile();
    float progress();

    const m::String &fileName(int idx) const
    {
        return m_queue[idx];
    }

protected:
    void run() override;

private:
    DownloadManager();
    ~DownloadManager();

    m::Atomic m_running;
    m::List<m::String> m_queue;
    m::Mutex m_lock;

    volatile int m_curFile;
    volatile float m_progress;
};
