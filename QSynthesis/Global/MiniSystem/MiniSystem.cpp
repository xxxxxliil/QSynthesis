#include "MiniSystem.h"
#include "MiniSystemNotifier.h"
#include "QUtils.h"

#include <QDir>
#include <QMessageBox>

Q_SINGLETON_DECLARE(MiniSystem)

MiniSystem::MiniSystem(QObject *parent) : QObject(parent) {
    m_thread = new QThread(this);
    m_fileWatcher.moveToThread(m_thread);
    m_dirWatcher.moveToThread(m_thread);

    connect(m_thread, &QThread::started, &m_fileWatcher, &MiniSystemWatcher::watch);
    connect(m_thread, &QThread::started, &m_dirWatcher, &MiniSystemWatcher::watch);

    connect(m_thread, &QThread::finished, this, &MiniSystem::handleThreadFinished);

    start();
}

MiniSystem::~MiniSystem() {
    stop();
}

void MiniSystem::start() {
    m_thread->start();
}

void MiniSystem::stop() {
    if (m_thread->isRunning()) {
        m_thread->quit();
    }
}

MiniSystemNotifier *MiniSystem::createNotifier(const QString &path, MiniSystem::Type type) {
    QFileInfo info(path);
    if (type == File) {
        QString dir = info.absolutePath();
        if (!info.isFile()) {
            return nullptr;
        }

        // Search
        auto it = filesMap.find(dir);
        long id = 0;
        if (it != filesMap.end()) {
            QPair<long, long> &pair = it.value();
            pair.first++;
            id = pair.second;
        } else {
            id = m_fileWatcher.addPath(dir, false);
            qDebug() << "[Mini System] Add File Watch" << id;
            if (id <= 0) {
                exitOnWatchFailed();
            }
            filesMap.insert(dir, qMakePair(1, id));
        }

        // Plus
        MiniSystemNotifier *n = new MiniSystemNotifier(path, id, type, this);
        n->moveToThread(m_thread);
        fileNotifiers.insert(n);
        return n;
    } else {
        QString dir = path;
        if (!info.isDir()) {
            return nullptr;
        }

        // Search
        auto it = dirsMap.find(dir);
        long id = 0;
        if (it != dirsMap.end()) {
            QPair<long, long> &pair = it.value();
            pair.first++;
            id = pair.second;
        } else {
            id = m_dirWatcher.addPath(dir, true);
            qDebug() << "[Mini System] Add Dir Watch" << id;
            if (id <= 0) {
                exitOnWatchFailed();
            }
            dirsMap.insert(dir, qMakePair(1, id));
        }

        // Plus
        MiniSystemNotifier *n = new MiniSystemNotifier(path, id, type, this);
        n->moveToThread(m_thread);
        dirNotifiers.insert(n);
        return n;
    }
}

void MiniSystem::removeNotifier(MiniSystemNotifier *notifier) {
    if (!notifier) {
        return;
    }

    QFileInfo info(notifier->path());
    if (notifier->type() == File) {
        QString dir = info.absolutePath();

        // Search
        auto it = filesMap.find(dir);
        if (it == filesMap.end()) {
            return;
        }

        // Subtract
        auto &pair = it.value();
        pair.first--;
        if (pair.first == 0) {
            m_fileWatcher.removePath(dir);
            qDebug() << "[Mini System] Remove File Watch" << pair.second;
            filesMap.erase(it);
        }

        auto it2 = fileNotifiers.find(notifier);
        if (it2 != fileNotifiers.end()) {
            fileNotifiers.erase(it2);
        }

        delete notifier;
    } else {
        QString dir = info.absoluteFilePath();

        // Search
        auto it = dirsMap.find(dir);
        if (it == dirsMap.end()) {
            return;
        }

        // Subtract
        auto &pair = it.value();
        pair.first--;
        if (pair.first == 0) {
            m_dirWatcher.removePath(dir);
            qDebug() << "[Mini System] Remove Dir Watch" << pair.second;
            dirsMap.erase(it);
        }

        auto it2 = dirNotifiers.find(notifier);
        if (it2 != dirNotifiers.end()) {
            dirNotifiers.erase(it2);
        }

        delete notifier;
    }
}

void MiniSystem::removeAllNotifiers() {
    for (auto it = fileNotifiers.begin(); it != fileNotifiers.end(); ++it) {
        delete *it;
    }
    m_fileWatcher.removeAllPaths();
    fileNotifiers.clear();
    filesMap.clear();
    for (auto it = dirNotifiers.begin(); it != dirNotifiers.end(); ++it) {
        delete *it;
    }
    m_dirWatcher.removeAllPaths();
    dirNotifiers.clear();
    dirsMap.clear();
}

void MiniSystem::handleThreadFinished() {
    removeAllNotifiers();
}

void MiniSystem::exitOnWatchFailed() {
    QMessageBox::critical(nullptr, ErrorTitle, tr("No write file permission!"));
    exit(1);
}
