// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef _CONTEXT_MEMBER_H
#define _CONTEXT_MEMBER_H
#include "Object.h"
#include "TeardownNotifier.h"
#include <assert.h>
#include MEMORY_HEADER

class CoreContext;

/// <summary>
/// A class that must be inherited in order to be a member of a context heriarchy
/// </summary>
class ContextMember:
  public Object,
  public TeardownNotifier
{
protected:
  ContextMember(const char* name = nullptr);
  std::weak_ptr<ContextMember> m_self;
  const char* m_name;

public:
  virtual ~ContextMember();

  friend class CoreContext;

protected:
  // Member variables:
  std::weak_ptr<CoreContext> m_context;

public:
  // Accessor methods:
  const char* GetName(void) const {return m_name;}
  bool IsOrphaned(void) const {return m_context.expired();}

  /// <summary>
  /// Assigns the context for this context member
  /// </summary>
  /// <remarks>
  /// This method may be used to assign the member's enclosing context to exactly one value.  It
  /// is an error to attempt to use this method to change the context member's context once it has
  /// been assigned.
  ///
  /// An exception to this rule is that a context member's context may be updated if the context
  /// member has been orphaned.
  ///
  /// This method is idempotent.
  /// </remarks>
  void SetContext(std::shared_ptr<CoreContext>& context) {
    assert(m_context.lock() == context || m_context.expired());
    m_context = context;
  }

  /// <summary>
  /// This method is invoked after all embedded Autowired members of this class are initialized
  /// </summary>
  /// <remarks>
  /// Not currently implemented
  /// </remarks>
  virtual void PostConstruct(void) {}

  /// <summary>
  /// Invoked by the parent context when the parent context is about to be destroyed
  /// </summary>
  /// <remarks>
  /// A context may be destroyed if and only if none of its members are running and none of
  /// them may enter a runnable state.  This happens when the last pointer to ContextMember
  /// is lost.  Resource cleanup must be started at this point.
  ///
  /// For contexts containing strictly heirarchial objects, implementors of this method do
  /// not need to do anything.  If, however, there are circular references anywhere in the
  /// context, callers should invoke reset() on each autowired member they control.
  ///
  /// TODO:  Eventually, we MUST eliminate this method and simply keep track of all Autowired
  /// instances in a single context.  This can be done safely by using the Autowired's internal
  /// m_tracker member, which can serve to notify listeners when the instance is destroyed.
  /// Alternatively, the Autowired instance could attach and detach itself from a linked list
  /// in a lock-free way in order to support chain detachment.
  /// </remarks>
  virtual void NotifyContextTeardown(void) {}

  /// <summary>
  /// Retrieves the context associated with this object.
  /// </summary>
  /// <remarks>
  /// By default, the context will be whatever the current context was in the thread
  /// where this object was constructed at the time of the object's construction.
  ///
  /// Note that, if the context is in the process of tearing down, this return value
  /// could be null.
  /// </remarks>
  std::shared_ptr<CoreContext> GetContext(void) const {return m_context.lock();}

  /// <summary>
  /// Returns a shared pointer that refers to ourselves
  /// </summary>
  template<class T>
  std::shared_ptr<T> GetSelf(void) {
    return std::static_pointer_cast<T, ContextMember>(m_self.lock());
  }
};

#endif
