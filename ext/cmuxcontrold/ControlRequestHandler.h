#ifndef __CONTROL_REQUEST_HANDLER__H__
#define __CONTROL_REQUEST_HANDLER__H__

namespace CMUX {
  class Package;
}

class IRemoteParty;

class ControlRequestHandler {
public:
  ControlRequestHandler(IRemoteParty *remote);
  ~ControlRequestHandler();

  void handleMessage(CMUX::Package &package);

private:
  int shutdown(int type);

  IRemoteParty *m_remote;
};

#endif
