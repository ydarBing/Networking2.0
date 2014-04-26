#pragma once
/************************************************************************/
/*
    File created by Nathan Hitchcock (nathan.hitchcock@digipen.edu)
    CS261 - Project 1
    1.29.2014
    All content copyright DigiPen Institute of Technology

    This file declares the message class for the end user
*/
/************************************************************************/
#include "CommonDefines.h"
namespace Networking
{
  //Note: Message is the engine-side-message, basically what we refer to as DATA
  //Message is essentially a ref-counted dynamically allocated buffer
  //As well as to make sure it gets cleaned up
  #define MESSAGE_HEADER_SIZE (sizeof(u16) + 1) //bytes for flags + data size

  namespace MessageError
  {
    enum MESSEGE_ERROR
    {
      OK,
      NEGATIVE_REF_COUNT,
      NETWORK_ERROR,
      EMPTY_MESSAGE,
      MESSAGE_ERROR_COUNT
    };
  };

  namespace MessagePriority
  {
    enum MESSAGE_PRIORITY
    {
      LOW,
      HIGH,
      URGENT,
      MESSAGE_PRIORITY_COUNT
    };
  };

  class Message
  {
  private:
    struct Data
    {
      Data(MessageError::MESSEGE_ERROR err, MessagePriority::MESSAGE_PRIORITY priority, u16 size, c08* buffer);
      mutable s16 refCount_;
      mutable MessageError::MESSEGE_ERROR error_; 
      MessagePriority::MESSAGE_PRIORITY priority_;
      u16 size_;
      c08* buffer_;
    };

    Data* data_;
    
    u08 flags_; //only used on sending side
    u32 sequenceNum_; //only used on recieving side
    u32 createTime_;

    bool mustSend_;

    void AttachToData(Data* d);
    void DetachFromData(void);
  
  public:
    enum MessageFlags
    {
      NORMAL = 0,
      IS_CONTINUED,
      END_CONTINUE
    };
    
    Message();
    Message(MessageError::MESSEGE_ERROR meErr);
    Message(std::string str);
    Message(const c08* buffer, u16 size, bool mustSend = false);
    Message(c08* nullTerminatedBuffer);
    Message(const Message& rhs);
    Message& operator=(const Message& rhs);
    ~Message();

    void SetPriority(MessagePriority::MESSAGE_PRIORITY priority);
    void SetFlags(MessageFlags flags);
    void SetSequence(u32 num);
    MessagePriority::MESSAGE_PRIORITY GetPriority();
    u16 GetSize(void);
    c08* GetBuffer(void);

    bool MustSend();
    void MustSend(bool send);

    u16 GetSendSize(void);
    u08* GetSendBuffer(void); //returns owning pointer to data
    u08 GetFlags(void);
    u32 GetSequence(void);
    MessageError::MESSEGE_ERROR GetError(void);

    u32 GetCreateTime();

    //used for sorting
    bool operator<(const Message& rhs) const;
    bool operator>(const Message& rhs) const;
  };
};
