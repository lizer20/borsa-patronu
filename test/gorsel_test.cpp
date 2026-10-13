// ============================================================================
// gorsel_test.cpp - ekran goruntusu ureten GORSEL test + YERLESIM DENETCISI
// ============================================================================
// Oyunu gizli bir pencerede acar, durumu elle kurup (sahte portfoy, rutbe,
// acik pencereler) her sahneyi UC YERLESIMDE ve IKI DILDE cizer:
//     y = yatay (1280x720)   g = genis (1680x720)   d = dikey / telefon (480x1000)
// (bunlarin PNG'si de kaydedilir) + yalnizca denetlenen dort uc boyut:
//     u = en genis (1920x720)  k = kare (1280x960)  s = kisa telefon (480x900)
//     t = tablet dikey (720x900)  z = uzun telefon (480x1180)
// ve:
//   1) test/cikti/ altina PNG olarak kaydeder (gozle bakmak icin),
//   2) cizilen her yazi/dugme/panelin kutusunu karsilastirip
//        - ust uste binen yazilari,
//        - bir dugmenin ustune binen yazilari,
//        - panelinden ya da ekrandan tasan yazilari,
//        - dugmesine sigmayan etiketleri
//      test/cikti/yerlesim_raporu.txt dosyasina yazar.
// Ardindan sahte tiklamalarla temel akislari (yatay ve dikey) sinar.
//
// Calistirma:  python build.py test
//
// Yeni bir ekran/pencere eklediginde Sahneler() icine bir sahne ekle; cakismalar
// elle oynamadan, uc yerlesimde ve iki dilde birden yakalanir.

#define BORSA_TEST
#define BORSA_ANA_YOK
#include "../borsa_patronu.cpp"
#include <cstdarg>

static FILE* g_rapor = nullptr;
static int g_sorunSayisi = 0;
static int g_sahneSayisi = 0;
static string g_onEk = "tr_y"; // dosya adi on eki: dil + yerlesim

static bool g_pngYaz = true;  // false: sahne yalnizca denetlenir, PNG yazilmaz (uc boyutlar)
static bool g_sessiz = false; // denetcinin kendini sinamasi sirasinda cikti basma
static void Yaz(const char* bicim, ...) {
    if (g_sessiz) return;
    char tampon[2048];
    va_list a;
    va_start(a, bicim);
    vsnprintf(tampon, sizeof(tampon), bicim, a);
    va_end(a);
    fputs(tampon, stdout);
    if (g_rapor) fputs(tampon, g_rapor);
}

static bool Kesisiyor(Rectangle a, Rectangle b, float pay) {
    float x0 = fmaxf(a.x, b.x), x1 = fminf(a.x + a.width, b.x + b.width);
    float y0 = fmaxf(a.y, b.y), y1 = fminf(a.y + a.height, b.y + b.height);
    return (x1 - x0) > pay && (y1 - y0) > pay;
}
static bool Icinde(Rectangle ic, Rectangle dis, float pay) {
    return ic.x >= dis.x - pay && ic.y >= dis.y - pay && ic.x + ic.width <= dis.x + dis.width + pay &&
           ic.y + ic.height <= dis.y + dis.height + pay;
}

static void YerlesimDenetle(const char* sahne) {
    const vector<TestKutu>& k = g_testKutular;
    auto sorun = [&](const char* tur, const TestKutu& a, const TestKutu* b) {
        g_sorunSayisi++;
        if (b) Yaz("  [%s/%s] %s: \"%s\" (%.0f,%.0f %.0fx%.0f)  <->  \"%s\" (%.0f,%.0f %.0fx%.0f)\n", g_onEk.c_str(), sahne, tur,
                   a.metin.c_str(), a.r.x, a.r.y, a.r.width, a.r.height, b->metin.c_str(), b->r.x, b->r.y, b->r.width, b->r.height);
        else Yaz("  [%s/%s] %s: \"%s\" (%.0f,%.0f %.0fx%.0f)\n", g_onEk.c_str(), sahne, tur, a.metin.c_str(), a.r.x, a.r.y, a.r.width, a.r.height);
    };
    const Rectangle ekran = {0, 0, (float)PENCERE_GENISLIK, (float)PENCERE_YUKSEKLIK};
    for (size_t i = 0; i < k.size(); i++) {
        const TestKutu& a = k[i];
        if (a.tur == 3) { sorun("ETIKET DUGMEYE SIGMIYOR", a, nullptr); continue; }
        if (a.tur == 2) continue;
        if (a.r.width <= 0.0f || a.r.height <= 0.0f) continue; // ekranda olmayan (sifir boyutlu) dugme
        if (!Icinde(a.r, ekran, 1.0f)) sorun("EKRANDAN TASIYOR", a, nullptr);
        for (size_t j = 0; j < k.size(); j++) {
            const TestKutu& b = k[j];
            if (i == j || a.katman != b.katman || b.tur == 3) continue;
            if (b.r.width <= 0.0f || b.r.height <= 0.0f) continue;
            if (b.tur == 2) { // panel: kesisiyorsa tamamen icinde olmali
                if (Kesisiyor(a.r, b.r, 2.0f) && !Icinde(a.r, b.r, 1.0f) && !Icinde(b.r, a.r, 1.0f)) sorun("PANELDEN TASIYOR", a, &b);
                continue;
            }
            if (j < i) continue; // her cifti bir kez
            if (!Kesisiyor(a.r, b.r, 1.5f)) continue;
            if (a.tur == 0 && b.tur == 0) sorun("YAZILAR UST USTE", a, &b);
            else if (a.tur != b.tur) sorun("YAZI DUGMENIN USTUNDE", a, &b);
            else sorun("DUGMELER UST USTE", a, &b);
        }
    }
}

