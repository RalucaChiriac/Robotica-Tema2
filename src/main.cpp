#include <Arduino.h>
#include <string.h>

#define BAUD 28800

char cuvant[25], cuvantTarget[25];
char dictionar [13][25] = {"robotica", "uno", "robot", "matematica", "caine", "elefant", "mama", "laptoparupa", "aripa", "tastatura", "casa", "masa", "parfum"};
int scor = 0;
int index = 0, dificultate = 0, stareLed = 0, clipiri = 0;
unsigned long timp[3] = {5300, 4100, 2800}, timpJoc = 33000, momentSelectareCuvant = 0, timpDebounce = 300;
int eroareIncepere = 50, timpIncepere = 3000;
char numeDificultati[3][10] = {"\nUsor\n", "\nMediu\n", "\nGreu\n"};
volatile unsigned long momentApasareDificultate = 0, momentApasareStart = 0, momentIncepereJoc = 0, ultimaClipire = 0;
bool repaus = true, rulare = false;

int LED_ROSU = 9;
int LED_VERDE = 6;
int LED_ALBASTRU = 5;

int BUTON_START = 3;
int BUTON_DIFICULTATE = 2;
int BACKSPACE = 8;

void setRGB(int valRosu, int valVerde, int valAlbastru) 
{
  analogWrite(LED_ROSU, valRosu);
  analogWrite(LED_VERDE, valVerde);
  analogWrite(LED_ALBASTRU, valAlbastru);
}

//Functia primeste ca parametru un sir de caractere si verifica daca se potriveste cu target-ul curent, returnand:
// 1 - daca se potriveste complet
// 0 - daca se potriveste partial
// -1 - daca nu se potriveste
int verificaCuvant(const char *cuvantVerificare)
{
  int lungime = strlen(cuvantVerificare);
  
  int i = 0;
  while (i < lungime) {
      if (cuvantVerificare[i] != cuvantTarget[i])
          return -1;
      i++;
  }
  if(lungime == int(strlen(cuvantTarget)))
    return 1;
  return 0;
}

void finalizare()
{
  Serial.println("\nTerminat!\nScor:");
  Serial.println(scor);
  Serial.println("\n");
  setRGB(255, 255, 255);
}

//Functia verifica daca timpul de joc a exipirat; 
// daca da, atunci se apeleaza functia de finalizare si se trece in starea de repaus
void verificaStare()
{
  if(rulare && millis() - momentIncepereJoc > timpJoc)
  {
    repaus = true;
    rulare = false;
    finalizare();
  }
}

//Functia este apelata in primele 3 secunde de la apasarea butonului de start; se ocupa de numaratoarea inversa pana la inceperea
//jocului si de clipitul LED-ului in cele 3 secunde
void incepere()
{
  if(millis() - ultimaClipire > timpIncepere/6)
  {
    ultimaClipire = millis();
    setRGB(255 * stareLed, 255 * stareLed, 255 * stareLed);
    stareLed = (stareLed + 1) % 2;

// Dacă LED-ul și-a schimbat starea de un număr par de ori mai mic decât 6 (inclusiv 0), se va afișa un număr care reprezintă
// numărătoarea inversă până la începerea jocului
    if(!(clipiri % 2))
    {
      Serial.println(char(3 - clipiri/2 + 48));
    }

    clipiri++;
  }

  //Cand LED-ul si-a schimbat starea de 6 ori (a clipit de 3 ori) se va selecta primul cuvant de tastat, LED-ul se va face verde,
  //si se reseteaza sirul de caracter "cuvant" in care se stocheaza ce se tasteaza
  if(clipiri == 6)
  {
    index = 0;
    strcpy(cuvant, "");
    strcpy(cuvantTarget,dictionar[(random(10) + momentIncepereJoc) % 10]);
    Serial.println(cuvantTarget);
    momentSelectareCuvant = millis();
    setRGB(0, 255, 0);
    clipiri++;
  }
}

void startJoc()
{
  momentIncepereJoc = millis();
  scor = 0;
  ultimaClipire = millis();
  clipiri = 0;
}

// debounce pentru butonul de dificultate + schimbarea dificultatii doar daca jocul este in repaus
void verificareButonDificultate()
{
  if(repaus)
  {
    if(millis() - momentApasareDificultate > timpDebounce && digitalRead(BUTON_DIFICULTATE) == LOW)
    {
      momentApasareDificultate = millis();
      dificultate = (dificultate + 1) % 3;
      if (dificultate == 0)
      {
        Serial.println();
        Serial.println();
        Serial.println("Easy mode ON");
      }
      else if (dificultate == 1)
      {
        Serial.println();
        Serial.println();
        Serial.println("Medium mode ON");
      }
      else if (dificultate == 2)
      {
        Serial.println();
        Serial.println();
        Serial.println("Hard mode ON");
      }
    }
  }
}

// debounce pentru butonul de start + schimbarea starii jocului
void verificareButonStart()
{
  if(millis() - momentApasareStart > timpDebounce && digitalRead(BUTON_START) == LOW)
  {
    momentApasareStart = millis();
    repaus = !repaus;
    rulare = !rulare;
    if(rulare)
    {
      startJoc();
    }else
    {
      finalizare();
    }
  }
}

//Functia citeste o litera de la tastatura si o adauga la sirul de caractere "cuvant" care se compara cu target-ul
void citireLitera()
{
  char litera = Serial.read();
  if(int(litera) == BACKSPACE)
  {
    if(index > 0)
    {
      cuvant[index - 1] = NULL;
      index--;
    }
  }else
  {
    cuvant[index] = litera;
    index++;
    cuvant[index] = NULL;
  }
}

void alegereCuvantNou(int verdict)
{
  if(verdict)
  {
    scor++;
    Serial.println("\n");
  }else
  {
    Serial.println("\nTimp expirat!\n");
  }
  strcpy(cuvant," ");
  index = 0;
  strcpy(cuvantTarget,dictionar[(random(13) + momentIncepereJoc) % 13]);
  Serial.println(cuvantTarget);
  momentSelectareCuvant = millis();
}

void setup()
{
    Serial.begin(BAUD);

    pinMode(LED_ROSU, OUTPUT);
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_ALBASTRU, OUTPUT);

    setRGB(255, 255, 255);

    pinMode(BUTON_START, INPUT_PULLUP);
    pinMode(BUTON_DIFICULTATE, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(BUTON_START), verificareButonStart, FALLING);
    attachInterrupt(digitalPinToInterrupt(BUTON_DIFICULTATE), verificareButonDificultate, FALLING);

    Serial.println("\nPuteti incepe jocul!\n");
    Serial.println("Dificultate: ");
    if (dificultate == 0)
      {
        Serial.println("Easy mode ON");
      }
      else if (dificultate == 1)
      {
        Serial.println("Medium mode ON");
      }
      else if (dificultate == 2)
      {
        Serial.println("Hard mode ON");
      }
}

void loop()
{
  verificaStare();

  if(rulare)
  {
    if(millis() - momentIncepereJoc <= timpIncepere + eroareIncepere)
    {
      incepere();
    }else
    {
      if(Serial.available()) 
      {
          citireLitera();
          
          if(verificaCuvant(cuvant) == 1)
          {
            alegereCuvantNou(1);
          }else if(verificaCuvant(cuvant) == -1)
          {
            setRGB(255, 0, 0);
          }else
            setRGB(0, 255, 0);
      }
      if(millis() - momentSelectareCuvant > timp[dificultate])
      {
        alegereCuvantNou(0);
      }
    }
  }
}