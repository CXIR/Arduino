/*
// ====================================================================================================================================
 
 Exemple de programme pour Terrarium, controle de la lumiÃ¨re, humiditÃ©, tempÃ©rature, pompe

 Auteur : BEZY Sylvain
 
// ====================================================================================================================================
*/

// LIBRAIRIES

#include <SPI.h> // Communication avec les peripheriques
#include <Wire.h> // Communication avec l'horloge par I2C
#include <RTClib.h> // Librairie de l'Horloge
#include "DHT.h" // Communication avec le senseur humidite/temperature
#include <Adafruit_GFX.h> // Noyau graphique
#include <Adafruit_ST7735.h> // Fonctions graphiques

// DEFINITION DES VARIABLES DE CULTURE DU TERRARIUM

// conditions ete
int mois_ete = 5; // 1 aÂ  12, 4 pour avril
int hum_max_ete = 60; // humidite max (%)
int hum_min_ete = 40; // humidite min (%)
int t_max_ete = 30; // temperature max (Â°C)
int t_min_ete = 20; // temperature min (Â°C)
int heure_lum_ete = 8; // heure allumage lampe
int duree_lum_ete = 14; // duree eclairage (h)
int heure_pompe_ete = 9; // heure de pompe
int duree_pompe_ete = 8; // plage horaire de la pompe (h)
int duree_arrosage_ete = 1; // durÃ©e d'un arrosage (minutes par heure)

// conditions hiver
int mois_hiver = 11;
int hum_max_hiver = 80;
int hum_min_hiver = 50;
int t_max_hiver = 22;
int t_min_hiver = 10;
int heure_lum_hiver = 9;
int duree_lum_hiver = 10;
int heure_pompe_hiver = 13;
int duree_pompe_hiver = 2;
int duree_arrosage_hiver = 1;

// conditions physiques
int seuil_niveau_eau = 3; // seuil d'alerte du manque d'eau (cm)
int seuil_niveau_critique = 2; // seuil critique d'arret de la pompe (cm)

// ====================================================================================================================================

// DECLARATION DES VARIABLES DU PROGRAMME

boolean saison = true, saison_old, mode_horaire;
boolean periode_pompe, periode_lampe;
boolean manque_eau, manque_eau_old, manque_critique, manque_critique_old;
boolean trop_sec = false, trop_sec_old, trop_humide = false, trop_humide_old, trop_chaud , trop_chaud_old, trop_froid = false, trop_froid_old;
boolean etat_pompe, etat_pompe_old;
boolean etat_ventilo, etat_ventilo_old;
boolean etat_lampe, etat_lampe_old;

int hum_max, hum_min, t_max, t_min, heure_lum_on, heure_lum_off, heure_pompe_on, heure_pompe_off;
int heure, minut, sec, annee, mois, jour, jour_sem;
String jour_sem_str, date_str, date_str_old;
int niveau_eau, niveau_eau_old = 9;
int t, t_old = -1, h, h_old = -1;

// DEFINITION DES PIN POUR LES RELAIS

#define pinLampe 5 // pin DIGITAL controle relais lampe
#define pinPompe 4 // pin DIGITAL controle relais pompe a eau
#define pinVentilo 3 // pin DIGITAL controle relais ventilateur
// #define pinFog 2 // pin DIGITAL controle relais fog

// DEFINITION DES PIN POUR LES SENSEURS

#define pinEau 2 // pin ANALOGUE capteur niveau d'eau
#define DHTPIN 6 // pin DIGITAL capteur Temperature
#define DHTTYPE DHT22 // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);//declaration du capteur
RTC_DS3231 RTC;

// DEFINITION DES PIN POUR L'ECRAN : 

#define sclk 12// SCL ou CSK violet
#define mosi 11// SDA bleu
#define dc   10// RS vert
#define rst  9 // RST jaune
#define cs   8 // CS orange
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, mosi, sclk, rst);

// DEFINITION DES COULEURS UTILISEES

#define Black 0x0000
#define DGreen 0x0340
#define LGreen 0x8FF3
#define Red 0xF800
#define White 0xFFFF
#define LBrown 0xEE4E
#define DBrown 0x6940
#define DBlue 0x0210
#define LBlue 0x7E1F 

