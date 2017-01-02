#ifndef message_h__
#define message_h__

#include <string>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include "robintr.h"
#include <windows.h>
#include "serial/serial.h"

//extern std::ofstream fserin;
//extern std::ofstream fserout;

using std::string;

// Message sent to / from the robot, according to the protocol
struct Message
{
#pragma pack(push, 1)
  struct Fields 
  {
    ushort opcode;	// message code
    short  data1; // First param
    short  data2; // Second param
    uchar  counter;
    uchar  csum;   // Checksum
  };
  struct AckFields
  {
    uchar x4E;
    ushort opcode;
    uchar  err_code;
    ushort padding;
    uchar  counter;
    uchar  csum;
  };
  union
  {
    Fields    f;
    AckFields af;
    uchar     bytes[8];
  };
#pragma pack(pop)

  Message(ushort c=0xFE00, short d1=0, short d2=0, uchar cnt=0)
  {
    f.opcode=c;
    f.data1=d1;
    f.data2=d2;
    f.counter=cnt;
    f.csum=std::accumulate(bytes,bytes+7,0);
  }

  string print()
  {
    std::ostringstream os;
    for(int i=0;i<8;++i)
      os << std::hex << std::setfill('0') << std::setw(2) << int(bytes[i]) << ' ';
    return os.str();
  }

//   uchar get_code()   const { return code; }
//   uchar get_value1() const { return value1; }
//   uchar get_value2() const { return value2; }
//   uchar get_csum()   const { return csum; }
  bool  valid()      const 
  { 
    return f.csum == (std::accumulate(bytes,bytes+7,0) & 0xFF);
  }

  // Non blocking send with timeout
  bool send(CSerial& s) 
  {
    uchar buffer[8];
    buffer[0]=bytes[1];
    buffer[1]=bytes[0];
    buffer[2]=bytes[3];
    buffer[3]=bytes[2];
    buffer[4]=bytes[5];
    buffer[5]=bytes[4];
    buffer[6]=bytes[6];
    buffer[7]=bytes[7];
    uchar* buf=buffer;
    //std::cout << "Send: " << print() << std::endl;
    DWORD act=0,cur=0;
    DWORD start=GetTickCount();
    while (act<8)
    {
      if (s.Write(buf,8-act,&cur,0,1000)==ERROR_SUCCESS)
      {
        if (cur>0)
        {
          buf+=cur;
          act+=cur;
        }
        else Sleep(1);
      }
      else Sleep(1);
      if ((GetTickCount()-start)>100)
      {
        std::cout << "Timeout in Send\n";
        return false;
      }
    }
    return true;
  }

  bool receive(CSerial& s)
  {
    DWORD start=GetTickCount();
    DWORD act=0;
    if (s.Read(bytes,8,&act,0,1000)==ERROR_SUCCESS)
    {
      DWORD stop=GetTickCount();
      //std::cout << "Recv: " << print() << "  after " << (stop-start) << "ms\n";
      if (bytes[0]==0x4E)
      {
        std::swap(bytes[1],bytes[2]);
      }
      else
      {
        std::swap(bytes[0],bytes[1]);
        std::swap(bytes[2],bytes[3]);
        std::swap(bytes[4],bytes[5]);
      }
      if (!valid()) throw "Invalid message received: "+print();
      return true;
    }
    else
    {
      DWORD stop=GetTickCount();
      std::cout << "Receive timeout after: " << (stop-start) << "ms\n";
    }
    return false;
  }

  bool check_ack(const Message& msg)
  {
    if (af.x4E != 0x4E) return false;
    if (af.opcode != msg.f.opcode) return false;
    if (af.err_code != 0) return false;
    if (af.counter != msg.f.counter) return false;
    return true;
  }

  bool check_done(const Message& msg)
  {
    if (af.x4E != 0x4E) return false;
    if (af.opcode != msg.f.opcode) return false;
    if (af.err_code != 0xFF) return false;
    if (af.counter != msg.f.counter) return false;
    return true;
  }
};
  /*
  // Non blocking receive with timeout and checksum check
  bool receive(CSerial& s, uchar expected_csum)
  {
    const int TIMEOUT=20;
    DWORD start=GetTickCount();
    uchar rcsum=~expected_csum;
    bool rc=true;
    //while (rcsum!=expected_csum)
    {
      DWORD cur=0,act=0;
      uchar* buffer=&code;
      while (act==0)
      {
        if (s.Read(&code,8-act,&cur,0,2000)==ERROR_SUCCESS)
        {
          if (cur>0)
          {
            buffer+=cur;
            act+=cur;
          }
          else Sleep(1);
          if ((GetTickCount()-start)>TIMEOUT) 
          {
            std::cout << ".";
            //rc=false;
            break;
          }
        }
        else
        {
          std::cout << "Read failed\n";
          rc=false;
          break;
        }
      }
      if (!valid())
      {
        std::cout << "^";
        value2=~expected_csum;
      }
      else if (value2!=expected_csum && act==4) std::cout << "*";
      rcsum=value2;
      //if ((GetTickCount()-start)>TIMEOUT) break;
    }
    if (rc) fserin.write((const char*)&code,4);
    else fserin.write("ZZZZ",4);
    fserin.flush();
    return rc;
  }
};

typedef std::vector<Message> msg_vec;

// The following functions generate message instances for each of the main command types

inline Message left_drive_message(int speed)    
{ 
  return Message('L',1,speed); 
}

inline Message right_drive_message(int speed)
{ 
  return Message('L',1,speed); 
}

inline Message turn_message(int dir, int speed) 
{ 
  return Message('L',dir,speed); 
}

inline Message turn_left_message(int speed)
{ 
  return turn_message(0,speed); 
}

inline Message turn_right_message(int speed)
{ 
  return turn_message(1,speed); 
}

inline Message query_encoder_message(int encoder)
{ 
  return Message('L',1,encoder); 
}

*/





#endif // message_h__