// Denetcinin kendisi calisiyor mu? Bilerek hatali kutular verip HEPSINI
// yakaladigini dogrular; yoksa "0 sorun" sonucu hicbir sey kanitlamaz.
static bool DenetciKendiniSinar() {
    int eskiSorun = g_sorunSayisi;
    g_testKutular.clear();
    auto ekle = [](Rectangle r, const char* m, int katman, int tur) { g_testKutular.push_back({r, m, katman, tur}); };
    ekle({100, 100, 200, 100}, "panel", 0, 2);
    ekle({110, 110, 80, 12}, "icerde", 0, 0);            // sorun yok
    ekle({250, 150, 90, 12}, "panelden tasan", 0, 0);    // 1: panelden tasiyor
    ekle({400, 400, 100, 12}, "yazi A", 0, 0);
    ekle({450, 404, 100, 12}, "yazi B", 0, 0);           // 2: yazilar ust uste
    ekle({600, 400, 80, 30}, "dugme", 0, 1);
    ekle({640, 410, 100, 12}, "dugmeye binen", 0, 0);    // 3: yazi dugmenin ustunde
    ekle({(float)PENCERE_GENISLIK - 30, 300, 80, 12}, "ekran disi", 0, 0); // 4: ekrandan tasiyor
    ekle({700, 500, 60, 30}, "cok uzun etiket", 0, 3);   // 5: etiket sigmiyor
    ekle({400, 400, 100, 12}, "baska katman", 1, 0);     // sorun yok (farkli katman)
    g_sessiz = true;
    YerlesimDenetle("kendini-sinama");
    g_sessiz = false;
    int bulunan = g_sorunSayisi - eskiSorun;
    g_sorunSayisi = eskiSorun;
    g_testKutular.clear();
    return bulunan == 5;
}

// Sahneyi cizer, PNG kaydeder, yerlesimi denetler.
static void Kaydet(Uygulama& u, const char* ad) {
    g_testKayit = true;
    KareCiz(u);
    g_testKayit = false;
    if (g_pngYaz) {
        Image img = LoadImageFromTexture(u.sahne.texture);
        ImageFlipVertical(&img);
        ExportImage(img, TextFormat("test/cikti/%s_%s.png", g_onEk.c_str(), ad));
        UnloadImage(img);
    }
    YerlesimDenetle(ad);
    g_sahneSayisi++;
}

static void Kareler(Uygulama& u, int n, float dt = 1.0f / 60.0f) {
    for (int i = 0; i < n; i++) KareGuncelle(u, dt, Girdi());
}
static void Tikla(Uygulama& u, Rectangle r) {
    Girdi g;
    g.fare = {r.x + r.width / 2.0f, r.y + r.height / 2.0f};
    g.tiklandi = true;
    KareGuncelle(u, 1.0f / 60.0f, g);
    Kareler(u, 2);
}

static void ArayuzuSifirla(Uygulama& u) {
    u.ui = Arayuz();
    ArayuzKur(u.ui, (int)u.oyun.hisseler.size());
}

// Durumu sifirlar: belirli rutbede, temiz bir oyun; bildirimler temizlenmis.
static void OyunKur(Uygulama& u, int senaryo, int rutbe, unsigned int tohum) {
    SetRandomSeed(tohum);
    SimTohumla(tohum);
    g_tarihZorla = 20261013;
    u.oyun.Baslat(senaryo, false);
    u.oyun.grafikModu = 0; // tercih oyunlar arasinda korunur; her sahne TL ekseniyle baslasin
    ArayuzuSifirla(u);
    u.oyun.rutbeIndex = rutbe;
    u.oyun.hedefIndex = rutbe;
    u.oyunVar = true;
    u.ekran = Ekran::OYUN;
    u.simBirikim = 0.0f;
    if (rutbe > 0) u.oyun.bakiye = RUTBELER[rutbe].esikNetDeger * 1.05f;
    u.oyun.enDusukNetDeger = u.oyun.bakiye;
    u.oyun.gorevBekleme = 9999.0f;       // sahneler kendi gorevini kurar
    Kareler(u, 60 * 40);                 // 40 saniye: grafik dolsun
    u.oyun.toastlar.liste.clear();
    u.oyun.yonetimOlayiAcik = false;
}

static int SiradakiHisse(const Oyun& o, int sira) { // acilma sirasina gore
    for (int i = 0; i < HisseSayisi(); i++)
        if (o.hisseler[i].acilmaSeviyesi == sira) return i;
    return 0;
}

static void PozisyonVer(Oyun& o, int idx, int adet, float ortOran) {
    Hisse& h = o.hisseler[idx];
    h.adet = adet;
    h.maliyet = fabsf((float)adet) * h.fiyat * ortOran;
    h.sahibiyim = !h.liman && (adet * 2 > TOPLAM_HISSE_SAYISI);
    h.yonetimOlayZamanlayici = 9999.0f;
}

static void GorevKur(Oyun& o, int tur, float hedef) {
    o.gorevTur = tur;
    o.gorevHedef = hedef;
    o.gorevOdul = 1200.0f;
    o.gorevBitisGunu = o.gun + 5;
}

