// ============================================================================
// arayuz_menu.h - ana menu, senaryo secimi ve basarimlar ekranlari
// ============================================================================

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
    u.ui.sifirlaOnaySuresi = 0.0f;
    u.ui.emirHisse = -1;
    u.ui.krediAcik = false;
    u.ui.ayarlarAcik = false;
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
    MenuArkaPlaniCiz(u.zaman);

    // Baslik hafifce nabiz atar; golgesi ayni yazinin kaydirilmis kopyasidir
    // (yerlesim denetcisi bunu cakisma saymasin diye golge kaydedilmez).
    float nabiz = 1.0f + sinf(u.zaman * 2.0f) * 0.03f;
    int boyut = (int)(64 * nabiz);
    const char* baslik = "BORSA PATRONU";
    int tw = YaziOlc(baslik, boyut);
    TestKaydiDurdur();
    YaziCiz(baslik, w / 2 - tw / 2 + 3, 133, boyut, Fade(BLACK, 0.5f));
    TestKaydiSurdur();
    YaziCiz(baslik, w / 2 - tw / 2, 130, boyut, RENK_ALTIN);
    YaziCizOrtali(M("Küçük bir sermayeyle borsa imparatorluğu kur", "Build a market empire from a small stake"), w / 2, 222, 20,
                  RENK_SOLUK_METIN);

    a.basBtn.Ciz(22);
    a.senaryoBtn.Ciz(16);
    a.yukleBtn.Ciz(16);
    a.basarimBtn.Ciz(16);
    if (PlatformdaCikisVar()) a.cikisMenuBtn.Ciz(16);
    a.dilBtn.Ciz(14);

    if (!a.menuMesaj.empty()) YaziCizOrtali(a.menuMesaj, w / 2, 584, 14, a.menuMesajRenk);

    YaziCizOrtali(M("Haberi oku, 5 saniye içinde karar ver, rütbe atla, şirketleri devral!",
                    "Read the news, decide within 5 seconds, rank up, take over companies!"),
                  w / 2, h - 64, 14, RENK_SOLUK_METIN);
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

static void SenaryoEkraniCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    const int w = PENCERE_GENISLIK;
    MenuArkaPlaniCiz(u.zaman);
    YaziCizOrtali(M("SENARYO SEÇ", "CHOOSE A SCENARIO"), w / 2, 44, 36, RENK_ALTIN);
    YaziCizOrtali(M("Her senaryonun kendi rekoru tutulur.", "Each scenario keeps its own record."), w / 2, 90, 14,
                  RENK_SOLUK_METIN);

    for (int i = 0; i < SENARYO_SAYISI; i++) {
        const Senaryo& s = SENARYOLAR[i];
        Rectangle k = a.senaryoKart[i];
        PanelCiz(k, 0.05f, RENK_PANEL, RENK_KENARLIK);
        DrawRectangleRounded({k.x, k.y, k.width, 6}, 0.9f, 4, s.renk);
        const int kx = (int)k.x + 18, ky = (int)k.y, kw = (int)k.width - 36;

        YaziCiz(s.ad.al(), kx, ky + 22, 22, s.renk);
        YaziCizSarili(s.aciklama.al(), kx, ky + 58, 13, kw, RENK_METIN, 6, 18);
        YaziCizSarili(s.hedef.al(), kx, ky + 176, 13, kw, RENK_ALTIN, 2, 18);

        string p1 = string(M("Başlangıç: ", "Start: ")) + Para(s.baslangicBakiye, 0);
        string p2 = string(M("Süre: ", "Time: ")) +
                    (s.gunSiniri > 0 ? to_string(s.gunSiniri) + M(" gün", " days") : string(M("sınırsız", "unlimited")));
        string p3 = string(M("Acemi koruması: ", "Beginner protection: ")) + (s.acemiKorumasi ? M("var", "yes") : M("yok", "no"));
        YaziCiz(p1, kx, ky + 228, 13, RENK_SOLUK_METIN);
        YaziCiz(p2, kx, ky + 250, 13, RENK_SOLUK_METIN);
        YaziCiz(p3, kx, ky + 272, 13, RENK_SOLUK_METIN);

        string rekor = M("Rekor: ", "Record: ");
        if (s.gunSiniri > 0) rekor += (g_profil.rekorNet[i] > 0.0f) ? Para(g_profil.rekorNet[i], 0) : string("-");
        else rekor += (g_profil.rekorGun[i] > 0) ? to_string(g_profil.rekorGun[i]) + M(" gün", " days") : string("-");
        YaziCiz(rekor, kx, ky + 310, 15, RENK_METIN);

        a.senaryoBaslaBtn[i].Ciz(16);
    }

    a.geriBtn.Ciz(16);
    if (u.oyunVar && o.ilkIslemYapildi)
        YaziCizSaga(M("Dikkat: yeni oyun, süren oyunun kaydedilmemiş ilerlemesini siler.",
                      "Careful: a new game discards unsaved progress in your current one."),
                    (int)(a.senaryoKart[SENARYO_SAYISI - 1].x + a.senaryoKart[SENARYO_SAYISI - 1].width), 654, 14, RENK_UYARI);
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
    MenuArkaPlaniCiz(u.zaman);
    string baslik = string(M("BAŞARIMLAR", "ACHIEVEMENTS")) + "   " + to_string(AcilanBasarimSayisi()) + " / " +
                    to_string((int)BASARIM_SAYISI);
    YaziCizOrtali(baslik, w / 2, 40, 32, RENK_ALTIN);

    const int sutun = 3;
    const float kw = 386, kh = 92, boslukX = 14, boslukY = 12;
    const float x0 = (w - (sutun * kw + (sutun - 1) * boslukX)) / 2.0f, y0 = 104;
    for (int i = 0; i < BASARIM_SAYISI; i++) {
        bool acik = g_profil.basarim[i];
        Rectangle k = {x0 + (i % sutun) * (kw + boslukX), y0 + (i / sutun) * (kh + boslukY), kw, kh};
        PanelCiz(k, 0.10f, RENK_PANEL, acik ? RENK_ALTIN : RENK_KENARLIK);

        // Madalya (ilkel sekillerle; fontta ikon glifi yok)
        Vector2 m = {k.x + 46, k.y + kh / 2.0f};
        DrawCircleV(m, 26.0f, acik ? Fade(RENK_ALTIN, 0.25f) : Fade(RENK_KENARLIK, 0.6f));
        DrawCircleV(m, 20.0f, acik ? RENK_ALTIN : RENK_KENARLIK);
        if (acik) DrawPoly(m, 5, 12.0f, -90.0f, RENK_ARKAPLAN_UST);
        else YaziCizOrtali("?", (int)m.x, (int)m.y - 9, 20, RENK_SOLUK_METIN);

        YaziCiz(BASARIMLAR[i].ad.al(), (int)k.x + 88, (int)k.y + 16, 18, acik ? RENK_ALTIN : RENK_METIN);
        YaziCizSarili(BASARIMLAR[i].aciklama.al(), (int)k.x + 88, (int)k.y + 44, 13, (int)kw - 88 - 14,
                      acik ? RENK_METIN : RENK_SOLUK_METIN, 2, 18);
    }
    a.geriBtn.Ciz(16);
}
