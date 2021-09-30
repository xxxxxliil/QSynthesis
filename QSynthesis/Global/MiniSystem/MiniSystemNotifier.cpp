#include "MiniSystemNotifier.h"
#include "mainwindow.h"

MiniSystemNotifier::MiniSystemNotifier(const QString &path, MiniSystem::Type type,
                                       MiniSystem *system, QObject *parent)
    : QTimer(parent), m_path(path), m_id(0), m_system(system), m_type(type) {
    init();
}

MiniSystemNotifier::MiniSystemNotifier(const QString &path, long id, MiniSystem::Type type,
                                       MiniSystem *system, QObject *parent)
    : QTimer(parent), m_path(path), m_id(id), m_system(system), m_type(type) {
    init();
}

MiniSystemNotifier::~MiniSystemNotifier() {
}

void MiniSystemNotifier::init() {
    connect(m_type == MiniSystem::File ? &m_system->m_fileWatcher : &m_system->m_dirWatcher,
            &MiniSystemWatcher::fileChanged, this, &MiniSystemNotifier::handleFileChanged);
    connect(qRoot, &MainWindow::awake, this, &MiniSystemNotifier::onAwake);
    connect(qRoot, &MainWindow::sleep, this, &MiniSystemNotifier::onSleep);
    connect(this, &QTimer::timeout, this, &MiniSystemNotifier::onTimer);
}

MiniSystem *MiniSystemNotifier::system() const {
    return m_system;
}

QString MiniSystemNotifier::path() const {
    return m_path;
}

MiniSystem::Type MiniSystemNotifier::type() const {
    return m_type;
}

long MiniSystemNotifier::id() const {
    return m_id;
}

void MiniSystemNotifier::setId(long id) {
    m_id = id;
}

void MiniSystemNotifier::handleFileChanged(long id, MiniSystemWatcher::Action action,
                                           const QString &filename, const QString &oldFilename) {
    if (id != m_id) {
        return;
    }
    switch (m_type) {
    case MiniSystem::File:
        if (filename == m_path) {
            sendChangedSignalDirectly(action, filename, oldFilename);
        }
        break;
    default:
        if (filename.startsWith(m_path)) {
            sendChangedSignal(action, filename, oldFilename);
        }
        break;
    }
}

void MiniSystemNotifier::sendChangedSignal(MiniSystemWatcher::Action action,
                                           const QString &filename, const QString &oldFilename) {
    switch (action) {
    case MiniSystemWatcher::Created:
        prepareChangedSignal(filename);
        break;
    case MiniSystemWatcher::Removed:
        prepareChangedSignal(filename);
        break;
    case MiniSystemWatcher::Modified:
        prepareChangedSignal(filename);
        break;
    case MiniSystemWatcher::Renamed:
        prepareChangedSignal(oldFilename);
        prepareChangedSignal(filename);
        break;
    default:
        break;
    }
}

void MiniSystemNotifier::sendChangedSignalDirectly(MiniSystemWatcher::Action action,
                                                   const QString &filename,
                                                   const QString &oldFilename) {
    switch (action) {
    case MiniSystemWatcher::Created:
        emit fileChanged({filename});
        break;
    case MiniSystemWatcher::Removed:
        emit fileChanged({filename});
        break;
    case MiniSystemWatcher::Modified:
        emit fileChanged({filename});
        break;
    case MiniSystemWatcher::Renamed:
        emit fileChanged({oldFilename, filename});
        break;
    default:
        break;
    }
}

void MiniSystemNotifier::prepareChangedSignal(const QString &filename) {
    m_pending.insert(filename);
    if (!qRoot->isActiveWindow() && !isActive()) {
        sendPendingSignals();
    }
}

void MiniSystemNotifier::sendPendingSignals() {
    if (!m_pending.isEmpty()) {
        start();
        emit fileChanged(m_pending.values());
        m_pending.clear();
    }
}

void MiniSystemNotifier::onTimer() {
    sendPendingSignals();
}

void MiniSystemNotifier::onAwake() {
    sendPendingSignals();
}

void MiniSystemNotifier::onSleep() {
    if (isActive()) {
        stop();
    }
}
