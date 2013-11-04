#include "stdafx.h"
#include "AutoPacketFactory.h"
#include "AutoPacket.h"

void AutoPacketFactory::AutoPacketResetter::operator()(AutoPacket& packet) const {
  // Eliminate references to any shared pointers
  packet.Release();
}

AutoPacketFactory::AutoPacketFactory():
  m_numSats(0)
{
}

AutoPacketFactory::~AutoPacketFactory()
{
}

std::shared_ptr<AutoPacket> AutoPacketFactory::NewPacket(void) {
  // Obtain a packet:
  std::shared_ptr<AutoPacket> retVal;
  m_packets(retVal);

  // Fill the packet with satisfaction information:
  retVal->Reset();

  // Done, return
  return retVal;
}