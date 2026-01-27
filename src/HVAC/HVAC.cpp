#include "HVAC.h"

// =====================================================
//                     CLASSE ZONA
// =====================================================

Zona::Zona(String n, float sp, int fancoil)
    : nome(n), temperatura(20.0), setpoint(sp),
      numeroFancoil(fancoil), richiestaCaldo(false),
      richiestaFreddo(false), statoFancoilPrecedente(false) {}

void Zona::aggiornaTemperatura(float t) {
    temperatura = t;
    aggiornaRichieste();
}

void Zona::setSetpoint(float sp) {
    setpoint = sp;
    aggiornaRichieste();
}

void Zona::aggiornaRichieste() {
    richiestaCaldo  = (temperatura < setpoint - ISTERESI);
    richiestaFreddo = (temperatura > setpoint + ISTERESI);
}

bool Zona::richiedeCaldo()  { return richiestaCaldo; }
bool Zona::richiedeFreddo() { return richiestaFreddo; }

int Zona::getNumeroFancoil() { return numeroFancoil; }
float Zona::getTemperatura() { return temperatura; }
float Zona::getSetpoint() { return setpoint; }
String Zona::getNome() { return nome; }


// =====================================================
//               CLASSE POMPA DI CALORE
// =====================================================

PompaDiCalore::PompaDiCalore(Modalita m, float sp)
    : modalita(m), setpoint(sp), temperaturaAmbiente(20.0),
      temperaturaEsterna(10.0), compressoreAttivo(false),
      velocitaVentola(LOW), sicurezzaAttiva(false),
      tempoUltimoCambioStato(0), tempoAccensione(0),
      inizioDefrost(0), finestraAperta(false),
      tempoInizioFinestra(0), circolatoreAttivo(false),
      tempoUltimoCambioCircolatore(0) {}

void PompaDiCalore::setModalita(Modalita m) { modalita = m; }
void PompaDiCalore::setSetpoint(float sp) { setpoint = sp; }

void PompaDiCalore::aggiornaTemperaturaInterna(float t) {
    temperaturaAmbiente = t;
    controllo();
}

void PompaDiCalore::aggiornaTemperaturaEsterna(float t) {
    temperaturaEsterna = t;
    controllo();
}

void PompaDiCalore::aggiungiZona(Zona* z) {
    zone.push_back(z);
}

void PompaDiCalore::setFinestraAperta(bool stato) {
    if (stato && !finestraAperta)
        tempoInizioFinestra = millis();
    finestraAperta = stato;
}


// =====================================================
//                 LOGICA PRINCIPALE
// =====================================================

