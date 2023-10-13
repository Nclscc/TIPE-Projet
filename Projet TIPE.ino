#include "HX711.h" // Inclusion du module HX711
#include <SoftwareSerial.h> // Inclusion du module SoftwareSerial
#define SERIAL Serial
#define calibration_factor 48000 //Calibrage des jauges de contrainte (fournies par HX711)
#define DOUT  3 //Définition du port d'une jauge de contrainte au port D2
#define CLK  2 // 
#define CLK2  4 //
#define DOUT2 5 // Définition du port d'une autre jauge de contrainte au port D4

HX711 jauge1(DOUT, CLK); // Inclusion des deux jauges
HX711 jauge2(DOUT2, CLK2); // 

// Définitions de variables

const int relayPin =  6; // définition d'une constante entière pour le pin du relais
float poids1 = 0.0;  // définition d'une constante float pour le poids d'une des jauges
float poids2 = 0.0; // définition d'une constante float pour le poids de l'autre jauge
float poidstot = 0.0; // définition d'une constante float pour le poids total réparti sur les deux jauges
int sensorturbidite = 0; // définition d'une constante entière pour la lecture de la valeur renvoyée par le capteur de turbidité
int sensordurete = 0; // définition d'une constante entière pour la lecture de la valeur renvoyée par le capteur de dureté
float durete = 0;  // définition d'une constante float pour la valeur de la dureté
float turbidite = 0; // définition d'une constante float pour la valeur de la turbidité
char cc; // définition d'une chaine de caractère utilisée pour recevoir des informations de l'application
bool K = false; // définition d'une valeur booléenne servant au mode automatique de l'application
SoftwareSerial bluetooth(8, 9); // Inclusion du capteur bluetooth au port D8

void setup() {
  pinMode(relayPin, OUTPUT); // Passage du pin du relais en mode OUTPUT donc sortie
  Serial.begin(9600); // Délais de communication entre la carte et le programme
  bluetooth.begin(9600); 
  jauge1.set_scale(calibration_factor); // calibration d'une des jauges de contrainte
  jauge2.set_scale(calibration_factor); // calibration de l'autre jauge de contrainte
  jauge1.tare(); // tarage de d'une des jauges de contraintes (masse de référence)
  jauge2.tare(); // tarage de l'autre jauge de contrainte 
}


void loop() {
 // Capteur bluetooth //
  if (bluetooth.available()){ // Boucle qui, si le capteur bluetooth est disponible, vérifie si une valeur est renvoyée par l'application
    cc = bluetooth.read(); // et si oui, on place cette valeur dans la chaine de caractère cc
  } 

  // CODE BALANCE //
  poids1 = jauge1.get_units() / 2,20462; // division afin d'obtenir la vraie valeur de la masse (fournie par le capteur)
  poids2 = jauge2.get_units() / 2,20462;
  poidstot= poids1+poids2; // somme des deux masses afin d'avoir la masse totale

  // CODE TURBIDITE //
  int sensorturbidite = analogRead(A2); // Lecture de la valeur analogue renvoyée par le capteur de turbidité au pin A2
    for(int i=0; i<800; i++) // Boucle itérée 800 fois afin de faire une moyenne
    {
        turbidite += ((float)sensorturbidite/1024)*5-0.11; // Calcul de la valeur de la turbidité en voltage (formule donnée par le capteur)
    }
    turbidite = turbidite/800; // Moyenne faite
    
  float turbiditentu = -1120.4*turbidite*turbidite + 5742.3*turbidite - 4352.9; // définition d'une autre valeur de turbidité en unité NTU, formule trouvée sur internet pour la conversion
   if (turbiditentu < 0) // Si la valeur est négative avec la marge d'erreur, on renvoie simplement 0
    {
        turbiditentu = 0;
   }

  // CODE DURETE //
  sensordurete = analogRead(A1); // Lecture de la valeur analogue renvoyée par le capteur de turbidité au pin A2
  durete = sensordurete*5/1024.0; // Conversion de la valeur analogique en voltage
  durete=(133.42/durete*durete*durete - 255.86*durete*durete + 857.39*durete)*0.5; // Conversion de la valeur en Volt en ppm (formule fournie par le capteur)

  // CODE RELAIS //
  if (cc == '1') // si l'application renvoie 1 ce qui correspond à allumer la pompe
  {
    digitalWrite(relayPin,HIGH); // on allume le relais relié électriquement à la pompe
  }
  if (cc == '2')  // si l'application renvoie 2 ce qui correspond à éteindre la pompe
  {
    digitalWrite(relayPin,LOW); // on éteint le relais relié électriquement à la pompe
  }
  if (K == true) // si la variable booléenne K est vraie (elle devient vraie en appuyant sur le bouton mode automatique de l'application, on peut le voir plus bas)
  {
    if (poidstot >= 1) // si le poids total est supérieur à 1 kg (contenance maximum de la cuve)
  {
      while (poidstot >= 0.9) // tant que le poids total de la cuve n'est pas inférieur à 0.9kg , soit 10% de moins que la masse maximum d'eau dans la cuve
      {
        poids1 = jauge1.get_units() / 2,20462; // on continue à récupérer les informations sur le poids pendant qu'on éxécute cette boucle
        poids2 = jauge2.get_units() / 2,20462;
        poidstot= poids1+poids2;
        digitalWrite(relayPin,HIGH); // on continue d'allumer la pompe étant dans la boucle, jusqu'a ce que le poids sois inférieur 0.9kg
      }
      digitalWrite(relayPin,LOW); // puis on éteint la pompe une fois le poids redevenu convenable
  }
  }
// DONNEES BLUETOOTH //

  if (cc == '3') // si l'application renvoie 3 ce qui correspond à l'appuie du boutton mode automatique de l'application
  {
    if (K == false) // si K est faux (il l'est à l'allumage premier du programme), ce qui correspond au mode automatique désactivé
  {   K=true; // on passe K à vraie (activé)
  }
    else // sinon ...(donc si le mode automatique est déjà activé)
  {
      K=false; // on le désactive
  }

  }
  cc='0'; // on passe la constante recue par l'application à 0 à chaque itération du void loop pour éviter tout problème


// AFFICHAGE // Nous ne détailleron pas la suite du programme n'étant simplement qu'une suite de print pour afficher les données essentielles.

  SERIAL.print("Durete (PPM): "); 
  SERIAL.print(durete);
  SERIAL.println(" ppm"); 
  //Serial.print("Reading 1: ");
 // Serial.print(value1, 1);
 // Serial.print(" kg"); 
 // Serial.print("Reading 2: ");
 // Serial.print(value2, 1); //scale.get_units() returns a float
 // Serial.print(" kg"); //You can change this to kg but you'll need to refactor the calibration_factor
  Serial.print("Poids total: ");
  Serial.print(poidstot, 1);
  Serial.println(" kg");
  Serial.print ("Turbidité (V):");
  Serial.println (turbidite);
  Serial.print ("Turbidité (NTU):");
  Serial.println (turbiditentu);
  bluetooth.print(turbiditentu);
  bluetooth.print(";");
  bluetooth.print(durete);  
  bluetooth.print(";");
  bluetooth.print(poidstot);
  delay(1000);

} // FIN DU PROGRAMME