// ====================================================================================================================================

void setup()
{
  // DEMARRAGE DES SERVICES

  Serial.begin(9600);
  Wire.begin();
  dht.begin();
  RTC.begin();
  
  
  // LIGNE A TELEVERSER UNE FOIS, PUIS A SUPPRIMER (car remet a zero l'horloge en cas de coupure de courant)
  RTC.adjust(DateTime(__DATE__,__TIME__));

  // COMMUNICATION AVEC L'ORDINATEUR

  Serial.println();
  Serial.print("TerraContolleur v1.0 : Bienvenue"); // Affichage sur PC 
  Serial.println();
  Serial.print("pour terrarium a Pinguicula"); // Affichage sur PC 
  Serial.println();
  Serial.println();

  // DISTRIBUTION DES ROLES DES PIN

  pinMode(pinLampe,OUTPUT); // Lampe
  pinMode(pinPompe,OUTPUT); // Pompe a eau
  pinMode(pinVentilo,OUTPUT); // Ventilateur

  // DEFINITIONS DES VALEURS AU DEMARRAGE

  digitalWrite(pinLampe, HIGH); // eteint
  etat_lampe = false;
  digitalWrite(pinPompe, HIGH); // eteint
  etat_pompe = false;
  digitalWrite(pinVentilo, HIGH); // eteint
  etat_ventilo = false;

  // PEREPARATION ECRAN, AFFICHAGE

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  tft.fillScreen(Black);

  // titre
  tft.fillRect(0, 2, 160, 11, DGreen);
  tft.setCursor(24, 4);
  tft.setTextColor(White);
  tft.setTextSize(1);
  tft.println("PingControleur v1.1");

  // temperature
  tft.fillRoundRect(3, 16, 90, 30, 5, LBrown);
  tft.setCursor(6, 19);
  tft.setTextColor(DGreen);
  tft.println("Temper.:");
  tft.setCursor(77, 19);
  tft.setTextColor(DBrown);
  tft.drawCircle(84, 20, 2, DBrown);
  afficher_message(LBrown, LGreen, "Temp. OK", "Tp chaud",false, 6, 33);

  // humidite
  tft.fillRoundRect(3, 50, 90, 30, 5, LBrown);
  tft.setCursor(6, 53);
  tft.setTextColor(DGreen);
  tft.println("Humid.:");
  tft.setCursor(77, 53);
  tft.setTextColor(DBrown);
  tft.println(" %");
  afficher_message(LBrown, LGreen, "Hum. OK", "Tp sec",false, 6, 67);

  // niveau d'eau
  tft.fillRoundRect(3, 84, 90, 30, 5, LBrown);
  tft.setCursor(6, 87);
  tft.setTextColor(DGreen);
  tft.println("Niveau.:");
  tft.setCursor(70, 86);
  tft.setTextColor(DBrown);
  tft.println(" cm");
  afficher_message(LBrown, LGreen, "Eau OK", "Mq. eau",false, 6, 101);

  // relais : lumiere
  tft.fillRoundRect(97, 16, 60, 30, 5, LBlue);
  tft.setCursor(106, 19);
  tft.setTextColor(Black);
  tft.println("Lumiere");
  afficher_message(LBlue, DGreen, "OFF", "ON",false, 118, 32);

  // relais : pompe
  tft.fillRoundRect(97, 50, 60, 30, 5, LBlue);
  tft.setCursor(112,53);
  tft.setTextColor(Black);
  tft.println("Pompe");
  afficher_message(LBlue, DGreen, "OFF", "ON",false, 118, 66);

  // relais : ventilateur
  tft.fillRoundRect(97, 84, 60, 30, 5, LBlue);
  tft.setCursor(106, 87);
  tft.setTextColor(Black);
  tft.println("Ventil.");
  afficher_message(LBlue, DGreen, "OFF", "ON",false, 118, 100);

  // date / etat
  tft.fillRect(0, 117, 160, 11, DGreen);
  tft.setCursor(3, 119);
}

// ====================================================================================================================================

