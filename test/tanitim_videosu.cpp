// ============================================================================
// tanitim_videosu.cpp - GERCEK oynanistan tanitim videosu (Chrome Web Store / YouTube)
// ============================================================================
// Oyun gizli bir pencerede acilir ve gercekten oynanir: imlec gercek dugmelere
// tiklar (sahte Girdi), aradaki uzun kisimlari denge testindeki Haberci botu
// oynar ve video bu kisimlari kararip atlar. Ustune yalnizca imlec, alt yazi ve
// gecis karartmasi cizilir. Kareler ffmpeg'e ham olarak aktarilir; ses izi
// oyunun kendi muzigidir (MuzikOrnekleri) + tiklama sesleri.
//
// Sabit tohum: Turkce ve Ingilizce videolar BIREBIR ayni oyunu gosterir.
//
// Kullanim: tanitim_videosu.exe <ffmpeg> <tr|en> <cikti.mp4>
// (python build.py video ikisini birden uretir)

#define BORSA_ANA_YOK
#include "../borsa_patronu.cpp"
#include <cstdarg>
#include <functional>
#include "rlgl.h"

static void Yaz(const char* bicim, ...) { // denge_botlari.h bunu ister
    va_list a;
    va_start(a, bicim);
    vprintf(bicim, a);
    va_end(a);
}
#include "denge_botlari.h"

static const int VW = 1280, VH = 720, FPS = 30;
static const float DT = 1.0f / FPS;

static Uygulama* U = nullptr;
static RenderTexture2D g_cikis;
static FILE* g_ffmpeg = nullptr;
static int g_kare = 0;
static vector<int> g_tikKareleri;

// Imlec sahne koordinatlarinda tutulur; cikis karesine g_ofs/g_olcek ile tasinir
// (dikey sahne telefon cercevesinin icinde kucultulmus cizilir).
static Vector2 g_imlec = {VW * 0.5f, VH * 0.6f};
static bool g_imlecVar = false;
static float g_halka = 0.0f;
static Vector2 g_ofs = {0, 0};
static float g_olcek = 1.0f;
static bool g_telefon = false;

static string g_altyazi;
static int g_altyaziBaslangic = 0;
static float g_karartma = 1.0f;   // 1 = tamamen siyah (video karartmayla baslar)
static float g_karartmaHedef = 0.0f;

static BotDurum g_bot;
static bool g_botAktif = false;
static int g_botSayac = 0;

static bool TR() { return g_dil == Dil::TR; }
static const char* T(const char* tr, const char* en) { return TR() ? tr : en; }

static void Altyazi(const char* tr, const char* en) {
    string yeni = T(tr, en);
    if (yeni != g_altyazi) {
        g_altyazi = yeni;
        g_altyaziBaslangic = g_kare;
    }
}

// ---- oyunu ilerlet (cizmeden) ----
static void Ilerle(const Girdi& g) {
    Oyun& o = U->oyun;
    if (g_botAktif && ++g_botSayac % 8 == 0) { // bot 0,25 sn'de bir karar verir
        BotHamle(o, g_bot, 0.25f);
        if (o.yonetimOlayiAcik) YonetimKarariUygula(o, YonetimBasariOlasiligi(o.hisseler[o.yonetimHisseIndex]) >= 0.55f);
    }
    if (o.sonPenceresiAcik) o.sonPenceresiAcik = false;
    KareGuncelle(*U, DT, g);
}
static Girdi ImlecGirdisi() {
    Girdi g;
    g.fare = g_imlecVar ? g_imlec : FARE_UZAK;
    return g;
}

