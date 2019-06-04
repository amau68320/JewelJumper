#pragma once
#include <mgpcl/String.h>
#include <mgpcl/Thread.h>
#include <mgpcl/Atomic.h>
#include <mgpcl/Mutex.h>
#include <mgpcl/Singleton.h>

/*
 * Macro a utiliser pour acceder a l'instance du gestionnaire de telechargements.
 */
#define dlMgr DownloadManager::instance()

/*
 * Gestionnaire de telechargement des skybox. Celles-ci etant lourde, une skybox
 * basse-definition est incluse par defaut dans GemStudio, est les skybox HD sont
 * telechargees au lancement du jeu.
 */
class DownloadManager : protected m::Thread, public m::Singleton<DownloadManager>
{
    friend class m::Singleton<DownloadManager>;

public:
    /*
     * Ajoute une skybox dans la file de telechargement.
     * Cette action ne peut etre effectuee que avant que
     * le gestionnaire de telechargement de soit lance.
     */
    void queueDownload(const m::String &fname);

    /*
     * Lance les telechargements dans un thread. Lorsque cette
     * methode est appelee, plus aucun telechargement ne peut
     * etre ajoute dans la file.
     */
    void startDownloadManager();

    /*
     * Fini le telechargement en cours et arrete le thread de
     * telechargement.
     */
    void onQuit();

    /*
     * Retourne le numero du fichier entrain d'etre telecharge.
     * Le nom corresspondant a ce fichier peut etre obtenu avec
     * fileName(id).
     */
    int currentFile();

    /*
     * Retourne l'avancement du telechargement en cours.
     * 0 = debut du telechargement, 1 = fin.
     */
    float progress();

    /*
     * Retourne le nom du fichier correspondant au numero passe
     * dans 'idx'.
     */
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
