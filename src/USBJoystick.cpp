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

#include "stdint.h"
#include "USBJoystick.h"
#include "usb_phy_api.h"


USBJoystick::USBJoystick(bool connect, uint16_t vendor_id, uint16_t product_id, uint16_t product_release):
  USBHID(get_usb_phy(), 0, 0, vendor_id, product_id, product_release)
{

}

USBJoystick::USBJoystick(USBPhy *phy, uint16_t vendor_id, uint16_t product_id, uint16_t product_release):
  USBHID(phy, 0, 0, vendor_id, product_id, product_release)
{
  // User or child must call connect or init when using this constructor. 
}

USBJoystick::~USBJoystick(void)
{
}


const uint8_t* USBJoystick::report_desc(void)
{
  // TODO: Make a HID-descriptor which can handle different axis-counts and multiple joysticks.
  static const uint8_t reportDescriptor[] = { 
        USAGE_PAGE(1), 0x01,        // Generic Desktop
        USAGE(1), 0x04,             // Joystick
        COLLECTION(1), 0x01,        // Collection Application

          // Report for buttons.
          COLLECTION(1), 0x00,      // Collection Physical. 
            //REPORT_ID(1), 0x01,         // ReportID = 01.
            REPORT_ID(1), this->REPORT_ID,  
            USAGE_PAGE(1), 0x09,        // Button page.
            USAGE_MINIMUM(1), 0x01,     // Min allowed UsageID 
            //USAGE_MAXIMUM(1), 0x20,     // Max allowed UsageID 
            USAGE_MAXIMUM(1), this->BUTTONS_MAX_NUMBER,     // Max allowed UsageID 
            LOGICAL_MINIMUM(1), 0x00,   // Min value 0 = button not pressed.
            LOGICAL_MAXIMUM(1), 0x01,   // Max value 1 = button pressed.
            //REPORT_COUNT(1), 0x20,      // 32 reports... (32 buttons)
            REPORT_COUNT(1), this->BUTTONS_MAX_NUMBER,      
            REPORT_SIZE(1), 0x01,       // ...of 1 bit of data per report.
            INPUT(1), 0x02,             // Data, Variable, Absolute
          END_COLLECTION(0),

          USAGE_PAGE(1), 0x01,      // Generic Desktop
          USAGE(1), 0x01,           // Pointer
          COLLECTION(1), 0x00,      // Collection Physical
            USAGE_PAGE(1), 0x01,        // Generic Desktop
            USAGE(1), 0x30,             // X
            USAGE(1), 0x31,             // Y
            USAGE(1), 0x32,             // Z
            USAGE(1), 0x33,             // Rx
            USAGE(1), 0x34,             // Ry
            USAGE(1), 0x35,             // Rz
            //LOGICAL_MINIMUM(2), 0x01, 0xfe, // 0xfe01 = -511. 10-bit resolution. 
            //LOGICAL_MAXIMUM(2), 0xff, 0x01, // 0x01ff = 511
            LOGICAL_MINIMUM(2), this->HID_AXIS_MIN, this->HID_AXIS_MIN >> this->BYTE_LENGTH,
            LOGICAL_MAXIMUM(2), this->HID_AXIS_MAX, this->HID_AXIS_MAX >> this->BYTE_LENGTH,
            REPORT_COUNT(1), 0x06,      // 6 reports ...
            REPORT_SIZE(1), 0x10,       // ...of 16 bits of data.
            INPUT(1), 0x02,             // Data, variable, absolute.
          END_COLLECTION(0),        // End collection Physical.

        END_COLLECTION(0)           // End collection Application
    };

  this->reportLength = sizeof(reportDescriptor); // reportLength is inherited from USBHID.
  return reportDescriptor;
}


#define DEFAULT_CONFIGURATION (1)
#define TOTAL_DESCRIPTOR_LENGTH ((1 * CONFIGURATION_DESCRIPTOR_LENGTH) \
                               + (1 * INTERFACE_DESCRIPTOR_LENGTH) \
                               + (1 * HID_DESCRIPTOR_LENGTH) \
                               + (1 * ENDPOINT_DESCRIPTOR_LENGTH))
                               //+ (2 * ENDPOINT_DESCRIPTOR_LENGTH)) 

