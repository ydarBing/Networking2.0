#pragma once
namespace Networking
{
//Adds a default initiliazed protocol listener
template <typename T>
void NetworkingSystem::AddCustomProtocol(u16 port)
{
  LockableResource<CustomProtocolData>* cpd = new LockableResource<CustomProtocolData>(port);
  (*cpd)->protocol = new T();

  conCustomProtocols_.Lock();
  conCustomProtocols_->push_back(cpd);
  conCustomProtocols_.Unlock();
}

//Adds a copy constructed protocol listener
template <typename T>
void NetworkingSystem::AddCustomProtocol(T& toCopy, u16 port)
{
  CustomProtocolData* pcp = new CustomProtocolData(port);
  LockableResource<CustomProtocolData*>* cpd = new LockableResource<CustomProtocolData*>(pcp);
  (**cpd)->protocol = new T(toCopy);

  conCustomProtocols_.Lock();
  conCustomProtocols_->push_back(cpd);
  conCustomProtocols_.Unlock();
}

};//~Networking