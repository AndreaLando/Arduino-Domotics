#ifndef HVAC_H
#define HVAC_H

#pragma once

#include <Arduino.h>
#include <vector>
#include <functional>

// =========================
//      COSTANTI GLOBALI
// =========================
#define ISTERESI 0.5
#define ISTERESI_VENTOLA 0.3

#define DIFF_LOW 0.5
#define DIFF_MED 1.5

#define SOGLIA_DEFROST 3
#define DURATA_DEFROST_MS 300000

#define MAX_TEMPO_ON_MS 7200000   // 2 ore
#define RITARDO_MINIMO_MS 120000  // 2 minuti

#define MIN_OFF_MS 180000         // 3 minuti anti-ciclo OFF

#define TEMP_ESTERNA_MIN -7
#define TEMP_ESTERNA_MAX 45

#define TEMPO_FINESTRA_MS 15000

#define POST_CIRCOLAZIONE_MS 60000
#define MIN_CICLO_CIRCOLATORE_MS 5000


// =========================
//          ZONA
// =========================
class Zona {
public:
    Zona(String n, float sp, int fancoil);

    void aggiornaTemperatura(float t);
    void setSetpoint(float sp);

    bool richiedeCaldo();
    bool richiedeFreddo();

    int getNumeroFancoil();
    float getTemperatura();
    float getSetpoint();
    String getNome();

    bool statoFancoilPrecedente = false;

private:
    void aggiornaRichieste();

    String nome;
    float temperatura;
    float setpoint;
    int numeroFancoil;

    bool richiestaCaldo;
    bool richiestaFreddo;
};


// =========================
//     POMPA DI CALORE
// =========================
class PompaDiCalore {
public:

    enum Modalita { SPENTO, MANUALE, INVERNO, ESTATE, AUTO, DEFROST };
    enum VelocitaVentola { LOW, MED, HIGH };

    PompaDiCalore(Modalita m, float sp);

    void setModalita(Modalita m);
    void setSetpoint(float sp);

    void aggiornaTemperaturaInterna(float t);
    void aggiornaTemperaturaEsterna(float t);

    void aggiungiZona(Zona* z);

    void setFinestraAperta(bool stato);

    void setCallbackCompressore(std::function<void(bool)> cb) { callbackCompressore = cb; }
    void setCallbackFancoil(std::function<void(String,int,bool)> cb) { callbackFancoil = cb; }
    void setCallbackCircolatore(std::function<void(bool)> cb) { callbackCircolatore = cb; }

    bool isCompressoreAttivo();
    VelocitaVentola getVelocitaVentola();
    Modalita getModalita();
    float getSetpoint();
    float getTemperaturaInterna();
    float getTemperaturaEsterna();

private:
    void controllo();
    void controlloRiscaldamento();
    void controlloRaffrescamento();

    void valutaRichiesteZone(bool &richiestaCaldo, bool &richiestaFreddo);
    void aggiornaFancoil(Modalita effettiva);

    void aggiornaVelocitaVentola(Modalita effettiva);
    void setVelocitaVentola(VelocitaVentola v);

    bool ritardoTrascorso();
    bool puoAccendersi();

    void attivaCompressore();
    void disattivaCompressore();
    void spegniCompressoreForzato();

    void avviaDefrost();
    void gestisciDefrost();

    void gestisciCircolatore();
    void attivaCircolatore();
    void disattivaCircolatore();

    void attivaSicurezza();
    void disattivaSicurezza();

    Modalita modalita;
    float setpoint;

    float temperaturaAmbiente;
    float temperaturaEsterna;

    bool compressoreAttivo;
    VelocitaVentola velocitaVentola;

    bool sicurezzaAttiva;

    unsigned long tempoUltimoCambioStato;
    unsigned long tempoAccensione;

    unsigned long inizioDefrost;

    bool finestraAperta = false;
    unsigned long tempoInizioFinestra = 0;

    bool circolatoreAttivo = false;
    unsigned long tempoUltimoCambioCircolatore = 0;

    std::vector<Zona*> zone;

    std::function<void(bool)> callbackCompressore;
    std::function<void(String,int,bool)> callbackFancoil;
    std::function<void(bool)> callbackCircolatore;
};

#endif
