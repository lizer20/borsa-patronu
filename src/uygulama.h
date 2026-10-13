// ============================================================================
// uygulama.h - pencere, sahne boyutu, girdi ve KARE DONGUSU
// ============================================================================
// Bir kare:
//   SahneBoyutunuGuncelle  pencerenin en-boy oranina gore sanal sahneyi secer
//   GirdiOku()             fare/klavye -> Girdi (sanal sahne koordinatlarinda)
//   KareGuncelle(u,dt,g)   tiklamalar + sabit adimli simulasyon
//   KareCiz(u)             her seyi sahne dokusuna cizer
// Sonra sahne dokusu gercek pencereye basilir. Testler ilk iki adimi atlayip
// sahne boyutunu ve girdiyi kendileri verir (SahneBoyutuAyarla / sahte Girdi).

// Sanal sahneyi verilen boyuta getirir: dokuyu yeniden olusturur ve yerlesimi
// yeniden kurar. Boyut degismediyse hicbir sey yapmaz.
static void SahneBoyutuAyarla(Uygulama& u, int w, int h, bool dikey) {
    if (w == PENCERE_GENISLIK && h == PENCERE_YUKSEKLIK && dikey == g_dikey && u.sahne.id != 0) return;
    PENCERE_GENISLIK = w;
    PENCERE_YUKSEKLIK = h;
    g_dikey = dikey;
    if (u.sahne.id != 0) UnloadRenderTexture(u.sahne);
    u.sahne = LoadRenderTexture(w, h);
    SetTextureFilter(u.sahne.texture, TEXTURE_FILTER_BILINEAR);
    ArayuzKur(u.ui, (int)u.oyun.hisseler.size());
}

// Pencerenin en-boy oranina gore sanal sahneyi secer (bkz. temel.h):
//   16:9'dan genis pencere -> YATAY, yukseklik 720, genislik 1280..1920
//   16:9'dan kare pencere  -> YATAY, genislik 1280, yukseklik 720..960
//   dar/uzun pencere       -> DIKEY: telefon yerlesimi
// Boylece yaygin her pencerede sahne pencereyi BOSLUKSUZ doldurur.
static void SahneBoyutunuGuncelle(Uygulama& u) {
    int en = GetScreenWidth(), boy = GetScreenHeight();
    if (en <= 0 || boy <= 0) return;
    float oran = (float)en / (float)boy;
    int w, h;
    bool dikey = oran < DIKEY_ESIK_ORAN;
    if (dikey) {
        h = (int)roundf((float)DIKEY_MIN_GENISLIK / oran);
        h = max(DIKEY_MIN_YUKSEKLIK, min(DIKEY_MAX_YUKSEKLIK, h));
        w = max(DIKEY_MIN_GENISLIK, (int)roundf((float)h * oran));
    } else if (oran >= (float)YATAY_MIN_GENISLIK / (float)YATAY_YUKSEKLIK) {
        h = YATAY_YUKSEKLIK;
        w = min(YATAY_MAX_GENISLIK, (int)roundf((float)h * oran));
    } else {
        w = YATAY_MIN_GENISLIK;
        h = min(YATAY_MAX_YUKSEKLIK, (int)roundf((float)w / oran));
    }
    w -= w % 2; // cift sayi (ortalamalar yarim piksele dusmesin)
    h -= h % 2;
    SahneBoyutuAyarla(u, w, h, dikey);
}

// Sanal sahnenin gercek ekrandaki olcegi ve konumu (sigmayan yonde letterbox).
static void SahneYerlesimi(float& olcek, float& ofsX, float& ofsY) {
    float ekranW = (float)GetScreenWidth();
    float ekranH = (float)GetScreenHeight();
    olcek = fminf(ekranW / PENCERE_GENISLIK, ekranH / PENCERE_YUKSEKLIK);
    if (olcek <= 0.0f) olcek = 1.0f;
    ofsX = (ekranW - PENCERE_GENISLIK * olcek) / 2.0f;
    ofsY = (ekranH - PENCERE_YUKSEKLIK * olcek) / 2.0f;
}

