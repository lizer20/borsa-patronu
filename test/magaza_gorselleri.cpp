// ============================================================================
// magaza_gorselleri.cpp - Chrome Web Store (ve itch.io) icin GERCEK ekran goruntuleri
// ============================================================================
// Elle kurulmus sahte durum YOK: oyun Klasik senaryoda bastan baslar, denge
// testindeki Haberci botu (haberi %85 isabetle okuyan oyuncu) oynar, yonetim
// kararlarinda sansa bakar. Belirli anlarda sahne dokusu PNG olarak yazilir.
// Magazanin istedigi boyut 1280x800; oyun bu oranda (16:10) "kare" yatay
// yerlesimi kullanir, yani goruntu oyunun o pencerede GERCEKTEN gorunen halidir.
//
// Calistirma:  python build.py magaza   ->  sayfa_gorselleri/chrome/*.png

#define BORSA_ANA_YOK
#include "../borsa_patronu.cpp"
#include <cstdarg>

static void Yaz(const char* bicim, ...) { // denge_botlari.h bunu ister
    va_list a;
    va_start(a, bicim);
    vprintf(bicim, a);
    va_end(a);
}
#include "denge_botlari.h"

static const char* CIKTI = "sayfa_gorselleri/chrome/";

static void Yakala(Uygulama& u, const char* ad) {
    KareCiz(u);
    Image img = LoadImageFromTexture(u.sahne.texture);
    ImageFlipVertical(&img);
    ExportImage(img, TextFormat("%s%s", CIKTI, ad));
    UnloadImage(img);
    printf("  %s%s  (gun %d, rutbe %d, net %.0f TL)\n", CIKTI, ad, u.oyun.gun, u.oyun.rutbeIndex, u.oyun.NetDeger());
}

// Bir kare oynar: bot hamlesi + oyunun kendi guncellemesi (arayuz animasyonlari dahil).
static void Oyna(Uygulama& u, BotDurum& b) {
    const float dt = 1.0f / 60.0f;
    Oyun& o = u.oyun;
    // Bot, denge testindeki gibi 0,25 sn'de bir karar verir (yeni haberleri bu
    // pencereye gore tanir); oyun ise her karede guncellenir.
    static int kare = 0;
    if (++kare % 15 == 0) BotHamle(o, b, 0.25f);
    if (o.yonetimOlayiAcik) YonetimKarariUygula(o, YonetimBasariOlasiligi(o.hisseler[o.yonetimHisseIndex]) >= 0.55f);
    if (o.sonPenceresiAcik) o.sonPenceresiAcik = false;
    KareGuncelle(u, dt, Girdi());
}

// Ekran "okunur" bir anda mi: bildirim yok, elde pozisyon var, bantta yeni bir haber var.
static bool GuzelAn(const Oyun& o) {
    if (!o.toastlar.liste.empty() || o.bekleyenHaberler.empty()) return false;
    for (int i = 0; i < HisseSayisi(); i++)
        if (o.hisseler[i].adet > 0) return true;
    return false;
}

int main() {
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    g_profilKalici = false; // gercek profil dosyasina ASLA yazma
    Uygulama u;
    UygulamaBaslat(u);
    sesAcik = false;
    g_profil = Profil();
    g_profil.rehberGoruldu = true;
    PaletiAyarla(false);
    SahneBoyutuAyarla(u, 1280, 800, false);
    printf("Magaza gorselleri (1280x800, gercek oyun):\n");

    // 1) Ana menu
    g_dil = Dil::TR;
    u.ekran = Ekran::MENU;
    for (int i = 0; i < 90; i++) KareGuncelle(u, 1.0f / 60.0f, Girdi());
    Yakala(u, "5_menu_1280x800.png");

    // 2) Oyun: Klasik, bastan
    SetRandomSeed(2026);
    SimTohumla(20261023);
    g_botRng.Tohumla(77);
    Oyun& o = u.oyun;
    o.Baslat(S_KLASIK, false);
    o.rehberAdim = -1;
    u.ui = Arayuz();
    ArayuzKur(u.ui, (int)o.hisseler.size());
    u.ekran = Ekran::OYUN;
    u.oyunVar = true;
    BotDurum b;
    b.tur = BOT_HABERCI;

    auto oynaKadar = [&](int rutbe, int azamiGun) {
        while (o.rutbeIndex < rutbe && o.gun < azamiGun) Oyna(u, b);
        for (int k = 0; k < 60 * 60 * 3 && !GuzelAn(o); k++) Oyna(u, b); // en fazla 3 dk bekle
    };

    oynaKadar(2, 120);
    Yakala(u, "1_oyun_1280x800.png");

    oynaKadar(4, 200);
    o.grafikModu = 1; // yuzde ekseni: farkli fiyatli hisseler ayni olcekte
    for (int k = 0; k < 30; k++) Oyna(u, b);
    Yakala(u, "2_oyun_yuzde_1280x800.png");
    o.grafikModu = 0;

    // Emir penceresi: elde tutulan bir hissede (oyuncunun tiklayacagi pencere)
    for (int k = 0; k < 60 * 60 * 3; k++) {
        bool var = false;
        for (int i = 0; i < HisseSayisi(); i++)
            if (o.hisseler[i].adet > 0 && o.HisseAcikMi(i)) { u.ui.emirHisse = i; var = true; break; }
        if (var) break;
        Oyna(u, b);
    }
    Yakala(u, "3_emirler_1280x800.png");
    u.ui.emirHisse = -1;

    // Ingilizce
    // Haber bandindaki eski satirlar yazildiklari dilde kalir; bant tamamen
    // Ingilizce satirlarla yenilenene kadar oyna.
    g_dil = Dil::EN;
    const auto eskiBant = o.haberBandi;
    auto bantEski = [&]() {
        for (const auto& satir : o.haberBandi)
            for (const auto& eski : eskiBant)
                if (satir == eski) return true;
        return false;
    };
    for (int k = 0; k < 60 * 60 * 20 && (bantEski() || !GuzelAn(o)); k++) Oyna(u, b);
    for (int k = 0; k < 10; k++) Oyna(u, b);
    Yakala(u, "4_english_1280x800.png");

    UygulamaKapat(u);
    return 0;
}
