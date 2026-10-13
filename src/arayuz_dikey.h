// ============================================================================
// arayuz_dikey.h - DIKEY (telefon) oyun ekrani
// ============================================================================
// Yatay yerlesim 6 karti alt alta dizer; telefonda buna yer yok ve dugmeler
// parmak icin kucuk kalir. Dikeyde bunun yerine:
//   - ust bar (ozet),  haber bandi,  grafik,
//   - VARLIK SECICI: acik her varlik icin bir "cip" (dokununca secilir),
//   - TEK BUYUK KART: secili varligin butun bilgisi ve buyuk AL / SAT dugmeleri,
//   - islem adedi paneli.
// Pencereler (duraklatma, emir, kredi...) yatayla AYNI kodu kullanir; dar
// sahnede kendileri alt alta dizilir (bkz. PencereleriKur).
//
// Ust bar / haber bandi / grafik / adet paneli cizimi ortak fonksiyonlardir;
// burada yalnizca dikeye ozgu parcalar var.

// Her karede: acik varliklar, secili varlik ve bunlara bagli dikdortgenler.
static void DikeyDuzenHesapla(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    const float W = (float)PENCERE_GENISLIK;

    // Acik varliklar: hisseler acilma sirasina gore, sonra (aciksa) guvenli limanlar.
    KartDuzeni kd = KartDuzeniHesapla(o, a);
    a.dCipVarlik = kd.acik;
    if (o.LimanlarAcik())
        for (int i = HisseSayisi(); i < (int)o.hisseler.size(); i++) a.dCipVarlik.push_back(i);

    bool seciliGecerli = false;
    for (int v : a.dCipVarlik)
        if (v == a.seciliVarlik) seciliGecerli = true;
    if (!seciliGecerli) a.seciliVarlik = a.dCipVarlik.empty() ? -1 : a.dCipVarlik[0];

    const int n = (int)a.dCipVarlik.size();
    const int sutun = 4;
    const int satir = max(1, (n + sutun - 1) / sutun);
    const float cipBosluk = 6.0f;
    const float seciciH = satir * DIKEY_CIP_YUK + (satir - 1) * cipBosluk;

    a.dKart = {DIKEY_KENAR, a.adetPanel.y - 6 - DIKEY_KART_YUK, W - 2 * DIKEY_KENAR, DIKEY_KART_YUK};
    a.dSecici = {DIKEY_KENAR, a.dKart.y - 6 - seciciH, W - 2 * DIKEY_KENAR, seciciH};
    float grafikY = a.haberBandi.y + a.haberBandi.height + 6;
    a.grafikPanel = {DIKEY_KENAR, grafikY, W - 2 * DIKEY_KENAR, fmaxf(120.0f, a.dSecici.y - 6 - grafikY)};
    a.grafikModBtn.rect = {a.grafikPanel.x + 10, a.grafikPanel.y + 8, 170, 30};

    float cipW = (a.dSecici.width - (sutun - 1) * cipBosluk) / (float)sutun;
    a.dCip.resize(n);
    for (int k = 0; k < n; k++)
        a.dCip[k] = {a.dSecici.x + (k % sutun) * (cipW + cipBosluk), a.dSecici.y + (k / sutun) * (DIKEY_CIP_YUK + cipBosluk), cipW,
                     DIKEY_CIP_YUK};

    // Dugmeler: sadece SECILI varligin dugmeleri ekranda (kartin altinda);
    // digerleri sifir boyutlu olur ki yanlislikla tiklanamasin.
    for (int i = 0; i < (int)a.alBtn.size(); i++) {
        a.alBtn[i].rect = {0, 0, 0, 0};
        a.satBtn[i].rect = {0, 0, 0, 0};
        a.emirBtn[i].rect = {0, 0, 0, 0};
    }
    if (a.seciliVarlik >= 0) {
        int i = a.seciliVarlik;
        const Hisse& h = o.hisseler[i];
        float ic = a.dKart.width - 24;
        float by = a.dKart.y + a.dKart.height - 70;
        float bx = a.dKart.x + 12;
        if (!h.liman && o.EmirlerAcik()) {
            float emirW = 104.0f;
            float w1 = (ic - emirW - 16) / 2.0f;
            a.alBtn[i].rect = {bx, by, w1, 58};
            a.satBtn[i].rect = {bx + w1 + 8, by, w1, 58};
            a.emirBtn[i].rect = {bx + 2 * (w1 + 8), by, emirW, 58};
        } else {
            float w1 = (ic - 8) / 2.0f;
            a.alBtn[i].rect = {bx, by, w1, 58};
            a.satBtn[i].rect = {bx + w1 + 8, by, w1, 58};
        }
    }
}

