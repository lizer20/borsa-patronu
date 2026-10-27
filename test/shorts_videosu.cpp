// ============================================================================
// shorts_videosu.cpp - YouTube Shorts icin DIKEY tanitim videolari (1080x1920)
// ============================================================================
// tanitim_videosu.cpp ile ayni yontem: oyun gizli pencerede GERCEKTEN oynanir,
// imlec gercek dugmelere tiklar, uzun bekleyisleri Haberci botu oynar ve video
// bunlari kararip atlar. Fark: oyun telefon (dikey) yerlesiminde cizilir ve
// karenin ustunde videonun "kancasi" olan buyuk bir alt yazi bandi durur
// (Shorts'ta ilk 2 saniye izleyiciyi tutar; alttaki bant YouTube'un kendi
// yazilarina kalir).
//
// Uc kisa video, her biri tek bir fikir:
//   haber     haberi oku, al, 5 saniye sonra fiyat ziplasin, karini al
//   devralma  501. hisseyi al, patron ol, yonetim kararinda kumar oyna
//   zirve     10.000 TL'den Efsane Yatirimci'ya: rutbe rutbe hizli gecis
//
// Kullanim: shorts_videosu.exe <ffmpeg> <tr|en> <haber|devralma|zirve> <cikti.mp4>
// (python build.py shorts hepsini uretir)

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

// Cikis karesi 1080x1920. Oyun sahnesi 540x960 (dikey yerlesim) ve 1,5 kat
// buyutulup ustte 250 px'lik baslik bandi birakacak sekilde yerlestirilir.
static const int VW = 1080, VH = 1920, FPS = 30;
static const int SW = 540, SH = 960;
static const float OLCEK = 1.5f;
static const Vector2 OFS = {(VW - SW * OLCEK) / 2.0f, 250.0f};
static const float DT = 1.0f / FPS;

static Uygulama* U = nullptr;
static RenderTexture2D g_cikis;
static FILE* g_ffmpeg = nullptr;
static int g_kare = 0;
static vector<int> g_tikKareleri;

static Vector2 g_imlec = {SW * 0.5f, SH * 0.75f}; // sahne koordinatlarinda
static bool g_imlecVar = false;
static float g_halka = 0.0f;

static string g_altyazi;
static int g_altyaziBaslangic = 0;
static float g_karartma = 1.0f;
static float g_karartmaHedef = 0.0f;

static BotDurum g_bot;
static bool g_botAktif = false;
static int g_botSayac = 0;

static bool TR() { return g_dil == Dil::TR; }
static const char* T(const char* tr, const char* en) { return TR() ? tr : en; }

static void Altyazi(const string& yeni) {
    if (yeni != g_altyazi) {
        g_altyazi = yeni;
        g_altyaziBaslangic = g_kare;
    }
}
static void Altyazi(const char* tr, const char* en) { Altyazi(string(T(tr, en))); }

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

// ---- cizim ----
static void ImlecCiz() {
    if (!g_imlecVar) return;
    Vector2 p = {OFS.x + g_imlec.x * OLCEK, OFS.y + g_imlec.y * OLCEK};
    if (g_halka > 0.0f) {
        float r = 16.0f + (1.0f - g_halka) * 40.0f;
        DrawRing(p, r - 5.0f, r, 0, 360, 32, Fade(RENK_ALTIN, g_halka));
    }
    const float k = 2.0f;
    Vector2 a = p, b = {p.x, p.y + 22 * k}, c = {p.x + 15 * k, p.y + 16 * k};
    DrawTriangle({a.x - 3, a.y - 4}, {b.x - 3, b.y + 4}, {c.x + 4, c.y + 1}, BLACK);
    DrawTriangle(a, b, c, WHITE);
}

static void BaslikBandiCiz() {
    YaziCizOrtali("BORSA PATRONU", VW / 2 + 3, 34 + 3, 40, Fade(BLACK, 0.6f));
    YaziCizOrtali("BORSA PATRONU", VW / 2, 34, 40, RENK_ALTIN);
    if (g_altyazi.empty()) return;
    float alfa = fminf(1.0f, (g_kare - g_altyaziBaslangic) / 6.0f);
    YaziCizSariliOrtali(g_altyazi, VW / 2, 104, 40, VW - 90, Fade(RENK_METIN, alfa), 50);
}

static void HamKareGonder() {
    void* px = rlReadTexturePixels(g_cikis.texture.id, VW, VH, g_cikis.texture.format);
    fwrite(px, 1, (size_t)VW * VH * 4, g_ffmpeg);
    RL_FREE(px);
    g_kare++;
}

