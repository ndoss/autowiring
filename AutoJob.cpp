#include "stdafx.h"
#include "AutoJob.h"
#include "CoreContext.h"
#include <future>


AutoJob::AutoJob(const char* name) :
  ContextMember(name),
  m_canAccept(false)
{
  AcceptDispatchDelivery();
}

void AutoJob::FireEvent(DispatchThunkBase& thunk){
  std::future<void> a = std::async(std::launch::async, [&thunk]{thunk();});
}

bool AutoJob::ShouldStop(void) const {
  shared_ptr<CoreContext> context = ContextMember::GetContext();
  return !context || context->IsShutdown();
}

void AutoJob::AcceptDispatchDelivery(void) {
  boost::unique_lock<boost::mutex> lk(m_lock);
  m_canAccept = true;
  m_stateCondition.notify_all();
}

void AutoJob::RejectDispatchDelivery(void) {
  boost::unique_lock<boost::mutex> lk(m_lock);
  m_canAccept = false;
  m_stateCondition.notify_all();
}

bool AutoJob::DelayUntilCanAccept(void) {
  boost::unique_lock<boost::mutex> lk(m_lock);
  m_stateCondition.wait(lk, [this] {return ShouldStop() || CanAccept(); });
  return !ShouldStop();
}

bool AutoJob::CanAccept(void) const {
  return m_canAccept;
}