// ---------------------------------------------------------------------------
// UST BAR (dikey)
// ---------------------------------------------------------------------------
static void DikeyUstBarCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    const int w = PENCERE_GENISLIK;
    const int orta = w / 2;

    DrawRectangleGradientV(0, 0, w, (int)DIKEY_UST_BAR_YUK, RENK_PANEL, RENK_ARKAPLAN_UST);
    DrawRectangle(0, (int)DIKEY_UST_BAR_YUK - 2, w, 2, RENK_KENARLIK);

    // 1. satir: rutbe (sol), zarf + Kredi + Duraklat (sag)
    string rutbeMetni = RUTBELER[o.rutbeIndex].ad.al();
    int solSinir = (int)(o.KrediAcik() ? a.krediBtn.rect.x : a.duraklatBtn.rect.x) - 10;
    if (o.insiderMektupVar) solSinir = (int)a.insiderRect.x - 12;
    rutbeMetni = Kisalt(rutbeMetni, 14, solSinir - (int)DIKEY_KENAR - 24);
    DrawRectangleRounded({DIKEY_KENAR, 12, (float)YaziOlc(rutbeMetni, 14) + 24.0f, 26}, 0.5f, 6, Fade(RENK_MAVI_VURGU, 0.9f));
    YaziCiz(rutbeMetni, (int)DIKEY_KENAR + 12, 18, 14, WHITE);
    a.duraklatBtn.Ciz(14);
    if (o.KrediAcik()) a.krediBtn.Ciz(14);
    ZarfCiz(u);

    // 2-4. satirlar: net deger, nakit / gun, kar-zarar
    float karZarar = o.NetDeger() - o.BaslangicSermayesi();
    YaziCizOrtali(string(M("Net Değer: ", "Net Worth: ")) + Para(o.gosterilenNetDeger.deger), orta, 54, 20, RENK_METIN);
    string nakit = string(M("Nakit: ", "Cash: ")) + Para(o.gosterilenBakiye.deger);
    string gun = string(M("Gün ", "Day ")) + to_string(o.gun);
    int sinir = o.Sen().gunSiniri;
    if (sinir > 0 && !o.sureDoldu) gun += " / " + to_string(sinir);
    YaziCiz(nakit, (int)DIKEY_KENAR, 80, 13, RENK_SOLUK_METIN);
    YaziCizSaga(gun, w - (int)DIKEY_KENAR, 80, 13, RENK_SOLUK_METIN);
    string kz = ParaIsaretli(karZarar) + "  (" + Yuzde(karZarar / o.BaslangicSermayesi() * 100.0f, 1, true) + ")";
    YaziCizOrtali(kz, orta, 99, 14, karZarar >= 0 ? RENK_YESIL : RENK_KIRMIZI);

    // 5. satir: rutbe cubugu; 6. satir: (varsa) borc / teminat ozeti
    RutbeCubuguCiz(u);
    string s1, s2;
    Color oranRenk;
    if (KaldiracOzeti(o, s1, s2, oranRenk)) {
        int toplamW = YaziOlc(s1, 12) + 14 + YaziOlc(s2, 12);
        int x = w - (int)DIKEY_KENAR - toplamW;
        // Rutbe cubugunun yanindaki hedef yazisiyla ayni satira sigmazsa bir alt satira iner.
        YaziCiz(s1, x, 135, 12, RENK_METIN);
        YaziCiz(s2, x + YaziOlc(s1, 12) + 14, 135, 12, oranRenk);
    }
}

