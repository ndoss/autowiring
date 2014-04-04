// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "AutowirableSlot.h"
#include "Autowired.h"
#include "autowiring_error.h"
#include "AutoNetworkMonitor.h"
#include "CoreContext.h"
#include <stdexcept>

using namespace std;

AutowirableSlot::AutowirableSlot(std::weak_ptr<CoreContext> context) :
  m_context(context),
  m_tracker(this, NullOp<AutowirableSlot*>)
{
  if (ENABLE_NETWORK_MONITOR) {
    // Obtain the network monitor:
    std::shared_ptr<AutoNetworkMonitor> pMon;
    LockContext() -> FindByType(pMon);
    if (pMon)
      // Pass notification:
      pMon->Notify(*this);
  }
}

void AutowirableSlot::NotifyWhenAutowired(const std::function<void()>& listener) {
  std::shared_ptr<CoreContext> context = LockContext();

  // Punt control over to the context
  context->NotifyWhenAutowired(*this, listener);
}

std::shared_ptr<CoreContext> AutowirableSlot::LockContext(void) {
  std::shared_ptr<CoreContext> retVal = m_context.lock();
  if(!retVal)
    throw_rethrowable autowiring_error("Attempted to autowire in a context that is tearing down");
  return retVal;
}
