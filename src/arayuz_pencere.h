// ============================================================================
// arayuz_pencere.h - duraklatma menusu, ayarlar, yonetim karari, cikis onayi
// ============================================================================
// Her pencere icin iki fonksiyon vardir: ...Guncelle (tiklamalar) ve ...Ciz.
// Hangi pencerenin tiki alacagina uygulama.h karar verir (en usttekine).

// Oyundan cikis istegi: kaydedilmemis ilerleme varsa once onay penceresi.
static void CikisIste(Uygulama& u) {
    if (!PlatformdaCikisVar()) return;
    if (u.oyunVar && u.oyun.ilkIslemYapildi) u.ui.cikisOnayiAcik = true;
    else u.cikisIstendi = true;
}

// Bir dugmenin ICINE elle yazilan ek metin (yerlesim denetcisi bunu "dugmenin
// ustune binmis serbest yazi" saymasin; sigmiyorsa da raporlasin diye).
static void DugmeIciYazi(const Buton& b, const string& metin, int dx, int dy, int boyut, Color renk) {
#ifdef BORSA_TEST
    if (YaziOlc(metin, boyut) > (int)b.rect.width - dx - 8) TestKutuEkle(b.rect, metin, 3);
    g_testButonIci = true;
#endif
    YaziCiz(metin, (int)b.rect.x + dx, (int)b.rect.y + dy, boyut, renk);
#ifdef BORSA_TEST
    g_testButonIci = false;
#endif
}

// ---------------------------------------------------------------------------
// DURAKLATMA MENUSU
// ---------------------------------------------------------------------------
static void DuraklatMenusuGuncelle(Uygulama& u, float dt, Vector2 fare, bool tik) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (a.devamBtn.Guncelle(dt, fare, tik)) {
        o.duraklatildi = false;
        Cal(o.sesler.tikla);
    }
    if (a.kaydetBtn.Guncelle(dt, fare, tik)) {
        if (OyunuKaydet(o)) {
            o.toastlar.Ekle(M("Oyun Kaydedildi", "Game Saved"),
                            M("İlerlemen başarıyla kaydedildi.", "Your progress has been saved."), RENK_YESIL, 3.0f);
            Cal(o.sesler.tikla);
        } else {
            o.toastlar.Ekle(M("Kayıt Başarısız", "Save Failed"), M("Kayıt yazılamadı.", "Could not write the save."),
                            RENK_KIRMIZI, 3.5f);
            Cal(o.sesler.hataUyari);
        }
    }
    if (a.ayarlarBtn.Guncelle(dt, fare, tik)) {
        a.ayarlarAcik = true;
        Cal(o.sesler.tikla);
    }
    if (a.anaMenuBtn.Guncelle(dt, fare, tik)) {
        u.ekran = Ekran::MENU;
        Cal(o.sesler.tikla);
    }
    if (PlatformdaCikisVar() && a.cikisBtn.Guncelle(dt, fare, tik)) CikisIste(u);
}

static void DuraklatMenusuCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    ModalKarart(0.55f);
    PanelCiz(a.dmPanel, 0.06f, RENK_PANEL, RENK_ALTIN);
    const int ortaX = PENCERE_GENISLIK / 2;
    YaziCizOrtali(M("DURAKLATILDI", "PAUSED"), ortaX, (int)a.dmPanel.y + 26, 30, RENK_ALTIN);

    a.devamBtn.Ciz(16);
    a.kaydetBtn.Ciz(16);
    a.ayarlarBtn.Ciz(16);
    a.anaMenuBtn.Ciz(16);
    if (PlatformdaCikisVar()) a.cikisBtn.Ciz(16);

    // Istatistikler: ayri bir ekran yerine burada - herkes bir noktada
    // duraklatir, yani kesin gorulen bir yer.
    string ist1 = string(M("İşlem: ", "Trades: ")) + to_string(o.islemSayisi) + "     " +
                  M("Gerçekleşen K/Z: ", "Realized P/L: ") + ParaIsaretli(o.gerceklesenKar);
    string ist2 = string(M("En iyi satış: ", "Best sale: ")) + ParaIsaretli(o.enIyiIslem) + "     " +
                  M("En kötü: ", "Worst: ") + Para(o.enKotuIslem);
    int y = (int)(a.dmPanel.y + a.dmPanel.height) + 18;
    YaziCizOrtali(ist1, ortaX, y, 14, o.gerceklesenKar >= 0 ? RENK_YESIL : RENK_KIRMIZI);
    YaziCizOrtali(ist2, ortaX, y + 22, 13, RENK_SOLUK_METIN);
}

