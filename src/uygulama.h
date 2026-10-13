// ============================================================================
// uygulama.h - pencere, girdi ve KARE DONGUSU
// ============================================================================
// Bir kare uc adimdir:
//   GirdiOku()            fare/klavye -> Girdi (sanal 1280x720 koordinatlarda)
//   KareGuncelle(u,dt,g)  tiklamalar + simulasyon
//   KareCiz(u)            her seyi sabit 1280x720 sahne dokusuna cizer
// Sonra sahne dokusu gercek pencere boyutuna, en-boy orani korunarak basilir.
// Testler ilk adimi atlayip KareGuncelle'ye SAHTE girdi verir.

// Sanal sahnenin gercek ekrandaki olcegi ve konumu (letterbox).
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
#if !BORSA_WEB
    // raylib'in WindowShouldClose()'u her cagrildiginda kapatma bayragini
    // sifirlar; istegi burada "yutup" kendi onay penceremizi gosterebiliyoruz.
    // (Web'de bu fonksiyon cagrilmamali: tarayicida pencere kapatma yoktur.)
    g.pencereKapatma = WindowShouldClose();
#endif
    return g;
}

static void UygulamaBaslat(Uygulama& u) {
    unsigned int bayraklar = FLAG_MSAA_4X_HINT;
#if BORSA_WEB
    bayraklar |= FLAG_WINDOW_RESIZABLE; // tuval, sayfanin/iframe'in boyutunu izlesin
#endif
    SetConfigFlags(bayraklar);
    InitWindow(PENCERE_GENISLIK, PENCERE_YUKSEKLIK, "Borsa Patronu - Trading Tycoon");
#if !BORSA_WEB
    SetTargetFPS(60); // web'de kare hizini tarayici (requestAnimationFrame) belirler
#endif
    SetExitKey(KEY_NULL);
    SetRandomSeed((unsigned int)time(NULL));

    // Oyun her zaman 1280x720 (tam 16:9) sanal sahneye cizilir; 1920x1080 /
    // 2560x1440 gibi yaygin ekranlarda olcek tam sayi cikar ve pillarbox kalmaz.
    u.sahne = LoadRenderTexture(PENCERE_GENISLIK, PENCERE_YUKSEKLIK);
    SetTextureFilter(u.sahne.texture, TEXTURE_FILTER_BILINEAR);

    InitAudioDevice();
    FontHazirla();

    ProfilYukle();
    if (g_profil.dil == 0 || g_profil.dil == 1) g_dil = (Dil)g_profil.dil;
    else g_dil = SistemDiliTurkce() ? Dil::TR : Dil::EN; // ilk acilis: sistem diline gore

    u.oyun.sesler.Yukle();
    g_sesler = &u.oyun.sesler;
    u.oyun.Baslat(S_KLASIK, false);
    ArayuzKur(u.ui, (int)u.oyun.hisseler.size());
    u.ui.yukleBtn.aktif = KayitVarMi();
}

static void UygulamaKapat(Uygulama& u) {
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
static void OyunEkraniGuncelle(Uygulama& u, float dt, const Girdi& g) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    KartDuzeni kd = KartDuzeniHesapla(o, a);
    KartDugmeleriniYerlestir(a, o, kd);

    // Oyun alaninin tiklanabilir / simulasyonun calisir olup olmadigi karenin
    // BASINDAKI duruma gore belirlenir: bu karede kapanan bir pencerenin
    // dugmesine basilan tik, altta kalan AL/SAT dugmesine "delip gecmesin".
    const bool pencereVar = o.sonPenceresiAcik || o.yonetimOlayiAcik || a.emirHisse >= 0 || a.krediAcik || a.ayarlarAcik;
    const bool etkin = !pencereVar && !o.duraklatildi && !a.cikisOnayiAcik;

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

    OyunAlaniGuncelle(u, dt, g, kd, etkin);
    if (etkin) SimulasyonAdimi(o, dt);
}

static void OyunEkraniCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    KartDuzeni kd = KartDuzeniHesapla(o, a);
    KartDugmeleriniYerlestir(a, o, kd);

    DrawRectangleGradientV(0, 0, PENCERE_GENISLIK, PENCERE_YUKSEKLIK, RENK_ARKAPLAN_UST, RENK_ARKAPLAN_ALT);
    UstBarCiz(u);
    HaberBandiCiz(u);
    KartlarCiz(u, kd);
    KartAltiCiz(u, kd);
    AdetPaneliCiz(u);
    GrafikCiz(u, kd);

    o.parcaciklar.Ciz();
    if (o.flashAlpha > 0.0f)
        DrawRectangle(0, 0, PENCERE_GENISLIK, PENCERE_YUKSEKLIK, Fade(o.flashRenk, o.flashAlpha * 0.25f));

    const bool pencereVar = o.sonPenceresiAcik || o.yonetimOlayiAcik || a.emirHisse >= 0 || a.krediAcik || a.ayarlarAcik;
    if (!pencereVar && !o.duraklatildi) RehberCiz(u, kd);

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
    o.toastlar.Ciz(PENCERE_GENISLIK);
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
    // Sekme arka plana alinip geri gelince (ozellikle web'de) dev bir dt gelebilir;
    // simulasyon tek karede dakikalarca ilerlemesin.
    float dt = fminf(GetFrameTime(), 0.1f);
    Girdi g = GirdiOku();
    KareGuncelle(u, dt, g);
    KareCiz(u);
    SahneyiEkranaBas(u);
}
