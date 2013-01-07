#include <stdexcept>
#include <fstream>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>

#include "CMUXManager.h"
#include "CMUXInstance.h"
#include "EventLoop.h"

CMUXManager::CMUXManager(EventLoop *loop) : m_loop(loop) {
  m_instance = this;
}

CMUXManager::~CMUXManager() {
  m_instance = 0;
}

CMUXInstance *CMUXManager::createMUX(ControlRequestHandler *owner) {
  return new CMUXInstance(this, owner);
}

void CMUXManager::installWatcher(DescriptorWatcher *watcher) {
  m_loop->addWatcher(watcher);
}

void CMUXManager::uninstallWatcher(DescriptorWatcher *watcher) {
  m_loop->removeWatcher(watcher);
}

CMUXManager *CMUXManager::m_instance = 0;
