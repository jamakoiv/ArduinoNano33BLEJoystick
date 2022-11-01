
// Include PluggableUSBHID.h explicitly to make sure we are using the Arduino-version of USBHID.
#include <PluggableUSBHID.h>
#include <USBJoystick.h>

#include <rtos.h> // Include this to use ThisThread::sleep_for

USBJoystick joystick;

void setup(void) {
  Serial.begin(57600);

  joystick.autoSend = false;  // If autoSend is false we must call 'update' method ourself.
}

void loop(void) {
  // You can use the 'ready()' method to check if the joystick-object has initialized properly.
  Serial.print("Joystick configured: ");
  if (joystick.ready()) Serial.println( "true" );
  else Serial.println( "false" );

  /*
  In actual implementation we would do something like.

  uint8_t buttonNumber = 5; 
  bool buttonPressed = digitalRead( BUTTON_PIN );
  if (buttonPressed == true) {
    joystick.pressButton( buttonNumber );
  }
  else { 
    joystick.releaseButton( buttonNumber );
  }
  joystick.update();
  */

  // Simulate pressing and releasing buttons 1-10.
  for (int i=0; i < 10; i++) {
    joystick.pressButton(i);
    joystick.update();
    rtos::ThisThread::sleep_for(100); // Keep the button pressed for 100 ms.
    joystick.releaseButton(i);
    joystick.update();
  }
  rtos::ThisThread::sleep_for(1000);

  // Simulate keeping a button pressed while pressing another button momentarily.
  joystick.autoSend = true; // Now update is called every time we use the pressButton or releaseButton.

  joystick.pressButton(0);
  rtos::ThisThread::sleep_for(1000);
  joystick.pressButton(1);
  rtos::ThisThread::sleep_for(100);
  joystick.releaseButton(1);
  rtos::ThisThread::sleep_for(1000);
  joystick.releaseButton(0);

  rtos::ThisThread::sleep_for(2000);

}