#include "Hal.h"
#include <ButtonDebounce.h>
#include <SoftwareSerial.h>
#include <MD_YX5300.h>

SoftwareSerial  MP3Stream(ARDUINO_RX, ARDUINO_TX);  // MP3 player serial stream for comms
MD_YX5300 mp3(MP3Stream);
ButtonDebounce buttonOne(TRIGGER, 1000); // One second

unsigned long timecode;
unsigned long currentTimecode;

bool unlock = false;

void(*resetSoftware)(void) = 0;

enum PuzzleState {
  Initialising,
  Running,
  Solved
};

PuzzleState puzzleState = Initialising;


bool sfxPlay = false;


void setup() {
  Serial.begin(9600);
  // while (!Serial); only debug
  Serial.println ("ESCAPE JUNGLE - PORTA DO COFRE");
  Serial.println ("Nicolau dos Brinquedos - Março 2023");
  Serial.println("Inicio do Setup");
  Serial.println();
  delay (500);
  Serial.println ("Iniciando MP3 Player");
  MP3Stream.begin(MD_YX5300::SERIAL_BPS);
  mp3.begin();
  mp3.setSynchronous(true);
  Serial.print ("Volume Max ");
  Serial.println (mp3.volumeMax());
  mp3.volume (mp3.volumeMax());
  Serial.print ("Check Valid Files in Folder 001 : ");
  Serial.println (mp3.queryFolderFiles(1));
  mp3.check();

  Serial.println ("Preparando o Sensor Magnético");
  pinMode( TRIGGER  ,  INPUT_PULLUP );
  Serial.print ("Estado da Entrada ");
  Serial.println (digitalRead (TRIGGER));

  Serial.println ("Preparando RELAYS");
  pinMode (MAGLOCK, OUTPUT);
  digitalWrite (MAGLOCK, INACTIVE);
  pinMode (SMOKE, OUTPUT);
  digitalWrite (SMOKE, INACTIVE);
  pinMode (FLASH, OUTPUT);
  digitalWrite (FLASH, INACTIVE);
  delay(1000);

  puzzleState = Running;
  Serial.println ("Fim do Setup");
  Serial.println ("=============");
}



void loop() {

  if (puzzleState == Running) {
    buttonOne.update();
    mp3.check();
    if (buttonOne.state() == LOW ) {
      mp3.check();
      Serial.println ("ATIVADO");
      delay(1000);

      timecode = millis();         // Começa a contar o tempo assim que o audio se inicia
      currentTimecode = timecode;  // Atualiza o timecode atual
      Serial.println ("Iniciando Efeito Especial...");

      while (true) {
        buttonOne.update();
        mp3.check();
        currentTimecode = millis();
        Serial.print ("Tempo :");
        Serial.print ((currentTimecode - timecode) / 1000);
        Serial.println (" segundos");
        if ((currentTimecode > timecode + 5000) && (currentTimecode < timecode + 25000)) {
          Serial.println("COM FUMACA ");
          digitalWrite(SMOKE, ACTIVE);
        } else {
          Serial.println("SEM FUMACA ");
          digitalWrite(SMOKE, INACTIVE);
        }

        if ((currentTimecode > timecode + 15000) && (currentTimecode < timecode + 15100)) {
          mp3.check();
          Serial.println ("EXPLOSAO");
          Serial.println("FLASH ON ");
          digitalWrite(FLASH, ACTIVE);  // Aciona o flash
          Serial.println ("DOOR OPEN");
          digitalWrite(MAGLOCK, ACTIVE);
          Serial.println ("SFX Trigger on ");
          mp3.playSpecific(1, 1);
        }

        if ((currentTimecode > timecode + 25000)) {  // desarma o flash
          Serial.println("FLASH OFF");
          digitalWrite(FLASH, INACTIVE);
        }

        if (currentTimecode > timecode + 28000) {
          Serial.println ("FIM DO EFEITO - 28s");
          puzzleState = Solved;
          break;
        }
      }

    } else {

      puzzleState = Running;
    }
  }

  if (puzzleState == Solved) {
    mp3.check();
    buttonOne.update();
    Serial.println ("CONCLUIDO");
    Serial.print ("TRIGGER ");
    Serial.println (digitalRead(TRIGGER));
    delay(1000);
    // Auto reset quando tirar todos os elementos.
    if (buttonOne.state() == HIGH) {
      Serial.println ("Reiniciando");
      delay (10000);
      Serial.println ();
      Serial.println ("Running Again");
      digitalWrite(MAGLOCK, HIGH);
      // puzzleState = Running;
      mp3.reset();
      delay(2000);
      resetSoftware ();
    }
  }
}
