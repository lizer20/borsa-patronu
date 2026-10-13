// ============================================================================
// arayuz_temel.h - arayuzun veri yapilari, YERLESIM ve ortak yardimcilar
// ============================================================================
// SAHNE BOYUTU DEGISKENDIR (bkz. temel.h): PENCERE_GENISLIK / PENCERE_YUKSEKLIK
// o anki sanal sahnedir. Butun dikdortgenler ArayuzKur() icinde, bu boyuta gore
// hesaplanir; boyut ya da yon (yatay/dikey) degisince ArayuzKur yeniden cagrilir.
//   YATAY  : 1280..1920 x 720. Sol panel sabit, fazla genislik grafige gider.
//   DIKEY  : telefon. Tek varlik karti + varlik secici (bkz. arayuz_dikey.h).
// Pencereler (modal) iki yonde AYNI kodu kullanir; genislikleri sahneye sigacak
// sekilde kisalir ve DarEkran() ise ic duzenleri alt alta dizilir.
//
// Dugme ETIKETLERI EtiketleriYenile() ile HER KAREDE yeniden yazilir: dil ya da
// durum degisince eski etiket kalmasi diye bir hata sinifi olamaz.
//
// YENI BIR SEY EKLERKEN: test/gorsel_test.cpp'yi calistir (python build.py test).
// Yerlesim denetcisi ust uste binen ya da panelinden tasan yazilari uc yerlesimde
// (yatay, genis, dikey) ve iki dilde otomatik raporlar.

enum class Ekran { MENU, SENARYO, BASARIM, OYUN };

// Bir karenin girdisi. Gercek oyunda GirdiOku() raylib'den doldurur; gorsel test
// sahte tiklamalar uretmek icin elle doldurur.
struct Girdi {
    Vector2 fare = {-1000.0f, -1000.0f}; // sanal sahne koordinatlarinda
    bool tiklandi = false;               // sol tus / dokunma bu karede basladi
    bool fareBasili = false;             // basili tutuluyor (kaydirici icin)
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

static const int AZAMI_LIMAN = 4;

struct Arayuz {
    // ---- yatay oyun ekrani ----
    float solX = 20.0f, solW = 380.0f;
    // Kart alani her zaman 3 kartlik yer ayirir (414 px). 1-3 hisse acikken
    // kartlar tam boy (130 px); 4-6 hissede ayni alana sigmak icin kuculur.
    float kartY0 = 154.0f, kartYukTaban = 130.0f, kartArasi = 12.0f;
    float kartAlaniYuk = 414.0f;
    float adetPanelY = 572.0f;
    int adetDegerleri[8] = {1, 5, 10, 25, 50, 100, 250, -1}; // -1 = MAKS
    Rectangle adetPanel = {};
    Rectangle grafikPanel = {};
    Rectangle limanKart[AZAMI_LIMAN] = {}; // grafik panelinin altindaki altin/dolar mini kartlari
    Rectangle insiderRect = {};
    Rectangle rutbeCubugu = {};
    Rectangle haberBandi = {};

    // ---- dikey (telefon) oyun ekrani; her karede DikeyDuzenHesapla doldurur ----
    Rectangle dSecici = {};         // varlik secici satir(lar)i
    Rectangle dKart = {};           // secili varligin karti
    vector<Rectangle> dCip;         // secici cipleri
    vector<int> dCipVarlik;         // ciplerin gosterdigi varlik indeksleri
    int seciliVarlik = -1;          // dikey yerlesimde karti gosterilen varlik

    // ---- oyun ekrani dugmeleri ----
    vector<Buton> alBtn, satBtn, emirBtn; // varlik basina (hisseler + limanlar)
    Buton adetBtn[8];
    Buton sifirlaBtn, duraklatBtn, krediBtn, grafikModBtn;

    // ---- duraklatma menusu ----
    Rectangle dmPanel = {};
    Buton devamBtn, kaydetBtn, ayarlarBtn, anaMenuBtn, cikisBtn;

