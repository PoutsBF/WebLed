/******************************************************************************
    Gestion des Boutons Poussoirs                   Stéphane Lepoutère (c) 2020

    Observe l'arrivée de changement d'état sur les BP (interruption)
    Gère le débounce
    Détecte les appuis courts, longs, doubles clics et l'état instantané
    2020-11-07 : à faire : modification de la philosophie du double clic :
        un double clic est considéré comme un clic qui arrive dans un délai 
        court après le précédent. Le premier clic est sensé avoir été pris 
        en compte.
*/

#include <Arduino.h>
#include <gestionBP.h>

//GestionBP::GestionBP(void){}
uint8_t GestionBP::pile_pos_entree = 0;
uint8_t GestionBP::pile_pos_sortie = 0;
BP_struct_pile GestionBP::pile[] = {0xFF, 0xFF, 0xFF};

void GestionBP::init(void)
{
    pinMode(BP0_pin, INPUT_PULLUP);
    pinMode(BP1_pin, INPUT_PULLUP);
    pinMode(BP2_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BP0_pin), interruption, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BP1_pin), interruption, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BP2_pin), interruption, CHANGE);

    for (int i = 0; i < BP_PILE_TAILLE; i++)
    {
        pile[i].idBP = 0xFF;
    }
}

ICACHE_RAM_ATTR void GestionBP::interruption(void)
{
    static uint8_t BP0_p                = 0;
    static unsigned long BP0_rebond     = 0;
    static uint8_t BP1_p                = 0;
    static unsigned long BP1_rebond     = 0;
    static uint8_t BP2_p                = 0;
    static unsigned long BP2_rebond     = 0;

    unsigned long time = millis();
    uint8_t BP0_val = ! digitalRead(BP0_pin);
    uint8_t BP1_val = ! digitalRead(BP1_pin);
    uint8_t BP2_val = ! digitalRead(BP2_pin);

    if ((BP0_p != BP0_val) && ((time - BP0_rebond) > BP_delai_rebond))
    {
        BP0_p = BP0_val;
        BP0_rebond = time;
        push(0, BP0_val, time);
    }
    else if ((BP1_p != BP1_val) && ((time - BP1_rebond) > BP_delai_rebond))
    {
        BP1_p = BP1_val;
        BP1_rebond = time;
        push(1, BP1_val, time);
    }
    else if ((BP2_p != BP2_val) && ((time - BP2_rebond) > BP_delai_rebond))
    {
        BP2_p = BP2_val;
        BP2_rebond = time;
        push(2, BP2_val, time);
    }
}

void GestionBP::push(uint8_t idBP, uint8_t etat, unsigned long time)
{
    if(pile[pile_pos_entree].idBP == 0xFF)
    {
        pile[pile_pos_entree].idBP = idBP;
        pile[pile_pos_entree].etat = etat;
        pile[pile_pos_entree].time = time;
        pile_pos_entree++;
        if(pile_pos_entree == BP_PILE_TAILLE)
            pile_pos_entree = 0;
    }
}

uint8_t GestionBP::pop(uint8_t *idBP, uint8_t *etat, unsigned long *time)
{
    if (pile[pile_pos_sortie].idBP != 0xFF)
    {
        *idBP = pile[pile_pos_sortie].idBP;
        pile[pile_pos_sortie].idBP = 0xFF;
        *etat = pile[pile_pos_sortie].etat;
        *time = pile[pile_pos_sortie].time;
        pile_pos_sortie++;
        if (pile_pos_sortie == BP_PILE_TAILLE)
            pile_pos_sortie = 0;

        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t GestionBP::handle(BP_struct_msg *msg)
{
    BP_struct_pile pile_traitement;
    uint8_t retour = 0;

    msg->idBP = 0xFF;

    noInterrupts();
    uint8_t mess_pop = pop(&pile_traitement.idBP, &pile_traitement.etat, &pile_traitement.time);
    interrupts();

    if (mess_pop)
    {
        switch (pile_traitement.idBP)
        {
            case 0:                 // BP 1 à traiter
            {
                static unsigned long delta = 0;
                static unsigned long delta_dbl = 0;
                static uint8_t flag_double = 0;
                if (pile_traitement.etat)       // Front montant
                {
                    if (flag_double)
                    {
                        flag_double = 0;
                    }
                    else
                    {
                        flag_double = (pile_traitement.time - delta_dbl > BP_delai_double) ? 0 : 1;
                    }

                    delta_dbl = pile_traitement.time;
                    delta = pile_traitement.time;
                }
                else                            // Front descendant
                {
                    if (pile_traitement.time - delta > BP_delai_appuie_court)
                    {           // si c'est un appuie long ?
                        msg->idMsg = BP_MESS_APPUIE_LONG;
                    }
                    else
                    {           // Si cest un appuie court ?
                        msg->idMsg = (flag_double) ? BP_MESS_APPUIE_DOUBLE : BP_MESS_APPUIE_COURT;
                    }
                    msg->idBP = 0;
                    retour = 1;        // envoyer le message appuie court / long / double sur le BP0
                }
            } break;
            case 1:                 // BP 1 à traiter
            {
                static unsigned long delta = 0;
                static unsigned long delta_dbl = 0;
                static uint8_t flag_double = 0;
                if (pile_traitement.etat)       // Front montant
                {
                    if (flag_double)
                    {
                        flag_double = 0;
                    }
                    else
                    {
                        flag_double = (pile_traitement.time - delta_dbl > BP_delai_double) ? 0 : 1;
                    }

                    delta_dbl = pile_traitement.time;
                    delta = pile_traitement.time;
                }
                else                            // Front descendant
                {
                    if (pile_traitement.time - delta > BP_delai_appuie_court)
                    {           // si c'est un appuie long ?
                        msg->idMsg = BP_MESS_APPUIE_LONG;
                    }
                    else
                    {           // Si cest un appuie court ?
                        msg->idMsg = (flag_double) ? BP_MESS_APPUIE_DOUBLE : BP_MESS_APPUIE_COURT;
                    }
                    msg->idBP = 1;
                    retour = 1;        // envoyer le message appuie court / long / double sur le BP1
                }
            } break;
            case 2:                 // BP 2 à traiter
            {
                static unsigned long delta = 0;
                static unsigned long delta_dbl = 0;
                static uint8_t flag_double = 0;
                if (pile_traitement.etat)       // Front montant
                {
                    if(flag_double)
                    {
                        flag_double = 0;
                    }
                    else
                    {
                        flag_double = (pile_traitement.time - delta_dbl > BP_delai_double) ? 0 : 1;
                    }

                    delta_dbl = pile_traitement.time;
                    delta = pile_traitement.time;
                }
                else                            // Front descendant
                {
                    if (pile_traitement.time - delta > BP_delai_appuie_court)
                    {           // si c'est un appuie long ?
                        msg->idMsg = BP_MESS_APPUIE_LONG;
                    }
                    else
                    {           // Si cest un appuie court ?
                        msg->idMsg = (flag_double) ? BP_MESS_APPUIE_DOUBLE : BP_MESS_APPUIE_COURT;
                    }
                    msg->idBP = 2;
                    retour = 1;        // envoyer le message appuie court / long / double sur le BP2
                }
            } break;
        }
    }
    return retour;
}

//-------------------------------------------------- That's all, Folks !!! ----