static void Sahneler(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;

    // ---- menuler ----
    ArayuzuSifirla(u);
    u.ekran = Ekran::MENU;
    u.oyunVar = false;
    Kareler(u, 3);
    Kaydet(u, "01_menu");
    u.ui.menuMesaj = M("Kayıt okunamadı (bozuk ya da çok eski olabilir).", "Could not read the save (it may be corrupt or too old).");
    u.ui.menuMesajSure = 99.0f;
    u.oyunVar = true;
    Kaydet(u, "02_menu_devam");
    u.ui.menuMesaj.clear();
    u.ui.cikisOnayiAcik = true;
    u.oyun.ilkIslemYapildi = true;
    Kaydet(u, "03_cikis_onayi");
    u.ui.cikisOnayiAcik = false;

    u.ekran = Ekran::SENARYO;
    g_profil.rekorGun[S_KLASIK] = 87;
    g_profil.rekorNet[S_ZAMAN] = 182340.0f;
    g_profil.gunlukTarih = 20261013;
    g_profil.gunlukSkor = 48320.0f;
    u.ui.senaryoOnay = 1;
    u.ui.senaryoOnaySuresi = 99.0f;
    Kaydet(u, "04_senaryo");
    u.ekran = Ekran::BASARIM;
    for (int i = 0; i < BASARIM_SAYISI; i += 2) g_profil.basarim[i] = true;
    Kaydet(u, "05_basarim");

    // ---- rehber: butun adimlar ----
    OyunKur(u, S_KLASIK, 0, 11);
    for (int adim = 0; adim < (int)REHBER_ADIMLARI.size(); adim++) {
        o.rehberAdim = adim;
        o.rehberBekleme = 0.0f;
        Kaydet(u, TextFormat("06_rehber_%d", adim + 1));
    }
    o.rehberAdim = -1;

    // ---- 1 hisse, pozisyon + baski + bildirimler + gorev ----
    OyunKur(u, S_KLASIK, 0, 12);
    {
        int i0 = SiradakiHisse(o, 0);
        PozisyonVer(o, i0, 180, 0.93f);
        o.hisseler[i0].akis = 160.0f;
        GorevKur(o, G_KAR_ET, 1500.0f);
        o.toastlar.Ekle(M("KÜRESEL GELİŞME!", "GLOBAL NEWS!"), KURESEL_OLAYLARI[2].metin.al(), RENK_UYARI, 99.0f);
        o.toastlar.Ekle(M("BAŞARIM AÇILDI!", "ACHIEVEMENT UNLOCKED!"), BASARIMLAR[B_KAR_1K].ad.al(), RENK_ALTIN, 99.0f);
        Kareler(u, 30);
        Kaydet(u, "07_oyun_1hisse");
        o.toastlar.liste.clear();
    }

    // ---- 3 hisse: emirler, aciga satis, guvenli limanlar, sureli senaryo ust bari ----
    OyunKur(u, S_ZAMAN, 2, 13);
    {
        int i0 = SiradakiHisse(o, 0), i1 = SiradakiHisse(o, 1), i2 = SiradakiHisse(o, 2);
        int altin = HisseSayisi(), dolar = HisseSayisi() + 1;
        PozisyonVer(o, i0, 320, 1.08f);
        PozisyonVer(o, i1, -150, 1.04f);
        PozisyonVer(o, i2, 40, 0.90f);
        PozisyonVer(o, altin, 3, 0.97f);
        o.hisseler[dolar].akis = 4000.0f;
        o.hisseler[i0].esasDeger = o.hisseler[i0].temelDeger / 1.34f; // BALON
        o.hisseler[i1].esasDeger = o.hisseler[i1].temelDeger / 0.82f; // KELEPIR
        o.hisseler[i2].esasDeger = o.hisseler[i2].temelDeger / 1.18f; // PAHALI
        EmirKoy(o, i0, EMIR_STOP, o.hisseler[i0].fiyat * 0.95f, 0);
        EmirKoy(o, i0, EMIR_KAR_AL, o.hisseler[i0].fiyat * 1.10f, 0);
        EmirKoy(o, i0, EMIR_LIMIT_ALIM, o.hisseler[i0].fiyat * 0.97f, 25);
        EmirKoy(o, i1, EMIR_STOP, o.hisseler[i1].fiyat * 1.05f, 0);
        o.hisseler[i1].akis = -120.0f;
        o.secilenAdet = 25;
        GorevKur(o, G_LIMAN, 0.0f);
        a.seciliVarlik = i0;
        Kareler(u, 2);
        o.toastlar.liste.clear();
        Kaydet(u, "08_oyun_3hisse");
        a.seciliVarlik = i1; // dikeyde: aciga satilmis hisse karti
        Kaydet(u, "08b_oyun_aciga_kart");
        a.seciliVarlik = altin; // dikeyde: guvenli liman karti
        Kaydet(u, "08c_oyun_liman_kart");
        a.seciliVarlik = i0;
        o.grafikModu = 1;
        Kaydet(u, "08d_grafik_yuzde");
        o.grafikModu = 2;
        for (int g = 0; g < 40; g++) o.netGecmisi.push_back(10000.0f + g * 550.0f + (g % 7) * 300.0f);
        Kaydet(u, "08e_grafik_net_deger");
        o.grafikModu = 0;
        a.emirHisse = i0;
        Kaydet(u, "09_emir_uzun");
        a.emirHisse = i1;
        Kaydet(u, "10_emir_aciga");
        o.secilenAdet = -1;
        o.hisseler[i2].adet = 0;
        a.emirHisse = i2;
        Kaydet(u, "11_emir_pozisyonsuz");
        a.emirHisse = -1;
        o.duraklatildi = true;
        Kaydet(u, "12_duraklatma");
        a.ayarlarAcik = true;
        Kaydet(u, "13_ayarlar");
        a.ayarlarAcik = false;
        o.duraklatildi = false;

        // renk koru modu
        PaletiAyarla(true);
        ArayuzKur(a, (int)o.hisseler.size());
        Kaydet(u, "13b_renk_koru");
        PaletiAyarla(false);
        ArayuzKur(a, (int)o.hisseler.size());
    }

    // ---- 6 hisse: kompakt kartlar, buyuk rakamlar, kredi, zarf, yuzde ekseni ----
    OyunKur(u, S_KLASIK, 5, 14);
    {
        for (int s = 0; s < 6; s++) PozisyonVer(o, SiradakiHisse(o, s), (s % 2 == 0) ? 1000 : 640, 0.71f + 0.07f * s);
        PozisyonVer(o, SiradakiHisse(o, 3), -500, 1.12f);
        PozisyonVer(o, HisseSayisi() + 1, 100000, 0.9f);
        int i5 = SiradakiHisse(o, 5);
        EmirKoy(o, i5, EMIR_STOP, o.hisseler[i5].fiyat * 0.9f, 0);
        EmirKoy(o, i5, EMIR_KAR_AL, o.hisseler[i5].fiyat * 1.2f, 0);
        o.hisseler[SiradakiHisse(o, 1)].akis = 900.0f;
        o.hisseler[SiradakiHisse(o, 2)].esasDeger = o.hisseler[SiradakiHisse(o, 2)].temelDeger / 0.80f;
        o.bakiye = 1234567.89f;
        o.borc = 480000.0f;
        o.gerceklesenKar = -1234567.0f;
        o.islemSayisi = 12345;
        o.tamamlananGorev = 123;
        o.enIyiIslem = 987654.0f;
        o.enKotuIslem = -456789.0f;
        o.insiderMektupVar = true;
        o.insiderMektupOmur = 8.0f;
        o.insiderHisseIndex = i5;
        o.insiderArtisOrani = 0.187f;
        o.insiderKalanSure = 14.3f;
        o.piyasaFaktoru = 0.01f;
        GorevKur(o, G_PAHALI_SAT, 0.0f);
        a.seciliVarlik = i5;
        Kareler(u, 2);
        o.toastlar.liste.clear();
        Kaydet(u, "14_oyun_6hisse");
        o.insiderMektupVar = false;
        o.insiderOkundu = true;
        o.grafikModu = 1;
        o.piyasaFaktoru = -0.01f;
        Kaydet(u, "15_oyun_6hisse_yuzde");
        a.krediAcik = true;
        Kaydet(u, "16_kredi");
        a.krediAcik = false;
        o.duraklatildi = true;
        Kaydet(u, "17_duraklatma_buyuk_rakam");
        o.duraklatildi = false;

        for (int s = 0; s < (int)YONETIM_OLAYLARI.size(); s++) {
            o.yonetimOlayiAcik = true;
            o.yonetimHisseIndex = i5;
            o.yonetimSablonIndex = s;
            Kaydet(u, TextFormat("18_yonetim_%d", s + 1));
        }
        o.yonetimOlayiAcik = false;

        o.sonPenceresiAcik = true;
        o.sonTuru = SON_ZIRVE; o.bitisGunu = 143; o.yeniRekor = true; o.toplamTemettu = 123456.0f; o.toplamFaiz = 65432.0f;
        Kaydet(u, "19_son_zirve");
        o.yeniRekor = false;
        Kaydet(u, "20_son_zirve_rekorsuz");
        o.senaryo = S_ZAMAN; o.sonTuru = SON_SURE; o.bitisGunu = 45; o.bitisNetDeger = 1234567.0f;
        Kaydet(u, "21_son_sure");
        o.senaryo = S_GUNLUK; o.gunlukTarih = 20261013; o.bitisGunu = 30; o.yeniRekor = true;
        Kaydet(u, "21b_son_gunluk");
        o.senaryo = S_KLASIK; o.sonTuru = SON_IFLAS; o.bitisGunu = 212;
        Kaydet(u, "22_son_iflas");
        o.sonPenceresiAcik = false;
        o.sonTuru = SON_YOK;
    }

    // ---- butun gorev turleri: grafik basliginda kirpilmadan duruyor mu? ----
    // (ipucu seridi de acik: en dar durum)
    OyunKur(u, S_KLASIK, 5, 17);
    o.insiderOkundu = true;
    o.insiderHisseIndex = SiradakiHisse(o, 5);
    o.insiderArtisOrani = 0.215f;
    o.insiderKalanSure = 19.9f;
    for (int t = G_YOK + 1; t <= G_DEVRAL; t++) {
        GorevKur(o, t, 12500.0f);
        o.gorevOdul = 125000.0f;
        o.gorevBitisGunu = o.gun + GOREV_SURE_GUN;
        if (t == G_DEVRAL || t == G_CESITLENDIR) o.gorevHedef = 3.0f;
        const int panelW = (int)a.grafikPanel.width - (g_dikey ? 24 : 40);
        if (YaziOlc(GorevMetni(o), 13) > panelW) {
            g_sorunSayisi++;
            Yaz("  [%s] GOREV METNI PANELE SIGMIYOR (tur %d): \"%s\"\n", g_onEk.c_str(), t, GorevMetni(o).c_str());
        }
        Kaydet(u, TextFormat("22b_gorev_%02d", t));
    }
    o.insiderOkundu = false;
    o.gorevTur = G_YOK;

    // ---- 4 ve 5 hisse (ara kompakt boylar) ----
    OyunKur(u, S_KRIZ, 3, 15);
    for (int s = 0; s < 4; s++) PozisyonVer(o, SiradakiHisse(o, s), 501 + s * 100, 1.0f + 0.05f * s);
    Kareler(u, 2);
    o.toastlar.liste.clear();
    Kaydet(u, "23_oyun_4hisse");
    OyunKur(u, S_SIFIR, 4, 16);
    for (int s = 0; s < 5; s++) PozisyonVer(o, SiradakiHisse(o, s), 77 + s * 13, 0.97f);
    Kareler(u, 2);
    o.toastlar.liste.clear();
    Kaydet(u, "24_oyun_5hisse");
}