static Girdi GirdiOku() {
    Girdi g;
    float olcek, ofsX, ofsY;
    SahneYerlesimi(olcek, ofsX, ofsY);
    Vector2 gercek = GetMousePosition();
    g.fare = {(gercek.x - ofsX) / olcek, (gercek.y - ofsY) / olcek};
    g.tiklandi = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    g.fareBasili = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    g.tusDuraklat = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_P);
    g.tusSes = IsKeyPressed(KEY_M);
    static const int tuslar[8] = {KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT};
    for (int i = 0; i < 8; i++)
        if (IsKeyPressed(tuslar[i])) g.tusAdet = i;
#if BORSA_WEB
    // Ayni kare icinde basilip birakilan tiklar raylib'in yoklamasindan kacar;
    // sayfadaki mousedown dinleyicisinden gelen tiki de sayiyoruz (bkz. platform.h).
    double bekleyenTik = js_tik_al();
    if (bekleyenTik >= 0.0) {
        float tx = (float)floor(bekleyenTik / 10000.0);
        float ty = (float)(bekleyenTik - (double)tx * 10000.0);
        g.fare = {(tx - ofsX) / olcek, (ty - ofsY) / olcek};
        g.tiklandi = true;
    }
#else
    // raylib'in WindowShouldClose()'u her cagrildiginda kapatma bayragini
    // sifirlar; istegi burada "yutup" kendi onay penceremizi gosterebiliyoruz.
    // (Web'de bu fonksiyon cagrilmamali: tarayicida pencere kapatma yoktur.)
    g.pencereKapatma = WindowShouldClose();
#endif
    return g;
}

static void UygulamaBaslat(Uygulama& u) {
    // Masaustunde pencere yeniden boyutlandirilabilir: buyutunce sahne pencereye
    // uyar. Web'de tuval boyutunu biz yonetiyoruz (asagida, her karede).
    unsigned int bayraklar = FLAG_MSAA_4X_HINT;
#if !BORSA_WEB
    bayraklar |= FLAG_WINDOW_RESIZABLE;
#endif
    SetConfigFlags(bayraklar);
    InitWindow(YATAY_MIN_GENISLIK, YATAY_YUKSEKLIK, "Borsa Patronu - Trading Tycoon");
#if !BORSA_WEB
    SetWindowMinSize(480, 360);
    SetTargetFPS(60); // web'de kare hizini tarayici (requestAnimationFrame) belirler
#endif
    SetExitKey(KEY_NULL);
    SetRandomSeed((unsigned int)time(NULL)); // (yalnizca gorsel efektler icin)
    SimTohumla((uint64_t)time(NULL) * 2654435761ULL + (uint64_t)clock());
#if BORSA_WEB
    js_tik_dinleyici_kur();
#endif

    InitAudioDevice();
    FontHazirla();

    ProfilYukle();
    if (g_profil.dil == 0 || g_profil.dil == 1) g_dil = (Dil)g_profil.dil;
    else g_dil = SistemDiliTurkce() ? Dil::TR : Dil::EN; // ilk acilis: sistem diline gore
    PaletiAyarla(g_profil.renkKoru);

    u.oyun.sesler.Yukle();
    g_sesler = &u.oyun.sesler;
    u.oyun.Baslat(S_KLASIK, false);
    SahneBoyutunuGuncelle(u); // sahne dokusunu olusturur ve yerlesimi kurar
    u.ui.yukleBtn.aktif = KayitVarMi();
}

// (Tarayicida "kapatma" yoktur - sekme kapaninca her sey birakilir; bu yuzden
// web derlemesinde cagrilmaz.)
[[maybe_unused]] static void UygulamaKapat(Uygulama& u) {
    UnloadRenderTexture(u.sahne);
    FontBosalt();
    u.oyun.sesler.Bosalt();
    g_sesler = nullptr;
    CloseAudioDevice();
    CloseWindow();
}

// ---------------------------------------------------------------------------
// OYUN EKRANI: guncelleme ve cizim
// ---------------------------------------------------------------------------
static bool OyundaPencereVar(const Uygulama& u) {
    return u.oyun.sonPenceresiAcik || u.oyun.yonetimOlayiAcik || u.ui.emirHisse >= 0 || u.ui.krediAcik || u.ui.ayarlarAcik;
}

