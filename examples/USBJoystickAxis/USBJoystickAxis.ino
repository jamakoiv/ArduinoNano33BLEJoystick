
// Include PluggableUSBHID.h explicitly to make sure we are using the Arduino-version of USBHID.
#include <PluggableUSBHID.h>
#include <USBJoystick.h>

#include <rtos.h> // Include this for ThisThread::sleep_for

USBJoystick joystick;

void setup(void) {
  Serial.begin(57600);

  joystick.autoSend = false;           // If autoSend is false we must call 'update' method ourself.
  joystick.setXAxisRange( -511, 511 ); // 10-bit resolution. 
  joystick.setYAxisRange( -100, 100 ); // Arbitary range from -100 to 100.
  joystick.setZAxisRange( 0, 255 );    // 8-bit resolution on positive side only.
}

void loop(void) {
	
	
  Serial.print("Joystick configured: ");
  if (joystick.ready()) Serial.println( "true" );
  else Serial.println( "false" );

  /*
  In actual implementation you would have something like

  int16_t Xvalue = ReadSomeHardwareSensor();
  joystick.setXAxis(Xvalue);
  joystick.update();
  */

  // Simulated X-axis input.
  for (int i=-511; i < 511; i++) {
    Serial.println( String("X-axis: ") + String(i) );
    joystick.setXAxis(i);
    joystick.update();
    rtos::ThisThread::sleep_for(20);
  }

  // Simulated Y-axis input. Note the values below -100 and above 100 do 
  // not change output since they are out the designated range for Y-axis.
  for (int i=-120; i < 120; i++) {
    Serial.println( String("Y-axis: ") + String(i) );
    joystick.setYAxis(i);
    joystick.update();
    rtos::ThisThread::sleep_for(40);
  }

  // Simulated Z-axis input. 
  for (int i=255; i >= 0; i--) {
    Serial.println( String("Z-axis: ") + String(i) );
    joystick.setZAxis(i);
    joystick.update();
    rtos::ThisThread::sleep_for(20);
  }

}