void PompaDiCalore::controllo() {

    // ---------------------------
    // Protezione temperatura esterna
    // ---------------------------
    if (temperaturaEsterna < TEMP_ESTERNA_MIN ||
        temperaturaEsterna > TEMP_ESTERNA_MAX) {

        sicurezzaAttiva = true;
        spegniCompressoreForzato();
        attivaCircolatore();
        return;
    }

    // Se rientra nei limiti → disattiva sicurezza
    if (sicurezzaAttiva &&
        temperaturaEsterna > TEMP_ESTERNA_MIN + 1 &&
        temperaturaEsterna < TEMP_ESTERNA_MAX - 1) {
        sicurezzaAttiva = false;
    }

    // ---------------------------
    // Finestra aperta
    // ---------------------------
    if (finestraAperta) {
        if (millis() - tempoInizioFinestra > TEMPO_FINESTRA_MS) {
            spegniCompressoreForzato();
            setVelocitaVentola(LOW);
            attivaCircolatore();
            return;
        }
    }

    // ---------------------------
    // Modalità SPENTO
    // ---------------------------
    if (modalita == SPENTO) {
        spegniCompressoreForzato();
        setVelocitaVentola(LOW);
        disattivaCircolatore();
        return;
    }

    // ---------------------------
    // Modalità MANUALE
    // ---------------------------
    if (modalita == MANUALE) {
        attivaCompressore();
        setVelocitaVentola(HIGH);
        attivaCircolatore();
        return;
    }

    // ---------------------------
    // DEFROST automatico
    // ---------------------------
    if (modalita != DEFROST && temperaturaEsterna < SOGLIA_DEFROST) {
        avviaDefrost();
    }

    if (modalita == DEFROST) {
        gestisciDefrost();
        gestisciCircolatore();
        return;
    }

    // ---------------------------
    // Modalità AUTO basata sulle zone
    // ---------------------------
    Modalita effettiva = modalita;

    bool richiestaCaldo = false;
    bool richiestaFreddo = false;
    valutaRichiesteZone(richiestaCaldo, richiestaFreddo);

    if (modalita == AUTO) {
        if (richiestaCaldo)  effettiva = INVERNO;
        if (richiestaFreddo) effettiva = ESTATE;
    }

    // ---------------------------
    // Anti-ON troppo lungo
    // ---------------------------
    if (compressoreAttivo &&
        (millis() - tempoAccensione > MAX_TEMPO_ON_MS)) {
        disattivaCompressore();
    }

    // ---------------------------
    // Logica caldo/freddo
    // ---------------------------
    if (effettiva == INVERNO)
        controlloRiscaldamento();
    else if (effettiva == ESTATE)
        controlloRaffrescamento();

    // ---------------------------
    // Ventola intelligente
    // ---------------------------
    if (effettiva == INVERNO || effettiva == ESTATE)
        aggiornaVelocitaVentola(effettiva);

    // ---------------------------
    // Fancoil
    // ---------------------------
    aggiornaFancoil(effettiva);

    // ---------------------------
    // Circolatore
    // ---------------------------
    gestisciCircolatore();
}


// =====================================================
//         RISCLADAMENTO / RAFFRESCAMENTO
// =====================================================

void PompaDiCalore::controlloRiscaldamento() {
    if (temperaturaAmbiente < setpoint - ISTERESI)
        attivaCompressore();
    else if (temperaturaAmbiente > setpoint + ISTERESI)
        disattivaCompressore();
}

void PompaDiCalore::controlloRaffrescamento() {
    if (temperaturaAmbiente > setpoint + ISTERESI)
        attivaCompressore();
    else if (temperaturaAmbiente < setpoint - ISTERESI)
        disattivaCompressore();
}


// =====================================================
//                     MULTIZONA
// =====================================================

void PompaDiCalore::valutaRichiesteZone(bool &richiestaCaldo, bool &richiestaFreddo) {
    richiestaCaldo = false;
    richiestaFreddo = false;

    for (Zona* z : zone) {
        if (z->richiedeCaldo())  richiestaCaldo = true;
        if (z->richiedeFreddo()) richiestaFreddo = true;
    }
}

void PompaDiCalore::aggiornaFancoil(Modalita effettiva) {
    for (Zona* z : zone) {

        bool stato = false;

        if (effettiva == INVERNO && z->richiedeCaldo())
            stato = true;
        else if (effettiva == ESTATE && z->richiedeFreddo())
            stato = true;

        if (callbackFancoil && stato != z->statoFancoilPrecedente)
            callbackFancoil(z->getNome(), z->getNumeroFancoil(), stato);

        z->statoFancoilPrecedente = stato;
    }
}


// =====================================================
//                        VENTOLA
// =====================================================

void PompaDiCalore::aggiornaVelocitaVentola(Modalita effettiva) {
    float diff = 0;

    if (effettiva == INVERNO)
        diff = setpoint - temperaturaAmbiente;
    else if (effettiva == ESTATE)
        diff = temperaturaAmbiente - setpoint;

    VelocitaVentola nuova = velocitaVentola;

    if (diff < DIFF_LOW - ISTERESI_VENTOLA)
        nuova = LOW;
    else if (diff < DIFF_MED - ISTERESI_VENTOLA)
        nuova = MED;
    else if (diff > DIFF_MED + ISTERESI_VENTOLA)
        nuova = HIGH;

    setVelocitaVentola(nuova);
}