// ---------------------------------------------------------------------------
// AYARLAR
// ---------------------------------------------------------------------------
static void AyarlarGuncelle(Uygulama& u, float dt, const Girdi& g) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    Vector2 fare = g.fare;
    bool tik = g.tiklandi;

    if (a.sesBtn.Guncelle(dt, fare, tik)) {
        sesAcik = !sesAcik;
        Cal(o.sesler.tikla);
    }
    if (a.rehberBtn.Guncelle(dt, fare, tik)) {
        if (o.rehberAdim >= 0) {
            RehberiBitir(o);
        } else {
            o.rehberAdim = 0;
            o.rehberBekleme = 0.3f;
            o.rehberSatisSayisi = 0;
        }
        Cal(o.sesler.tikla);
    }
    if (PlatformdaCikisVar()) {
        if (a.modPencereliBtn.Guncelle(dt, fare, tik)) {
            if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) ToggleBorderlessWindowed();
            if (IsWindowFullscreen()) ToggleFullscreen();
            Cal(o.sesler.tikla);
        }
        if (a.modTamEkranBtn.Guncelle(dt, fare, tik)) {
            if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) ToggleBorderlessWindowed();
            if (!IsWindowFullscreen()) ToggleFullscreen();
            Cal(o.sesler.tikla);
        }
        if (a.modKenarsizBtn.Guncelle(dt, fare, tik)) {
            if (IsWindowFullscreen()) ToggleFullscreen();
            if (!IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) ToggleBorderlessWindowed();
            Cal(o.sesler.tikla);
        }
        if (a.kucultBtn.Guncelle(dt, fare, tik)) {
            Cal(o.sesler.tikla);
            MinimizeWindow();
        }
    }
    if (a.dilTrBtn.Guncelle(dt, fare, tik)) {
        DilAyarla(Dil::TR);
        Cal(o.sesler.tikla);
    }
    if (a.dilEnBtn.Guncelle(dt, fare, tik)) {
        DilAyarla(Dil::EN);
        Cal(o.sesler.tikla);
    }

    // Ses seviyesi kaydiricisi ("tut ve surukle")
    Rectangle iz = a.sesKaydirici;
    if (g.fareBasili && IsAudioDeviceReady() &&
        CheckCollisionPointRec(fare, {iz.x - 8, iz.y - 10, iz.width + 16, iz.height + 20})) {
        SetMasterVolume(Clamp((fare.x - iz.x) / iz.width, 0.0f, 1.0f));
    }

    bool disariTik = tik && !CheckCollisionPointRec(fare, a.ayarPanel);
    if (a.ayarlarKapatBtn.Guncelle(dt, fare, tik) || disariTik) {
        a.ayarlarAcik = false;
        Cal(o.sesler.tikla);
    }
}

static void AyarlarCiz(Uygulama& u) {
    Arayuz& a = u.ui;
    ModalKarart(0.6f);
    Rectangle p = a.ayarPanel;
    PanelCiz(p, 0.04f, RENK_PANEL, RENK_KENARLIK);
    const int px = (int)p.x, py = (int)p.y;

    YaziCiz(M("AYARLAR", "SETTINGS"), px + 30, py + 22, 24, RENK_ALTIN);

    YaziCiz(M("SES VE REHBER", "SOUND AND GUIDE"), px + 30, py + 68, 14, RENK_SOLUK_METIN);
    a.sesBtn.Ciz(14);
    a.rehberBtn.Ciz(14);

    float seviye = IsAudioDeviceReady() ? GetMasterVolume() : 0.0f;
    Rectangle iz = a.sesKaydirici;
    YaziCiz(string(M("Ses Seviyesi: ", "Volume: ")) + Yuzde(seviye * 100.0f, 0), px + 30, py + 138, 14, RENK_METIN);
    DrawRectangleRounded(iz, 0.5f, 8, RENK_KENARLIK);
    DrawRectangleRounded({iz.x, iz.y, iz.width * seviye, iz.height}, 0.5f, 8, sesAcik ? RENK_YESIL : RENK_KENARLIK);
    DrawCircleV({iz.x + iz.width * seviye, iz.y + iz.height / 2.0f}, 11.0f, RENK_ALTIN);

    YaziCiz(M("PENCERE MODU", "WINDOW MODE"), px + 30, py + 202, 14, RENK_SOLUK_METIN);
    if (PlatformdaCikisVar()) {
        bool tamEkran = IsWindowFullscreen();
        bool kenarsiz = IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
        auto secili = [](Buton& b, bool aktif) {
            Color eski = b.renk;
            if (aktif) b.renk = RENK_ALTIN;
            b.Ciz(13);
            b.renk = eski;
        };
        secili(a.modPencereliBtn, !tamEkran && !kenarsiz);
        secili(a.modTamEkranBtn, tamEkran);
        secili(a.modKenarsizBtn, kenarsiz);
        a.kucultBtn.Ciz(14);
    } else {
        YaziCizSarili(M("Tarayıcıda oynuyorsun: tam ekran için sayfadaki tam ekran düğmesini kullan.",
                        "You are playing in a browser: use the page's fullscreen button for fullscreen."),
                      px + 30, py + 232, 14, (int)p.width - 60, RENK_METIN, 3, 20);
    }

    YaziCiz(M("DİL", "LANGUAGE"), px + 30, py + 332, 14, RENK_SOLUK_METIN);
    Color eski = a.dilTrBtn.renk;
    if (g_dil == Dil::TR) a.dilTrBtn.renk = RENK_ALTIN;
    a.dilTrBtn.Ciz(14);
    a.dilTrBtn.renk = eski;
    eski = a.dilEnBtn.renk;
    if (g_dil == Dil::EN) a.dilEnBtn.renk = RENK_ALTIN;
    a.dilEnBtn.Ciz(14);
    a.dilEnBtn.renk = eski;

    a.ayarlarKapatBtn.Ciz(15);
}