// ---- bir kare ciz ve ffmpeg'e yaz ----
static void ImlecCiz() {
    if (!g_imlecVar) return;
    Vector2 p = {g_ofs.x + g_imlec.x * g_olcek, g_ofs.y + g_imlec.y * g_olcek};
    if (g_halka > 0.0f) {
        float r = 10.0f + (1.0f - g_halka) * 26.0f;
        DrawRing(p, r - 3.0f, r, 0, 360, 32, Fade(RENK_ALTIN, g_halka));
    }
    const float k = 1.25f;
    Vector2 a = p, b = {p.x, p.y + 22 * k}, c = {p.x + 15 * k, p.y + 16 * k};
    DrawTriangle({a.x - 2, a.y - 3}, {b.x - 2, b.y + 3}, {c.x + 3, c.y + 1}, BLACK);
    DrawTriangle(a, b, c, WHITE);
}

static void AltyaziCiz() {
    if (g_altyazi.empty()) return;
    float alfa = fminf(1.0f, (g_kare - g_altyaziBaslangic) / 8.0f);
    const int boy = 28;
    int w = YaziOlc(g_altyazi, boy);
    Rectangle r;
    if (g_telefon) r = {70.0f, VH * 0.5f - 60.0f, 470.0f, 120.0f};
    else {
        // Grafik panelinin altina ortala: soldaki kartlari ve adet panelini (az once
        // tiklanan dugmeleri) ortmesin. Sigmazsa ekranin ortasina.
        const Rectangle& gp = U->ui.grafikPanel;
        float merkez = (gp.x + gp.width / 2.0f) * g_olcek;
        float sol = merkez - w / 2.0f - 28.0f;
        if (sol < gp.x * g_olcek) merkez = VW / 2.0f;
        r = {merkez - w / 2.0f - 28.0f, VH - 112.0f, (float)w + 56.0f, 58.0f};
    }
    DrawRectangleRounded(r, 0.35f, 8, Fade(Color{8, 10, 14, 255}, 0.86f * alfa));
    DrawRectangleRoundedLinesEx(r, 0.35f, 8, 2.0f, Fade(RENK_ALTIN, alfa));
    if (g_telefon) YaziCizSariliOrtali(g_altyazi, (int)(r.x + r.width / 2), (int)r.y + 26, boy, (int)r.width - 40, Fade(RENK_ALTIN, alfa), 36);
    else YaziCiz(g_altyazi, (int)r.x + 28, (int)r.y + 15, boy, Fade(RENK_ALTIN, alfa));
}

static void KareyiYaz() {
    KareCiz(*U); // oyunun kendi cizimi (sahne dokusuna)
    BeginTextureMode(g_cikis);
    ClearBackground(BLACK);
    const Texture2D& t = U->sahne.texture;
    if (!g_telefon) {
        g_ofs = {0, 0};
        g_olcek = (float)VW / t.width;
        DrawTexturePro(t, {0, 0, (float)t.width, -(float)t.height}, {0, 0, (float)VW, (float)VH}, {0, 0}, 0, WHITE);
    } else {
        // Telefon: koyu zemin, sagda yuvarlak koseli cerceve icinde dikey sahne
        DrawRectangleGradientV(0, 0, VW, VH, RENK_ARKAPLAN_UST, RENK_ARKAPLAN_ALT);
        g_olcek = 640.0f / t.height;
        float w = t.width * g_olcek, h = t.height * g_olcek;
        g_ofs = {VW * 0.70f - w / 2.0f, (VH - h) / 2.0f};
        DrawRectangleRounded({g_ofs.x - 14, g_ofs.y - 14, w + 28, h + 28}, 0.10f, 12, Color{40, 44, 54, 255});
        DrawRectangleRounded({g_ofs.x - 6, g_ofs.y - 6, w + 12, h + 12}, 0.06f, 12, BLACK);
        DrawTexturePro(t, {0, 0, (float)t.width, -(float)t.height}, {g_ofs.x, g_ofs.y, w, h}, {0, 0}, 0, WHITE);
    }
    ImlecCiz();
    AltyaziCiz();
    if (g_karartma > 0.001f) DrawRectangle(0, 0, VW, VH, Fade(BLACK, g_karartma));
    EndTextureMode();

    void* px = rlReadTexturePixels(g_cikis.texture.id, VW, VH, g_cikis.texture.format);
    fwrite(px, 1, (size_t)VW * VH * 4, g_ffmpeg);
    RL_FREE(px);
    g_kare++;
    g_halka = fmaxf(0.0f, g_halka - 0.06f);
    g_karartma += Clamp(g_karartmaHedef - g_karartma, -0.12f, 0.12f);
}

