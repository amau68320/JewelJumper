#include "DownloadManager.h"
#include <mgpcl/HTTPRequest.h>
#include <mgpcl/FileIOStream.h>
#include <mgpcl/Math.h>
#include <mgpcl/Logger.h>

DownloadManager::DownloadManager() : m::Thread("DL-MGR"), m_curFile(0), m_progress(0.0f)
{
}

DownloadManager::~DownloadManager()
{
}

void DownloadManager::startDownloadManager()
{
    if(m_running.get() == 0) {
        m_running.set(1);
        start();
    }
}

void DownloadManager::onQuit()
{
    if(m_running.get() != 0) {
        m_running.set(0);
        join();
    }
}

void DownloadManager::queueDownload(const m::String &fname)
{
    mAssert(m_running.get() == 0, "impossible d'ajouter un objet a la liste des telechargements lorsque le thread est deja demarre");
    m_queue.add(fname);
}

int DownloadManager::currentFile()
{
    int ret;
    m_lock.lock();
    ret = m_curFile;
    m_lock.unlock();

    return ret;
}

float DownloadManager::progress()
{
    float ret;
    m_lock.lock();
    ret = m_progress;
    m_lock.unlock();

    return ret;
}

void DownloadManager::run()
{
    const uint32_t bufSz = 65536;
    uint8_t *buf = new uint8_t[bufSz];

    for(int i = 0; i < m_queue.size() && m_running.get() != 0; i++) {
        m_lock.lock();
        m_curFile = i;
        m_progress = 0.0f;
        m_lock.unlock();

        mlogger.debug(M_LOG, "Telechargement de %s...", m_queue[i].raw());

        m::FileOutputStream fos;
        if(!fos.open(m::String("textures/") + m_queue[i], m::FileOutputStream::kOM_Truncate)) {
            mlogger.error(M_LOG, "Erreur lors de l'ouverture du fichier de sortie pour telecharger %s", m_queue[i].raw());
            continue;
        }

        m::HTTPRequest req(m::String("https://montoyo.net/JJSkyboxes/") + m_queue[i]);
        if(!req.perform()) {
            mlogger.error(M_LOG, "Erreur lors de la requete HTTP pour telecharger %s", m_queue[i].raw());
            continue;
        }

        m::SSharedPtr<m::HTTPInputStream> is(req.inputStream<m::RefCounter>().staticCast<m::HTTPInputStream>());
        if(!is) {
            mlogger.error(M_LOG, "Erreur lors de la lecture de la reponse HTTP pour telecharger %s", m_queue[i].raw());
            continue;
        }

        if(req.responseCode() != 200) {
            mlogger.error(M_LOG, "Erreur lors du telechargement de %s: code d'erreur HTTP %d", m_queue[i].raw(), req.responseCode());
            continue;
        }

        uint32_t sz = req.contentLength();
        while(sz > 0) {
            int rd = static_cast<int>(m::math::minimum(sz, bufSz));
            rd = is->read(buf, rd);

            if(rd <= 0) {
                mlogger.error(M_LOG, "Erreur lors de la lecture des donnees pour telecharger %s", m_queue[i].raw());
                break;
            }

            if(!m::IO::writeFully(&fos, buf, rd)) {
                mlogger.error(M_LOG, "Erreur lors de l'ecriture des donnees pour telecharger %s", m_queue[i].raw());
                break;
            }

            m_lock.lock();
            m_progress = is->progress();
            m_lock.unlock();

            sz -= static_cast<uint32_t>(rd);
        }

        fos.close();
        is->close();
    }

    delete[] buf;

    m_lock.lock();
    m_curFile = -1;
    m_lock.unlock();
}
