/******************************************************************************
    Gestion des Boutons Poussoirs                   Stéphane Lepoutère (c) 2020

    Observe l'arrivée de changement d'état sur les BP (interruption)
    Gère le débounce
    Détecte les appuis courts, longs, doubles clics et l'état instantané
*/

#pragma once
#ifndef GESTIONBP_H
#define GESTIONBP_H
#include <Arduino.h>

//-----------------------------------------------------------------------------
//---- Définition des constantes ----
const unsigned long BP_delai_appuie_court = 500;
const unsigned long BP_delai_appuie_long = 500;
const unsigned long BP_delai_double = 500;
const unsigned long BP_delai_rebond = 5;
enum BP_MESSAGES
{
    BP_MESS_IDLE,
    BP_MESS_APPUYE,
    BP_MESS_RELACHE,
    BP_MESS_APPUIE_COURT,
    BP_MESS_APPUIE_LONG,
    BP_MESS_APPUIE_DOUBLE
};

const uint8_t BP0_pin = D5;
const uint8_t BP1_pin = D6;
const uint8_t BP2_pin = D7;

#define BP_PILE_TAILLE      16

//-----------------------------------------------------------------------------
//---- Définition des structures ----
struct BP_struct_msg
{
    uint8_t idBP;
    uint8_t idMsg;
};

struct BP_struct_pile
{
    uint8_t idBP;
    uint8_t etat;
    unsigned long delta;
};
//-----------------------------------------------------------------------------
//---- Définition des structures ----

class GestionBP
{
    public:
//        GestionBP(void);
        void init(void);
        static ICACHE_RAM_ATTR void interruption(void);

        uint8_t handle(BP_struct_msg *msg);

    private:
        static void push(uint8_t idBP, uint8_t etat, unsigned long delta);
        uint8_t pop(uint8_t *idBP, uint8_t *etat, unsigned long *delta);

        static BP_struct_pile pile[BP_PILE_TAILLE];
        static uint8_t pile_pos_entree;
        static uint8_t pile_pos_sortie;
};

#endif