const uint8_t* USBJoystick::configuration_desc(uint8_t index)
{
  if (index != 0) { return NULL; } // Is 'index' the configuration number??

  // TODO: 
  uint8_t configuration_descriptor_temp[] = {  
    CONFIGURATION_DESCRIPTOR_LENGTH,    // bLength
    CONFIGURATION_DESCRIPTOR,           // bDescriptorType
    LSB(TOTAL_DESCRIPTOR_LENGTH),       // wTotalLength (LSB)
    MSB(TOTAL_DESCRIPTOR_LENGTH),       // wTotalLength (MSB)
    0x01,                               // bNumInterfaces
    DEFAULT_CONFIGURATION,              // bConfigurationValue
    0x00,                               // iConfiguration
    C_RESERVED | C_SELF_POWERED,        // bmAttributes
    C_POWER(0),                         // bMaxPower

    INTERFACE_DESCRIPTOR_LENGTH,        // bLength
    INTERFACE_DESCRIPTOR,               // bDescriptorType
    0x00,                               // bInterfaceNumber
    0x00,                               // bAlternateSetting
    0x01,                               // bNumEndpoints
    HID_CLASS,                          // bInterfaceClass
    HID_SUBCLASS_NONE,                  // bInterfaceSubClass
    HID_PROTOCOL_NONE,                  // bInterfaceProtocol
    0x00,                               // iInterface

    HID_DESCRIPTOR_LENGTH,              // bLength
    HID_DESCRIPTOR,                     // bDescriptorType
    LSB(HID_VERSION_1_11),              // bcdHID (LSB)
    MSB(HID_VERSION_1_11),              // bcdHID (MSB)
    0x00,                               // bCountryCode
    0x01,                               // bNumDescriptors
    REPORT_DESCRIPTOR,                  // bDescriptorType
    (uint8_t)(LSB(report_desc_length())), // wDescriptorLength (LSB)
    (uint8_t)(MSB(report_desc_length())), // wDescriptorLength (MSB)

    ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
    ENDPOINT_DESCRIPTOR,                // bDescriptorType
    _int_in,                            // bEndpointAddress
    E_INTERRUPT,                        // bmAttributes
    LSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (LSB)
    MSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (MSB)
    1,                                  // bInterval (milliseconds)

// OUT-endpoint is not necessary for regular joystick that only sends data to host.
//    ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
//    ENDPOINT_DESCRIPTOR,                // bDescriptorType
//    _int_out,                           // bEndpointAddress
//    E_INTERRUPT,                        // bmAttributes
//    LSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (LSB)
//    MSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (MSB)
//    1,                                  // bInterval (milliseconds)
  };

  MBED_STATIC_ASSERT( sizeof(configuration_descriptor_temp) == sizeof(this->_configuration_descriptor), "Length of 'configuration_descriptor_temp' in 'USBJoystick::configuration_desc' must be identical to the length of array 'USBJoystick::_configuration_descriptor'." );
  memcpy( this->_configuration_descriptor, configuration_descriptor_temp, sizeof(this->_configuration_descriptor) );
  return this->_configuration_descriptor;
}




void USBJoystick::updateHIDreport(void) {

  // Buttons part of the HID-report.
  uint8_t dataBytesAmount = this->BUTTONS_MAX_NUMBER / this->BYTE_LENGTH;

  this->HIDreport.data[0] = this->REPORT_ID;
  this->HIDreport.length = 1;

  for (uint8_t i=0; i < dataBytesAmount; i++) {
    this->HIDreport.data[i+1] = this->buttonState[i];
    this->HIDreport.length++;
  }

  // Axis part of the report.
  
  // NOTE: The axis-values are 16-bit, assigning them to the 
  // 8-bit array-elements chops away higher bits that don't fit the target variable.
  // TODO: Should we write 'static_cast<uint8_t>()' to show this more explicitly?
  // TODO: Or use the axis16bitToByte?

  this->HIDreport.data[this->HIDreport.length++] = this->axis.X;
  this->HIDreport.data[this->HIDreport.length++] = this->axis.X >> this->BYTE_LENGTH;

  this->HIDreport.data[this->HIDreport.length++] = this->axis.Y;
  this->HIDreport.data[this->HIDreport.length++] = this->axis.Y >> this->BYTE_LENGTH;

  this->HIDreport.data[this->HIDreport.length++] = this->axis.Z;
  this->HIDreport.data[this->HIDreport.length++] = this->axis.Z >> this->BYTE_LENGTH;

  this->HIDreport.data[this->HIDreport.length++] = this->axis.Rx;
  this->HIDreport.data[this->HIDreport.length++] = this->axis.Rx >> this->BYTE_LENGTH;

  this->HIDreport.data[this->HIDreport.length++] = this->axis.Ry;
  this->HIDreport.data[this->HIDreport.length++] = this->axis.Ry >> this->BYTE_LENGTH;

  this->HIDreport.data[this->HIDreport.length++] = this->axis.Rz;
  this->HIDreport.data[this->HIDreport.length++] = this->axis.Rz >> this->BYTE_LENGTH;

}



