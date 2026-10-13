// ============================================================================
// profil.h - oyuncu PROFILI (basarimlar, rekorlar, dil) ve satir-satir okuyucu
// ============================================================================
// Profil, oyun kaydindan (borsa_kayit.sav) AYRI bir dosyadir (borsa_profil.sav):
// "Yeni Oyun" ya da baska bir kaydi yuklemek basarimlari ve rekorlari silmez.

// Satir-satir metin okuyucu. Istisna (exception) KULLANMAZ: web surumunde
// istisnalar varsayilan olarak kapalidir ve stoi/stof'un firlattigi bir hata
// oyunu dogrudan cokertirdi. Bozuk/eksik bir satirda `hata` bayragi kalkar,
// cagiran taraf en sonda buna bakip yuklemeyi toptan reddeder.
struct SatirOkuyucu {
    istringstream akis;
    bool hata = false;

    explicit SatirOkuyucu(const string& icerik) : akis(icerik) {}

    string Str() {
        string s;
        if (!std::getline(akis, s)) {
            hata = true;
            return "";
        }
        if (!s.empty() && s.back() == '\r') s.pop_back(); // Windows satir sonu
        return s;
    }
    float Float() {
        string s = Str();
        if (hata) return 0.0f;
        char* son = nullptr;
        float v = strtof(s.c_str(), &son);
        if (son == s.c_str() || !isfinite(v)) {
            hata = true;
            return 0.0f;
        }
        return v;
    }
    int Int() {
        string s = Str();
        if (hata) return 0;
        char* son = nullptr;
        long v = strtol(s.c_str(), &son, 10);
        if (son == s.c_str()) {
            hata = true;
            return 0;
        }
        return (int)v;
    }
    bool Bool() { return Int() != 0; }
    // 64 bit isaretsiz sayi (rastgele sayi ureteci durumu icin).
    uint64_t U64() {
        string s = Str();
        if (hata) return 0;
        char* son = nullptr;
        unsigned long long v = strtoull(s.c_str(), &son, 10);
        if (son == s.c_str()) {
            hata = true;
            return 0;
        }
        return (uint64_t)v;
    }
    // Okunacak satir kaldi mi? (sona eklenen istege bagli alanlar icin)
    bool Kaldi() {
        akis >> std::ws;
        return !akis.eof() && akis.good();
    }
};

struct Profil {
    bool basarim[BASARIM_SAYISI] = {};
    int rekorGun[SENARYO_SAYISI] = {};   // zirveye en kisa surede ulasilan gun (0 = henuz yok)
    float rekorNet[SENARYO_SAYISI] = {}; // sureli senaryoda en yuksek net deger (0 = henuz yok)
    bool rehberGoruldu = false;
    int dil = -1; // -1 = henuz secilmedi (ilk acilista sistem diline bakilir)
    // ---- v2.1 ile eklenenler (dosyanin SONUNDA; eski profillerde yoktur) ----
    bool muzikAcik = true;
    bool renkKoru = false;
    int gunlukTarih = 0;      // Gunun Piyasasi: en son oynanan gun (YYYYAAGG)
    float gunlukSkor = 0.0f;  // o gunun en iyi skoru
};

static Profil g_profil;
// Testlerde false yapilir: botlarin actigi basarimlar gercek profil dosyasina yazilmasin.
static bool g_profilKalici = true;

static const char* PROFIL_ETIKETI = "BORSAPATRONU_PROFIL_V1";

static void ProfilKaydet() {
    if (!g_profilKalici) return;
    ostringstream f;
    f.precision(9);
    f << PROFIL_ETIKETI << "\n";
    f << g_profil.dil << "\n";
    f << (g_profil.rehberGoruldu ? 1 : 0) << "\n";
    f << (int)BASARIM_SAYISI << "\n";
    for (int i = 0; i < BASARIM_SAYISI; i++) f << (g_profil.basarim[i] ? 1 : 0) << "\n";
    f << (int)SENARYO_SAYISI << "\n";
    for (int i = 0; i < SENARYO_SAYISI; i++) f << g_profil.rekorGun[i] << "\n" << g_profil.rekorNet[i] << "\n";
    f << (g_profil.muzikAcik ? 1 : 0) << "\n";
    f << (g_profil.renkKoru ? 1 : 0) << "\n";
    f << g_profil.gunlukTarih << "\n";
    f << g_profil.gunlukSkor << "\n";
    DepoYaz(PROFIL_DOSYA_ADI, f.str());
}

