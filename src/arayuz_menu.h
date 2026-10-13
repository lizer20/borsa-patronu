// ============================================================================
// arayuz_menu.h - ana menu, senaryo secimi ve basarimlar ekranlari
// ============================================================================
// Uc ekran da iki yerlesimde calisir: yatayda yan yana kartlar / izgara, dar
// (dikey) sahnede alt alta satirlar. Konumlar MenuleriKur() icinde hesaplanir.

static void MenuArkaPlaniCiz(float zaman) {
    const int w = PENCERE_GENISLIK, h = PENCERE_YUKSEKLIK;
    DrawRectangleGradientV(0, 0, w, h, RENK_ARKAPLAN_UST, RENK_ARKAPLAN_ALT);
    // Arkaplanda yavasca dalgalanan soluk "hisse cizgileri"
    for (int i = 0; i < (int)SIRKETLER.size(); i++) {
        Color c = SIRKETLER[i].renk;
        Vector2 onceki = {0, 0};
        for (int x = 0; x <= 20; x++) {
            float fx = (float)x / 20.0f * w;
            float fy = h * 0.62f + i * 40.0f + sinf(zaman * 0.6f + x * 0.4f + i * 2.0f) * 40.0f;
            if (x > 0) DrawLineEx(onceki, {fx, fy}, 2.0f, Fade(c, 0.18f));
            onceki = {fx, fy};
        }
    }
}

// Yeni bir oyun baslatir. Rehber, daha once hic gorulmediyse Klasik senaryoda
// kendiliginden acilir.
static void YeniOyunBaslat(Uygulama& u, int senaryo) {
    bool rehber = !g_profil.rehberGoruldu && senaryo == S_KLASIK;
    u.oyun.Baslat(senaryo, rehber);
    u.oyunVar = true;
    u.ekran = Ekran::OYUN;
    u.simBirikim = 0.0f;
    u.ui.sifirlaOnaySuresi = 0.0f;
    u.ui.emirHisse = -1;
    u.ui.krediAcik = false;
    u.ui.ayarlarAcik = false;
    u.ui.seciliVarlik = -1;
}

// ---------------------------------------------------------------------------
// ANA MENU
// ---------------------------------------------------------------------------
static void MenuGuncelle(Uygulama& u, float dt, Vector2 fare, bool tik) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (a.menuMesajSure > 0.0f) a.menuMesajSure -= dt;
    else a.menuMesaj.clear();

    // Kayit var mi? Her karede diske/depoya bakmak yerine yarim saniyede bir.
    static float kayitKontrol = 0.0f;
    kayitKontrol -= dt;
    if (kayitKontrol <= 0.0f) {
        kayitKontrol = 0.5f;
        a.yukleBtn.aktif = KayitVarMi();
    }

    if (a.basBtn.Guncelle(dt, fare, tik)) {
        Cal(o.sesler.tikla);
        if (u.oyunVar) u.ekran = Ekran::OYUN; // kaldigi yerden devam
        else YeniOyunBaslat(u, S_KLASIK);
    }
    if (a.senaryoBtn.Guncelle(dt, fare, tik)) {
        Cal(o.sesler.tikla);
        a.senaryoOnay = -1;
        u.ekran = Ekran::SENARYO;
    }
    if (a.yukleBtn.Guncelle(dt, fare, tik)) {
        if (OyunuYukle(o)) {
            Cal(o.sesler.tikla);
            u.oyunVar = true;
            u.ekran = Ekran::OYUN;
            u.simBirikim = 0.0f;
            a.seciliVarlik = -1;
            o.toastlar.Ekle(M("Oyun Yüklendi", "Game Loaded"),
                            M("Kaydedilmiş ilerlemen geri geldi.", "Your saved progress is back."), RENK_YESIL, 4.0f);
        } else {
            Cal(o.sesler.hataUyari);
            a.menuMesaj = M("Kayıt okunamadı (bozuk ya da çok eski olabilir).",
                            "Could not read the save (it may be corrupt or too old).");
            a.menuMesajRenk = RENK_KIRMIZI;
            a.menuMesajSure = 3.5f;
        }
    }
    if (a.basarimBtn.Guncelle(dt, fare, tik)) {
        Cal(o.sesler.tikla);
        u.ekran = Ekran::BASARIM;
    }
    if (a.dilBtn.Guncelle(dt, fare, tik)) {
        DilAyarla(g_dil == Dil::TR ? Dil::EN : Dil::TR);
        Cal(o.sesler.tikla);
    }
    if (PlatformdaCikisVar() && a.cikisMenuBtn.Guncelle(dt, fare, tik)) CikisIste(u);
}