static int g_etkilesimHata = 0;
static void Bekle(bool kosul, const char* ad) {
    Yaz("  %-62s %s\n", ad, kosul ? "ok" : "HATA");
    if (!kosul) { g_etkilesimHata++; g_sorunSayisi++; }
}

// Tiklama akisi: sahte fare ile gercek dugmelere basip sonucu dogrular (yatay).
static void EtkilesimTesti(Uygulama& u) {
    Yaz("\n--- etkilesim testi: yatay (sahte tiklamalar) ---\n");
    SahneBoyutuAyarla(u, 1280, 720, false);
    g_dil = Dil::TR;
    g_profil = Profil();
    SimTohumla(424242);
    u.oyun.Baslat(S_KLASIK, false);
    u.oyun.grafikModu = 0; // tercih oyunlar arasinda korunur; sahnelerden kalan degeri sifirla
    ArayuzuSifirla(u);
    u.ekran = Ekran::MENU;
    u.oyunVar = false;
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;

    Kareler(u, 2);
    Tikla(u, a.basBtn.rect);
    Bekle(u.ekran == Ekran::OYUN && u.oyunVar, "menu: BASLA oyunu acar");
    Bekle(o.rehberAdim == 0, "ilk oyunda rehber kendiliginden baslar");
    o.rehberBekleme = 0.0f;
    KartDuzeni kd = KartDuzeniHesapla(o, a);
    int i0 = kd.acik[0];
    float bakiyeOnce = o.bakiye;
    Tikla(u, a.alBtn[i0].rect);
    Bekle(o.hisseler[i0].adet == 1 && o.bakiye < bakiyeOnce, "AL dugmesi 1 adet alir");
    Bekle(o.rehberAdim == 1, "rehber: alimdan sonra 2. adima gecer");
    o.rehberBekleme = 0.0f;
    Kareler(u, 2);
    RehberKutusuHesapla(a, o);
    Tikla(u, a.rehberTamamBtn.rect);
    Bekle(o.rehberAdim == 2, "rehber: 'Anladim' sonraki adima gecirir");
    Tikla(u, a.satBtn[i0].rect);
    Bekle(o.hisseler[i0].adet == 0, "SAT dugmesi satar");
    Bekle(o.rehberAdim == 3, "rehber: satistan sonra 4. adima gecer");
    o.rehberBekleme = 0.0f;
    RehberKutusuHesapla(a, o);
    Tikla(u, a.rehberKapatBtn.rect);
    Bekle(o.rehberAdim == -1 && g_profil.rehberGoruldu, "rehber: 'Rehberi Kapat' kapatir ve profilde isaretler");

    Tikla(u, a.adetBtn[7].rect);
    Bekle(o.secilenAdet == -1, "adet paneli: MAKS secilir");
    Tikla(u, a.alBtn[i0].rect);
    Bekle(o.hisseler[i0].adet > 100 && o.bakiye < o.hisseler[i0].fiyat * 2.0f, "MAKS alim nakdin tamamini kullanir");
    int adetOnce = o.hisseler[i0].adet;
    Tikla(u, a.satBtn[i0].rect);
    Bekle(o.hisseler[i0].adet == 0 && adetOnce > 0, "MAKS satis tum pozisyonu kapatir");
    Bekle(o.NetDeger() < BASLANGIC_BAKIYE, "al-sat dongusu net degeri DUSURUR");

    Tikla(u, a.grafikModBtn.rect);
    Bekle(o.grafikModu == 1, "grafik dugmesi: TL -> yuzde");
    Tikla(u, a.grafikModBtn.rect);
    Tikla(u, a.grafikModBtn.rect);
    Bekle(o.grafikModu == 0, "grafik dugmesi: yuzde -> net deger -> TL");

    Tikla(u, a.duraklatBtn.rect);
    Bekle(o.duraklatildi, "Duraklat dugmesi duraklatir");
    float sureOnce = o.oyunSuresi;
    Kareler(u, 30);
    Bekle(o.oyunSuresi == sureOnce, "duraklatmada simulasyon durur");
    int adetD = o.hisseler[i0].adet;
    Tikla(u, a.alBtn[i0].rect);
    Bekle(o.hisseler[i0].adet == adetD, "duraklatmada AL tiklanamaz");
    Tikla(u, a.sifirlaBtn.rect);
    Bekle(a.sifirlaOnaySuresi == 0.0f, "duraklatmada Yeni Oyun tiklanamaz");
    Tikla(u, a.ayarlarBtn.rect);
    Bekle(a.ayarlarAcik, "duraklatma menusu: Ayarlar acilir");
    Tikla(u, a.dilEnBtn.rect);
    Bekle(g_dil == Dil::EN, "ayarlar: English secilir");
    Tikla(u, a.dilTrBtn.rect);
    Bekle(g_dil == Dil::TR, "ayarlar: Turkce secilir");
    Tikla(u, a.renkKoruBtn.rect);
    Bekle(g_renkKoru && a.alBtn[i0].renk.b == 255, "ayarlar: renk koru modu paleti ve dugme renklerini degistirir");
    Tikla(u, a.renkKoruBtn.rect);
    Bekle(!g_renkKoru, "ayarlar: renk koru modu kapanir");
    bool muzikOnce = g_profil.muzikAcik;
    Tikla(u, a.muzikBtn.rect);
    Bekle(g_profil.muzikAcik != muzikOnce, "ayarlar: muzik acilip kapanir");
    Tikla(u, a.ayarlarKapatBtn.rect);
    Bekle(!a.ayarlarAcik && o.duraklatildi, "ayarlar kapanir, duraklatma surer");
    Tikla(u, a.devamBtn.rect);
    Bekle(!o.duraklatildi, "Devam Et oyunu surdurur");

    // emirler
    o.rutbeIndex = 3;
    o.bakiye = 200000.0f;
    o.gorevBekleme = 9999.0f;
    // Simulasyon sabit adimli (0,05 sn): rutbe atlayisi ve onunla degisen kart
    // yerlesimi ancak bir adim calisinca oturur; dugme konumunu ondan SONRA oku.
    Kareler(u, 12);
    kd = KartDuzeniHesapla(o, a);
    o.secilenAdet = 100;
    Tikla(u, a.alBtn[i0].rect);
    Tikla(u, a.emirBtn[i0].rect);
    Bekle(a.emirHisse == i0, "EMIR dugmesi emir penceresini acar");
    sureOnce = o.oyunSuresi;
    Kareler(u, 20);
    Bekle(o.oyunSuresi == sureOnce, "emir penceresi acikken simulasyon durur");
    Tikla(u, a.emirSeviyeBtn[EMIR_STOP][1].rect);
    Bekle(o.hisseler[i0].emirler[EMIR_STOP].aktif && o.hisseler[i0].emirler[EMIR_STOP].fiyat < o.hisseler[i0].fiyat,
          "emir: zarar kes seviyesi fiyatin altina konur");
    Tikla(u, a.emirKaldirBtn[EMIR_STOP].rect);
    Bekle(!o.hisseler[i0].emirler[EMIR_STOP].aktif, "emir: Kaldir emri siler");
    Tikla(u, a.emirSeviyeBtn[EMIR_STOP][0].rect);
    Tikla(u, a.emirKapatBtn.rect);
    Bekle(a.emirHisse == -1, "emir penceresi kapanir");
    o.hisseler[i0].fiyat *= 0.90f; // fiyat stopun altina duser
    Kareler(u, 6);
    Bekle(o.hisseler[i0].adet == 0 && o.gerceklesenEmirSayisi == 1, "zarar kes emri fiyat dusunce pozisyonu satar");

    // aciga satis
    o.secilenAdet = 50;
    Tikla(u, a.satBtn[i0].rect);
    Bekle(o.hisseler[i0].adet == -50, "elde hisse yokken SAT aciga satar (rutbe 2+)");
    Tikla(u, a.alBtn[i0].rect);
    Bekle(o.hisseler[i0].adet == 0, "AL acigi kapatir");

    // guvenli liman
    int altin = HisseSayisi();
    o.secilenAdet = 5;
    Tikla(u, a.alBtn[altin].rect);
    Bekle(o.hisseler[altin].adet == 5, "guvenli liman karti: AL altin alir");
    Tikla(u, a.satBtn[altin].rect);
    Tikla(u, a.satBtn[altin].rect);
    Bekle(o.hisseler[altin].adet == 0, "guvenli liman: SAT satar, aciga DUSMEZ");

    // kredi
    Tikla(u, a.krediBtn.rect);
    Bekle(a.krediAcik, "Kredi dugmesi kredi masasini acar");
    Tikla(u, a.krediAlBtn[1].rect);
    Bekle(fabsf(o.borc - 5000.0f) < 1.0f, "kredi: +5.000 borc alir");
    Tikla(u, a.krediOdeBtn[3].rect);
    Bekle(o.borc == 0.0f, "kredi: TUMU borcu kapatir");
    Tikla(u, a.krediKapatBtn.rect);
    Bekle(!a.krediAcik, "kredi masasi kapanir");

    // teminat cagrisi
    o.secilenAdet = -1;
    o.bakiye = 10000.0f; // pozisyon, borca gore anlamli buyuklukte olsun
    KrediAl(o, 1e9f);
    Tikla(u, a.alBtn[i0].rect);
    Bekle(o.borc > 9000.0f && o.hisseler[i0].adet > 300, "kaldiracli alim yapilir");
    o.hisseler[i0].fiyat *= 0.55f;
    o.hisseler[i0].temelDeger *= 0.55f;
    Kareler(u, 6);
    Bekle(o.teminatCagrisiSayisi == 1 && o.borc == 0.0f, "fiyat cokunce teminat cagrisi borcu zorla kapatir");

    // gorev
    o.sonPenceresiAcik = false;
    o.sonTuru = SON_YOK;
    o.bakiye = 50000.0f;
    o.gorevTur = G_NET_DEGER;
    o.gorevHedef = o.NetDeger() - 1.0f;
    o.gorevOdul = 777.0f;
    o.gorevBitisGunu = o.gun + 5;
    float nakitOnce = o.bakiye;
    int gorevOnce = o.tamamlananGorev;
    Kareler(u, 6);
    Bekle(o.tamamlananGorev == gorevOnce + 1 && fabsf(o.bakiye - nakitOnce - 777.0f) < 1.0f, "gorev: kosul saglaninca odul nakde eklenir");

    // kayit gidis-donus (bellekte; gercek dosyaya dokunmaz)
    o.toastlar.liste.clear();
    EmirKoy(o, i0, EMIR_LIMIT_ALIM, 12.5f, 40);
    o.borc = 321.0f;
    o.hisseler[altin].adet = 7;
    o.hisseler[altin].maliyet = 7.0f * o.hisseler[altin].fiyat;
    o.hisseler[i0].esasDeger = o.hisseler[i0].temelDeger / 1.2f;
    GorevKur(o, G_EMIR, 0.0f);
    string metin = OyunuMetneYaz(o);
    uint64_t rngOnce = g_rngPiyasa.durum;
    Oyun kopya = o;
    kopya.Baslat(S_KRIZ, false);
    bool yuklendi = OyunuMetindenYukle(kopya, metin);
    Bekle(yuklendi && fabsf(kopya.bakiye - o.bakiye) < 0.01f && kopya.rutbeIndex == o.rutbeIndex &&
              kopya.hisseler[i0].emirler[EMIR_LIMIT_ALIM].aktif && kopya.hisseler[i0].emirler[EMIR_LIMIT_ALIM].adet == 40 &&
              fabsf(kopya.borc - 321.0f) < 0.01f && kopya.senaryo == o.senaryo,
          "kayit: yaz -> oku ayni durumu verir (V6)");
    Bekle(yuklendi && kopya.hisseler[altin].adet == 7 && kopya.gorevTur == G_EMIR &&
              fabsf(Degerleme(kopya.hisseler[i0]) - 1.2f) < 0.01f && kopya.netGecmisi.size() == o.netGecmisi.size(),
          "kayit: guvenli liman, gorev, degerleme ve net deger gecmisi korunur");
    Bekle(g_rngPiyasa.durum == rngOnce, "kayit: yukleme rastgele sayi uretecini kayittaki duruma getirir");
    {
        // Eski surum uyumlulugu: her yeni surum sadece SONA alan ekledigi icin,
        // guncel metnin sonundaki bloklari atip etiketi degistirerek gercek bir
        // V5 ve V4 dosyasi uretiyoruz.
        vector<string> satirlar;
        istringstream ss(metin);
        string satir;
        while (std::getline(ss, satir)) satirlar.push_back(satir);
        const size_t H = (size_t)HisseSayisi(), L = o.hisseler.size() - H;
        const size_t v6Satir = 3 + H + 1 + L * (8 + 1 + EMIR_TUR_SAYISI * 3) + 7 + 2;
        const size_t v5Satir = 14 + H * (4 + EMIR_TUR_SAYISI * 3);
        auto eskiSurum = [&](const char* etiket, size_t atilacak) {
            string s = string(etiket) + "\n";
            for (size_t i = 1; i + atilacak < satirlar.size(); i++) s += satirlar[i] + "\n";
            return s;
        };
        Oyun eski = o;
        eski.Baslat(S_ZAMAN, true);
        bool v5 = OyunuMetindenYukle(eski, eskiSurum(KAYIT_ETIKETI_V5, v6Satir));
        Bekle(v5 && fabsf(eski.borc - 321.0f) < 0.01f && eski.hisseler[i0].emirler[EMIR_LIMIT_ALIM].aktif &&
                  eski.hisseler[altin].adet == 0 && fabsf(Degerleme(eski.hisseler[i0]) - 1.0f) < 0.001f && eski.gorevTur == G_YOK,
              "kayit: eski V5 dosyasi yuklenir, v2.1 alanlari varsayilanda kalir");
        eski.Baslat(S_ZAMAN, true);
        bool v4 = OyunuMetindenYukle(eski, eskiSurum(KAYIT_ETIKETI_V4, v6Satir + v5Satir));
        Bekle(v4 && eski.senaryo == S_KLASIK && eski.rehberAdim == -1 && eski.borc == 0.0f &&
                  fabsf(eski.bakiye - o.bakiye) < 0.01f && eski.islemSayisi == o.islemSayisi,
              "kayit: eski V4 dosyasi yuklenir, yeni alanlar varsayilanda kalir");
    }
    uint64_t rngBozukOnce = g_rngPiyasa.durum;
    Bekle(!OyunuMetindenYukle(kopya, "BORSAPATRONU_SAVE_V1\n1\n2\n"), "kayit: V1 dosyasi reddedilir");
    Bekle(!OyunuMetindenYukle(kopya, metin.substr(0, metin.size() / 2)), "kayit: yarim (bozuk) dosya reddedilir, cokme yok");
    Bekle(!OyunuMetindenYukle(kopya, "BORSAPATRONU_SAVE_V6\nabc\n"), "kayit: sayi yerine metin iceren dosya reddedilir");
    Bekle(g_rngPiyasa.durum == rngBozukOnce, "kayit: reddedilen dosya rastgele sayi uretecine dokunmaz");
    o.borc = 0.0f;

    // yonetim karari: guvenli = kesin etki + toparlanma, riskli = kumar
    {
        Hisse& hy = o.hisseler[i0];
        o.toastlar.liste.clear();
        hy.adet = 600;
        hy.maliyet = 600.0f * hy.fiyat;
        hy.sahibiyim = true;
        hy.riskPrimi = 0.02f;
        hy.yonetimOlayZamanlayici = 9999.0f;
        auto ac = [&]() {
            o.yonetimOlayiAcik = true;
            o.yonetimHisseIndex = i0;
            o.yonetimSablonIndex = 0; // personel karari: riskli x1,15 / guvenli x1,02
            Kareler(u, 2);
        };
        ac();
        float f0 = hy.fiyat, r0 = hy.riskPrimi;
        float sans0 = YonetimBasariOlasiligi(hy);
        Tikla(u, a.yonGuvenliBtn.rect);
        Bekle(!o.yonetimOlayiAcik && fabsf(hy.fiyat / f0 - 1.02f) < 0.015f && hy.riskPrimi < r0 && YonetimBasariOlasiligi(hy) > sans0,
              "yonetim: guvenli karar kesin etkiyi uygular ve sirketi toparlar");
        ac();
        f0 = hy.fiyat;
        r0 = hy.riskPrimi;
        sans0 = YonetimBasariOlasiligi(hy);
        Tikla(u, a.yonRiskliBtn.rect);
        float oran = hy.fiyat / f0;
        bool tuttu = fabsf(oran - 1.15f) < 0.02f, tutmadi = fabsf(oran - YonetimBasarisizlikCarpani(1.15f)) < 0.02f;
        Bekle(!o.yonetimOlayiAcik && (tuttu || tutmadi) && hy.riskPrimi > r0 && YonetimBasariOlasiligi(hy) < sans0,
              "yonetim: riskli karar ya kazandirir ya kaybettirir; sonraki sans duser");
        Bekle(!o.toastlar.liste.empty(), "yonetim: riskli kararin sonucu bildirimle gosterilir");
        hy.adet = 0;
        hy.maliyet = 0.0f;
        hy.sahibiyim = false;
    }
}

