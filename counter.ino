void count() {
  lcd.setCursor(1, 1);
  lcd.print("Available: ");
  lcd.setCursor(12, 1);
  NPadNo--;
  EEPROM.update(address, NPadNo);
  NPadNo = EEPROM.read(address);
  EEPROM.update(address2, NPadNo);
  Serial.println(NPadNo);
  delay(200);
  lcd.print(NPadNo);
}
