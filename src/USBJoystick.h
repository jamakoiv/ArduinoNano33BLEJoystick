/*
 * Copyright (c) 2022, Jaakko Koivisto. 
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef USBJOYSTICK_H
#define USBJOYSTICK_H

#include "PluggableUSBHID.h"
#include "PlatformMutex.h"

namespace arduino {

enum {
  MSB,
  LSB
} ;

class USBJoystick: public USBHID { 
private:
  // TODO: Change BUTTONS_MAX_NUMBER to be set by user.
  static const uint8_t BUTTON_ARRAY_MAX_SIZE = 32;  // Absolute maximum number of buttons 32*8 = 256.
  static const uint8_t BUTTONS_MAX_NUMBER = 64;      // Actual max amount of buttons we are using.
  static const uint8_t BYTE_LENGTH = 8;             // How many bits is in a single byte of data. 


  // Axis data will be constrained to this range when sending it over the USB. Default 10-bit res.
  // TODO: 16-bit resolution works on windows but not on linux. Maybe something wrong with the HID-report?
  //static const int16_t HID_AXIS_MIN = -511;   // 10-bit
  //static const int16_t HID_AXIS_MAX = 511;
  //static const int16_t HID_AXIS_MIN = -1023;  // 11-bit
  //static const int16_t HID_AXIS_MAX = 1023;
  static const int16_t HID_AXIS_MIN = -2047;    // 12-bit
  static const int16_t HID_AXIS_MAX = 2047;


  // TODO:  Report IDs have to be declared static or otherwise the report-descriptor becomes corrupted and
  //        the whole USB-device goes down.
  static const uint8_t REPORT_ID = 0x10;

  // Buttons amount must be byte-aligned because I don't want to deal with padding-data in the HID-report :)
  MBED_STATIC_ASSERT( BUTTONS_MAX_NUMBER % BYTE_LENGTH == 0, 
                      "BUTTONS_MAX_NUMBER does not align to byte-length" ); 


  uint8_t _configuration_descriptor[34]; // 34 bytes. // TODO: Hardcoded magic number 
  HID_REPORT HIDreport;
  PlatformMutex _mutex;
  

public:

  bool sendBlocking = true;
  bool autoSend = false;

  uint8_t buttonState[ BUTTON_ARRAY_MAX_SIZE ];   

  struct _axis_ {
    int16_t X;
    int16_t Y;
    int16_t Z;
    int16_t Rx;
    int16_t Ry;
    int16_t Rz;
    int16_t throttle;
    int16_t rudder;
  } ;

  // TODO: Hardcoded magic default numbers.
  _axis_ axis = {0, 0, 0, 0, 0, 0, 0, 0};
  _axis_ axisMin = {-511, -511, -511, -511, -511, -511, -511, -511 };
  _axis_ axisMax = {511, 511, 511, 511, 511, 511, 511, 511 };


  /*
    Constuctors and destructors.
  */
  USBJoystick(bool connect_blocking=true, uint16_t vendor_id=0x1235, 
              uint16_t product_id=0x0050, uint16_t product_release=0x0001);



  USBJoystick(USBPhy *phy, uint16_t vendor_id=0x1235, 
              uint16_t product_id=0x0050, uint16_t product_release=0x0001);

  virtual ~USBJoystick(void);



  /*
   Construct the HID-report descriptor.

   @returns pointer to the report descriptor.
  */
  // TODO: Who calls this function and when??? 
  // Probably when the class is constructed and the USB-system goes through with the regular init-hoops.
  // Check USBHID and underlying classes, maybe we find something...
  virtual const uint8_t *report_desc(void);


  // TODO: Do we need these? 'report_rx' is used only when Host sends HID-report to the Device.
  // Do we need to use 'report_tx' or simply use 'send' periodically???
  //virtual void report_rx(void); // Called when there is a hid report that can be read.
  //virtual void report_tx(void); // Called when there is space to send a hid report (space in the USB-bus??)

  
  /*
    Send a USB HID-report to the host. 'send' for blocking and 'send_nb' for non-blocking.
    These are inherited from the USBHID class.
  */
  // bool send(HID_REPORT report);
  // bool send_nb(HID_REPORT report);

  /*
    Read a USB HID-report from the host. 'read' for blocking and 'read_nb' for non-blocking.
    These are inherited from the USBHID class.
  */
  // bool read(HID_REPORT report);
  // bool read_nb(HID_REPORT report);
 

  /*
    Send joystick state update to the host. 

    @returns true if there was no error, false otherwise. Return values passed through from 'USBHID::send'. 
  */
  bool update(void);

  /*
    Wrapper. For API-compliance with the MHeironimus-ArduinoJoystickLibrary.
  */
  void sendState(void) { this->update(); }

  /*
    These don't really do much currently. For API-compliance with the MHeironimus-ArduinoJoystickLibrary.
  */
  void begin(bool autoSendState) { this->autoSend = autoSendState; }
  void end(void) { }

  /*
    Update the HID-report with the current joystick-state (axis-values, button-states etc.).
  */
  void updateHIDreport(void);

  /*
    Get the lower (LSB) or higher (MSB) 8-bits of 16-bit axis-value.
    You have to call this twice with MSB and LSB to get the full axis value. 
    Used to store the axis value in the HID-report which is made of 8-bit fields.

    @returns 8-bit integer containing the least-significant or most significant byte. 
  */
  int8_t axis16bitToByte(int16_t axisValue, bool MSB_OR_LSB);


  /*
    Template implementation of Arduino map-function for arbitary input and output data-types.
  */
  //template<typename fromType, typename toType>
  //static toType map(fromType x, fromType in_min, fromType in_max, toType out_min, toType out_max)
  //{
  //  return (x -in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  //}


  /*
    Maps value in the range [in_min:in_max] to [out_min:out_max].
    Local implementation of Arduino map-function for floating-point data input. 
    Takes input as 'float' and output as 'int16_t'.
    
    @returns mapped value as 16-bit integer.
  */
  static inline int16_t mapfi(float x, float in_min, float in_max, int16_t out_min, int16_t out_max)
  {
    return (x-in_min) * (out_max-out_min) / (in_max-in_min) + out_min;
  }


  /*
    Set axis value to the given value.
  */
  void setXAxis(float value);
  void setYAxis(float value);
  void setZAxis(float value);
  void setRxAxis(float value);
  void setRyAxis(float value);
  void setRzAxis(float value);
  void setThrottleAxis(float value);
  void setRudderAxis(float value);


  /*
    Set the allowed minimum and maximum values. 
  */
  void setXAxisRange(int16_t min, int16_t max);
  void setYAxisRange(int16_t min, int16_t max);
  void setZAxisRange(int16_t min, int16_t max);
  void setRxAxisRange(int16_t min, int16_t max);
  void setRyAxisRange(int16_t min, int16_t max);
  void setRzAxisRange(int16_t min, int16_t max);
  void setThrottleAxisRange(int16_t min, int16_t max);
  void setRudderAxisRange(int16_t min, int16_t max);
  void setAllAxisRange(int16_t min, int16_t max);


  /*
    Press button. Sets button 'buttonNumber' state to 1.
    Release button. Sets button 'buttonNumber' state to 0.
    Toggle button. Invert the current state of the button 'buttonNumber'.
    Set button 'buttonNumber' state to 0 if value is 0, 1 if value is non-zero.
  */
  void pressButton(uint8_t buttonNumber);
  void releaseButton(uint8_t buttonNumber);
  void toggleButton(uint8_t buttonNumber);
  void setButton(uint8_t buttonNumber, uint8_t value);


protected:
  /*
   Get configuration descriptor.

   @returns pointer to the configuration descriptor.
  */
  // TODO: Who calls this function? Something deep inside the USB-stack???
  virtual const uint8_t *configuration_desc(uint8_t index);


} ; // End of 'class USBJoystick'.


} // End of 'namespace arduino'.


#endif // USBJOYSTICK_H