// ---------------------------------------------------------------------------
// YONETIM KARARI (devraldigin sirkette)
// ---------------------------------------------------------------------------
static void YonetimGuncelle(Uygulama& u, float dt, Vector2 fare, bool tik) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (a.yonRiskliBtn.Guncelle(dt, fare, tik)) {
        YonetimKarariUygula(o, true);
        Cal(o.sesler.tikla);
    } else if (a.yonGuvenliBtn.Guncelle(dt, fare, tik)) {
        YonetimKarariUygula(o, false);
        Cal(o.sesler.tikla);
    }
}

static void YonetimCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (o.yonetimHisseIndex < 0 || o.yonetimHisseIndex >= (int)o.hisseler.size()) return;
    if (o.yonetimSablonIndex < 0 || o.yonetimSablonIndex >= (int)YONETIM_OLAYLARI.size()) return;
    Hisse& hy = o.hisseler[o.yonetimHisseIndex];
    const YonetimOlaySablonu& sablon = YONETIM_OLAYLARI[o.yonetimSablonIndex];

    ModalKarart(0.65f);
    Rectangle p = a.yonPanel;
    PanelCiz(p, 0.05f, RENK_PANEL, RENK_ALTIN);
    const int px = (int)p.x, py = (int)p.y;
    YaziCiz(hy.kod + " - " + sablon.baslik.al(), px + 30, py + 22, 22, RENK_ALTIN);
    YaziCizSarili(sablon.aciklama.al(), px + 30, py + 60, 15, (int)p.width - 60, RENK_METIN, 2, 20);
    YaziCizSarili(M("Çoğunluk hissesine sahip olduğun şirketin patronu olarak karar sana ait.",
                    "As the boss of a company you hold a majority stake in, the call is yours."),
                  px + 30, py + 110, 13, (int)p.width - 60, RENK_SOLUK_METIN, 2, 18);

    a.yonRiskliBtn.Ciz(15);
    a.yonGuvenliBtn.Ciz(15);
    DugmeIciYazi(a.yonRiskliBtn, sablon.riskli.etiket.al(), 14, 8, 15, WHITE);
    DugmeIciYazi(a.yonRiskliBtn, M("Anında kazanç, kalıcı risk artışı", "Instant gain, lasting risk"), 14, 32, 11,
                 Fade(WHITE, 0.85f));
    DugmeIciYazi(a.yonGuvenliBtn, sablon.guvenli.etiket.al(), 14, 8, 15, WHITE);
    DugmeIciYazi(a.yonGuvenliBtn, M("Az/hiç kazanç, risk artışı yok", "Little or no gain, no added risk"), 14, 32, 11,
                 Fade(WHITE, 0.85f));
}

// ---------------------------------------------------------------------------
// CIKIS ONAYI
// ---------------------------------------------------------------------------
static void CikisOnayiGuncelle(Uygulama& u, float dt, Vector2 fare, bool tik) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (a.cikisKaydetBtn.Guncelle(dt, fare, tik)) {
        if (OyunuKaydet(o)) {
            u.cikisIstendi = true;
        } else {
            a.cikisOnayiAcik = false;
            o.toastlar.Ekle(M("Kayıt Başarısız", "Save Failed"),
                            M("Kayıt yazılamadı, çıkış iptal edildi.", "Could not write the save; quit cancelled."),
                            RENK_KIRMIZI, 4.0f);
            Cal(o.sesler.hataUyari);
        }
    }
    if (a.cikisKaydetmeBtn.Guncelle(dt, fare, tik)) u.cikisIstendi = true;
    if (a.cikisVazgecBtn.Guncelle(dt, fare, tik)) {
        a.cikisOnayiAcik = false;
        Cal(o.sesler.tikla);
    }
}

static void CikisOnayiCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    ModalKarart(0.7f);
    Rectangle p = a.coPanel;
    PanelCiz(p, 0.05f, RENK_PANEL, RENK_ALTIN);
    const int ortaX = (int)(p.x + p.width / 2.0f);
    YaziCizOrtali(M("ÇIKMAK İSTEDİĞİNE EMİN MİSİN?", "ARE YOU SURE YOU WANT TO QUIT?"), ortaX, (int)p.y + 30, 24, RENK_ALTIN);
    YaziCizOrtali(M("Son kayıttan sonraki ilerlemen kaybolacak.", "Progress since your last save will be lost."), ortaX,
                  (int)p.y + 72, 15, RENK_METIN);
    string durum = string(M("Net Değer: ", "Net Worth: ")) + Para(o.NetDeger()) + "   |   " + M("Gün ", "Day ") + to_string(o.gun);
    YaziCizOrtali(durum, ortaX, (int)p.y + 100, 13, RENK_SOLUK_METIN);
    a.cikisKaydetBtn.Ciz(14);
    a.cikisKaydetmeBtn.Ciz(14);
    a.cikisVazgecBtn.Ciz(14);
}