// ---- senaryo yardimcilari ----
static void Bekle(int kare, int hiz = 1) {
    for (int i = 0; i < kare; i++) {
        for (int k = 0; k < hiz; k++) Ilerle(ImlecGirdisi());
        KareyiYaz();
    }
}
static void Git(const std::function<Rectangle()>& hedef, int kare = 16) {
    g_imlecVar = true;
    Vector2 bas = g_imlec;
    for (int i = 1; i <= kare; i++) {
        Rectangle r = hedef();
        Vector2 son = {r.x + r.width * 0.5f, r.y + r.height * 0.55f};
        float t = (float)i / kare;
        t = t * t * (3.0f - 2.0f * t); // yumusak baslayip yumusak dur
        g_imlec = {bas.x + (son.x - bas.x) * t, bas.y + (son.y - bas.y) * t};
        Ilerle(ImlecGirdisi());
        KareyiYaz();
    }
}
static void Tikla(const std::function<Rectangle()>& hedef, int kare = 16) {
    Git(hedef, kare);
    Bekle(4);
    Girdi g = ImlecGirdisi();
    g.tiklandi = true;
    Ilerle(g);
    g_halka = 1.0f;
    g_tikKareleri.push_back(g_kare);
    KareyiYaz();
    Bekle(6);
}
// Karararak kes: ekran kararir, oyun kosul saglanana kadar CIZILMEDEN ilerler,
// sonra yeniden acilir. Uzun bekleyisler videoya girmez.
static bool Kes(const std::function<bool()>& kosul, int azami = 60 * 60 * 40) {
    g_karartmaHedef = 1.0f;
    g_altyazi.clear();
    while (g_karartma < 0.999f) Bekle(1);
    g_imlecVar = false;
    bool oldu = false;
    for (int i = 0; i < azami; i++) {
        if (kosul()) { oldu = true; break; }
        Ilerle(Girdi());
    }
    g_karartmaHedef = 0.0f;
    return oldu;
}
static void Ac(int kare = 10) {
    g_karartmaHedef = 0.0f;
    Bekle(kare);
}

static int EnYeniSirketHaberi(float esik) {
    const Oyun& o = U->oyun;
    for (int k = (int)o.bekleyenHaberler.size() - 1; k >= 0; k--) {
        const BeklemedeHaber& bh = o.bekleyenHaberler[k];
        if (bh.tur == HABER_SIRKET && bh.carpan > esik && bh.kalanSure > HABER_ETKI_GECIKME_SN - 0.4f && o.HisseAcikMi(bh.hisseIndex))
            return k;
    }
    return -1;
}

// ---- ses izi: oyunun muzigi + tiklama sesleri ----
static void SesIziYaz(const char* yol, float saniye) {
    const int hiz = MUZIK_ORNEK_HIZI;
    vector<short> muzik = MuzikOrnekleri();
    const int n = (int)(saniye * hiz);
    vector<float> iz(n, 0.0f);
    for (int i = 0; i < n; i++) {
        float t = (float)i / hiz;
        float zarf = fminf(1.0f, t / 1.5f) * fminf(1.0f, (saniye - t) / 2.5f);
        iz[i] = muzik[i % muzik.size()] / 32768.0f * 0.75f * zarf;
    }
    for (int k : g_tikKareleri) { // kisa, yumusak "tik"
        int bas = (int)((float)k / FPS * hiz);
        for (int i = 0; i < hiz / 25 && bas + i < n; i++) {
            float t = (float)i / hiz;
            iz[bas + i] += sinf(2.0f * PI * 1100.0f * t) * expf(-90.0f * t) * 0.35f;
        }
    }
    FILE* f = fopen(yol, "wb");
    auto u32 = [&](uint32_t v) { fwrite(&v, 4, 1, f); };
    auto u16 = [&](uint16_t v) { fwrite(&v, 2, 1, f); };
    fwrite("RIFF", 1, 4, f); u32(36 + n * 2); fwrite("WAVEfmt ", 1, 8, f);
    u32(16); u16(1); u16(1); u32(hiz); u32(hiz * 2); u16(2); u16(16);
    fwrite("data", 1, 4, f); u32(n * 2);
    for (float x : iz) { short s = (short)(Clamp(x, -1.0f, 1.0f) * 32000.0f); fwrite(&s, 2, 1, f); }
    fclose(f);
}