static void MenuCiz(Uygulama& u) {
    Arayuz& a = u.ui;
    const int w = PENCERE_GENISLIK, h = PENCERE_YUKSEKLIK;
    const bool dar = DarEkran();
    MenuArkaPlaniCiz(u.zaman);

    // Baslik hafifce nabiz atar; golgesi ayni yazinin kaydirilmis kopyasidir
    // (yerlesim denetcisi bunu cakisma saymasin diye golge kaydedilmez).
    // Dar sahnede baslik sahneye sigacak boya kuculur.
    float nabiz = 1.0f + sinf(u.zaman * 2.0f) * 0.03f;
    int tabanBoyut = dar ? max(30, min(64, (w - 48) / 9)) : 64;
    int boyut = (int)(tabanBoyut * nabiz);
    const char* baslik = "BORSA PATRONU";
    int tw = YaziOlc(baslik, boyut);
    int baslikY = (int)a.basBtn.rect.y - 170;
    TestKaydiDurdur();
    YaziCiz(baslik, w / 2 - tw / 2 + 3, baslikY + 3, boyut, Fade(BLACK, 0.5f));
    TestKaydiSurdur();
    YaziCiz(baslik, w / 2 - tw / 2, baslikY, boyut, RENK_ALTIN);
    YaziCizSariliOrtali(M("Küçük bir sermayeyle borsa imparatorluğu kur", "Build a market empire from a small stake"), w / 2,
                        baslikY + tabanBoyut + 28, dar ? 16 : 20, w - 40, RENK_SOLUK_METIN, 24);

    a.basBtn.Ciz(22);
    a.senaryoBtn.Ciz(16);
    a.yukleBtn.Ciz(16);
    a.basarimBtn.Ciz(16);
    if (PlatformdaCikisVar()) a.cikisMenuBtn.Ciz(16);
    a.dilBtn.Ciz(14);

    if (!a.menuMesaj.empty())
        YaziCizSariliOrtali(a.menuMesaj, w / 2, (int)(a.cikisMenuBtn.rect.y + a.cikisMenuBtn.rect.height) + 18, 14, w - 40,
                            a.menuMesajRenk, 18);

    YaziCizSariliOrtali(M("Haberi oku, 5 saniye içinde karar ver, rütbe atla, şirketleri devral!",
                          "Read the news, decide within 5 seconds, rank up, take over companies!"),
                        w / 2, h - (dar ? 84 : 64), 14, w - 40, RENK_SOLUK_METIN, 18);
    if (!dar)
        YaziCizOrtali(M("Kısayollar:  Boşluk / P duraklat   -   1-8 işlem adedi   -   M ses",
                        "Shortcuts:  Space / P pause   -   1-8 trade size   -   M sound"),
                      w / 2, h - 40, 12, Fade(RENK_SOLUK_METIN, 0.75f));
    YaziCizSaga(OYUN_SURUMU, w - 16, h - 26, 14, RENK_KENARLIK);
}

// ---------------------------------------------------------------------------
// SENARYO SECIMI
// ---------------------------------------------------------------------------
static void SenaryoEkraniGuncelle(Uygulama& u, float dt, Vector2 fare, bool tik) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (a.senaryoOnaySuresi > 0.0f) a.senaryoOnaySuresi -= dt;
    else a.senaryoOnay = -1;

    // Islem yapilmis bir oyun varsa yeni oyun onu siler: iki asamali onay.
    const bool onayGerek = u.oyunVar && o.ilkIslemYapildi;
    for (int i = 0; i < SENARYO_SAYISI; i++) {
        if (!a.senaryoBaslaBtn[i].Guncelle(dt, fare, tik)) continue;
        if (onayGerek && a.senaryoOnay != i) {
            a.senaryoOnay = i;
            a.senaryoOnaySuresi = SIFIRLA_ONAY_SN;
            Cal(o.sesler.hataUyari);
        } else {
            a.senaryoOnay = -1;
            Cal(o.sesler.tikla);
            YeniOyunBaslat(u, i);
        }
    }
    if (a.geriBtn.Guncelle(dt, fare, tik)) {
        Cal(o.sesler.tikla);
        u.ekran = Ekran::MENU;
    }
}