// ---------------------------------------------------------------------------
// VARLIK SECICI ve SECILI VARLIK KARTI
// ---------------------------------------------------------------------------
static void DikeySeciciCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    for (int k = 0; k < (int)a.dCip.size(); k++) {
        int i = a.dCipVarlik[k];
        const Hisse& h = o.hisseler[i];
        Rectangle r = a.dCip[k];
        bool secili = (i == a.seciliVarlik);
        DrawRectangleRounded({r.x - 2, r.y - 2, r.width + 4, r.height + 4}, 0.3f, 6, secili ? RENK_ALTIN : RENK_KENARLIK);
        DrawRectangleRounded(r, 0.3f, 6, secili ? RENK_PANEL_ACIK : KartIcRengi(h));
        DrawRectangleRounded({r.x, r.y, 6, r.height}, 0.0f, 4, h.renk);
        bool yukseldi = h.fiyat >= h.oncekiFiyat;
        YaziCiz(h.kod, (int)r.x + 13, (int)r.y + 6, 13, WHITE);
        // Alt satir: pozisyon varsa kar/zarar yuzdesi, yoksa guncel fiyat + yon oku
        float kzT = 0, kzO = 0;
        string alt = SayiFormatla(h.gosterilenFiyat.deger, h.fiyat >= 1000.0f ? 0 : 2) + (yukseldi ? " ^" : " v");
        Color altRenk = yukseldi ? RENK_YESIL : RENK_KIRMIZI;
        if (PozisyonKarZarari(h, kzT, kzO)) {
            alt = Yuzde(kzO, 0, true);
            altRenk = kzT >= 0 ? RENK_YESIL : RENK_KIRMIZI;
        }
        if (YaziOlc(alt, 12) <= (int)r.width - 20) YaziCiz(alt, (int)r.x + 13, (int)r.y + 24, 12, altRenk);
        if (h.adet != 0) DrawCircleV({r.x + r.width - 10, r.y + 10}, 4.0f, h.adet > 0 ? RENK_ALTIN : RENK_UYARI);
    }
}