// ---- kapanis karti ----
static void KapanisKarti(int kare) {
    g_imlecVar = false;
    g_altyazi.clear();
    for (int i = 0; i < kare; i++) {
        BeginTextureMode(g_cikis);
        DrawRectangleGradientV(0, 0, VW, VH, RENK_ARKAPLAN_UST, RENK_ARKAPLAN_ALT);
        // arkada yavasca yukselen altin cizgi
        vector<Vector2> pts;
        for (int x = 0; x <= VW; x += 8) {
            float t = (float)x / VW;
            pts.push_back({(float)x, VH * 0.88f - t * VH * 0.32f + sinf(x * 0.037f + i * 0.05f) * 9.0f + sinf(x * 0.011f) * 14.0f});
        }
        GlowCizgiCiz(pts, Fade(RENK_ALTIN, 0.55f), 3.0f);
        YaziCizOrtali("BORSA PATRONU", VW / 2 + 4, 176 + 5, 80, Fade(BLACK, 0.6f));
        YaziCizOrtali("BORSA PATRONU", VW / 2, 176, 80, RENK_ALTIN);
        YaziCizOrtali(T("Haberi oku, 5 saniyede karar ver, borsa imparatorluğunu kur",
                        "Read the news, decide in 5 seconds, build your market empire"), VW / 2, 290, 26, RENK_METIN);
        YaziCizOrtali(T("Ücretsiz  -  Çevrimdışı  -  Türkçe / English", "Free  -  Offline  -  English / Türkçe"), VW / 2, 350, 22,
                      RENK_SOLUK_METIN);
        YaziCizOrtali(T("Chrome Web Store'da ve tarayıcında: emrebiltekin.me/borsa-patronu",
                        "On the Chrome Web Store and in your browser: emrebiltekin.me/borsa-patronu"),
                      VW / 2, 420, 22, RENK_YESIL);
        float k = (i < 12) ? 1.0f - i / 12.0f : (i > kare - 24 ? (i - (kare - 24)) / 24.0f : 0.0f);
        if (k > 0.0f) DrawRectangle(0, 0, VW, VH, Fade(BLACK, k));
        EndTextureMode();
        void* px = rlReadTexturePixels(g_cikis.texture.id, VW, VH, g_cikis.texture.format);
        fwrite(px, 1, (size_t)VW * VH * 4, g_ffmpeg);
        RL_FREE(px);
        g_kare++;
    }
}