static void OyunEkraniGuncelle(Uygulama& u, float dt, const Girdi& g) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    KartDuzeni kd = KartDuzeniHesapla(o, a);
    if (g_dikey) DikeyDuzenHesapla(u);
    else KartDugmeleriniYerlestir(a, o, kd);

    // Oyun alaninin tiklanabilir / simulasyonun calisir olup olmadigi karenin
    // BASINDAKI duruma gore belirlenir: bu karede kapanan bir pencerenin
    // dugmesine basilan tik, altta kalan AL/SAT dugmesine "delip gecmesin".
    const bool etkin = !OyundaPencereVar(u) && !o.duraklatildi && !a.cikisOnayiAcik;

    // En ustteki pencere tiki alir.
    if (o.sonPenceresiAcik) OyunSonuGuncelle(u, dt, g.fare, g.tiklandi);
    else if (o.yonetimOlayiAcik) YonetimGuncelle(u, dt, g.fare, g.tiklandi);
    else if (a.emirHisse >= 0) EmirPenceresiGuncelle(u, dt, g.fare, g.tiklandi);
    else if (a.krediAcik) KrediPenceresiGuncelle(u, dt, g.fare, g.tiklandi);
    else if (a.ayarlarAcik) AyarlarGuncelle(u, dt, g);
    else {
        bool duraklatTik = a.duraklatBtn.Guncelle(dt, g.fare, g.tiklandi);
        if (duraklatTik || g.tusDuraklat) {
            o.duraklatildi = !o.duraklatildi;
            Cal(o.sesler.tikla);
        } else if (o.duraklatildi) {
            DuraklatMenusuGuncelle(u, dt, g.fare, g.tiklandi);
        }
        if (g.tusSes) sesAcik = !sesAcik;
    }
    if (u.ekran != Ekran::OYUN) return; // bir pencere bizi ana menuye gonderdi

    if (g_dikey) DikeyOyunAlaniGuncelle(u, dt, g, etkin);
    else OyunAlaniGuncelle(u, dt, g, kd, etkin);

    // SABIT ADIMLI SIMULASYON: gecen sure biriktirilir ve SIM_ADIM_SN'lik esit
    // adimlarla harcanir. Kare hizi ne olursa olsun ayni tohum ayni piyasayi verir.
    if (etkin) {
        u.simBirikim += dt;
        int adim = 0;
        while (u.simBirikim >= SIM_ADIM_SN && adim < 8) {
            SimulasyonAdimi(o, SIM_ADIM_SN);
            u.simBirikim -= SIM_ADIM_SN;
            adim++;
        }
        if (adim >= 8) u.simBirikim = 0.0f; // cok geride kaldiysak yetismeye calisma
    }
}

static void OyunEkraniCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    KartDuzeni kd = KartDuzeniHesapla(o, a);
    if (g_dikey) DikeyDuzenHesapla(u);
    else KartDugmeleriniYerlestir(a, o, kd);

    DrawRectangleGradientV(0, 0, PENCERE_GENISLIK, PENCERE_YUKSEKLIK, RENK_ARKAPLAN_UST, RENK_ARKAPLAN_ALT);
    if (g_dikey) {
        DikeyOyunCiz(u, kd);
    } else {
        UstBarCiz(u);
        HaberBandiCiz(u);
        KartlarCiz(u, kd);
        KartAltiCiz(u, kd);
        AdetPaneliCiz(u);
        GrafikCiz(u, kd);
    }

    o.parcaciklar.Ciz();
    if (o.flashAlpha > 0.0f)
        DrawRectangle(0, 0, PENCERE_GENISLIK, PENCERE_YUKSEKLIK, Fade(o.flashRenk, o.flashAlpha * 0.25f));

    if (!OyundaPencereVar(u) && !o.duraklatildi) RehberCiz(u, kd);

    // Pencereler alttan uste. Zorunlu bir karar (yonetim / oyun sonu) acikken
    // duraklatma menusu gosterilmez - karardan kacilamaz.
    if (o.duraklatildi && !o.yonetimOlayiAcik && !o.sonPenceresiAcik) DuraklatMenusuCiz(u);
    if (a.ayarlarAcik) AyarlarCiz(u);
    if (a.emirHisse >= 0) EmirPenceresiCiz(u);
    if (a.krediAcik) KrediPenceresiCiz(u);
    if (o.yonetimOlayiAcik) YonetimCiz(u);
    if (o.sonPenceresiAcik) OyunSonuCiz(u);

    // Bildirimler her seyin ustunde (duraklatma menusunden gelen "Kaydedildi"
    // bildirimi karartmanin altinda kalmasin).
    KatmanBaslat();
    float toastY = 118.0f;
    if (g_dikey) {
        toastY = a.haberBandi.y + a.haberBandi.height + 8.0f;
        // Dikeyde rehber kutusu bildirimlerle AYNI yerde (grafigin ustunde) durur;
        // bildirimler kutunun altindan baslar ki metni ve dugmeleri ortulmesin.
        if (o.rehberAdim >= 0 && o.rehberBekleme <= 0.0f && !OyundaPencereVar(u) && !o.duraklatildi) {
            Rectangle kutu = RehberKutusuHesapla(a, o);
            toastY = kutu.y + kutu.height + 8.0f;
        }
    }
    o.toastlar.Ciz(PENCERE_GENISLIK, toastY);
}

