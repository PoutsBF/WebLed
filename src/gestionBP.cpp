/******************************************************************************
    Gestion des Boutons Poussoirs                   Stéphane Lepoutère (c) 2020

    Observe l'arrivée de changement d'état sur les BP (interruption)
    Gère le débounce
    Détecte les appuis courts, longs, doubles clics et l'état instantané
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
    // DEBUG
    pinMode(D2, OUTPUT);
    pinMode(D3, OUTPUT);
}

ICACHE_RAM_ATTR void GestionBP::interruption(void)
{
    static uint8_t BP0_p                = 0;
    static unsigned long BP0_timer_p    = 0;
    static unsigned long BP0_rebond     = 0;
    static uint8_t BP1_p                = 0;
    static unsigned long BP1_timer_p    = 0;
    static unsigned long BP1_rebond     = 0;
    static uint8_t BP2_p                = 0;
    static unsigned long BP2_timer_p    = 0;
    static unsigned long BP2_rebond     = 0;

    unsigned long BP_delta;
    uint8_t BP;
    uint8_t etat;

    unsigned long time = millis();
    uint8_t BP0_val = ! digitalRead(BP0_pin);
    uint8_t BP1_val = ! digitalRead(BP1_pin);
    uint8_t BP2_val = ! digitalRead(BP2_pin);

    if ((BP0_p != BP0_val) && ((time - BP0_rebond) > BP_delai_rebond))
    {
        etat = BP0_p = BP0_val;
        BP = 0;
        BP_delta = time - BP0_timer_p;
        BP0_timer_p = time;
        BP0_rebond = time;
        push(BP, etat, BP_delta);
    }
    else if ((BP1_p != BP1_val) && ((time - BP1_rebond) > BP_delai_rebond))
    {
        etat = BP1_p = BP1_val;
        BP = 1;
        BP_delta = time - BP1_timer_p;
        BP1_timer_p = time;
        BP1_rebond = time;
        push(BP, etat, BP_delta);
    }
    else if ((BP2_p != BP2_val) && ((time - BP2_rebond) > BP_delai_rebond))
    {
        etat = BP2_p = BP2_val;
        BP = 2;
        BP_delta = time - BP2_timer_p;
        BP2_timer_p = time;
        BP2_rebond = time;
        push(BP, etat, BP_delta);
    }
}


void GestionBP::push(uint8_t idBP, uint8_t etat, unsigned long delta)
{
    if(pile[pile_pos_entree].idBP == 0xFF)
    {
        pile[pile_pos_entree].idBP = idBP;
        pile[pile_pos_entree].etat = etat;
        pile[pile_pos_entree].delta = delta;
        pile_pos_entree++;
        if(pile_pos_entree == BP_PILE_TAILLE)
            pile_pos_entree = 0;
    }
}

uint8_t GestionBP::pop(uint8_t *idBP, uint8_t *etat, unsigned long *delta)
{
    if (pile[pile_pos_sortie].idBP != 0xFF)
    {
        *idBP = pile[pile_pos_sortie].idBP;
        pile[pile_pos_sortie].idBP = 0xFF;
        *etat = pile[pile_pos_sortie].etat;
        *delta = pile[pile_pos_sortie].delta;
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
    static uint8_t mode = 0;
    static unsigned long delta_mode_2 = 0;

    BP_struct_pile pile_traitement;
    uint8_t retour = 0;

    msg->idBP = 0xFF;

    noInterrupts();
    uint8_t mess_pop = pop(&pile_traitement.idBP, &pile_traitement.etat, &pile_traitement.delta);
    interrupts();

    if (mess_pop)
    {
        switch (mode)
        {
            case 0 :
            {
                if (pile_traitement.etat == 1)      // front montant, passe au mode suivant
                {
                    mode = 1;
                }
            } break;
            case 1 :
            {
                if (pile_traitement.etat == 0)      // Front descendant, à tester suivant le delta
                {
                    if(pile_traitement.delta < BP_delai_appuie_court)
                    {
                        delta_mode_2 = millis();        // note l'heure pour ne pas attendre
                                                        // indéfiniment le prochain appuie
                        mode = 2;                       // attend pour vérifier s'il y a un second appuie
                    }
                    else                            // appuie long
                    {
                                                    // on envoie directement 
                        msg->idBP = pile_traitement.idBP;
                        msg->idMsg = BP_MESS_APPUIE_LONG;
                        retour = 1;
                                                         // repart dans le mode d'attente
                        mode = 0;
                    }                    
                }
                else                                // Erreur, on a loupé un coup...
                {
                    mode = 0;
                }
            } break;
            case 2 :
            {
                if (pile_traitement.etat == 1)      // Front ascendant, à tester suivant le delta
                {
                    if(pile_traitement.delta < BP_delai_double)
                    {
                        mode = 3;       // attend pour vérifier s'il y a un second appuie
                        Serial.println("mode 3");        // DEBUG
                    }
                    else                            // appuie long
                    {
                                                    // on envoie directement 
                        msg->idBP = pile_traitement.idBP;
                        msg->idMsg = BP_MESS_APPUIE_COURT;
                        Serial.println("appuie court");        // DEBUG
                        retour = 1;
                                                    // repart dans le mode d'attente
                        mode = 0;
                    }                    
                }
                else                                // Erreur, on a loupé un coup...
                {
                    mode = 0;
                }
            } break;
            case 3 :
            {
                if (pile_traitement.etat == 0)      // Front descendant, à tester suivant le delta
                {
                    if(pile_traitement.delta < BP_delai_appuie_court)
                    {
                        msg->idBP = pile_traitement.idBP;
                        msg->idMsg = BP_MESS_APPUIE_DOUBLE;
                        retour = 1;
                    }
                    else                            // appuie long
                    {
                                                    // on envoie directement 
                                                    // Chercher une solution pour envoyer aussi
                                                    // le premier clic...
                        msg->idBP = pile_traitement.idBP;
                        msg->idMsg = BP_MESS_APPUIE_LONG;
                        retour = 1;
                    }                    
                }
                mode = 0;        // repart dans le mode d'attente
            } break;
        }
    }
    else
    {
        // Cas particulier du mode 2 : au delà du délai, envoie un message court et repasse en mode 2
        if(mode == 2)
        {
            if ((millis() - delta_mode_2) > BP_delai_double)
            {
                // on envoie directement
                msg->idBP = pile_traitement.idBP;
                msg->idMsg = BP_MESS_APPUIE_COURT;
                retour = 1;
                // repart dans le mode d'attente
                mode = 0;
            }
        }
    }

    return retour;
}

//-------------------------------------------------- That's all, Folks !!! ----