int main(int argc, char** argv) {
    if (argc < 4) {
        printf("kullanim: tanitim_videosu <ffmpeg> <tr|en> <cikti.mp4>\n");
        return 2;
    }
    const string ffmpeg = argv[1], dil = argv[2], cikti = argv[3];
    const string sessiz = cikti + ".sessiz.mp4", ses = cikti + ".wav";

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    g_profilKalici = false; // gercek profil dosyasina ASLA yazma
    Uygulama u;
    U = &u;
    UygulamaBaslat(u);
    sesAcik = false; // video uretilirken bilgisayardan ses cikmasin
    g_profil = Profil();
    g_profil.rehberGoruldu = true;
    g_dil = (dil == "en") ? Dil::EN : Dil::TR;
    PaletiAyarla(false);
    SahneBoyutuAyarla(u, VW, VH, false);
    g_cikis = LoadRenderTexture(VW, VH);

    string komut = "\"\"" + ffmpeg + "\" -y -loglevel error -f rawvideo -pix_fmt rgba -s " + to_string(VW) + "x" + to_string(VH) +
                   " -r " + to_string(FPS) + " -i - -vf \"vflip,scale=1920:1080:flags=lanczos\" -c:v libx264 -preset slow -crf 18 "
                   "-pix_fmt yuv420p \"" + sessiz + "\"\"";
    g_ffmpeg = _popen(komut.c_str(), "wb");
    if (!g_ffmpeg) { printf("ffmpeg baslatilamadi\n"); return 1; }

    SetRandomSeed(2026);
    SimTohumla(20261023);
    g_botRng.Tohumla(77);
    g_bot = BotDurum();
    g_bot.tur = BOT_HABERCI;
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;

    // ---- 0) Ana menu -> BASLA ----
    u.ekran = Ekran::MENU;
    Ac(20);
    Bekle(25);
    Tikla([&] { return a.basBtn.rect; }, 22);
    o.rehberAdim = -1;
    o.gorevBekleme = 9999.0f; // ilk sahnede gorev satiri dikkat dagitmasin

    // ---- 1) Haber -> al -> kar al (imlecle, gercek dugmeler) ----
    Kes([&] { return EnYeniSirketHaberi(1.04f) >= 0; });
    int hk = EnYeniSirketHaberi(1.04f);
    int hisse = hk >= 0 ? o.bekleyenHaberler[hk].hisseIndex : 0;
    string haberMetni = hk >= 0 ? o.bekleyenHaberler[hk].metin : "";
    Altyazi("Haber çıkar, fiyata 5 saniye sonra yansır", "News breaks. It hits the price 5 seconds later.");
    Ac(12);
    Bekle(10);
    Tikla([&] { return a.adetBtn[7].rect; });
    Altyazi("İyi mi kötü mü? Okuyup karar vermek sana kalmış", "Good or bad? You read it and you decide.");
    Tikla([&] { return a.alBtn[hisse].rect; });
    for (int i = 0; i < 60 * 8; i++) { // haber fiyata yansiyana kadar
        bool bekliyor = false;
        for (auto& bh : o.bekleyenHaberler)
            if (bh.metin == haberMetni) bekliyor = true;
        if (!bekliyor) break;
        Bekle(1);
    }
    Bekle(40);
    Altyazi("Kârını al", "Take your profit");
    Tikla([&] { return a.satBtn[hisse].rect; });
    Bekle(55);

    // ---- 2) Rutbe atlayislari (hizli) ----
    g_botAktif = true;
    o.gorevBekleme = GOREV_ARA_SN;
    // Rutbe atlayisina kadar CIZMEDEN ilerle, kutlama anini (bildirimler,
    // acilan yeni hisse) goster.
    for (int r : {2, 3}) {
        Kes([&] { return o.rutbeIndex >= r; });
        Altyazi("Rütbe atla: yeni hisseler ve araçlar açılır", "Rank up to unlock new stocks and tools");
        Ac(8);
        Bekle(80);
    }

    // ---- 3) Emirler ----
    g_botAktif = true;
    Kes([&] {
        if (!o.EmirlerAcik()) return false;
        for (int i = 0; i < HisseSayisi(); i++)
            if (o.HisseAcikMi(i) && o.hisseler[i].adet > 0) return true;
        return false;
    });
    g_botAktif = false;
    for (int i = 0; i < HisseSayisi(); i++)
        if (o.HisseAcikMi(i) && o.hisseler[i].adet > 0) { hisse = i; break; }
    Altyazi("Zarar kes, kâr al: emirlerin sen bakmazken çalışır", "Stop-loss and take-profit orders work while you look away");
    Ac(10);
    Bekle(10);
    Tikla([&] { return a.emirBtn[hisse].rect; });
    Bekle(12);
    Tikla([&] { return a.emirSeviyeBtn[EMIR_STOP][1].rect; });
    Tikla([&] { return a.emirSeviyeBtn[EMIR_KAR_AL][1].rect; });
    Bekle(25);
    Tikla([&] { return a.emirKapatBtn.rect; });
    Bekle(30);

    // ---- 4) Kuresel olay ----
    g_botAktif = true;
    Kes([&] {
        for (auto& bk : o.bekleyenKureselHaberler)
            if (bk.kalanSure > KURESEL_OLAY_ETKI_GECIKME_SN - 0.3f) return true;
        return false;
    });
    Altyazi("Küresel olaylar tüm piyasayı sarsar", "Global events shake the whole market");
    Ac(8);
    for (int i = 0; i < 30 * 9 && !o.bekleyenKureselHaberler.empty(); i++) Bekle(1);
    Bekle(60);

    // ---- 5) Devralma ve yonetim karari ----
    Kes([&] { return o.rutbeIndex >= 4; });
    g_botAktif = false;
    for (int i = 0; i < (int)o.hisseler.size(); i++) BotPozisyonKapat(o, i);
    int ucuz = -1;
    for (int i = 0; i < HisseSayisi(); i++)
        if (o.HisseAcikMi(i) && (ucuz < 0 || o.hisseler[i].fiyat < o.hisseler[ucuz].fiyat)) ucuz = i;
    int sayac = 0;
    Kes([&] {
        if (o.yonetimOlayiAcik) return true;
        if (!o.hisseler[ucuz].sahibiyim && ++sayac % 60 == 0) { // 2 sn'de bir 25 adet
            int n = min(25, 501 - o.hisseler[ucuz].adet);
            if (n > 0) IslemYap(o, ucuz, true, n, IslemKaynagi::OYUNCU);
        }
        return false;
    });
    Altyazi("Şirketi devral, kararları sen ver", "Take over a company and make the calls");
    Ac(10);
    Bekle(45);
    bool riskli = o.yonetimOlayiAcik && YonetimBasariOlasiligi(o.hisseler[o.yonetimHisseIndex]) >= 0.55f;
    Altyazi("Riskli seçenek gerçek bir kumar: şansın düğmede yazar", "The risky option is a real gamble: the odds are on the button");
    Bekle(20);
    Tikla([&] { return riskli ? a.yonRiskliBtn.rect : a.yonGuvenliBtn.rect; }, 20);
    Bekle(75);

    // ---- 6) Telefon ----
    g_botAktif = true;
    Kes([&] { return true; });
    SahneBoyutuAyarla(u, 480, 1000, true);
    g_telefon = true;
    g_imlec = {240, 700};
    Altyazi("Telefonda da oynanır", "Plays on your phone too");
    Ac(10);
    Bekle(30);
    if (a.dCip.size() > 1) Tikla([&] { return a.dCip[1]; }, 18);
    Bekle(25);
    Tikla([&] { return a.alBtn[a.seciliVarlik >= 0 ? a.seciliVarlik : 0].rect; }, 16);
    Bekle(55);
    g_karartmaHedef = 1.0f;
    while (g_karartma < 0.999f) Bekle(1);

    // ---- 7) Kapanis ----
    KapanisKarti(FPS * 5);

    _pclose(g_ffmpeg);
    float saniye = (float)g_kare / FPS;
    SesIziYaz(ses.c_str(), saniye);
    string birlestir = "\"\"" + ffmpeg + "\" -y -loglevel error -i \"" + sessiz + "\" -i \"" + ses +
                       "\" -c:v copy -c:a aac -b:a 160k -shortest -movflags +faststart \"" + cikti + "\"\"";
    int sonuc = system(birlestir.c_str());
    remove(sessiz.c_str());
    remove(ses.c_str());
    printf("  %s: %.1f sn, %d kare, %d tik (%s)\n", cikti.c_str(), saniye, g_kare, (int)g_tikKareleri.size(),
           sonuc == 0 ? "tamam" : "ffmpeg HATA");
    UnloadRenderTexture(g_cikis);
    UygulamaKapat(u);
    return sonuc == 0 ? 0 : 1;
}
