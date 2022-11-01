
// Include PluggableUSBHID.h explicitly to make sure we are using the Arduino-version of USBHID.
#include <PluggableUSBHID.h>
#include <USBJoystick.h>

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

  // Simulate pressing and releasing buttons 1-10 momentarily.
  for (int i=0; i < 10; i++) {
    joystick.pressButton(i);
    joystick.update();
    delay(100);
    joystick.releaseButton(i);
    joystick.update();
  }

  delay(2000);

  // Simulate keeping a button pressed while pressing another button momentarily.
  joystick.autoSend = true; // Now update is called every time we use the pressButton or releaseButton.

  joystick.pressButton(0);
  delay(1000);
  joystick.pressButton(1);
  delay(100);
  joystick.releaseButton(1);
  delay(1000);
  joystick.releaseButton(0);

  delay(2000);

}