// Dosya yoksa/bozuksa profil varsayilan haliyle kalir (hata vermez).
// Basarim/senaryo SAYISI dosyada yazili oldugu icin, ileride sona yeni basarim
// ya da senaryo eklenirse eski profil dosyalari sorunsuz okunur.
static void ProfilYukle() {
    string icerik;
    if (!DepoOku(PROFIL_DOSYA_ADI, icerik)) return;
    SatirOkuyucu r(icerik);
    if (r.Str() != PROFIL_ETIKETI) return;

    Profil yeni;
    yeni.dil = r.Int();
    yeni.rehberGoruldu = r.Bool();
    int basarimSayisi = r.Int();
    if (r.hata || basarimSayisi < 0 || basarimSayisi > 1000) return;
    for (int i = 0; i < basarimSayisi; i++) {
        bool v = r.Bool();
        if (i < BASARIM_SAYISI) yeni.basarim[i] = v;
    }
    int senaryoSayisi = r.Int();
    if (r.hata || senaryoSayisi < 0 || senaryoSayisi > 1000) return;
    for (int i = 0; i < senaryoSayisi; i++) {
        int g = r.Int();
        float n = r.Float();
        if (i < SENARYO_SAYISI) {
            yeni.rekorGun[i] = max(0, g);
            yeni.rekorNet[i] = fmaxf(0.0f, n);
        }
    }
    if (r.hata) return;
    // v2.1 alanlari: eski profillerde yok, o zaman varsayilanda kalir.
    if (r.Kaldi()) {
        yeni.muzikAcik = r.Bool();
        yeni.renkKoru = r.Bool();
        yeni.gunlukTarih = r.Int();
        yeni.gunlukSkor = fmaxf(0.0f, r.Float());
        if (r.hata) return;
    }
    if (yeni.dil != 0 && yeni.dil != 1) yeni.dil = -1;
    g_profil = yeni;
}

static int AcilanBasarimSayisi() {
    int n = 0;
    for (int i = 0; i < BASARIM_SAYISI; i++)
        if (g_profil.basarim[i]) n++;
    return n;
}

static void BasarimVer(Oyun& o, int id) {
    if (id < 0 || id >= BASARIM_SAYISI || g_profil.basarim[id]) return;
    g_profil.basarim[id] = true;
    ProfilKaydet();
    o.toastlar.Ekle(M("BAŞARIM AÇILDI!", "ACHIEVEMENT UNLOCKED!"), BASARIMLAR[id].ad.al(), RENK_ALTIN, 6.0f);
    o.parcaciklar.Patlat({(float)PENCERE_GENISLIK - 170.0f, 150.0f}, RENK_ALTIN, 30, 220.0f);
    Cal(o.sesler.basarim);
}

// Zirveye ulasma suresi rekoru (az olan iyi). Yeni rekorsa true doner.
static bool RekorGunGuncelle(int senaryo, int gun) {
    if (senaryo < 0 || senaryo >= SENARYO_SAYISI || gun <= 0) return false;
    if (g_profil.rekorGun[senaryo] == 0 || gun < g_profil.rekorGun[senaryo]) {
        g_profil.rekorGun[senaryo] = gun;
        ProfilKaydet();
        return true;
    }
    return false;
}

// Sureli senaryo net deger rekoru (cok olan iyi). Yeni rekorsa true doner.
static bool RekorNetGuncelle(int senaryo, float net) {
    if (senaryo < 0 || senaryo >= SENARYO_SAYISI) return false;
    if (net > g_profil.rekorNet[senaryo]) {
        g_profil.rekorNet[senaryo] = net;
        ProfilKaydet();
        return true;
    }
    return false;
}

// Gunun Piyasasi skoru: rekor O GUNE aittir; yeni bir gunde sifirdan baslar.
static bool GunlukSkorGuncelle(int tarih, float skor) {
    if (tarih <= 0) return false;
    if (g_profil.gunlukTarih != tarih || skor > g_profil.gunlukSkor) {
        g_profil.gunlukTarih = tarih;
        g_profil.gunlukSkor = skor;
        ProfilKaydet();
        return true;
    }
    return false;
}

// Bugun icin kayitli en iyi skor (bugun hic oynanmadiysa 0).
static float BugunkuGunlukSkor() {
    return (g_profil.gunlukTarih == BugunTarih()) ? g_profil.gunlukSkor : 0.0f;
}

static void DilAyarla(Dil d) {
    g_dil = d;
    g_profil.dil = (int)d;
    ProfilKaydet();
}