// ---------------------------------------------------------------------------
// KARE
// ---------------------------------------------------------------------------
static void KareGuncelle(Uygulama& u, float dt, const Girdi& girdi) {
    Arayuz& a = u.ui;
    Girdi g = girdi;
    u.zaman += dt;
    EtiketleriYenile(u);

    if (g.pencereKapatma) CikisIste(u);
    if (a.cikisOnayiAcik) {
        CikisOnayiGuncelle(u, dt, g.fare, g.tiklandi);
        // Onay penceresi acikken girdi ALTTAKI ekrana gecmesin.
        g = Girdi();
    }

    switch (u.ekran) {
        case Ekran::MENU: MenuGuncelle(u, dt, g.fare, g.tiklandi); break;
        case Ekran::SENARYO: SenaryoEkraniGuncelle(u, dt, g.fare, g.tiklandi); break;
        case Ekran::BASARIM: BasarimEkraniGuncelle(u, dt, g.fare, g.tiklandi); break;
        case Ekran::OYUN: OyunEkraniGuncelle(u, dt, g); break;
    }
    GorselGuncelle(u.oyun, dt);
    MuzikGuncelle(g_profil.muzikAcik);
}

static void KareCiz(Uygulama& u) {
#ifdef BORSA_TEST
    g_testKutular.clear();
    g_testKatman = 0;
#endif
    EtiketleriYenile(u);
    BeginTextureMode(u.sahne);
    switch (u.ekran) {
        case Ekran::MENU: MenuCiz(u); break;
        case Ekran::SENARYO: SenaryoEkraniCiz(u); break;
        case Ekran::BASARIM: BasarimEkraniCiz(u); break;
        case Ekran::OYUN: OyunEkraniCiz(u); break;
    }
    // Cikis onayi her ekranin ustunde.
    if (u.ui.cikisOnayiAcik) CikisOnayiCiz(u);
    EndTextureMode();
}

static void SahneyiEkranaBas(Uygulama& u) {
    float olcek, ofsX, ofsY;
    SahneYerlesimi(olcek, ofsX, ofsY);
    BeginDrawing();
    ClearBackground(BLACK);
    Rectangle kaynak = {0, 0, (float)u.sahne.texture.width, -(float)u.sahne.texture.height};
    Rectangle hedef = {ofsX, ofsY, PENCERE_GENISLIK * olcek, PENCERE_YUKSEKLIK * olcek};
    DrawTexturePro(u.sahne.texture, kaynak, hedef, {0, 0}, 0.0f, WHITE);
    EndDrawing();
}

static void UygulamaKare(Uygulama& u) {
#if BORSA_WEB
    // Tuvali sayfanin boyutuna (ve ekranin piksel yogunluguna) uydur. Tarayici
    // penceresi/telefon yonu degisince burada yakalanir.
    double hedef = js_tuval_hedef_boyutu();
    int hedefW = (int)floor(hedef / 10000.0);
    int hedefH = (int)(hedef - (double)hedefW * 10000.0);
    if (hedefW > 0 && hedefH > 0 && (hedefW != GetScreenWidth() || hedefH != GetScreenHeight())) SetWindowSize(hedefW, hedefH);
#endif
    SahneBoyutunuGuncelle(u);

    // Sekme arka plana alinip geri gelince (ozellikle web'de) dev bir dt gelebilir;
    // simulasyon tek karede dakikalarca ilerlemesin.
    float dt = fminf(GetFrameTime(), 0.1f);
    Girdi g = GirdiOku();
    KareGuncelle(u, dt, g);
    KareCiz(u);
    SahneyiEkranaBas(u);
}