// Senaryonun rekor satiri ("Rekor: 87 gun" / "Bugun: 48.320 TL" / "Rekor: -").
static string SenaryoRekorMetni(int i) {
    const Senaryo& s = SENARYOLAR[i];
    if (s.gunluk) {
        float bugun = BugunkuGunlukSkor();
        return string(M("Bugün: ", "Today: ")) + (bugun > 0.0f ? Para(bugun, 0) : string("-"));
    }
    string rekor = M("Rekor: ", "Record: ");
    if (s.gunSiniri > 0) rekor += (g_profil.rekorNet[i] > 0.0f) ? Para(g_profil.rekorNet[i], 0) : string("-");
    else rekor += (g_profil.rekorGun[i] > 0) ? to_string(g_profil.rekorGun[i]) + M(" gün", " days") : string("-");
    return rekor;
}

static void SenaryoEkraniCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    const int w = PENCERE_GENISLIK;
    const bool dar = DarEkran();
    MenuArkaPlaniCiz(u.zaman);
    const int kay = (int)MenuKaymasi();
    YaziCizOrtali(M("SENARYO SEÇ", "CHOOSE A SCENARIO"), w / 2, dar ? 28 : 44 + kay, dar ? 26 : 36, RENK_ALTIN);
    if (!dar)
        YaziCizOrtali(M("Her senaryonun kendi rekoru tutulur.", "Each scenario keeps its own record."), w / 2, 90 + kay, 14,
                      RENK_SOLUK_METIN);

    for (int i = 0; i < SENARYO_SAYISI; i++) {
        const Senaryo& s = SENARYOLAR[i];
        Rectangle k = a.senaryoKart[i];
        PanelCiz(k, dar ? 0.10f : 0.05f, RENK_PANEL, RENK_KENARLIK);
        string sure = (s.gunSiniri > 0) ? to_string(s.gunSiniri) + M(" gün", " days") : string(M("sınırsız", "unlimited"));
        string koruma = s.acemiKorumasi ? M("var", "yes") : M("yok", "no");

        if (dar) {
            // Dikey: tek satirlik genis kart. Sol kenarda senaryo rengi.
            DrawRectangleRounded({k.x, k.y, 6, k.height}, 0.0f, 4, s.renk);
            const int kx = (int)k.x + 16, ky = (int)k.y, sagX = (int)(k.x + k.width) - 14;
            YaziCiz(s.ad.al(), kx, ky + 10, 18, s.renk);
            string rekor = SenaryoRekorMetni(i);
            if (sagX - YaziOlc(rekor, 12) > kx + YaziOlc(s.ad.al(), 18) + 10) YaziCizSaga(rekor, sagX, ky + 14, 12, RENK_METIN);
            YaziCizSarili(s.aciklama.al(), kx, ky + 36, 12, (int)k.width - 30, RENK_METIN, 3, 15);
            int solGenislik = (int)a.senaryoBaslaBtn[i].rect.x - kx - 10;
            YaziCiz(Kisalt(s.hedef.al(), 12, solGenislik), kx, ky + 90, 12, RENK_ALTIN);
            string ozet = Para(s.baslangicBakiye, 0) + "  -  " + sure + "  -  " + M("acemi koruması ", "protection ") + koruma;
            YaziCiz(Kisalt(ozet, 11, solGenislik), kx, ky + 110, 11, RENK_SOLUK_METIN);
        } else {
            DrawRectangleRounded({k.x, k.y, k.width, 6}, 0.9f, 4, s.renk);
            const int kx = (int)k.x + 16, ky = (int)k.y, kw = (int)k.width - 32;
            YaziCiz(Kisalt(s.ad.al(), 18, kw), kx, ky + 22, 18, s.renk);
            YaziCizSarili(s.aciklama.al(), kx, ky + 54, 13, kw, RENK_METIN, 7, 17);
            YaziCizSarili(s.hedef.al(), kx, ky + 184, 13, kw, RENK_ALTIN, 2, 17);
            YaziCiz(Kisalt(string(M("Başlangıç: ", "Start: ")) + Para(s.baslangicBakiye, 0), 12, kw), kx, ky + 232, 12, RENK_SOLUK_METIN);
            YaziCiz(Kisalt(string(M("Süre: ", "Time: ")) + sure, 12, kw), kx, ky + 252, 12, RENK_SOLUK_METIN);
            YaziCiz(Kisalt(string(M("Acemi koruması: ", "Beginner protection: ")) + koruma, 12, kw), kx, ky + 272, 12, RENK_SOLUK_METIN);
            YaziCiz(Kisalt(SenaryoRekorMetni(i), 14, kw), kx, ky + 306, 14, RENK_METIN);
            if (s.gunluk)
                YaziCiz(Kisalt(TarihMetni(BugunTarih()), 12, kw), kx, ky + 328, 12, RENK_SOLUK_METIN);
        }
        a.senaryoBaslaBtn[i].Ciz(dar ? 14 : 16);
    }

    a.geriBtn.Ciz(dar ? 14 : 16);
    if (u.oyunVar && o.ilkIslemYapildi) {
        const char* uyari = M("Dikkat: yeni oyun, süren oyunun kaydedilmemiş ilerlemesini siler.",
                              "Careful: a new game discards unsaved progress in your current one.");
        int solX = (int)(a.geriBtn.rect.x + a.geriBtn.rect.width) + 14;
        int sagX = (int)(a.senaryoKart[SENARYO_SAYISI - 1].x + a.senaryoKart[SENARYO_SAYISI - 1].width);
        if (dar) sagX = w - 12;
        YaziCizSarili(uyari, solX, (int)a.geriBtn.rect.y + (dar ? 4 : 14), dar ? 11 : 14, sagX - solX, RENK_UYARI, 2, dar ? 14 : 18);
    }
}