// Dikey (telefon) yerlesimde temel akis: varlik sec, al, sat, emir penceresi.
static void DikeyEtkilesimTesti(Uygulama& u) {
    Yaz("\n--- etkilesim testi: dikey / telefon ---\n");
    SahneBoyutuAyarla(u, 480, 1000, true);
    g_dil = Dil::TR;
    g_profil = Profil();
    g_profil.rehberGoruldu = true;
    SimTohumla(777);
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    o.Baslat(S_KLASIK, false);
    ArayuzuSifirla(u);
    u.ekran = Ekran::MENU;
    u.oyunVar = false;
    Kareler(u, 2);
    Tikla(u, a.basBtn.rect);
    Bekle(u.ekran == Ekran::OYUN && g_dikey, "dikey: BASLA oyunu acar");
    o.rutbeIndex = 3;
    o.bakiye = 60000.0f;
    o.gorevBekleme = 9999.0f;
    Kareler(u, 12);
    Bekle((int)a.dCip.size() == 4 + (int)LIMANLAR.size(), "dikey: secicide acik 4 hisse + guvenli limanlar var");
    Bekle(a.seciliVarlik >= 0 && a.alBtn[a.seciliVarlik].rect.width > 100.0f, "dikey: secili varligin AL dugmesi buyuk ve ekranda");
    int ikinci = a.dCipVarlik[1];
    Tikla(u, a.dCip[1]);
    Bekle(a.seciliVarlik == ikinci, "dikey: cipe dokununca varlik secilir");
    o.secilenAdet = 10;
    Tikla(u, a.alBtn[ikinci].rect);
    Bekle(o.hisseler[ikinci].adet == 10, "dikey: AL secili varligi alir");
    Tikla(u, a.emirBtn[ikinci].rect);
    Bekle(a.emirHisse == ikinci, "dikey: EMIR dugmesi emir penceresini acar");
    Tikla(u, a.emirSeviyeBtn[EMIR_KAR_AL][0].rect);
    Bekle(o.hisseler[ikinci].emirler[EMIR_KAR_AL].aktif, "dikey: emir penceresinde seviye dugmesi calisir");
    Tikla(u, a.emirKapatBtn.rect);
    Tikla(u, a.satBtn[ikinci].rect);
    Bekle(o.hisseler[ikinci].adet == 0, "dikey: SAT satar");
    int sonCip = (int)a.dCip.size() - 1;
    Tikla(u, a.dCip[sonCip]);
    Bekle(o.hisseler[a.seciliVarlik].liman, "dikey: guvenli liman secilebilir");
    Tikla(u, a.alBtn[a.seciliVarlik].rect);
    Bekle(o.hisseler[a.seciliVarlik].adet == 10, "dikey: guvenli liman alinir");
    Tikla(u, a.duraklatBtn.rect);
    Bekle(o.duraklatildi, "dikey: Duraklat calisir");
    Tikla(u, a.devamBtn.rect);
    Bekle(!o.duraklatildi, "dikey: Devam Et calisir");
}