bool USBJoystick::update(void)
{
  this->updateHIDreport();

  this->_mutex.lock();  // The underlying USB-system and -hardware is most probably shared
                        // by all threads, so we need to acquire lock before using it.

  bool sendSuccessful;
  if (this->sendBlocking) {
    sendSuccessful = this->send( &(this->HIDreport) );
  }
  else {
    sendSuccessful = this->send_nb( &(this->HIDreport) );
  }

  this->_mutex.unlock();
  return sendSuccessful;
}


// TODO: Cut & paste of same code in 'pressButton', 'releaseButton' and ' toggleButton'.
void USBJoystick::pressButton(uint8_t buttonNumber)
{
  // buttonState is array of 8-bit integers where each bit represents current button state.
  // buttonState[0] has buttons 0-7, buttonState[1] has buttons 8-15 etc. 
 
  // Get the array index and bit position for the button number 'buttonNumber'.
  uint8_t index = buttonNumber / this->BYTE_LENGTH;
  uint8_t position = buttonNumber % this->BYTE_LENGTH; 

  bitSet( this->buttonState[index], position );
  if (this->autoSend) this->update();
}

void USBJoystick::releaseButton(uint8_t buttonNumber)
{
  // See 'USBJoystick::pressButton for explanation of 'index' and 'position'.
  uint8_t index = buttonNumber / this->BYTE_LENGTH; 
  uint8_t position = buttonNumber % this->BYTE_LENGTH;

  bitClear( this->buttonState[index], position );
  if (this->autoSend) this->update();
}

void USBJoystick::toggleButton(uint8_t buttonNumber)
{
  // See 'USBJoystick::pressButton for explanation of 'index' and 'position'.
  uint8_t index = buttonNumber / this->BYTE_LENGTH; 
  uint8_t position = buttonNumber % this->BYTE_LENGTH;

  this->buttonState[index] ^= 0x01 << position; // XOR to toggle a bit.
  if (this->autoSend) this->update();
}

void USBJoystick::setButton(uint8_t buttonNumber, uint8_t value)
{
  if (value == 0) this->releaseButton(buttonNumber);
  else this->pressButton(buttonNumber);
}



int8_t USBJoystick::axis16bitToByte(int16_t axisValue, bool MSB_OR_LSB) {
  switch (MSB_OR_LSB) {
    case MSB:
      return static_cast<int8_t>( axisValue >> this->BYTE_LENGTH );
    case LSB:
      return static_cast<int8_t>( axisValue );
  }
}


// TODO: Too much cut & pasting. Can we just write one function which takes axis as input?
void USBJoystick::setXAxis(float value) {
  value = constrain( value, this->axisMin.X, this->axisMax.X );
  this->axis.X = USBJoystick::mapfi(value, this->axisMin.X, this->axisMax.X, 
                    this->HID_AXIS_MIN, this->HID_AXIS_MAX);

  if (this->autoSend) this->update();
}

void USBJoystick::setYAxis(float value) {
  value = constrain (value, this->axisMin.Y, this->axisMax.Y );
  this->axis.Y = USBJoystick::mapfi(value, this->axisMin.Y, this->axisMax.Y,
                    this->HID_AXIS_MIN, this->HID_AXIS_MAX);
 
  if (this->autoSend) this->update();
}

