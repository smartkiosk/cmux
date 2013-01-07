#ifndef __CONTROL_REQUEST_HANDLER__H__
#define __CONTROL_REQUEST_HANDLER__H__

namespace CMUX {
  class Package;
}

class IRemoteParty;
class CMUXInstance;

class ControlRequestHandler {
public:
  ControlRequestHandler(IRemoteParty *remote);
  ~ControlRequestHandler();

  void handleMessage(CMUX::Package &package);
  void multiplexerTerminated();

private:

  IRemoteParty *m_remote;
  CMUXInstance *m_mux;
};

#endif