    // ---- ayarlar ----
    Rectangle ayarPanel = {};
    Rectangle sesKaydirici = {};
    Buton sesBtn, muzikBtn, rehberBtn, renkKoruBtn;
    Buton modPencereliBtn, modTamEkranBtn, modKenarsizBtn, kucultBtn;
    Buton dilTrBtn, dilEnBtn, ayarlarKapatBtn;

    // ---- yonetim karari ----
    Rectangle yonPanel = {};
    Buton yonRiskliBtn, yonGuvenliBtn;

    // ---- cikis onayi ----
    Rectangle coPanel = {};
    Buton cikisKaydetBtn, cikisKaydetmeBtn, cikisVazgecBtn;

    // ---- emir penceresi ----
    Rectangle emirPanel = {};
    float emirSatirYuk = 96.0f;
    Buton emirSeviyeBtn[EMIR_TUR_SAYISI][4];
    Buton emirKaldirBtn[EMIR_TUR_SAYISI];
    Buton emirKapatBtn;

    // ---- kredi penceresi ----
    Rectangle krediPanel = {};
    Buton krediAlBtn[4], krediOdeBtn[4], krediKapatBtn;

    // ---- oyun sonu ----
    Rectangle sonPanel = {};
    Buton sonBirBtn, sonIkiBtn, sonKopyaBtn;

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
    int emirHisse = -1;             // >= 0 ise o varligin emir penceresi acik
    float sifirlaOnaySuresi = 0.0f; // "Yeni Oyun" iki asamali onay sayaci
    int senaryoOnay = -1;           // senaryo ekraninda onay bekleyen kart
    float senaryoOnaySuresi = 0.0f;
    float kopyaBildirimSuresi = 0.0f; // "Kopyalandi" yazisinin ekranda kalma suresi
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
    float simBirikim = 0.0f; // sabit adimli simulasyon icin biriken sure
    // Bu oturumda bir oyun baslatildi/yuklendi mi? (ana menude BASLA yerine
    // DEVAM ET gostermek ve cikis onayinin gerekip gerekmedigini bilmek icin)
    bool oyunVar = false;
};

// Dar sahne: pencereler ve menuler ic duzenlerini alt alta dizer. Dikey sahne
// 480..720 genisliginde, yatay sahne en az 1280 oldugu icin bu, pratikte "dikey
// yerlesim" demektir; yine de karar yone degil GENISLIGE baglidir.
static bool DarEkran() { return PENCERE_GENISLIK < 1000; }

// Yatay sahne 720'den yuksekse (kare pencere) menu ekranlarinin icerigi bu kadar
// asagi kayar; boylece icerik ustte yigilmaz, dikeyde ortalanir. Darda 0.
static float MenuKaymasi() { return DarEkran() ? 0.0f : (float)(PENCERE_YUKSEKLIK - YATAY_YUKSEKLIK) / 2.0f; }

// Sahnenin ortasinda, sahneye SIGACAK sekilde kisaltilmis bir panel.
static Rectangle OrtaliPanel(float w, float h) {
    w = fminf(w, (float)PENCERE_GENISLIK - 24.0f);
    h = fminf(h, (float)PENCERE_YUKSEKLIK - 16.0f);
    return {(PENCERE_GENISLIK - w) / 2.0f, (PENCERE_YUKSEKLIK - h) / 2.0f, w, h};
}

// ---------------------------------------------------------------------------
// YERLESIM: oyun ekrani (yatay)
// ---------------------------------------------------------------------------
static void OyunYerlesimiYatay(Arayuz& a) {
    // Kart alani 720'lik sahnede tam 3 kart boyudur; sahne daha yuksekse (16:9'dan
    // daha kare pencere) fazlasi buraya eklenir: 4-5 kart kompakta dusmeden sigar,
    // 1-3 kartta altta kalan yer portfoy ozetine gider.
    a.kartAlaniYuk = 3 * (a.kartYukTaban + a.kartArasi) - a.kartArasi + (float)(PENCERE_YUKSEKLIK - YATAY_YUKSEKLIK);
    a.adetPanelY = a.kartY0 + a.kartAlaniYuk + 4;
    a.adetPanel = {a.solX, a.adetPanelY, a.solW, 100};
    for (int i = 0; i < 8; i++)
        a.adetBtn[i].rect = {a.solX + 14 + (i % 4) * 86.0f, a.adetPanelY + 30 + (i / 4) * 34.0f, 80, 32};
    a.sifirlaBtn.rect = {a.solX + 14, a.adetPanelY + 30 + 2 * 34 + 8, a.solW - 28, 30};

    a.rutbeCubugu = {20, 72, 220, 8};
    a.haberBandi = {0, 100, (float)PENCERE_GENISLIK, 38};

    // Ust bar sag kose: Duraklat, altinda Kredi; solunda iceriden bilgi zarfi.
    a.duraklatBtn.rect = {(float)PENCERE_GENISLIK - 100, 16, 80, 30};
    a.krediBtn.rect = {(float)PENCERE_GENISLIK - 100, 56, 80, 28};
    a.insiderRect = {(float)PENCERE_GENISLIK - 148, 13, 38, 34};

    float gx = a.solX + a.solW + 20;
    a.grafikPanel = {gx, a.kartY0, (float)PENCERE_GENISLIK - gx - 20, (float)PENCERE_YUKSEKLIK - a.kartY0 - 20};
    // Grafik modu dugmesi panel basliginin SAGINDAKI bos alanda (sag ust dolu:
    // "PIYASA MODU" yazisi ve iceriden bilgi seridi orada).
    a.grafikModBtn.rect = {a.grafikPanel.x + 250, a.grafikPanel.y + 12, 170, 26};

    // Guvenli liman mini kartlari: grafik panelinin en altinda yan yana.
    int limanSayisi = min((int)LIMANLAR.size(), AZAMI_LIMAN);
    const int H = HisseSayisi();
    if (limanSayisi > 0) {
        float bosluk = 12.0f;
        float kw = (a.grafikPanel.width - 40 - (limanSayisi - 1) * bosluk) / (float)limanSayisi;
        for (int k = 0; k < limanSayisi; k++) {
            Rectangle r = {a.grafikPanel.x + 20 + k * (kw + bosluk), a.grafikPanel.y + a.grafikPanel.height - 58, kw, 46};
            a.limanKart[k] = r;
            if (H + k < (int)a.alBtn.size()) {
                a.alBtn[H + k].rect = {r.x + r.width - 122, r.y + 8, 54, 30};
                a.satBtn[H + k].rect = {r.x + r.width - 62, r.y + 8, 54, 30};
                a.emirBtn[H + k].rect = {0, 0, 0, 0};
            }
        }
    }
}

// ---------------------------------------------------------------------------
// YERLESIM: oyun ekrani (dikey) - sabit kisimlar. Secici ve kart her karede
// DikeyDuzenHesapla ile hesaplanir (acik varlik sayisina bagli).
// ---------------------------------------------------------------------------
static const float DIKEY_UST_BAR_YUK = 150.0f;
static const float DIKEY_KENAR = 12.0f;
static const float DIKEY_KART_YUK = 214.0f;
static const float DIKEY_ADET_YUK = 126.0f;
static const float DIKEY_CIP_YUK = 42.0f;

static void OyunYerlesimiDikey(Arayuz& a) {
    const float W = (float)PENCERE_GENISLIK, H = (float)PENCERE_YUKSEKLIK;
    a.duraklatBtn.rect = {W - DIKEY_KENAR - 96, 8, 96, 36};
    a.krediBtn.rect = {W - DIKEY_KENAR - 96 - 8 - 80, 8, 80, 36};
    a.insiderRect = {W - DIKEY_KENAR - 96 - 8 - 80 - 8 - 40, 9, 40, 34};
    a.rutbeCubugu = {DIKEY_KENAR, 124, 170, 8};
    a.haberBandi = {0, DIKEY_UST_BAR_YUK, W, 38};

    a.adetPanel = {DIKEY_KENAR, H - 8 - DIKEY_ADET_YUK, W - 2 * DIKEY_KENAR, DIKEY_ADET_YUK};
    float bw = (a.adetPanel.width - 20 - 3 * 8) / 4.0f;
    for (int i = 0; i < 8; i++)
        a.adetBtn[i].rect = {a.adetPanel.x + 10 + (i % 4) * (bw + 8), a.adetPanel.y + 30 + (i / 4) * 47.0f, bw, 41};
    a.sifirlaBtn.rect = {0, 0, 0, 0}; // dikeyde ekranda yok (yanlis dokunusla sifirlama olmasin; ana menuden)

    // Grafik paneli ve secici/kart her karede yeniden hesaplanir; burada makul bir baslangic.
    a.grafikPanel = {DIKEY_KENAR, DIKEY_UST_BAR_YUK + 44, W - 2 * DIKEY_KENAR, 240};
    a.grafikModBtn.rect = {a.grafikPanel.x + 10, a.grafikPanel.y + 8, 170, 30};
    for (int k = 0; k < AZAMI_LIMAN; k++) a.limanKart[k] = {0, 0, 0, 0};
}

// ---------------------------------------------------------------------------
// YERLESIM: pencereler (iki yonde ortak; dar sahnede alt alta)
// ---------------------------------------------------------------------------
static void PencereleriKur(Arayuz& a) {
    const bool dar = DarEkran();

    // ---- duraklatma menusu ----
    {
        vector<Buton*> dugmeler = {&a.devamBtn, &a.kaydetBtn, &a.ayarlarBtn, &a.anaMenuBtn};
        if (PlatformdaCikisVar()) dugmeler.push_back(&a.cikisBtn);
        const float w = 340, btnH = 52, bosluk = 16;
        int n = (int)dugmeler.size();
        // Dar sahnede istatistikler panelin ICINE girer (disinda, karartilmis
        // oyun ekraninin ustunde okunmuyordu); yatayda eskisi gibi panelin altinda.
        a.dmPanel = OrtaliPanel(w, 80 + n * btnH + (n - 1) * bosluk + (dar ? 132.0f : 56.0f));
        float y = a.dmPanel.y + 80;
        for (Buton* b : dugmeler) {
            b->rect = {a.dmPanel.x + 30, y, a.dmPanel.width - 60, btnH};
            y += btnH + bosluk;
        }
    }

    // ---- ayarlar ----
    {
        const bool pencereModu = PlatformdaCikisVar();
        a.ayarPanel = OrtaliPanel(520, pencereModu ? 560.0f : 470.0f);
        float px = a.ayarPanel.x, py = a.ayarPanel.y, pw = a.ayarPanel.width;
        float yari = (pw - 60 - 10) / 2.0f;
        a.sesBtn.rect = {px + 30, py + 90, yari, 38};
        a.muzikBtn.rect = {px + 30 + yari + 10, py + 90, yari, 38};
        a.sesKaydirici = {px + 30, py + 164, pw - 60, 14};
        a.rehberBtn.rect = {px + 30, py + 196, yari, 38};
        a.renkKoruBtn.rect = {px + 30 + yari + 10, py + 196, yari, 38};
        float y = py + 250;
        if (pencereModu) {
            float modW = (pw - 60 - 20) / 3.0f;
            a.modPencereliBtn.rect = {px + 30, y + 24, modW, 38};
            a.modTamEkranBtn.rect = {px + 30 + modW + 10, y + 24, modW, 38};
            a.modKenarsizBtn.rect = {px + 30 + 2 * (modW + 10), y + 24, modW, 38};
            a.kucultBtn.rect = {px + 30, y + 70, pw - 60, 38};
            y += 126;
        }
        // (y: "DIL" basliginin satiri)
        a.dilTrBtn.rect = {px + 30, y + 24, yari, 38};
        a.dilEnBtn.rect = {px + 30 + yari + 10, y + 24, yari, 38};
        a.ayarlarKapatBtn.rect = {px + 30, py + a.ayarPanel.height - 58, pw - 60, 40};
    }

    // ---- yonetim karari ----
    {
        a.yonPanel = OrtaliPanel(560, dar ? 380.0f : 300.0f);
        Rectangle p = a.yonPanel;
        if (dar) {
            a.yonRiskliBtn.rect = {p.x + 24, p.y + p.height - 148, p.width - 48, 56};
            a.yonGuvenliBtn.rect = {p.x + 24, p.y + p.height - 80, p.width - 48, 56};
        } else {
            float bw = (p.width - 60 - 20) / 2.0f;
            a.yonRiskliBtn.rect = {p.x + 30, p.y + 210, bw, 56};
            a.yonGuvenliBtn.rect = {p.x + 30 + bw + 20, p.y + 210, bw, 56};
        }
    }

    // ---- cikis onayi ----
    {
        a.coPanel = OrtaliPanel(600, dar ? 330.0f : 240.0f);
        Rectangle p = a.coPanel;
        if (dar) {
            a.cikisKaydetBtn.rect = {p.x + 24, p.y + 140, p.width - 48, 50};
            a.cikisKaydetmeBtn.rect = {p.x + 24, p.y + 198, p.width - 48, 50};
            a.cikisVazgecBtn.rect = {p.x + 24, p.y + 256, p.width - 48, 50};
        } else {
            float bw = (p.width - 60 - 40) / 3.0f;
            a.cikisKaydetBtn.rect = {p.x + 30, p.y + 150, bw, 54};
            a.cikisKaydetmeBtn.rect = {p.x + 30 + bw + 20, p.y + 150, bw, 54};
            a.cikisVazgecBtn.rect = {p.x + 30 + 2 * (bw + 20), p.y + 150, bw, 54};
        }
    }

    // ---- emir penceresi ----
    {
        a.emirSatirYuk = dar ? 118.0f : 96.0f;
        a.emirPanel = OrtaliPanel(640, 90 + 3 * a.emirSatirYuk + 62);
        Rectangle p = a.emirPanel;
        float kaldirW = dar ? 84.0f : 96.0f;
        float seviyeBosluk = 6.0f;
        float seviyeW = dar ? (p.width - 48 - 3 * seviyeBosluk) / 4.0f : 84.0f;
        for (int t = 0; t < EMIR_TUR_SAYISI; t++) {
            float ry = p.y + 90 + t * a.emirSatirYuk;
            float by = ry + (dar ? 66.0f : 44.0f);
            for (int k = 0; k < 4; k++) a.emirSeviyeBtn[t][k].rect = {p.x + 24 + k * (seviyeW + seviyeBosluk), by, seviyeW, dar ? 36.0f : 30.0f};
            // Dar sahnede "Kaldir" dugmesi baslik satirinin sagina, genis sahnede
            // seviye dugmelerinin sagina yerlesir.
            if (dar) a.emirKaldirBtn[t].rect = {p.x + p.width - 24 - kaldirW, ry - 4, kaldirW, 26};
            else a.emirKaldirBtn[t].rect = {p.x + p.width - 24 - kaldirW, by, kaldirW, 30};
        }
        a.emirKapatBtn.rect = {p.x + 24, p.y + p.height - 52, p.width - 48, 38};
    }

    // ---- kredi penceresi ----
    {
        a.krediPanel = OrtaliPanel(600, 440);
        Rectangle p = a.krediPanel;
        float bosluk = 8.0f;
        float bw = (p.width - 48 - 3 * bosluk) / 4.0f;
        for (int k = 0; k < 4; k++) {
            a.krediAlBtn[k].rect = {p.x + 24 + k * (bw + bosluk), p.y + 236, bw, 36};
            a.krediOdeBtn[k].rect = {p.x + 24 + k * (bw + bosluk), p.y + 312, bw, 36};
        }
        a.krediKapatBtn.rect = {p.x + 24, p.y + p.height - 56, p.width - 48, 38};
    }

    // ---- oyun sonu ----
    {
        a.sonPanel = OrtaliPanel(640, 500);
        Rectangle p = a.sonPanel;
        float kenar = dar ? 20.0f : 30.0f;
        float bw = (p.width - 2 * kenar - 16) / 2.0f;
        float by = p.y + p.height - 66;
        a.sonBirBtn.rect = {p.x + kenar, by, bw, 50};
        a.sonIkiBtn.rect = {p.x + kenar + bw + 16, by, bw, 50};
        a.sonKopyaBtn.rect = {p.x + kenar, by - 48, p.width - 2 * kenar, 38};
    }
}

// ---------------------------------------------------------------------------
// YERLESIM: ana menu, senaryo ve basarim ekranlari
// ---------------------------------------------------------------------------
static void MenuleriKur(Arayuz& a) {
    const float W = (float)PENCERE_GENISLIK, H = (float)PENCERE_YUKSEKLIK;
    const bool dar = DarEkran();

    // ana menu: dugme sutunu dikeyde ekranin ortasina yakin
    {
        float x = W / 2.0f - 130;
        float y0 = dar ? fmaxf(300.0f, H * 0.36f) : 300.0f + MenuKaymasi();
        a.basBtn.rect = {x, y0, 260, 56};
        a.senaryoBtn.rect = {x, y0 + 68, 260, 42};
        a.yukleBtn.rect = {x, y0 + 120, 260, 42};
        a.basarimBtn.rect = {x, y0 + 172, 260, 42};
        a.cikisMenuBtn.rect = {x, y0 + 224, 260, 42};
        a.dilBtn.rect = {W - 190, dar ? 12.0f : 20.0f, 170, 34};
    }

    // senaryo ekrani: yatayda yan yana kartlar, dikeyde alt alta satirlar
    if (dar) {
        // Satirlar basligin altindan "Geri" dugmesinin ustune kadar olan alani
        // paylasir (en kisa dikey sahne 900 px: satir basina 147 px kalir).
        const float bosluk = 8, y0 = 70;
        const float satirH = fminf(150.0f, floorf((H - 62 - y0 - (SENARYO_SAYISI - 1) * bosluk) / (float)SENARYO_SAYISI));
        for (int i = 0; i < SENARYO_SAYISI; i++) {
            a.senaryoKart[i] = {12, y0 + i * (satirH + bosluk), W - 24, satirH};
            a.senaryoBaslaBtn[i].rect = {W - 12 - 14 - 118, a.senaryoKart[i].y + satirH - 46, 118, 36};
        }
        a.geriBtn.rect = {12, H - 52, 130, 40};
    } else {
        const float bosluk = 14, h = 430;
        float w = fminf(250.0f, (W - 40 - (SENARYO_SAYISI - 1) * bosluk) / (float)SENARYO_SAYISI);
        float x0 = (W - (SENARYO_SAYISI * w + (SENARYO_SAYISI - 1) * bosluk)) / 2.0f;
        for (int i = 0; i < SENARYO_SAYISI; i++) {
            a.senaryoKart[i] = {x0 + i * (w + bosluk), 130 + MenuKaymasi(), w, h};
            a.senaryoBaslaBtn[i].rect = {a.senaryoKart[i].x + 16, a.senaryoKart[i].y + h - 60, w - 32, 44};
        }
        a.geriBtn.rect = {x0, 640 + MenuKaymasi(), 170, 44};
    }
}

// Butun yerlesimi o anki sahne boyutuna gore kurar. Dugme nesneleri (ve hover
// animasyonlari) korunur; sadece konum ve renkleri yeniden yazilir. Sahne boyutu,
// yon ya da renk paleti degistiginde cagrilir.
static void ArayuzKur(Arayuz& a, int varlikSayisi) {
    if ((int)a.alBtn.size() != varlikSayisi) {
        a.alBtn.assign(varlikSayisi, Buton());
        a.satBtn.assign(varlikSayisi, Buton());
        a.emirBtn.assign(varlikSayisi, Buton());
    }
    // --- renkler (palet degismis olabilir) ---
    for (auto& b : a.alBtn) b.renk = RENK_YESIL;
    for (auto& b : a.satBtn) b.renk = RENK_KIRMIZI;
    for (auto& b : a.emirBtn) b.renk = RENK_MAVI_VURGU;
    for (auto& b : a.adetBtn) b.renk = RENK_MAVI_VURGU;
    a.sifirlaBtn.renk = a.duraklatBtn.renk = a.grafikModBtn.renk = RENK_KENARLIK;
    a.krediBtn.renk = RENK_MAVI_VURGU;
    a.devamBtn.renk = RENK_YESIL;
    a.kaydetBtn.renk = RENK_MAVI_VURGU;
    a.ayarlarBtn.renk = a.anaMenuBtn.renk = RENK_KENARLIK;
    a.cikisBtn.renk = RENK_KIRMIZI;
    a.sesBtn.renk = a.muzikBtn.renk = a.rehberBtn.renk = a.renkKoruBtn.renk = a.kucultBtn.renk = RENK_KENARLIK;
    a.modPencereliBtn.renk = a.modTamEkranBtn.renk = a.modKenarsizBtn.renk = RENK_MAVI_VURGU;
    a.dilTrBtn.renk = a.dilEnBtn.renk = RENK_MAVI_VURGU;
    a.ayarlarKapatBtn.renk = RENK_YESIL;
    a.yonRiskliBtn.renk = RENK_KIRMIZI;
    a.yonGuvenliBtn.renk = RENK_YESIL;
    a.cikisKaydetBtn.renk = RENK_YESIL;
    a.cikisKaydetmeBtn.renk = RENK_KIRMIZI;
    a.cikisVazgecBtn.renk = RENK_KENARLIK;
    for (int t = 0; t < EMIR_TUR_SAYISI; t++) {
        for (int k = 0; k < 4; k++) a.emirSeviyeBtn[t][k].renk = RENK_MAVI_VURGU;
        a.emirKaldirBtn[t].renk = RENK_KENARLIK;
    }
    a.emirKapatBtn.renk = RENK_YESIL;
    for (int k = 0; k < 4; k++) {
        a.krediAlBtn[k].renk = RENK_MAVI_VURGU;
        a.krediOdeBtn[k].renk = RENK_KENARLIK;
    }
    a.krediKapatBtn.renk = RENK_YESIL;
    a.sonBirBtn.renk = RENK_YESIL;
    a.sonIkiBtn.renk = RENK_KENARLIK;
    a.sonKopyaBtn.renk = RENK_MAVI_VURGU;
    a.rehberTamamBtn.renk = RENK_YESIL;
    a.rehberKapatBtn.renk = RENK_KENARLIK;
    a.basBtn.renk = RENK_YESIL;
    a.senaryoBtn.renk = a.yukleBtn.renk = RENK_MAVI_VURGU;
    a.basarimBtn.renk = a.dilBtn.renk = a.geriBtn.renk = RENK_KENARLIK;
    a.cikisMenuBtn.renk = RENK_KIRMIZI;

    // --- konumlar ---
    if (g_dikey) OyunYerlesimiDikey(a);
    else OyunYerlesimiYatay(a);
    PencereleriKur(a);
    MenuleriKur(a);
}

// ---------------------------------------------------------------------------
// HISSE KARTI DUZENI (yatay; hem guncelleme hem cizim ayni hesabi kullanir)
// ---------------------------------------------------------------------------
struct KartDuzeni {
    vector<int> acik; // acik HISSELERIN indeksleri, acilma sirasina gore
    int sayi = 0;
    float kartYuk = 130.0f;
    bool kompakt = false;
};

static KartDuzeni KartDuzeniHesapla(const Oyun& o, const Arayuz& a) {
    KartDuzeni kd;
    for (int i = 0; i < HisseSayisi(); i++)
        if (o.HisseAcikMi(i)) kd.acik.push_back(i);
    sort(kd.acik.begin(), kd.acik.end(),
         [&](int x, int y) { return o.hisseler[x].acilmaSeviyesi < o.hisseler[y].acilmaSeviyesi; });
    kd.sayi = (int)kd.acik.size();
    kd.kartYuk = a.kartYukTaban;
    if (kd.sayi > 0)
        kd.kartYuk = fminf(a.kartYukTaban, (a.kartAlaniYuk - (kd.sayi - 1) * a.kartArasi) / (float)kd.sayi);
    // Tam kart 130 px icin tasarlandi; ondan kisa her kart KOMPAKT cizilir.
    // Kompakt kartin denenmis boy araligi 59-95 px: sahne yuksekse (kare pencere)
    // kart bundan fazla uzamaz, artan yer altta portfoy ozetine kalir.
    kd.kompakt = kd.kartYuk < a.kartYukTaban - 0.5f;
    if (kd.kompakt) kd.kartYuk = fminf(kd.kartYuk, 95.0f);
    return kd;
}

static Rectangle KartRect(const Arayuz& a, const KartDuzeni& kd, int slot) {
    return {a.solX, a.kartY0 + slot * (kd.kartYuk + a.kartArasi), a.solW, kd.kartYuk};
}

// AL / SAT / EMIR dugmelerini gercek kart boyuna gore yerlestirir (yatay). Emirler
// henuz acilmadiysa EMIR dugmesi yoktur ve AL/SAT genis kalir.
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
static Rectangle RehberKutusuHesapla(Arayuz& a, const Oyun& o) {
    if (o.rehberAdim < 0 || o.rehberAdim >= (int)REHBER_ADIMLARI.size()) return {0, 0, 0, 0};
    float w = DarEkran() ? (float)PENCERE_GENISLIK - 24.0f : 400.0f;
    int metinYuk = SariliYukseklik(REHBER_ADIMLARI[o.rehberAdim].metin.al(), 14, (int)w - 36, 0, 19);
    float h = 44.0f + (float)metinYuk + 56.0f;
    Rectangle kutu = DarEkran() ? Rectangle{12, a.grafikPanel.y + 6, w, h}
                                : Rectangle{a.grafikPanel.x + 40, a.grafikPanel.y + 84, w, h};
    a.rehberTamamBtn.rect = {kutu.x + 18, kutu.y + h - 46, 140, 34};
    a.rehberKapatBtn.rect = {kutu.x + 168, kutu.y + h - 46, 170, 34};
    return kutu;
}

// Metni verilen genislige sigdirir; sigmiyorsa sonunu "..." ile keser.
static string Kisalt(const string& metin, int boyut, int maxGenislik) {
    if (YaziOlc(metin, boyut) <= maxGenislik) return metin;
    string s = metin;
    while (!s.empty() && YaziOlc(s + "...", boyut) > maxGenislik) {
        // UTF-8: cok baytli bir karakterin ortasinda kesme
        do { s.pop_back(); } while (!s.empty() && ((unsigned char)s.back() & 0xC0) == 0x80);
    }
    return s + "...";
}

// Degerleme etiketi (KELEPIR / UCUZ / MAKUL / PAHALI / BALON) - kucuk renkli hap.
// Cizilen etiketin genisligini doner; limanlarda hicbir sey cizmez (0).
static const char* DegerlemeAdi(int seviye) {
    switch (seviye) {
        case 0: return M("KELEPİR", "BARGAIN");
        case 1: return M("UCUZ", "CHEAP");
        case 3: return M("PAHALI", "PRICEY");
        case 4: return M("BALON", "BUBBLE");
        default: return M("MAKUL", "FAIR");
    }
}

static Color DegerlemeRengi(int seviye) {
    switch (seviye) {
        case 0: return {64, 190, 255, 255};
        case 1: return {70, 140, 190, 255};
        case 3: return {235, 150, 40, 255};
        case 4: return {230, 70, 70, 255};
        default: return {90, 98, 112, 255};
    }
}

static int DegerlemeEtiketiCiz(const Hisse& h, int x, int y, int boyut) {
    if (h.liman) return 0;
    int seviye = DegerlemeSeviyesi(h);
    const char* ad = DegerlemeAdi(seviye);
    int tw = YaziOlc(ad, boyut);
    DrawRectangleRounded({(float)x, (float)y, (float)tw + 12, (float)boyut + 6}, 0.5f, 4, Fade(DegerlemeRengi(seviye), 0.85f));
    YaziCiz(ad, x + 6, y + 3, boyut, WHITE);
    return tw + 12;
}
