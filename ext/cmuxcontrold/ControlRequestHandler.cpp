#include <cmux/Package.h>
#include <cmux/Connection.h>

#include <stdexcept>

#include <errno.h>
#include <stdio.h>

#include "IRemoteParty.h"
#include "ControlRequestHandler.h"
#include "CMUXManager.h"
#include "CMUXInstance.h"

using namespace CMUX;

ControlRequestHandler::ControlRequestHandler(IRemoteParty *remote) : m_remote(remote), m_mux(0) {

}

ControlRequestHandler::~ControlRequestHandler() {
  if(m_mux)
    m_mux->shutdown();
}

void ControlRequestHandler::handleMessage(CMUX::Package &package) {
  Package reply;

  int request = package.readByte();
  if(package.hasError()) {
    request = -1;
  }

  try {
    if(m_mux == NULL)
      m_mux = CMUXManager::instance()->createMUX(this);

    switch(request) {
      case RequestOpen: {
        std::string string = package.readString();
        if(package.hasError())
          throw std::runtime_error("malformed request");

        m_mux->open(string);
        reply.writeByte(1);

        break;
      }

      case RequestActivate:
        m_mux->activate();
        reply.writeByte(1);

        break;

      case RequestOpenPort: {
        int port = package.readInt();
        if(package.hasError())
          throw std::runtime_error("malformed request");
        std::string path = m_mux->openPort(port, m_remote->uid());
        reply.writeByte(1);
        reply.writeString(path);

        break;
      }

      case RequestClosePort: {
        int port = package.readInt();
        if(package.hasError())
          throw std::runtime_error("malformed request");
        m_mux->closePort(port);
        reply.writeByte(1);

        break;
      }

      default:
        throw std::runtime_error("unimplemented call");
    }
  } catch(std::exception &e) {
    reply.writeByte(0);
    reply.writeString(e.what());
  }

  m_remote->sendMessage(reply);
}

void ControlRequestHandler::multiplexerTerminated() {
  if(m_mux) {
    m_mux = 0;
    m_remote->close();
  }
}