void PompaDiCalore::setVelocitaVentola(VelocitaVentola v) {
    if (velocitaVentola != v)
        velocitaVentola = v;
}


// =====================================================
//                     COMPRESSORE
// =====================================================

bool PompaDiCalore::ritardoTrascorso() {
    return (millis() - tempoUltimoCambioStato) > RITARDO_MINIMO_MS;
}

bool PompaDiCalore::puoAccendersi() {
    return (millis() - tempoUltimoCambioStato) > MIN_OFF_MS;
}

void PompaDiCalore::attivaCompressore() {
    if (!compressoreAttivo && ritardoTrascorso() && puoAccendersi()) {
        compressoreAttivo = true;
        tempoUltimoCambioStato = millis();
        tempoAccensione = millis();
        if (callbackCompressore) callbackCompressore(true);
    }
}

void PompaDiCalore::disattivaCompressore() {
    if (compressoreAttivo && ritardoTrascorso()) {
        compressoreAttivo = false;
        tempoUltimoCambioStato = millis();
        if (callbackCompressore) callbackCompressore(false);
    }
}

void PompaDiCalore::spegniCompressoreForzato() {
    if (compressoreAttivo) {
        compressoreAttivo = false;
        if (callbackCompressore) callbackCompressore(false);
    }
}


// =====================================================
//                        DEFROST
// =====================================================

void PompaDiCalore::avviaDefrost() {
    modalita = DEFROST;
    inizioDefrost = millis();
    spegniCompressoreForzato();
    setVelocitaVentola(MED);
    attivaCircolatore();
}

void PompaDiCalore::gestisciDefrost() {
    if (millis() - inizioDefrost > DURATA_DEFROST_MS) {
        modalita = AUTO;
        return;
    }
    spegniCompressoreForzato();
    setVelocitaVentola(MED);
    attivaCircolatore();
}


// =====================================================
//                     CIRCOLATORE
// =====================================================

void PompaDiCalore::gestisciCircolatore() {

    // Compressore ON → circolatore ON
    if (compressoreAttivo) {
        attivaCircolatore();
        return;
    }

    // Post-circolazione
    if (!compressoreAttivo &&
        millis() - tempoUltimoCambioStato < POST_CIRCOLAZIONE_MS) {
        attivaCircolatore();
        return;
    }

    // Altrimenti OFF
    disattivaCircolatore();
}

void PompaDiCalore::attivaCircolatore() {
    if (!circolatoreAttivo &&
        millis() - tempoUltimoCambioCircolatore > MIN_CICLO_CIRCOLATORE_MS) {

        circolatoreAttivo = true;
        tempoUltimoCambioCircolatore = millis();

        if (callbackCircolatore)
            callbackCircolatore(true);
    }
}

void PompaDiCalore::disattivaCircolatore() {
    if (circolatoreAttivo &&
        millis() - tempoUltimoCambioCircolatore > MIN_CICLO_CIRCOLATORE_MS) {

        circolatoreAttivo = false;
        tempoUltimoCambioCircolatore = millis();

        if (callbackCircolatore)
            callbackCircolatore(false);
    }
}


// =====================================================
//                        GETTER
// =====================================================

bool PompaDiCalore::isCompressoreAttivo() { return compressoreAttivo; }
PompaDiCalore::VelocitaVentola PompaDiCalore::getVelocitaVentola() { return velocitaVentola; }
PompaDiCalore::Modalita PompaDiCalore::getModalita() { return modalita; }
float PompaDiCalore::getSetpoint() { return setpoint; }
float PompaDiCalore::getTemperaturaInterna() { return temperaturaAmbiente; }
float PompaDiCalore::getTemperaturaEsterna() { return temperaturaEsterna; }
