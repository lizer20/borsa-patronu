// ============================================================================
// arayuz_temel.h - arayuzun veri yapilari, sabit YERLESIM ve dugme etiketleri
// ============================================================================
// Butun dikdortgenler (panel/dugme konumlari) ArayuzKur() icinde, tek yerde
// hesaplanir. Dugme ETIKETLERI ise EtiketleriYenile() ile HER KAREDE yeniden
// yazilir: boylece dil degisince ya da durum degisince (Duraklat <-> Devam Et)
// eski etiket kalmasi diye bir hata sinifi olamaz.
//
// YENI BIR SEY EKLERKEN: test/gorsel_test.cpp'yi calistir (python build.py test).
// Yerlesim denetcisi ust uste binen ya da panelinden tasan yazilari iki dilde de
// otomatik raporlar. Grafik panelinin SAG USTU doludur ("PIYASA MODU" yazisi ve
// iceriden bilgi seridi) - oraya dugme koyma.

enum class Ekran { MENU, SENARYO, BASARIM, OYUN };

// Bir karenin girdisi. Gercek oyunda GirdiOku() raylib'den doldurur; gorsel test
// sahte tiklamalar uretmek icin elle doldurur.
struct Girdi {
    Vector2 fare = {-1000.0f, -1000.0f}; // sanal (1280x720) koordinatlarda
    bool tiklandi = false;               // sol tus bu karede basildi
    bool fareBasili = false;             // sol tus basili tutuluyor (kaydirici icin)
    bool tusDuraklat = false;            // Bosluk / P
    int tusAdet = -1;                    // 0..7 : klavyeden 1-8
    bool tusSes = false;                 // M
    bool pencereKapatma = false;         // X / Alt+F4
};

static const Vector2 FARE_UZAK = {-1000.0f, -1000.0f};

// Emir penceresindeki hazir seviyeler (guncel fiyata gore yuzde uzaklik).
static const float EMIR_SEVIYE_STOP[4] = {0.03f, 0.05f, 0.10f, 0.15f};
static const float EMIR_SEVIYE_KAR[4] = {0.05f, 0.10f, 0.20f, 0.30f};
static const float EMIR_SEVIYE_LIMIT[4] = {0.03f, 0.05f, 0.10f, 0.15f};

// Isaretli oran: uzun pozisyonda stop ASAGIDA, kar al YUKARIDA; acik pozisyonda
// tam tersi. Limit alim her zaman asagida.
static float EmirSeviyeOrani(int tur, int k, bool acikPozisyon) {
    if (tur == EMIR_STOP) return acikPozisyon ? EMIR_SEVIYE_STOP[k] : -EMIR_SEVIYE_STOP[k];
    if (tur == EMIR_KAR_AL) return acikPozisyon ? -EMIR_SEVIYE_KAR[k] : EMIR_SEVIYE_KAR[k];
    return -EMIR_SEVIYE_LIMIT[k];
}

// Kredi penceresindeki hazir tutarlar (-1 = tamami / limitin tamami).
static const float KREDI_TUTARLARI[4] = {1000.0f, 5000.0f, 25000.0f, -1.0f};

struct Arayuz {
    // ---- sol panel yerlesimi ----
    float solX = 20.0f, solW = 380.0f;
    // Kart alani her zaman 3 kartlik yer ayirir (414 px). 1-3 hisse acikken
    // kartlar tam boy (130 px); 4-6 hissede ayni alana sigmak icin kuculur.
    float kartY0 = 154.0f, kartYukTaban = 130.0f, kartArasi = 12.0f;
    float kartAlaniYuk = 414.0f;
    float adetPanelY = 572.0f;
    int adetDegerleri[8] = {1, 5, 10, 25, 50, 100, 250, -1}; // -1 = MAKS
    Rectangle adetPanel = {};
    Rectangle grafikPanel = {};
    Rectangle insiderRect = {};
    Rectangle rutbeCubugu = {};
    Rectangle haberBandi = {};

    // ---- oyun ekrani dugmeleri ----
    vector<Buton> alBtn, satBtn, emirBtn; // hisse basina
    Buton adetBtn[8];
    Buton sifirlaBtn, duraklatBtn, krediBtn, grafikModBtn;

    // ---- duraklatma menusu ----
    Rectangle dmPanel = {};
    Buton devamBtn, kaydetBtn, ayarlarBtn, anaMenuBtn, cikisBtn;