static void KareyiYaz() {
    KareCiz(*U); // oyunun kendi cizimi (sahne dokusuna)
    BeginTextureMode(g_cikis);
    DrawRectangleGradientV(0, 0, VW, VH, RENK_ARKAPLAN_UST, RENK_ARKAPLAN_ALT);
    const Texture2D& t = U->sahne.texture;
    float w = SW * OLCEK, h = SH * OLCEK;
    DrawRectangleRounded({OFS.x - 8, OFS.y - 8, w + 16, h + 16}, 0.03f, 12, Color{40, 44, 54, 255});
    DrawTexturePro(t, {0, 0, (float)t.width, -(float)t.height}, {OFS.x, OFS.y, w, h}, {0, 0}, 0, WHITE);
    ImlecCiz();
    if (g_karartma > 0.001f) DrawRectangle(0, 0, VW, VH, Fade(BLACK, g_karartma));
    BaslikBandiCiz(); // bant karartmanin USTUNDE: gecislerde de kanca okunur
    EndTextureMode();
    HamKareGonder();
    g_halka = fmaxf(0.0f, g_halka - 0.06f);
    g_karartma += Clamp(g_karartmaHedef - g_karartma, -0.15f, 0.15f);
}

// ---- senaryo yardimcilari ----
static void Bekle(int kare) {
    for (int i = 0; i < kare; i++) {
        Ilerle(ImlecGirdisi());
        KareyiYaz();
    }
}
static void Git(const std::function<Rectangle()>& hedef, int kare = 14) {
    g_imlecVar = true;
    Vector2 bas = g_imlec;
    for (int i = 1; i <= kare; i++) {
        Rectangle r = hedef();
        Vector2 son = {r.x + r.width * 0.5f, r.y + r.height * 0.55f};
        float t = (float)i / kare;
        t = t * t * (3.0f - 2.0f * t);
        g_imlec = {bas.x + (son.x - bas.x) * t, bas.y + (son.y - bas.y) * t};
        Ilerle(ImlecGirdisi());
        KareyiYaz();
    }
}
static void Tikla(const std::function<Rectangle()>& hedef, int kare = 14) {
    Git(hedef, kare);
    Bekle(3);
    Girdi g = ImlecGirdisi();
    g.tiklandi = true;
    Ilerle(g);
    g_halka = 1.0f;
    g_tikKareleri.push_back(g_kare);
    KareyiYaz();
    Bekle(5);
}
// Karararak kes: oyun kosul saglanana kadar CIZILMEDEN ilerler.
static bool Kes(const std::function<bool()>& kosul, int azami = 60 * 60 * 40) {
    g_karartmaHedef = 1.0f;
    while (g_karartma < 0.999f) Bekle(1);
    bool oldu = false;
    for (int i = 0; i < azami; i++) {
        if (kosul()) { oldu = true; break; }
        Ilerle(Girdi());
    }
    g_karartmaHedef = 0.0f;
    return oldu;
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

// Dikey ekranda bir varligin cipi (secili degilse once ona dokunulur).
static void VarlikSec(int i) {
    Arayuz& a = U->ui;
    if (a.seciliVarlik == i) return;
    for (int k = 0; k < (int)a.dCipVarlik.size(); k++)
        if (a.dCipVarlik[k] == i) { Tikla([&a, k] { return a.dCip[k]; }); return; }
}

// ---- kapanis karti ----
static void KapanisKarti(int kare) {
    g_imlecVar = false;
    for (int i = 0; i < kare; i++) {
        BeginTextureMode(g_cikis);
        DrawRectangleGradientV(0, 0, VW, VH, RENK_ARKAPLAN_UST, RENK_ARKAPLAN_ALT);
        vector<Vector2> pts;
        for (int x = 0; x <= VW; x += 8) {
            float t = (float)x / VW;
            pts.push_back({(float)x, VH * 0.80f - t * VH * 0.22f + sinf(x * 0.037f + i * 0.06f) * 12.0f + sinf(x * 0.011f) * 18.0f});
        }
        GlowCizgiCiz(pts, Fade(RENK_ALTIN, 0.55f), 4.0f);
        YaziCizOrtali("BORSA PATRONU", VW / 2 + 5, 560 + 6, 90, Fade(BLACK, 0.6f));
        YaziCizOrtali("BORSA PATRONU", VW / 2, 560, 90, RENK_ALTIN);
        int y = 720;
        y += YaziCizSariliOrtali(T("Haberi oku, 5 saniyede karar ver, borsa imparatorluğunu kur",
                                   "Read the news, decide in 5 seconds, build your market empire"),
                                 VW / 2, y, 40, VW - 140, RENK_METIN, 52);
        YaziCizOrtali(T("Ücretsiz  -  Çevrimdışı  -  Tarayıcıda", "Free  -  Offline  -  In your browser"), VW / 2, y + 40, 32,
                      RENK_SOLUK_METIN);
        YaziCizOrtali("emrebiltekin.me/borsa-patronu", VW / 2, y + 120, 40, RENK_YESIL);
        YaziCizOrtali(T("Chrome Web Store'da da var", "Also on the Chrome Web Store"), VW / 2, y + 180, 30, RENK_SOLUK_METIN);
        float k = (i < 8) ? 1.0f - i / 8.0f : 0.0f;
        if (k > 0.0f) DrawRectangle(0, 0, VW, VH, Fade(BLACK, k));
        EndTextureMode();
        HamKareGonder();
    }
}

// ---- ses izi: oyunun muzigi + tiklama sesleri ----
static void SesIziYaz(const char* yol, float saniye) {
    const int hiz = MUZIK_ORNEK_HIZI;
    vector<short> muzik = MuzikOrnekleri();
    const int n = (int)(saniye * hiz);
    vector<float> iz(n, 0.0f);
    for (int i = 0; i < n; i++) {
        float t = (float)i / hiz;
        float zarf = fminf(1.0f, t / 0.4f) * fminf(1.0f, (saniye - t) / 1.5f);
        iz[i] = muzik[i % muzik.size()] / 32768.0f * 0.75f * zarf;
    }
    for (int k : g_tikKareleri) {
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

// ============================================================================
// Uc video
// ============================================================================

// 1) Haber: oku, al, 5 saniye bekle, sat.
static void HaberVideosu() {
    Oyun& o = U->oyun;
    Arayuz& a = U->ui;
    Altyazi("Bu haber 5 saniye sonra fiyatı vuracak. Alır mısın?", "This headline hits the price in 5 seconds. Do you buy?");
    Kes([&] { return EnYeniSirketHaberi(1.05f) >= 0; });
    int hk = EnYeniSirketHaberi(1.05f);
    int hisse = hk >= 0 ? o.bekleyenHaberler[hk].hisseIndex : 0;
    string haberMetni = hk >= 0 ? o.bekleyenHaberler[hk].metin : "";
    Bekle(14);
    VarlikSec(hisse);
    Tikla([&] { return a.adetBtn[7].rect; });
    float once = o.bakiye;
    Tikla([&] { return a.alBtn[hisse].rect; });
    for (int i = 0; i < FPS * 8; i++) { // geri sayim: haber fiyata yansiyana kadar
        float kalan = -1.0f;
        for (auto& bh : o.bekleyenHaberler)
            if (bh.metin == haberMetni) kalan = bh.kalanSure;
        if (kalan < 0.0f) break;
        Altyazi(string(T("Aldın. Fiyata yansımasına: ", "Bought. Hits the price in: ")) + to_string((int)ceilf(kalan)) + "...");
        Bekle(1);
    }
    Altyazi("Haber fiyata yansıdı!", "The news hits the price!");
    Bekle(36);
    Altyazi("Şimdi kârını al", "Now take your profit");
    Tikla([&] { return a.satBtn[hisse].rect; });
    float kar = o.bakiye - once;
    Altyazi(string(T("Kâr: ", "Profit: ")) + ParaIsaretli(kar, 0) + T("  -  10 saniyede", "  -  in 10 seconds"));
    Bekle(50);
}

// 2) Devralma: 501. hisse, patronluk, yonetim kumari.
static void DevralmaVideosu() {
    Oyun& o = U->oyun;
    Arayuz& a = U->ui;
    g_botAktif = true;
    Kes([&] { return o.rutbeIndex >= 4; });
    g_botAktif = false;
    for (int i = 0; i < (int)o.hisseler.size(); i++) BotPozisyonKapat(o, i);
    int ucuz = -1;
    for (int i = 0; i < HisseSayisi(); i++)
        if (o.HisseAcikMi(i) && (ucuz < 0 || o.hisseler[i].fiyat < o.hisseler[ucuz].fiyat)) ucuz = i;
    int sayac = 0;
    Kes([&] { // 2 sn'de bir 25'er topla; son 25'i video gosterir
        if (o.hisseler[ucuz].adet >= 476) return true;
        if (++sayac % 60 == 0) IslemYap(o, ucuz, true, min(25, 476 - o.hisseler[ucuz].adet), IslemKaynagi::OYUNCU);
        return false;
    });
    a.seciliVarlik = ucuz;
    // Kanca, oyun GORUNURKEN okunmali (Shorts'ta ilk 2 saniye)
    Altyazi("Bir şirketin 501 hissesini topla, patronu sen ol", "Collect 501 shares and the company is yours");
    Bekle(50);
    Altyazi(string(T("Elinde ", "You hold ")) + to_string(o.hisseler[ucuz].adet) + T(" hisse. 25 tane daha...", " shares. 25 more..."));
    Tikla([&] { return a.adetBtn[3].rect; }, 16);
    Tikla([&] { return a.alBtn[ucuz].rect; });
    if (o.hisseler[ucuz].sahibiyim) Altyazi("Artık şirketin patronu sensin!", "You are now the boss!");
    Bekle(55);
    Altyazi("Her karar bir kumar: şansın düğmede yazar", "Every call is a gamble: the odds are on the button");
    Kes([&] { return o.yonetimOlayiAcik; });
    Bekle(55);
    if (!o.yonetimOlayiAcik) return;
    bool riskli = YonetimBasariOlasiligi(o.hisseler[o.yonetimHisseIndex]) >= 0.55f;
    int hy = o.yonetimHisseIndex;
    float once = o.hisseler[hy].fiyat;
    Tikla([&] { return riskli ? a.yonRiskliBtn.rect : a.yonGuvenliBtn.rect; }, 18);
    float degisim = (o.hisseler[hy].fiyat / once - 1.0f) * 100.0f;
    string etki = Yuzde(degisim, 0, true);
    if (!riskli) Altyazi(string(T("Güvenli seçim: kesin ", "Safe call: a sure ")) + etki);
    else if (degisim >= 0.0f) Altyazi(string(T("Risk tuttu: ", "The gamble paid off: ")) + etki);
    else Altyazi(string(T("Risk tutmadı: ", "The gamble failed: ")) + etki + T(". Bir dahakine?", ". Next time?"));
    Bekle(65);
}

// 3) Zirve: rutbe rutbe 10.000 TL'den 250.000 TL'ye.
static void ZirveVideosu() {
    Oyun& o = U->oyun;
    g_botAktif = true;
    Altyazi("10.000 TL ile başla. 250.000 TL'ye çıkabilir misin?", "Start with 10,000 TL. Can you reach 250,000?");
    Bekle(45);
    const int son = (int)RUTBELER.size() - 1;
    for (int r = 1; r <= son; r++) {
        if (!Kes([&] { return o.rutbeIndex >= r; })) break;
        if (r == son) o.grafikModu = 2; // net deger gecmisi: bastan sona yukselis
        Altyazi(string(T("Gün ", "Day ")) + to_string(o.gun) + "  -  " + RUTBELER[r].ad.al() + "  -  " + Para(o.NetDeger(), 0));
        Bekle(r == son ? 85 : 48);
    }
}

int main(int argc, char** argv) {
    if (argc < 5) {
        printf("kullanim: shorts_videosu <ffmpeg> <tr|en> <haber|devralma|zirve> <cikti.mp4>\n");
        return 2;
    }
    const string ffmpeg = argv[1], dil = argv[2], tur = argv[3], cikti = argv[4];
    const string sessiz = cikti + ".sessiz.mp4", ses = cikti + ".wav";

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    g_profilKalici = false; // gercek profil dosyasina ASLA yazma
    Uygulama u;
    U = &u;
    UygulamaBaslat(u);
    sesAcik = false;
    g_profil = Profil();
    g_profil.rehberGoruldu = true;
    g_dil = (dil == "en") ? Dil::EN : Dil::TR;
    PaletiAyarla(false);
    SahneBoyutuAyarla(u, SW, SH, true);
    g_cikis = LoadRenderTexture(VW, VH);

    string komut = "\"\"" + ffmpeg + "\" -y -loglevel error -f rawvideo -pix_fmt rgba -s " + to_string(VW) + "x" + to_string(VH) +
                   " -r " + to_string(FPS) + " -i - -vf vflip -c:v libx264 -preset slow -crf 18 -pix_fmt yuv420p \"" + sessiz + "\"\"";
    g_ffmpeg = _popen(komut.c_str(), "wb");
    if (!g_ffmpeg) { printf("ffmpeg baslatilamadi\n"); return 1; }

    // Klasik oyun, bastan; menu atlanir (Shorts ilk saniyeden aksiyon ister).
    SetRandomSeed(2026);
    SimTohumla(20261023);
    g_botRng.Tohumla(77);
    g_bot = BotDurum();
    g_bot.tur = BOT_HABERCI;
    Oyun& o = u.oyun;
    o.Baslat(S_KLASIK, false);
    o.rehberAdim = -1;
    o.gorevBekleme = 9999.0f; // gorev satiri dikkat dagitmasin
    u.ui = Arayuz();
    ArayuzKur(u.ui, (int)o.hisseler.size());
    u.ekran = Ekran::OYUN;
    u.oyunVar = true;

    if (tur == "haber") HaberVideosu();
    else if (tur == "devralma") DevralmaVideosu();
    else ZirveVideosu();
    g_altyazi.clear();
    g_karartmaHedef = 1.0f;
    while (g_karartma < 0.999f) Bekle(1);
    KapanisKarti(FPS * 3);

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
