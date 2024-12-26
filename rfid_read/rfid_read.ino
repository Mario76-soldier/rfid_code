#include <SPI.h> 
#include <MFRC522.h> 
 
#define SS_PIN 5
#define RST_PIN 22
  
MFRC522 rfid(SS_PIN, RST_PIN);  // RFID 리더기 객체 생성

MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(115200);
  SPI.begin();              // SPI 통신 시작
  rfid.PCD_Init();          // RFID 초기화

  // 기본 키 설정 (0xFF로 초기화된 키 사용)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  Serial.println("RFID를 준비 중입니다. 카드를 스캔하세요...");
}

void loop() {
  // RFID 카드가 감지되지 않으면 리턴
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  byte startBlock = 1;  // 데이터를 읽을 첫 번째 블록 번호
  int totalBlocks = 2;  // 16바이트 단위로 나눈 블록 수 (예: 32바이트의 이메일 주소가 있을 경우 2블록)
  String email = "";     // 읽어온 데이터를 저장할 문자열

  for (int i = 0; i < totalBlocks; i++) {
    // 블록에 접근하기 위한 인증 수행
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

    // 블록에서 데이터 읽기
    byte buffer[18];       // 최대 16바이트 데이터 + 스테이터스 바이트
    byte bufferSize = sizeof(buffer);
    MFRC522::StatusCode status = rfid.MIFARE_Read(startBlock + i, buffer, &bufferSize);
    if (status != MFRC522::STATUS_OK) {
      Serial.print("블록 ");
      Serial.print(startBlock + i);
      Serial.print(" 읽기 실패: ");
      Serial.println(rfid.GetStatusCodeName(status));
      break;
    }

    // 버퍼에서 문자열 추가
    for (byte j = 0; j < 16; j++) {
      if (buffer[j] != 0) { // 데이터 유효성 체크
        email += (char)buffer[j];
      }
    }
  }

  Serial.print("읽은 이메일 주소: ");
  Serial.println(email);

  // RFID 카드와의 통신 종료
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}