    // ---- ayarlar ----
    Rectangle ayarPanel = {};
    Rectangle sesKaydirici = {};
    Buton sesBtn, rehberBtn, modPencereliBtn, modTamEkranBtn, modKenarsizBtn, kucultBtn;
    Buton dilTrBtn, dilEnBtn, ayarlarKapatBtn;

    // ---- yonetim karari ----
    Rectangle yonPanel = {};
    Buton yonRiskliBtn, yonGuvenliBtn;

    // ---- cikis onayi ----
    Rectangle coPanel = {};
    Buton cikisKaydetBtn, cikisKaydetmeBtn, cikisVazgecBtn;

    // ---- emir penceresi ----
    Rectangle emirPanel = {};
    Buton emirSeviyeBtn[EMIR_TUR_SAYISI][4];
    Buton emirKaldirBtn[EMIR_TUR_SAYISI];
    Buton emirKapatBtn;

    // ---- kredi penceresi ----
    Rectangle krediPanel = {};
    Buton krediAlBtn[4], krediOdeBtn[4], krediKapatBtn;

    // ---- oyun sonu ----
    Rectangle sonPanel = {};
    Buton sonBirBtn, sonIkiBtn;

    // ---- rehber ----
    Buton rehberTamamBtn, rehberKapatBtn;

    // ---- ana menu / senaryo / basarim ekranlari ----
    Buton basBtn, senaryoBtn, yukleBtn, basarimBtn, cikisMenuBtn, dilBtn;
    Rectangle senaryoKart[SENARYO_SAYISI] = {};
    Buton senaryoBaslaBtn[SENARYO_SAYISI];
    Buton geriBtn;

    // ---- gecici arayuz durumu (kaydedilmez) ----
    bool ayarlarAcik = false;
    bool cikisOnayiAcik = false;
    bool krediAcik = false;
    int emirHisse = -1;             // >= 0 ise o hissenin emir penceresi acik
    float sifirlaOnaySuresi = 0.0f; // "Yeni Oyun" iki asamali onay sayaci
    int senaryoOnay = -1;           // senaryo ekraninda onay bekleyen kart
    float senaryoOnaySuresi = 0.0f;
    string menuMesaj;
    float menuMesajSure = 0.0f;
    Color menuMesajRenk = RENK_KIRMIZI;
};

struct Uygulama {
    Oyun oyun;
    Arayuz ui;
    Ekran ekran = Ekran::MENU;
    RenderTexture2D sahne = {};
    bool cikisIstendi = false;
    float zaman = 0.0f;
    // Bu oturumda bir oyun baslatildi/yuklendi mi? (ana menude BASLA yerine
    // DEVAM ET gostermek ve cikis onayinin gerekip gerekmedigini bilmek icin)
    bool oyunVar = false;
};

// ---------------------------------------------------------------------------
// SABIT YERLESIM
// ---------------------------------------------------------------------------
static Rectangle OrtaliPanel(float w, float h) {
    return {(PENCERE_GENISLIK - w) / 2.0f, (PENCERE_YUKSEKLIK - h) / 2.0f, w, h};
}

