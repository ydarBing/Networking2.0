/////////////////////////////////////////////////
/*
 * Definition of Address class
 *
 * Author: Harrison Beachey
 * 
 *  Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
*/
/////////////////////////////////////////////////

#pragma once
#include <ostream>
class Address
{
  public:
    Address();
    Address(u08 a, u08 b, u08 c, u08 d, u16 port);
    Address(u32 netAddr, u16 port);

    bool operator==(const Address& rhs);
    friend std::ostream& operator<<(std::ostream& str, const Address& addr);

    void SetAddress(u08 a, u08 b, u08 c, u08 d, u16 port);
    void SetAddress(u32 addr, u16 port);
    void SetAddrPort(u16 port);

    u16 GetPort() const;
    u32 GetAddress() const;
    istring GetStringAddress() const;
    const sockaddr_in& GetSendToAddress() const;

  private:
    u32 addr_;
    u16 port_;
    sockaddr_in sockAddr_;
};
