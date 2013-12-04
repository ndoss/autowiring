#include "stdafx.h"
#include "SimpleOwnershipValidator.h"
#include <assert.h>
#include <iostream>

using namespace std;

SimpleOwnershipValidator::~SimpleOwnershipValidator(void)
{
  // Find violating entries, gather them, and destroy our array:
  vector<const type_info*> violating;
  for(size_t i = 0; i < m_entries.size(); i++) {
    auto cur = m_entries[i];
    if(!cur->Validate())
      violating.push_back(&cur->GetType());
    delete cur;
  }

  if(violating.empty())
    return;

  // If no listeners, we can only assert.  If you're getting a breakpoint here, check the contents
  // of the "violating" vector for an indication of the types that are dangling and/or members of
  // a cycle.  Generally just setting a breakpoint on this line will allow any interested parties
  // an opportunity to examine the components of a cycle.
  assert(!listeners.empty());

  // And now notify all listeners:
  for(size_t i = 0; i < listeners.size(); i++)
    listeners[i](violating);
}

void SimpleOwnershipValidator::PrintToStdErr(const std::vector<const type_info*>& violating) {
  cerr
    << "A context is being destroyed, but some of its members are not being destroyed" << endl
    << "at the same time.  The user has indicated that this context was supposed to" << endl
    << "have simple ownership over its members.  The following types are participants" << endl
    << "in what may be a cycle:" << endl
    << endl;

  for(size_t i = 0; i < violating.size(); i++)
    cerr << violating[i]->name() << endl;
}

void SimpleOwnershipValidator::AddValidationListener(const std::function < void(const std::vector<const type_info*>&)>& fn)
{
  listeners.push_back(fn);
}