void USBJoystick::setZAxis(float value) {
  value = constrain( value, this->axisMin.Z, this->axisMax.Z );
  this->axis.Z = USBJoystick::mapfi(value, this->axisMin.Z, this->axisMax.Z,
                    this->HID_AXIS_MIN, this->HID_AXIS_MAX);
 
  if (this->autoSend) this->update();
}
void USBJoystick::setRxAxis(float value) {
  value = constrain( value, this->axisMin.Rx, this->axisMax.Rx );
  this->axis.Rx = map(value, this->axisMin.Rx, this->axisMax.Rx, 
                      this->HID_AXIS_MIN, this->HID_AXIS_MAX);
  
  if (this->autoSend) this->update();
}
void USBJoystick::setRyAxis(float value) {
  value = constrain( value, this->axisMin.Ry, this->axisMax.Ry );
  this->axis.Ry = map(value, this->axisMin.Ry, this->axisMax.Ry, 
                      this->HID_AXIS_MIN, this->HID_AXIS_MAX);
  
  if (this->autoSend) this->update();
}
void USBJoystick::setRzAxis(float value) {
  value = constrain( value, this->axisMin.Rz, this->axisMax.Rz );
  this->axis.Rz = map(value, this->axisMin.Rz, this->axisMax.Rz, 
                      this->HID_AXIS_MIN, this->HID_AXIS_MAX);
  
  if (this->autoSend) this->update();
}

void USBJoystick::setThrottleAxis(float value) {
  value = constrain( value, this->axisMin.throttle, this->axisMax.throttle );
  this->axis.throttle = map(value, this->axisMin.throttle, this->axisMax.throttle, 
                          this->HID_AXIS_MIN, this->HID_AXIS_MAX);
  
  if (this->autoSend) this->update();
}

void USBJoystick::setRudderAxis(float value) {
  value = constrain( value, this->axisMin.rudder, this->axisMax.rudder );
  this->axis.rudder = map(value, this->axisMin.rudder, this->axisMax.rudder,
                        this->HID_AXIS_MIN, this->HID_AXIS_MAX);

  if (this->autoSend) this->update();
}


// TODO: Too much cut & pasting, again...
void USBJoystick::setXAxisRange(int16_t minimum, int16_t maximum) {
  this->axisMin.X = min(minimum, maximum);
  this->axisMax.X = max(minimum, maximum);
}
void USBJoystick::setYAxisRange(int16_t minimum, int16_t maximum) {
  this->axisMin.Y = min(minimum, maximum);
  this->axisMax.Y = max(minimum, maximum);
}
void USBJoystick::setZAxisRange(int16_t minimum, int16_t maximum) {
  this->axisMin.Z = min(minimum, maximum);
  this->axisMax.Z = max(minimum, maximum);
}
void USBJoystick::setRxAxisRange(int16_t minimum, int16_t maximum) {
  this->axisMin.Rx = min(minimum, maximum);
  this->axisMax.Rx = max(minimum, maximum);
}
void USBJoystick::setRyAxisRange(int16_t minimum, int16_t maximum) {
  this->axisMin.Ry = min(minimum, maximum);
  this->axisMax.Ry = max(minimum, maximum);
}
void USBJoystick::setRzAxisRange(int16_t minimum, int16_t maximum) {
  this->axisMin.Rz = min(minimum, maximum);
  this->axisMax.Rz = max(minimum, maximum);
}
void USBJoystick::setThrottleAxisRange(int16_t minimum, int16_t maximum) {
  this->axisMin.throttle = min(minimum, maximum);
  this->axisMax.throttle = max(minimum, maximum);
}
void USBJoystick::setRudderAxisRange(int16_t minimum, int16_t maximum) {
  this->axisMin.rudder = min(minimum, maximum);
  this->axisMax.rudder = max(minimum, maximum);
}
void USBJoystick::setAllAxisRange(int16_t minimum, int16_t maximum) {
  this->setXAxisRange(minimum, maximum);
  this->setYAxisRange(minimum, maximum);
  this->setZAxisRange(minimum, maximum);
  this->setRxAxisRange(minimum, maximum);
  this->setRyAxisRange(minimum, maximum);
  this->setRzAxisRange(minimum, maximum);
  this->setThrottleAxisRange(minimum, maximum);
  this->setRudderAxisRange(minimum, maximum);
}

