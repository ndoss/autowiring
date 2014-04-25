#include "stdafx.h"
#include "AutoPacketFactory.h"
#include "AutoPacket.h"
#include "AutoPacketListener.h"

AutoPacketFactory::AutoPacketResetter::AutoPacketResetter(void) {}

AutoPacketFactory::AutoPacketResetter::AutoPacketResetter(AutoPacketResetter&& rhs) :
  m_apl(std::move(rhs.m_apl))
{}

AutoPacketFactory::AutoPacketResetter::AutoPacketResetter(AutoFired<AutoPacketListener>&& apl) :
  m_apl(std::move(apl))
{}

AutoPacket* AutoPacketFactory::AutoPacketCreator::operator()() const {
  return new AutoPacket(*factory);
}

void AutoPacketFactory::AutoPacketResetter::operator()(AutoPacket& packet) const {
  // Notify all listeners that a packet has just returned home:
  m_apl(&AutoPacketListener::OnPacketReturned)(packet);

  // Eliminate references to any shared pointers
  packet.Release();
}

AutoPacketFactory::AutoPacketFactory(void) {}

AutoPacketFactory::AutoPacketFactory(std::shared_ptr<JunctionBox<AutoPacketListener>>&& apl) :
  m_packets(~0, ~0, AutoPacketResetter(AutoFired<AutoPacketListener>(apl)))
{
  m_packets.SetAlloc(AutoPacketCreator(this));
}

AutoPacketFactory::~AutoPacketFactory()
{}

std::shared_ptr<AutoPacket> AutoPacketFactory::NewPacket(void) {
  // Obtain a packet:
  std::shared_ptr<AutoPacket> retVal;
  m_packets(retVal);

  // Fill the packet with satisfaction information:
  retVal->Reset();

  // Done, return
  return retVal;
}

void AutoPacketFactory::AddSubscriber(const AutoPacketSubscriber& rhs) {
  const std::type_info& ti = *rhs.GetSubscriberTypeInfo();

  // Determine whether this subscriber already exists--perhaps, it is formerly disabled
  auto q = m_subMap.find(ti);
  size_t subscriberIndex;
  if(q != m_subMap.end()) {
    AutoPacketSubscriber& entry = m_subscribers[q->second];
    subscriberIndex = q->second;

    if(!entry.empty())
      // Already registered, no need to register a second time
      return;

    // Registered but previously removed, re-initialize
    entry = std::move(rhs);
  }
  else {
    // Register the subscriber and pre-populate the arity:
    subscriberIndex = m_subscribers.size();
    m_subMap[ti] = subscriberIndex;
    m_subscribers.push_back(std::move(rhs));
  }

  // Prime the satisfaction graph for each element:
  for(
    auto pCur = rhs.GetSubscriberInput();
    *pCur;
    pCur++
  ) {
    // Decide what to do with this entry:
    switch(pCur->subscriberType) {
    case inTypeInvalid:
      // Should never happen--trivially ignore this entry
      break;
    case inTypeRequired:
    case inTypeOptional:
      {
        // Obtain the decorator type at this position:
        auto r = m_decorations.find(*pCur->ti);
        if(r == m_decorations.end())
          // Decorator formerly not encountered, introduce it:
          r = m_decorations.insert(
            t_decMap::value_type(
              *pCur->ti,
              AdjacencyEntry(*pCur->ti)
            )
          ).first;

        // Now we need to update the adjacency entry with the new subscriber:
        r->second.subscribers.push_back(
          std::make_pair(
            subscriberIndex,
            pCur->subscriberType == inTypeOptional
          )
        );
      }
      break;
    case outTypeRef:
    case outTypeRefAutoReady:
      // We don't do anything with these types.
      // Optionally, we might want to register them as outputs, or do some kind
      // of runtime detection that a multi-satisfaction case exists--but for now,
      // we just trivially ignore them.
      break;
    }
  }
}

void AutoPacketFactory::RemoveSubscriber(const std::type_info &idx) {
  auto q = m_subMap.find(idx);
    
  if(q != m_subMap.end()) {
    // Clear out the matched subscriber:
    auto& subscriber = m_subscribers[q->second];

    //Also remove subscriber from the decoration list
    for( auto pSInfo = subscriber.GetSubscriberInput(); *pSInfo; pSInfo++ ) {
      auto decoration = m_decorations.find(*pSInfo->ti);
      if( decoration != m_decorations.end() ) {
        auto& decSubs = decoration->second.subscribers;

        decSubs.erase( std::remove_if(decSubs.begin(), decSubs.end(),
          [&q](std::pair<size_t,bool> a) { return a.first == q->second; }),
          decSubs.end()
        );
        
        //Remove decoration if there are no longer any subscribers
        if( decSubs.empty() )
          m_decorations.erase(decoration);
      }
    }
    m_subscribers[q->second].ReleaseSubscriber();
  }
}

bool AutoPacketFactory::HasSubscribers(const std::type_info& ti) const {
  auto decorator = FindDecorator(ti);
  return !!decorator;
}