int main() {
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    g_profilKalici = false; // gercek profil dosyasina ASLA yazma
    Uygulama u;
    UygulamaBaslat(u);
    sesAcik = false;
    g_profil = Profil();
    PaletiAyarla(false);
    g_rapor = fopen("test/cikti/yerlesim_raporu.txt", "w");
    Yaz("BORSA PATRONU %s - GORSEL TEST / YERLESIM RAPORU\n", OYUN_SURUMU);
    Yaz("font: %s\n", g_fontHazir ? "Turkce glifli atlas hazir" : "UYARI: varsayilan font (Turkce glif YOK)");
    if (!g_fontHazir) g_sorunSayisi++;
    SahneBoyutuAyarla(u, 1280, 720, false);
    bool denetciSaglam = DenetciKendiniSinar();
    Yaz("denetci kendini sinadi: %s\n\n", denetciSaglam ? "5 kasitli hatanin 5'ini yakaladi" : "BASARISIZ - denetci guvenilmez!");
    if (!denetciSaglam) g_sorunSayisi++;

    struct Yerlesim { const char* ek; int w, h; bool dikey; bool png; };
    const Yerlesim yerlesimler[] = {
        {"y", 1280, 720, false, true},  {"g", 1680, 720, false, true}, {"d", 480, 1000, true, true},
        {"u", 1920, 720, false, false}, {"k", 1280, 960, false, true}, {"s", 480, 900, true, false},
        {"t", 720, 900, true, true},    {"z", 480, 1180, true, false},
    };
    const int yerlesimSayisi = (int)(sizeof(yerlesimler) / sizeof(yerlesimler[0]));
    for (const Yerlesim& y : yerlesimler) {
        SahneBoyutuAyarla(u, y.w, y.h, y.dikey);
        g_pngYaz = y.png;
        for (int d = 0; d < 2; d++) {
            g_dil = (d == 0) ? Dil::TR : Dil::EN;
            g_onEk = string(d == 0 ? "tr_" : "en_") + y.ek;
            g_profil = Profil();
            Sahneler(u);
        }
    }
    int yerlesimSorunu = g_sorunSayisi;
    g_pngYaz = true;
    Yaz("\n%d sahne cizildi (%d yerlesim x 2 dil). Yerlesim sorunu: %d\n", g_sahneSayisi, yerlesimSayisi, yerlesimSorunu);

    EtkilesimTesti(u);
    DikeyEtkilesimTesti(u);
    Yaz("  etkilesim testleri: %s\n", g_etkilesimHata == 0 ? "TEMIZ" : "BASARISIZ");

    Yaz("\nSONUC: %s\n", g_sorunSayisi == 0 ? "TEMIZ" : "SORUN VAR - yukaridaki satirlara bak");
    if (g_rapor) fclose(g_rapor);
    g_tarihZorla = 0;
    UygulamaKapat(u);
    return g_sorunSayisi == 0 ? 0 : 1;
}