static void ArayuzKur(Arayuz& a, int hisseSayisi) {
    a.alBtn.assign(hisseSayisi, Buton());
    a.satBtn.assign(hisseSayisi, Buton());
    a.emirBtn.assign(hisseSayisi, Buton());
    for (auto& b : a.alBtn) b.renk = RENK_YESIL;
    for (auto& b : a.satBtn) b.renk = RENK_KIRMIZI;
    for (auto& b : a.emirBtn) b.renk = RENK_MAVI_VURGU;

    a.kartAlaniYuk = 3 * (a.kartYukTaban + a.kartArasi) - a.kartArasi;
    a.adetPanelY = a.kartY0 + a.kartAlaniYuk + 4;
    a.adetPanel = {a.solX, a.adetPanelY, a.solW, 100};
    for (int i = 0; i < 8; i++) {
        a.adetBtn[i].rect = {a.solX + 14 + (i % 4) * 86.0f, a.adetPanelY + 30 + (i / 4) * 34.0f, 80, 32};
        a.adetBtn[i].renk = RENK_MAVI_VURGU;
    }
    a.sifirlaBtn.rect = {a.solX + 14, a.adetPanelY + 30 + 2 * 34 + 8, a.solW - 28, 30};
    a.sifirlaBtn.renk = RENK_KENARLIK;

    a.rutbeCubugu = {20, 72, 220, 8};
    a.haberBandi = {0, 100, (float)PENCERE_GENISLIK, 38};

    // Ust bar sag kose: Duraklat, altinda Kredi; solunda iceriden bilgi zarfi.
    a.duraklatBtn.rect = {(float)PENCERE_GENISLIK - 100, 16, 80, 30};
    a.duraklatBtn.renk = RENK_KENARLIK;
    a.krediBtn.rect = {(float)PENCERE_GENISLIK - 100, 56, 80, 28};
    a.krediBtn.renk = RENK_MAVI_VURGU;
    a.insiderRect = {(float)PENCERE_GENISLIK - 148, 13, 38, 34};

    float gx = a.solX + a.solW + 20;
    a.grafikPanel = {gx, a.kartY0, (float)PENCERE_GENISLIK - gx - 20, (float)PENCERE_YUKSEKLIK - a.kartY0 - 20};
    // Grafik ekseni dugmesi panel basliginin SAGINDAKI bos alanda (sag ust dolu).
    a.grafikModBtn.rect = {a.grafikPanel.x + 250, a.grafikPanel.y + 12, 120, 26};
    a.grafikModBtn.renk = RENK_KENARLIK;

    // ---- duraklatma menusu ----
    {
        vector<Buton*> dugmeler = {&a.devamBtn, &a.kaydetBtn, &a.ayarlarBtn, &a.anaMenuBtn};
        if (PlatformdaCikisVar()) dugmeler.push_back(&a.cikisBtn);
        const float w = 340, btnH = 52, bosluk = 16;
        int n = (int)dugmeler.size();
        float h = 80 + n * btnH + (n - 1) * bosluk + 56;
        a.dmPanel = OrtaliPanel(w, h);
        float y = a.dmPanel.y + 80;
        for (Buton* b : dugmeler) {
            b->rect = {a.dmPanel.x + 30, y, w - 60, btnH};
            b->renk = RENK_KENARLIK;
            y += btnH + bosluk;
        }
        a.devamBtn.renk = RENK_YESIL;
        a.kaydetBtn.renk = RENK_MAVI_VURGU;
        a.cikisBtn.renk = RENK_KIRMIZI;
    }

    // ---- ayarlar ----
    {
        a.ayarPanel = OrtaliPanel(520, 500);
        float px = a.ayarPanel.x, py = a.ayarPanel.y, pw = a.ayarPanel.width;
        a.sesBtn.rect = {px + 30, py + 90, 200, 38};
        a.rehberBtn.rect = {px + 250, py + 90, 240, 38};
        a.sesKaydirici = {px + 30, py + 162, pw - 60, 14};
        float modW = (pw - 60 - 20) / 3.0f;
        a.modPencereliBtn.rect = {px + 30, py + 226, modW, 38};
        a.modTamEkranBtn.rect = {px + 30 + modW + 10, py + 226, modW, 38};
        a.modKenarsizBtn.rect = {px + 30 + 2 * (modW + 10), py + 226, modW, 38};
        a.kucultBtn.rect = {px + 30, py + 272, pw - 60, 38};
        a.dilTrBtn.rect = {px + 30, py + 356, 225, 38};
        a.dilEnBtn.rect = {px + 265, py + 356, 225, 38};
        a.ayarlarKapatBtn.rect = {px + 30, py + 440, pw - 60, 40};
        a.sesBtn.renk = a.rehberBtn.renk = a.kucultBtn.renk = RENK_KENARLIK;
        a.modPencereliBtn.renk = a.modTamEkranBtn.renk = a.modKenarsizBtn.renk = RENK_MAVI_VURGU;
        a.dilTrBtn.renk = a.dilEnBtn.renk = RENK_MAVI_VURGU;
        a.ayarlarKapatBtn.renk = RENK_YESIL;
    }

    // ---- yonetim karari ----
    {
        a.yonPanel = OrtaliPanel(560, 300);
        float bw = (a.yonPanel.width - 60 - 20) / 2.0f;
        a.yonRiskliBtn.rect = {a.yonPanel.x + 30, a.yonPanel.y + 210, bw, 56};
        a.yonGuvenliBtn.rect = {a.yonPanel.x + 30 + bw + 20, a.yonPanel.y + 210, bw, 56};
        a.yonRiskliBtn.renk = RENK_KIRMIZI;
        a.yonGuvenliBtn.renk = RENK_YESIL;
    }

    // ---- cikis onayi ----
    {
        a.coPanel = OrtaliPanel(600, 240);
        float bw = (a.coPanel.width - 60 - 40) / 3.0f;
        float by = a.coPanel.y + 150;
        a.cikisKaydetBtn.rect = {a.coPanel.x + 30, by, bw, 54};
        a.cikisKaydetmeBtn.rect = {a.coPanel.x + 30 + bw + 20, by, bw, 54};
        a.cikisVazgecBtn.rect = {a.coPanel.x + 30 + 2 * (bw + 20), by, bw, 54};
        a.cikisKaydetBtn.renk = RENK_YESIL;
        a.cikisKaydetmeBtn.renk = RENK_KIRMIZI;
        a.cikisVazgecBtn.renk = RENK_KENARLIK;
    }

    // ---- emir penceresi ----
    {
        a.emirPanel = OrtaliPanel(640, 440);
        float px = a.emirPanel.x, py = a.emirPanel.y;
        for (int t = 0; t < EMIR_TUR_SAYISI; t++) {
            float ry = py + 90 + t * 96.0f;
            for (int k = 0; k < 4; k++) {
                a.emirSeviyeBtn[t][k].rect = {px + 24 + k * 90.0f, ry + 44, 84, 30};
                a.emirSeviyeBtn[t][k].renk = RENK_MAVI_VURGU;
            }
            a.emirKaldirBtn[t].rect = {px + 640 - 24 - 96, ry + 44, 96, 30};
            a.emirKaldirBtn[t].renk = RENK_KENARLIK;
        }
        a.emirKapatBtn.rect = {px + 24, py + 440 - 54, 640 - 48, 38};
        a.emirKapatBtn.renk = RENK_YESIL;
    }

    // ---- kredi penceresi ----
    {
        a.krediPanel = OrtaliPanel(600, 440);
        float px = a.krediPanel.x, py = a.krediPanel.y;
        for (int k = 0; k < 4; k++) {
            a.krediAlBtn[k].rect = {px + 24 + k * 140.0f, py + 236, 132, 34};
            a.krediOdeBtn[k].rect = {px + 24 + k * 140.0f, py + 310, 132, 34};
            a.krediAlBtn[k].renk = RENK_MAVI_VURGU;
            a.krediOdeBtn[k].renk = RENK_KENARLIK;
        }
        a.krediKapatBtn.rect = {px + 24, py + 440 - 56, 600 - 48, 38};
        a.krediKapatBtn.renk = RENK_YESIL;
    }

    // ---- oyun sonu ----
    {
        a.sonPanel = OrtaliPanel(640, 470);
        float bw = (a.sonPanel.width - 60 - 20) / 2.0f;
        float by = a.sonPanel.y + a.sonPanel.height - 74;
        a.sonBirBtn.rect = {a.sonPanel.x + 30, by, bw, 50};
        a.sonIkiBtn.rect = {a.sonPanel.x + 30 + bw + 20, by, bw, 50};
        a.sonBirBtn.renk = RENK_YESIL;
        a.sonIkiBtn.renk = RENK_KENARLIK;
    }

    a.rehberTamamBtn.renk = RENK_YESIL;
    a.rehberKapatBtn.renk = RENK_KENARLIK;

    // ---- ana menu ----
    {
        float x = PENCERE_GENISLIK / 2.0f - 130;
        a.basBtn.rect = {x, 300, 260, 56};
        a.senaryoBtn.rect = {x, 368, 260, 42};
        a.yukleBtn.rect = {x, 420, 260, 42};
        a.basarimBtn.rect = {x, 472, 260, 42};
        a.cikisMenuBtn.rect = {x, 524, 260, 42};
        a.dilBtn.rect = {(float)PENCERE_GENISLIK - 190, 20, 170, 34};
        a.basBtn.renk = RENK_YESIL;
        a.senaryoBtn.renk = a.yukleBtn.renk = RENK_MAVI_VURGU;
        a.basarimBtn.renk = a.dilBtn.renk = RENK_KENARLIK;
        a.cikisMenuBtn.renk = RENK_KIRMIZI;
    }

    // ---- senaryo ekrani ----
    {
        const float w = 285, bosluk = 16, h = 430;
        float x0 = (PENCERE_GENISLIK - (SENARYO_SAYISI * w + (SENARYO_SAYISI - 1) * bosluk)) / 2.0f;
        for (int i = 0; i < SENARYO_SAYISI; i++) {
            a.senaryoKart[i] = {x0 + i * (w + bosluk), 130, w, h};
            a.senaryoBaslaBtn[i].rect = {a.senaryoKart[i].x + 18, a.senaryoKart[i].y + h - 60, w - 36, 44};
            a.senaryoBaslaBtn[i].renk = RENK_YESIL;
        }
        a.geriBtn.rect = {x0, 640, 170, 44};
        a.geriBtn.renk = RENK_KENARLIK;
    }
}