static void DikeyKartCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (a.seciliVarlik < 0) return;
    const int i = a.seciliVarlik;
    Hisse& h = o.hisseler[i];
    Rectangle kart = a.dKart;
    const int kx = (int)kart.x + 18, ky = (int)kart.y;
    const int sagX = (int)(kart.x + kart.width) - 14;

    PanelCiz(kart, 0.08f, KartIcRengi(h), RENK_KENARLIK);
    DrawRectangleRounded({kart.x, kart.y, 8, kart.height}, 0.0f, 4, h.renk);

    // 1. satir: kod + degerleme etiketi (sol), PATRON rozeti (sag)
    YaziCiz(h.kod, kx, ky + 12, 22, WHITE);
    int etiketX = kx + YaziOlc(h.kod, 22) + 10;
    int solBitis = etiketX + DegerlemeEtiketiCiz(h, etiketX, ky + 14, 11) + 10;
    if (h.sahibiyim) {
        const char* rozet = M("PATRONUSUN", "THE BOSS");
        int rw = YaziOlc(rozet, 11);
        if (sagX - rw - 16 >= solBitis) {
            Rectangle rr = {(float)(sagX - rw - 16), kart.y + 13, (float)rw + 16, 20};
            DrawRectangleRounded(rr, 0.5f, 6, RENK_ALTIN);
            YaziCiz(rozet, (int)rr.x + 8, (int)rr.y + 4, 11, BLACK);
        }
    }

    // 2. satir: tam ad (sol), bilanco (sag)
    string tamAd = h.tamAd.al();
    YaziCiz(tamAd, kx, ky + 42, 12, RENK_SOLUK_METIN);
    if (!h.liman) {
        int adBitis = kx + YaziOlc(tamAd, 12) + 12;
        string bil = BilancoMetni(o, h, false);
        if (sagX - YaziOlc(bil, 12) < adBitis) bil = BilancoMetni(o, h, true);
        if (sagX - YaziOlc(bil, 12) >= adBitis) YaziCizSaga(bil, sagX, ky + 42, 12, RENK_SOLUK_METIN);
    }

    // 3. satir: fiyat + emir baskisi
    bool yukseldi = h.fiyat >= h.oncekiFiyat;
    string fiyat = Para(h.gosterilenFiyat.deger) + (yukseldi ? " ^" : " v");
    YaziCiz(fiyat, kx, ky + 62, 26, yukseldi ? RENK_YESIL : RENK_KIRMIZI);
    string baski = BaskiMetni(h);
    if (!baski.empty()) {
        int bx = kx + YaziOlc(fiyat, 26) + 12;
        if (bx + YaziOlc(baski, 12) <= sagX) YaziCiz(baski, bx, ky + 72, 12, RENK_UYARI);
    }

    // 4. satir: sahiplik (sol), kar/zarar (sag)
    string sahiplik = SahiplikMetni(h, false);
    YaziCiz(sahiplik, kx, ky + 96, 13, h.adet < 0 ? RENK_UYARI : RENK_SOLUK_METIN);
    float kzTutar = 0.0f, kzOran = 0.0f;
    if (PozisyonKarZarari(h, kzTutar, kzOran)) {
        string uzun = Yuzde(kzOran, 1, true) + " (" + ParaIsaretli(kzTutar, 0) + ")";
        string kisa = Yuzde(kzOran, 1, true);
        int sol = kx + YaziOlc(sahiplik, 13) + 12;
        string kzM = (sagX - YaziOlc(uzun, 13) >= sol) ? uzun : kisa;
        if (sagX - YaziOlc(kzM, 13) >= sol) YaziCizSaga(kzM, sagX, ky + 96, 13, kzTutar >= 0 ? RENK_YESIL : RENK_KIRMIZI);
    }

    // 5. satir: bekleyen emirler (hisse) ya da kisa aciklama (guvenli liman)
    if (h.liman) {
        YaziCiz(Kisalt(M("Güvenli liman: piyasa düşerken yükselir.", "Safe haven: rises when the market falls."), 12,
                       (int)kart.width - 36),
                kx, ky + 120, 12, RENK_SOLUK_METIN);
    } else if (o.EmirlerAcik()) {
        EmirEtiketleriCiz(h, sagX, ky + 119);
    }

    a.alBtn[i].Ciz(18);
    a.satBtn[i].Ciz(18);
    if (!h.liman && o.EmirlerAcik()) a.emirBtn[i].Ciz(14);
}

// ---------------------------------------------------------------------------
// DIKEY OYUN ALANININ GIRDISI
// ---------------------------------------------------------------------------
static void DikeyOyunAlaniGuncelle(Uygulama& u, float dt, const Girdi& g, bool etkin) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    Vector2 fare = etkin ? g.fare : FARE_UZAK;
    bool tik = etkin && g.tiklandi;

    if (RehberGirdisi(u, dt, fare, tik)) tik = false;

    // Varlik secici
    if (tik) {
        for (int k = 0; k < (int)a.dCip.size(); k++) {
            if (CheckCollisionPointRec(fare, a.dCip[k])) {
                if (a.seciliVarlik != a.dCipVarlik[k]) Cal(o.sesler.tikla);
                a.seciliVarlik = a.dCipVarlik[k];
                DikeyDuzenHesapla(u); // dugmeler yeni secime gore yerlessin
                tik = false;
                break;
            }
        }
    }

    if (a.seciliVarlik >= 0) {
        const Hisse& h = o.hisseler[a.seciliVarlik];
        VarlikDugmeleriniIsle(u, a.seciliVarlik, dt, fare, tik, !h.liman && o.EmirlerAcik());
    }
    OrtakOyunGirdisi(u, dt, g, fare, tik, etkin);
}

// Dikey oyun ekraninin pencere ALTINDA kalan kismi (pencereleri uygulama.h cizer).
static void DikeyOyunCiz(Uygulama& u, const KartDuzeni& kd) {
    DikeyUstBarCiz(u);
    HaberBandiCiz(u);
    GrafikCiz(u, kd);
    DikeySeciciCiz(u);
    DikeyKartCiz(u);
    AdetPaneliCiz(u);
}
