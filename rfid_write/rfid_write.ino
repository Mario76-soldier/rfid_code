#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);  // RFID 리더기 객체 생성
MFRC522::MIFARE_Key key;
String email;

void setup() {
  Serial.begin(115200);
  SPI.begin();              // SPI 통신 시작
  rfid.PCD_Init();          // RFID 초기화

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println("RFID를 준비 중입니다. 카드를 스캔하세요...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // 기존 데이터를 지우기 위한 초기화
  byte clearData[16] = {0};

  // RFID 카드와의 통신 초기화
  email = Serial.readStringUntil('\n');
  Serial.println(email);
  byte startBlock = 1;  // 데이터를 기록할 첫 번째 블록 번호
  int emailLength = email.length();
  int totalBlocks = 2;

  for (int i = 0; i < totalBlocks && i < 16; i++) {
    MFRC522::StatusCode authStatus = rfid.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A,
        startBlock + i,
        &key,
        &(rfid.uid)
    );
    delay(50);
    if (authStatus != MFRC522::STATUS_OK) {
        Serial.print("블록 ");
        Serial.print(startBlock + i);
        Serial.print(" 인증 실패: ");
        Serial.println(rfid.GetStatusCodeName(authStatus));
        break;
    }

    byte buffer[16] = { 0 };
    email.substring(i * 15, (i + 1) * 15).getBytes(buffer, sizeof(buffer));

    MFRC522::StatusCode status = rfid.MIFARE_Write(startBlock + i, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print("블록 ");
      Serial.print(startBlock + i);
      Serial.print(" 기록 실패: ");
      Serial.println(rfid.GetStatusCodeName(status));
      break;
    }
    Serial.println("이메일 주소가 성공적으로 카드에 기록되었습니다.");
  }

  delay(1000);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