// ---------------------------------------------------------------------------
// HISSE KARTI DUZENI (hem guncelleme hem cizim ayni hesabi kullanir)
// ---------------------------------------------------------------------------
struct KartDuzeni {
    vector<int> acik; // acik hisselerin indeksleri, acilma sirasina gore
    int sayi = 0;
    float kartYuk = 130.0f;
    bool kompakt = false;
};

static KartDuzeni KartDuzeniHesapla(const Oyun& o, const Arayuz& a) {
    KartDuzeni kd;
    for (int i = 0; i < (int)o.hisseler.size(); i++)
        if (o.HisseAcikMi(i)) kd.acik.push_back(i);
    sort(kd.acik.begin(), kd.acik.end(),
         [&](int x, int y) { return o.hisseler[x].acilmaSeviyesi < o.hisseler[y].acilmaSeviyesi; });
    kd.sayi = (int)kd.acik.size();
    kd.kartYuk = a.kartYukTaban;
    if (kd.sayi > 0)
        kd.kartYuk = fminf(a.kartYukTaban, (a.kartAlaniYuk - (kd.sayi - 1) * a.kartArasi) / (float)kd.sayi);
    kd.kompakt = kd.kartYuk < 120.0f;
    return kd;
}

static Rectangle KartRect(const Arayuz& a, const KartDuzeni& kd, int slot) {
    return {a.solX, a.kartY0 + slot * (kd.kartYuk + a.kartArasi), a.solW, kd.kartYuk};
}