void loop()
{
  delay(3000);

  // RECUPERATION DATE ET HEURE

  DateTime now = RTC.now(); // Recuperation de la date/heure actuelle
  annee = now.year();
  mois = now.month();
  jour = now.day();
  jour_sem = now.dayOfTheWeek();
  jour_sem_str = "";
  heure = now.hour();
  minut = now.minute();
  date_str = jour_sem_str + jour + "/" + mois + "/" + String(annee).substring(2) + " " + heure + ":" + minut +" " + return_saison();

  // CHANGEMENT HEURE ETE/HIVER

  if(jour_sem == 7 & mois == 3 & jour <=7 & heure == 3 & mode_horaire == false)
  {
    RTC.adjust(DateTime(annee, mois, jour, heure + 1, minut,sec));
    mode_horaire = true;
  }
  if(jour_sem == 7 & mois == 10 & jour >=23 & heure == 3 & mode_horaire == true)
  {
    RTC.adjust(DateTime(annee, mois, jour, heure - 1, minut,sec));
    mode_horaire = false;
  }

  // CALCUL DE PERIODE CULTURE ETE/HIVER

  if(mois_ete <= mois & mois < mois_hiver){
    saison = true;
  }
  else
  {
    saison = false;
  }

  if(saison == true & saison_old != saison){
    hum_max = hum_max_ete; 
    hum_min = hum_min_ete; 
    t_max = t_max_ete; 
    t_min = t_min_ete;
    heure_lum_on = heure_lum_ete;
    heure_lum_off = heure_lum_ete + duree_lum_ete;
    heure_pompe_on = heure_pompe_ete;
    heure_pompe_off = heure_pompe_ete + duree_pompe_ete;
    saison_old = saison;
  }
  else
  {
    hum_max = hum_max_hiver; 
    hum_min = hum_min_hiver; 
    t_max = t_max_hiver; 
    t_min = t_min_hiver;
    heure_lum_on = heure_lum_hiver;
    heure_lum_off = heure_lum_hiver + duree_lum_hiver;
    heure_pompe_on = heure_pompe_hiver;
    heure_pompe_off = heure_pompe_hiver + duree_pompe_hiver;
    saison_old = saison;
  }

  // CALCUL DES PERIODES LUMIERE/POMPE

  // lumiere
  if(heure_lum_on <= heure & heure < heure_lum_off){
    periode_lampe = true;
  }
  else
  {
    periode_lampe = false;
  }

  //pompe
  if(heure_pompe_on <= heure & heure < heure_pompe_off & minut > 58){
    periode_pompe = true;
  }
  else
  {
    periode_pompe = false;
  }

  // LECTURE DES SENSEURS

  niveau_eau = niveau_cm(analogRead(pinEau)); // Lecture du niveau d'eau
  h = dht.readHumidity(); // hygrometrie
  t = dht.readTemperature(); // temperature en celsius

  //  if (isnan(h) || isnan(t))
  //  {
  //    Serial.println("! Impossible de lire DHT22 !");
  //    return;
  //  }

  // COMMUNICATION AVEC L'ORDINATEUR

  Serial.println();
  Serial.print(niveau_eau); 
  Serial.print(" cm ("); 
  Serial.print(analogRead(pinEau));
  Serial.print("); "); 
  Serial.print(h);
  Serial.print(" %; ");
  Serial.print(t);
  Serial.print(" C; ");
  Serial.print(heure);
  Serial.print(":");
  Serial.print(minut);
  Serial.print(":");
  Serial.print(sec);
  Serial.print("; ");
  Serial.print(jour_sem_str);
  Serial.print(jour);
  Serial.print("/");
  Serial.print(mois);
  Serial.print("/");
  Serial.print(annee);

  // ANALYSE DE L'ENVIRONNEMENT

  // pour le niveau d'eau
  if(niveau_eau < seuil_niveau_eau){
    manque_critique = true;
  }
  if(niveau_eau >= seuil_niveau_eau){
    manque_critique = false;
  }

  // pour l'humidite
  if(h > hum_max){
    trop_humide = true;
  }
  if(h <= hum_max - 1){
    trop_humide = false;
  }
  if(h < hum_min){
    trop_sec = true;
  }
  if(h >= hum_min + 1){
    trop_sec = false;
  }

  // pour la temperature
  if(t > t_max){
    trop_chaud = true;
  }
  if(t <= t_max - 1){
    trop_chaud = false;
  }
  if(t < t_min){
    trop_froid = true;
  }
  if(t >= t_min + 1){
    trop_froid = false;
  }

  // MISE EN SITUATION DES PERIPHERIQUES

  // lampe
  if(periode_lampe == true & etat_lampe == false)
  {
    digitalWrite (pinLampe, LOW); // allume la lampe SI periode de lampe
    etat_lampe = true; // etat de la lampe : allumee (true)
  }
  if(periode_lampe == false & etat_lampe == true)
  {
    digitalWrite (pinLampe, HIGH); // eteint la lampe si hord periode de lampe
    etat_lampe = false; // etat de la lampe : eteint (false)
  }

  // pompe
  if((periode_pompe == false || trop_humide == true) & etat_pompe == true)
  {
    digitalWrite(pinPompe, HIGH); // eteint la pompe si pas assez d'eau, ou si plus dans periode, ou si trop humide
    etat_pompe = false; // etat de la pompe : eteinte (false)
  }
  if(etat_pompe == false & periode_pompe == true & trop_humide == false)
  {
    digitalWrite(pinPompe, LOW); // allume la pompe SI niveau ok ET heure dans la periode pompe ET pas trop humide
    etat_pompe = true; // etat de la pompe : allumee (true)
  }

  // ventilo
  if((trop_humide == true || trop_chaud == true) & etat_ventilo == false)
  {
    digitalWrite(pinVentilo, LOW); // allume le ventilo SI trop chaud ou trop humide
    etat_ventilo = true; // etat du ventilo : allume (true)
  }
  if(trop_humide == false & trop_chaud == false & etat_ventilo == true)
  {
    digitalWrite(pinVentilo, HIGH); // eteint le ventilo si assez sec et assez frais
    etat_ventilo = false; // etat du ventilo : eteint (false)
  }

  // AFFICHAGE DES VARIABLES A L'ECRAN SI CHANGEMENT

  // temperature
  if (t == t_old)
  {
    goto humidite;
  }
  else {
    t_old = afficher_valeur(LBrown, DBrown, t, t_old, 58, 19);
  }
humidite:
  if (h == h_old)
  {
    goto niveau;
  }
  else {
    h_old = afficher_valeur(LBrown, DBrown, h, h_old, 57, 53);
  }
niveau:
  if (niveau_eau == niveau_eau_old)
  {
    goto trop_chaud;
  }
  else {
    niveau_eau_old = afficher_valeur(LBrown, DBrown,niveau_eau , niveau_eau_old, 61, 87);
  }
trop_chaud:
  if (trop_chaud == trop_chaud_old)
  {
    goto trop_froid;
  }
  else {
    if(trop_chaud == true)
    {
      trop_chaud_old = afficher_message(LBrown, Red, "Tp. chaud", "Temp. OK",true, 6, 33);
    }
    else
    {
      trop_chaud_old = afficher_message(LBrown, LGreen, "Temp. OK", "Tp. chaud",false, 6, 33);
    }
  }
trop_froid:
  if (trop_froid == trop_froid_old)
  {
    goto trop_sec;
  }
  else {
    if(trop_froid == true)
    {
      trop_froid_old = afficher_message(LBrown, Red, "Tp. froid", "Temp. OK",true, 6, 33);
    }
    else
    {
      trop_froid_old = afficher_message(LBrown, LGreen, "Temp. OK", "Tp. froid",false, 6, 33);
    }
  }
trop_sec:
  if (trop_sec == trop_sec_old)
  {
    goto trop_humide;
  }
  else {
    if(trop_sec == true)
    {
      trop_sec_old = afficher_message(LBrown, Red, "Tp. sec", "Hum. OK",true, 6, 67);
    }
    else
    {
      trop_sec_old = afficher_message(LBrown, LGreen, "Hum. OK", "Tp. sec",false, 6, 67);
    }
  }
trop_humide:
  if (trop_humide == trop_humide_old)
  {
    goto pas_deau;
  }
  else {
    if(trop_humide == true)
    {
      trop_humide_old = afficher_message(LBrown, Red, "Tp. hum.", "Hum. OK",true, 6, 67);
    }
    else
    {
      trop_humide_old = afficher_message(LBrown, LGreen, "Hum. OK", "Tp. hum.",false, 6, 67);
    }
  }
pas_deau:
  if (manque_critique == manque_critique_old)
  {
    goto lampe;
  }
  else {
    if(manque_critique == true)
    {
      manque_critique_old = afficher_message(LBrown, Red, "Mq. eau", "Eau OK",true, 6, 101);
    }
    else
    {
      manque_critique_old = afficher_message(LBrown, LGreen, "Eau OK", "Mq. eau",false, 6, 101);
    }
  }
lampe:
  if (etat_lampe == etat_lampe_old)
  {
    goto eau;
  }
  else {
    if(etat_lampe == true)
    {
      etat_lampe_old = afficher_message(LBlue, DBlue, "ON", "OFF",true, 118, 32);
    }
    else
    {
      etat_lampe_old = afficher_message(LBlue, DGreen, "OFF", "ON",false, 118, 32);
    }
  }
eau:
  if (etat_pompe == etat_pompe_old)
  {
    goto air;
  }
  else {
    if(etat_pompe == true)
    {
      etat_pompe_old = afficher_message(LBlue, DBlue, "ON", "OFF",true, 118, 66);
    }
    else
    {
      etat_pompe_old = afficher_message(LBlue, DGreen, "OFF", "ON",false, 118, 66);
    }
  }

air:
  if (etat_ventilo == etat_ventilo_old)
  {
    goto date;
  }
  else {
    if(etat_ventilo == true)
    {
      etat_ventilo_old = afficher_message(LBlue, DBlue, "ON", "OFF",true, 118, 100);
    }
    else
    {
      etat_ventilo_old = afficher_message(LBlue, DGreen, "OFF", "ON",false, 118, 100);
    }
  }
date:
  if (date_str == date_str_old)
  {
    goto fin;
  }
  else {
    afficher_message(DGreen, White, date_str, date_str_old,true, 3, 119);
    date_str_old = date_str;
  }
fin:
  delay(3000);
}