// ---------------------------------------------------------------------------
// BASARIMLAR
// ---------------------------------------------------------------------------
static void BasarimEkraniGuncelle(Uygulama& u, float dt, Vector2 fare, bool tik) {
    if (u.ui.geriBtn.Guncelle(dt, fare, tik)) {
        Cal(u.oyun.sesler.tikla);
        u.ekran = Ekran::MENU;
    }
}

static void BasarimEkraniCiz(Uygulama& u) {
    Arayuz& a = u.ui;
    const int w = PENCERE_GENISLIK;
    const bool dar = DarEkran();
    MenuArkaPlaniCiz(u.zaman);
    string baslik = string(M("BAŞARIMLAR", "ACHIEVEMENTS")) + "   " + to_string(AcilanBasarimSayisi()) + " / " +
                    to_string((int)BASARIM_SAYISI);
    YaziCizOrtali(baslik, w / 2, dar ? 24 : 40 + (int)MenuKaymasi(), dar ? 24 : 32, RENK_ALTIN);

    // Yatay: 3 sutun, genis karo. Dikey: 2 sutun, kucuk karo.
    const int sutun = dar ? 2 : 3;
    const float boslukX = dar ? 8.0f : 14.0f, boslukY = dar ? 6.0f : 10.0f;
    const float kw = dar ? (w - 24 - boslukX) / 2.0f : fminf(386.0f, (w - 40 - 2 * boslukX) / 3.0f);
    const float kh = dar ? 76.0f : 78.0f;
    const float x0 = (w - (sutun * kw + (sutun - 1) * boslukX)) / 2.0f;
    const float y0 = dar ? 68.0f : 100.0f + MenuKaymasi();
    for (int i = 0; i < BASARIM_SAYISI; i++) {
        bool acik = g_profil.basarim[i];
        Rectangle k = {x0 + (i % sutun) * (kw + boslukX), y0 + (i / sutun) * (kh + boslukY), kw, kh};
        PanelCiz(k, 0.10f, RENK_PANEL, acik ? RENK_ALTIN : RENK_KENARLIK);

        // Madalya (ilkel sekillerle; fontta ikon glifi yok)
        float r = dar ? 13.0f : 20.0f;
        Vector2 m = dar ? Vector2{k.x + 22, k.y + 20} : Vector2{k.x + 40, k.y + kh / 2.0f};
        DrawCircleV(m, r + 5.0f, acik ? Fade(RENK_ALTIN, 0.25f) : Fade(RENK_KENARLIK, 0.6f));
        DrawCircleV(m, r, acik ? RENK_ALTIN : RENK_KENARLIK);
        if (acik) DrawPoly(m, 5, r * 0.6f, -90.0f, RENK_ARKAPLAN_UST);

        if (dar) {
            YaziCiz(Kisalt(BASARIMLAR[i].ad.al(), 13, (int)kw - 52), (int)k.x + 44, (int)k.y + 9, 13, acik ? RENK_ALTIN : RENK_METIN);
            YaziCizSarili(BASARIMLAR[i].aciklama.al(), (int)k.x + 10, (int)k.y + 38, 11, (int)kw - 20,
                          acik ? RENK_METIN : RENK_SOLUK_METIN, 2, 14);
        } else {
            YaziCiz(Kisalt(BASARIMLAR[i].ad.al(), 16, (int)kw - 88), (int)k.x + 76, (int)k.y + 12, 16, acik ? RENK_ALTIN : RENK_METIN);
            YaziCizSarili(BASARIMLAR[i].aciklama.al(), (int)k.x + 76, (int)k.y + 36, 12, (int)kw - 88,
                          acik ? RENK_METIN : RENK_SOLUK_METIN, 2, 16);
        }
    }
    a.geriBtn.Ciz(dar ? 14 : 16);
}