// AL / SAT / EMIR dugmelerini gercek kart boyuna gore yerlestirir. Emirler henuz
// acilmadiysa EMIR dugmesi yoktur ve AL/SAT genis kalir.
static void KartDugmeleriniYerlestir(Arayuz& a, const Oyun& o, const KartDuzeni& kd) {
    const bool emirVar = o.EmirlerAcik();
    for (int slot = 0; slot < kd.sayi; slot++) {
        int i = kd.acik[slot];
        Rectangle k = KartRect(a, kd, slot);
        float btnH = kd.kompakt ? fminf(24.0f, fmaxf(15.0f, kd.kartYuk * 0.22f)) : 30.0f;
        float btnY = k.y + (kd.kompakt ? (kd.kartYuk - btnH - 2.0f) : (kd.kartYuk - 40.0f));
        if (emirVar) {
            a.alBtn[i].rect = {k.x + 14, btnY, 124, btnH};
            a.satBtn[i].rect = {k.x + 144, btnY, 124, btnH};
            a.emirBtn[i].rect = {k.x + 274, btnY, 92, btnH};
        } else {
            a.alBtn[i].rect = {k.x + 14, btnY, 160, btnH};
            a.satBtn[i].rect = {k.x + 180, btnY, 160, btnH};
            a.emirBtn[i].rect = {0, 0, 0, 0};
        }
    }
}

// ---------------------------------------------------------------------------
// REHBER KUTUSU (boyu metne/dile gore degisir; her karede hesaplanir)
// ---------------------------------------------------------------------------
static const int REHBER_KUTU_W = 400;

static Rectangle RehberKutusuHesapla(Arayuz& a, const Oyun& o) {
    if (o.rehberAdim < 0 || o.rehberAdim >= (int)REHBER_ADIMLARI.size()) return {0, 0, 0, 0};
    int metinYuk = SariliYukseklik(REHBER_ADIMLARI[o.rehberAdim].metin.al(), 14, REHBER_KUTU_W - 36, 0, 19);
    float h = 44.0f + (float)metinYuk + 56.0f;
    Rectangle kutu = {a.grafikPanel.x + 40, a.grafikPanel.y + 84, (float)REHBER_KUTU_W, h};
    a.rehberTamamBtn.rect = {kutu.x + 18, kutu.y + h - 46, 140, 32};
    a.rehberKapatBtn.rect = {kutu.x + 168, kutu.y + h - 46, 170, 32};
    return kutu;
}