// ====================================================================================================================================

// FONCTIONS SUPPLEMENTAIRES

int afficher_valeur(int color_bg, int color,  int f, int g, int posX, int posY) // affichage message ecran
{
  tft.setCursor(posX, posY);
  tft.setTextColor(color_bg);
  tft.setTextSize(2);
  tft.print(int(g));
  tft.setCursor(posX, posY);
  tft.setTextColor(color);
  tft.print(int(f));
  tft.setTextSize(1);
  return f;
}

boolean afficher_message(int color_bg, int color,  String message, String message_old, boolean var, int posX, int posY)
{
  tft.setCursor(posX, posY);
  tft.setTextColor(color_bg);
  tft.print(message_old);
  tft.setCursor(posX, posY);
  tft.setTextColor(color);
  tft.print(message);
  return var;
}

int niveau_cm(float x) // fonction de conversion signal/cm capteur d'eau
{
  int y;
  if(x<650){
    return 0;
  }
  if(650 <= x & x < 700){
    y = 1;
  }
  if(700 <= x & x < 750){
    y = 2;
  }
  if(750 <= x & x < 800){
    y = 3;
  }
  if(800 <= x & x < 850){
    y = 4;
  }
  if(850 <= x){
    y = 5;
  }
  return y;
}

String relayState(boolean x) // fonction de conversion etat des relais
{
  switch (x)
  {
  case true: 
    return "ON"; 
    break;
  case false: 
    return "OFF"; 
    break;
  default: 
    return "DEF_ON"; 
    break;
  }
}

String jours(int x)
{
  switch (x)
  {
  case 1: 
    return "lun. "; 
    break;
  case 2: 
    return "mar. "; 
    break;
  case 3: 
    return "mer. "; 
    break;
  case 4: 
    return "jeu. "; 
    break;
  case 5: 
    return "ven. "; 
    break;
  case 6: 
    return "sam. "; 
    break;
  case 7: 
    return "dim. "; 
    break;
  }

}

String return_saison()
{
  if(saison == true){
    return "ete";
  }
  else
  {
    return "hiver";
  }

}


