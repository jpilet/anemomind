#ifndef __MAIN_H_
#define __MAIN_H_

#include <nan.h>

#include <iostream>
#include <time.h>
#include <unistd.h>
#include <deque>

#include <NMEA2000.h> 
#include <N2kMsg.h>

class NodeNmea2000 : public Nan::ObjectWrap, public tNMEA2000 {
 public:
  virtual ~NodeNmea2000() { }

  static void Init(v8::Local<v8::Object> exports);

  static NAN_METHOD(New);
  static NAN_METHOD(pushCanFrame);
  static NAN_METHOD(setSendCanFrame);
  static NAN_METHOD(open);
  static NAN_METHOD(parseMessages);
  static NAN_METHOD(getDeviceConfig);
  static NAN_METHOD(setDeviceConfig);
  static NAN_METHOD(onDeviceConfigChange);

  bool sendFrame(unsigned long id, unsigned char len,
                 const unsigned char *buf, bool wait_sent);

  struct Packet {
    unsigned long id;
    unsigned char data[8];
    unsigned char length;

    Packet(unsigned long id, const char* data, unsigned char length);
  };

 protected:
  // Virtual functions for different interfaces.
  bool CANSendFrame(unsigned long id, unsigned char len,
                    const unsigned char *buf, bool wait_sent) override;
  bool CANOpen() override;
  bool CANGetFrame(unsigned long &id,
                           unsigned char &len, unsigned char *buf) override;
 private:

  static Nan::Persistent<v8::Function> constructor;

  Nan::Persistent<v8::Function> sendPacketCb_;
  Nan::Persistent<v8::Object> sendPacketHandle_;

  Nan::Persistent<v8::Function> deviceConfigCb_;
  Nan::Persistent<v8::Object> deviceConfigHandle_;

  std::deque<Packet> packets_;
};

#endif  // __MAIN_H_
