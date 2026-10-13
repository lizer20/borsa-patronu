// ============================================================================
// arayuz_oyun.h - oyun ekraninin SOL yarisi ve ust bar: etiketler, ust bar,
//                 haber bandi, hisse kartlari, portfoy ozeti, adet paneli
// ============================================================================

static void EtiketleriYenile(Uygulama& u) {
    Arayuz& a = u.ui;
    Oyun& o = u.oyun;

    for (int i = 0; i < (int)o.hisseler.size() && i < (int)a.alBtn.size(); i++) {
        const Hisse& h = o.hisseler[i];
        a.alBtn[i].etiket = (h.adet < 0) ? M("AÇIĞI KAPAT", "COVER") : M("AL", "BUY");
        a.satBtn[i].etiket = (h.adet <= 0 && o.AcigaSatisAcik()) ? M("AÇIĞA SAT", "SHORT") : M("SAT", "SELL");
        int n = AktifEmirSayisi(h);
        a.emirBtn[i].etiket = M("EMİR", "ORDER");
        if (n > 0) a.emirBtn[i].etiket += " (" + to_string(n) + ")";
    }
    for (int i = 0; i < 8; i++)
        a.adetBtn[i].etiket = (a.adetDegerleri[i] > 0) ? to_string(a.adetDegerleri[i]) : string(M("MAKS", "MAX"));

    if (a.sifirlaOnaySuresi > 0.0f) {
        a.sifirlaBtn.etiket = M("EMİN MİSİN? Tekrar tıkla", "ARE YOU SURE? Click again");
        a.sifirlaBtn.renk = RENK_KIRMIZI;
    } else {
        a.sifirlaBtn.etiket = M("Yeni Oyun", "New Game");
        a.sifirlaBtn.renk = RENK_KENARLIK;
    }
    a.duraklatBtn.etiket = o.duraklatildi ? M("Devam Et", "Resume") : M("Duraklat", "Pause");
    a.krediBtn.etiket = M("Kredi", "Credit");
    a.grafikModBtn.etiket = o.normalizeGrafik ? M("Eksen: %", "Axis: %") : M("Eksen: TL", "Axis: TL");

    a.devamBtn.etiket = M("Devam Et", "Resume");
    a.kaydetBtn.etiket = M("Kaydet", "Save");
    a.ayarlarBtn.etiket = M("Ayarlar", "Settings");
    a.anaMenuBtn.etiket = M("Ana Menü", "Main Menu");
    a.cikisBtn.etiket = M("Çıkış", "Quit");

    a.sesBtn.etiket = sesAcik ? M("Ses: Açık", "Sound: On") : M("Ses: Kapalı", "Sound: Off");
    a.rehberBtn.etiket = (o.rehberAdim >= 0) ? M("Rehberi Kapat", "Turn Guide Off") : M("Rehberi Başlat", "Start Guide");
    a.modPencereliBtn.etiket = M("Pencereli", "Windowed");
    a.modTamEkranBtn.etiket = M("Tam Ekran", "Fullscreen");
    a.modKenarsizBtn.etiket = M("Kenarsız", "Borderless");
    a.kucultBtn.etiket = M("Simge Durumuna Küçült", "Minimize");
    a.dilTrBtn.etiket = "Türkçe";
    a.dilEnBtn.etiket = "English";
    a.ayarlarKapatBtn.etiket = M("Kapat", "Close");

    a.cikisKaydetBtn.etiket = M("Kaydet ve Çık", "Save and Quit");
    a.cikisKaydetmeBtn.etiket = M("Kaydetmeden Çık", "Quit Anyway");
    a.cikisVazgecBtn.etiket = M("Vazgeç", "Cancel");

    if (a.emirHisse >= 0 && a.emirHisse < (int)o.hisseler.size()) {
        const bool acik = o.hisseler[a.emirHisse].adet < 0;
        for (int t = 0; t < EMIR_TUR_SAYISI; t++) {
            for (int k = 0; k < 4; k++)
                a.emirSeviyeBtn[t][k].etiket = Yuzde(EmirSeviyeOrani(t, k, acik) * 100.0f, 0, true);
            a.emirKaldirBtn[t].etiket = M("Kaldır", "Remove");
        }
    }
    a.emirKapatBtn.etiket = M("Kapat", "Close");

    for (int k = 0; k < 4; k++) {
        if (KREDI_TUTARLARI[k] > 0.0f) {
            a.krediAlBtn[k].etiket = "+" + SayiFormatla(KREDI_TUTARLARI[k], 0);
            a.krediOdeBtn[k].etiket = "-" + SayiFormatla(KREDI_TUTARLARI[k], 0);
        } else {
            a.krediAlBtn[k].etiket = M("MAKS", "MAX");
            a.krediOdeBtn[k].etiket = M("TÜMÜ", "ALL");
        }
    }
    a.krediKapatBtn.etiket = M("Kapat", "Close");

    if (o.sonTuru == SON_IFLAS) a.sonBirBtn.etiket = M("Yeniden Dene", "Try Again");
    else a.sonBirBtn.etiket = M("Devam Et", "Keep Playing");
    a.sonIkiBtn.etiket = M("Ana Menü", "Main Menu");

    a.rehberTamamBtn.etiket = M("Anladım", "Got it");
    a.rehberKapatBtn.etiket = M("Rehberi Kapat", "Turn Guide Off");

    a.basBtn.etiket = u.oyunVar ? M("DEVAM ET", "CONTINUE") : M("BAŞLA", "START");
    a.senaryoBtn.etiket = M("Yeni Oyun / Senaryolar", "New Game / Scenarios");
    a.yukleBtn.etiket = M("Kayıtlı Oyunu Yükle", "Load Saved Game");
    a.basarimBtn.etiket = string(M("Başarımlar", "Achievements")) + "  " + to_string(AcilanBasarimSayisi()) + "/" +
                          to_string((int)BASARIM_SAYISI);
    a.cikisMenuBtn.etiket = M("ÇIKIŞ", "QUIT");
    a.dilBtn.etiket = (g_dil == Dil::TR) ? "Dil: Türkçe" : "Language: English";

    for (int i = 0; i < SENARYO_SAYISI; i++) {
        bool onay = (a.senaryoOnay == i && a.senaryoOnaySuresi > 0.0f);
        a.senaryoBaslaBtn[i].etiket = onay ? M("EMİN MİSİN?", "ARE YOU SURE?") : M("Başla", "Start");
        a.senaryoBaslaBtn[i].renk = onay ? RENK_KIRMIZI : RENK_YESIL;
    }
    a.geriBtn.etiket = M("Geri", "Back");
}

// ---------------------------------------------------------------------------
// UST BAR
// ---------------------------------------------------------------------------
static void UstBarCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    const int w = PENCERE_GENISLIK;

    DrawRectangleGradientV(0, 0, w, 100, RENK_PANEL, RENK_ARKAPLAN_UST);
    DrawRectangle(0, 98, w, 2, RENK_KENARLIK);

    YaziCiz("BORSA PATRONU", 20, 12, 24, RENK_ALTIN);
    if (o.senaryo != S_KLASIK)
        YaziCiz(o.Sen().ad.al(), 20 + YaziOlc("BORSA PATRONU", 24) + 14, 21, 13, RENK_SOLUK_METIN);

    string rutbeMetni = RUTBELER[o.rutbeIndex].ad.al();
    DrawRectangleRounded({20, 42, (float)YaziOlc(rutbeMetni, 14) + 24.0f, 22}, 0.5f, 6, Fade(RENK_MAVI_VURGU, 0.9f));
    YaziCiz(rutbeMetni, 32, 47, 14, WHITE);

    // Rutbe ilerleme cubugu + hedef rakami
    {
        float esikMevcut = RUTBELER[o.rutbeIndex].esikNetDeger;
        bool sonRutbe = (o.rutbeIndex + 1 >= (int)RUTBELER.size());
        float esikSonraki = sonRutbe ? esikMevcut + 1.0f : RUTBELER[o.rutbeIndex + 1].esikNetDeger;
        float oran = Clamp((o.NetDeger() - esikMevcut) / (esikSonraki - esikMevcut), 0.0f, 1.0f);
        Rectangle bar = a.rutbeCubugu;
        DrawRectangleRounded(bar, 0.5f, 6, RENK_KENARLIK);
        DrawRectangleRounded({bar.x, bar.y, bar.width * oran, bar.height}, 0.5f, 6, RENK_ALTIN);
        if (sonRutbe) {
            YaziCiz(M("ZİRVE", "TOP"), (int)(bar.x + bar.width + 10), (int)bar.y - 3, 12, RENK_ALTIN);
        } else {
            string hedef = SayiFormatla(o.NetDeger(), 0) + " / " + Para(esikSonraki, 0);
            YaziCiz(hedef, (int)(bar.x + bar.width + 10), (int)bar.y - 3, 12, RENK_SOLUK_METIN);
        }
    }

    // Net deger / nakit / kar-zarar (ortada)
    {
        float karZarar = o.NetDeger() - o.BaslangicSermayesi();
        Color kzRenk = karZarar >= 0 ? RENK_YESIL : RENK_KIRMIZI;
        string netMetin = string(M("Net Değer: ", "Net Worth: ")) + Para(o.gosterilenNetDeger.deger);
        string bakiyeMetni = string(M("Nakit: ", "Cash: ")) + Para(o.gosterilenBakiye.deger);
        string kzMetni = ParaIsaretli(karZarar) + "   (" + Yuzde(karZarar / o.BaslangicSermayesi() * 100.0f, 1, true) + ")";
        string gunMetni = string(M("Gün ", "Day ")) + to_string(o.gun);
        int sinir = o.Sen().gunSiniri;
        if (sinir > 0 && !o.sureDoldu) gunMetni += " / " + to_string(sinir);
        YaziCizOrtali(netMetin, w / 2, 14, 20, RENK_METIN);
        YaziCizOrtali(bakiyeMetni, w / 2, 40, 13, RENK_SOLUK_METIN);
        YaziCizOrtali(kzMetni, w / 2, 60, 15, kzRenk);
        YaziCizOrtali(gunMetni, w / 2, 80, 14, RENK_SOLUK_METIN);
    }

    // Kaldirac ozeti (sag): borc / aciga satis varsa teminat orani surekli gorunur.
    {
        float borc = o.borc, aciga = o.AcigaDegeri();
        if (borc > 0.5f || aciga > 0.5f) {
            string satir1;
            if (borc > 0.5f && aciga > 0.5f) satir1 = string(M("Borç+Açık: ", "Debt+Short: ")) + Para(borc + aciga, 0);
            else if (borc > 0.5f) satir1 = string(M("Borç: ", "Debt: ")) + Para(borc, 0);
            else satir1 = string(M("Açığa: ", "Short: ")) + Para(aciga, 0);
            float oran = o.TeminatOrani();
            Color oranRenk = RENK_YESIL;
            if (oran < TEMINAT_UYARI_ORANI) oranRenk = RENK_KIRMIZI;
            else if (oran < 1.0f) oranRenk = RENK_UYARI;
            string satir2 = string(M("Teminat: ", "Margin: ")) + Yuzde(fminf(oran, 9.99f) * 100.0f, 0);
            int sagX = (int)a.krediBtn.rect.x - 12;
            YaziCizSaga(satir1, sagX, 58, 13, RENK_METIN);
            YaziCizSaga(satir2, sagX, 76, 13, oranRenk);
        }
    }

    a.duraklatBtn.Ciz(13);
    if (o.KrediAcik()) a.krediBtn.Ciz(13);

    // Iceriden bilgi zarfi (fontta zarf glifi yok; ilkel sekillerle ciziliyor)
    if (o.insiderMektupVar) {
        Rectangle ir = a.insiderRect;
        float nabiz = 1.0f + sinf(u.zaman * 5.0f) * 0.09f;
        float zw = ir.width * nabiz, zh = ir.height * nabiz;
        Rectangle zr = {ir.x + ir.width / 2.0f - zw / 2.0f, ir.y + ir.height / 2.0f - zh / 2.0f, zw, zh};
        DrawRectangleRounded({zr.x - 5, zr.y - 5, zr.width + 10, zr.height + 10}, 0.3f, 6, Fade(RENK_ALTIN, 0.22f));
        DrawRectangleRounded(zr, 0.18f, 6, RENK_ALTIN);
        Vector2 uc = {zr.x + zr.width / 2.0f, zr.y + zr.height * 0.60f};
        DrawLineEx({zr.x + 3, zr.y + 4}, uc, 2.5f, RENK_ARKAPLAN_UST);
        DrawLineEx({zr.x + zr.width - 3, zr.y + 4}, uc, 2.5f, RENK_ARKAPLAN_UST);
        float oran = Clamp(o.insiderMektupOmur / INSIDER_MEKTUP_OMUR_SN, 0.0f, 1.0f);
        DrawRectangle((int)zr.x, (int)(zr.y + zr.height + 3), (int)(zr.width * oran), 3, RENK_ALTIN);
    }
}

// ---------------------------------------------------------------------------
// KAYAN HABER BANDI
// ---------------------------------------------------------------------------
static void HaberBandiCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    const int w = PENCERE_GENISLIK;
    Rectangle bant = u.ui.haberBandi;
    DrawRectangle((int)bant.x, (int)bant.y, (int)bant.width, (int)bant.height, {6, 8, 10, 255});
    DrawRectangle(0, (int)(bant.y + bant.height), w, 1, RENK_KENARLIK);

    // En yeniden en eskiye yazilir; kaydirma sifirlandiginda ekrana ilk giren
    // sey en taze haber olur.
    string tum = M("EN YENİ ->   ", "LATEST ->   ");
    for (auto it = o.haberBandi.rbegin(); it != o.haberBandi.rend(); ++it) tum += *it + "   |   ";
    int metinW = max(1, YaziOlc(tum, 15));
    float x = (float)w - fmodf(o.bantKaydirma, (float)(metinW + w));
    TestKaydiDurdur(); // bant bilerek ekrandan tasar; yerlesim denetcisi saymasin
    YaziCiz(tum, (int)x, (int)bant.y + 11, 15, RENK_YESIL);
    YaziCiz(tum, (int)(x + metinW + 40), (int)bant.y + 11, 15, RENK_YESIL);
    TestKaydiSurdur();
}

// ---------------------------------------------------------------------------
// HISSE KARTLARI
// ---------------------------------------------------------------------------
static string BilancoMetni(const Oyun& o, const Hisse& h, bool kisa) {
    int kalan = max(0, h.bilancoGunu - o.gun);
    if (kisa) {
        const char* isaret = h.bilancoBeklenti > 0 ? "+" : (h.bilancoBeklenti < 0 ? "-" : "?");
        return string(M("bilanço ", "earnings ")) + to_string(kalan) + M("g", "d") + " (" + isaret + ")";
    }
    const char* beklenti = h.bilancoBeklenti > 0 ? M("olumlu", "upbeat")
                                                 : (h.bilancoBeklenti < 0 ? M("olumsuz", "downbeat") : M("belirsiz", "mixed"));
    return string(M("Bilanço: ", "Earnings: ")) + to_string(kalan) + M(" gün", "d") + " - " + beklenti;
}

// Pozisyonun gerceklesmemis kar/zarari. Uzun: (fiyat - ort) * adet. Aciga: tersi.
static bool PozisyonKarZarari(const Hisse& h, float& tutar, float& oran) {
    if (h.adet == 0) return false;
    float adet = (float)abs(h.adet);
    float ort = h.maliyet / adet;
    if (ort <= 0.01f) return false;
    float fark = (h.adet > 0) ? (h.fiyat - ort) : (ort - h.fiyat);
    tutar = fark * adet;
    oran = fark / ort * 100.0f;
    return true;
}

// Kartin sag ustune, saga yaslanmis kucuk emir etiketleri cizer.
static void EmirEtiketleriCiz(const Hisse& h, int sagX, int y) {
    static const Color renkler[EMIR_TUR_SAYISI] = {RENK_KIRMIZI, RENK_YESIL, RENK_MAVI_VURGU};
    int x = sagX;
    for (int t = EMIR_TUR_SAYISI - 1; t >= 0; t--) {
        if (!h.emirler[t].aktif) continue;
        const char* ad = (t == EMIR_STOP) ? "STOP" : (t == EMIR_KAR_AL ? M("KÂR", "TGT") : M("LİMİT", "LIMIT"));
        string metin = string(ad) + " " + SayiFormatla(h.emirler[t].fiyat, h.emirler[t].fiyat < 100.0f ? 2 : 1);
        int tw = YaziOlc(metin, 10);
        x -= tw + 10;
        DrawRectangleRounded({(float)x, (float)y, (float)tw + 10, 14}, 0.5f, 4, Fade(renkler[t], 0.28f));
        YaziCiz(metin, x + 5, y + 2, 10, RENK_METIN);
        x -= 4;
    }
}

static void KartlarCiz(Uygulama& u, const KartDuzeni& kd) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;

    for (int slot = 0; slot < kd.sayi; slot++) {
        int i = kd.acik[slot];
        Hisse& h = o.hisseler[i];
        Rectangle kart = KartRect(a, kd, slot);
        const int kx = (int)kart.x, ky = (int)kart.y;
        const int sagX = (int)(kart.x + kart.width) - 14;

        Color icRenk = RENK_PANEL;
        if (h.flashSuresi > 0.0f) {
            float oran = Clamp(h.flashSuresi / 0.6f, 0.0f, 1.0f);
            Color hedef = h.flashIyi ? RENK_YESIL : RENK_KIRMIZI;
            icRenk.r = (unsigned char)Lerp((float)RENK_PANEL.r, (float)hedef.r, oran * 0.5f);
            icRenk.g = (unsigned char)Lerp((float)RENK_PANEL.g, (float)hedef.g, oran * 0.5f);
            icRenk.b = (unsigned char)Lerp((float)RENK_PANEL.b, (float)hedef.b, oran * 0.5f);
        }
        PanelCiz(kart, 0.12f, icRenk, RENK_KENARLIK);
        DrawRectangleRounded({kart.x, kart.y, 8, kart.height}, 0.0f, 4, h.renk);

        bool yukseldi = h.fiyat >= h.oncekiFiyat;
        Color okRenk = yukseldi ? RENK_YESIL : RENK_KIRMIZI;
        string fiyatMetni = Para(h.gosterilenFiyat.deger) + (yukseldi ? " ^" : " v");

        // Oyuncunun kendi emir baskisi (bir sonraki emrin ne kadar pahali oldugu)
        float baski = EtkiEgrisi(h.akis) * 100.0f;
        string baskiMetni = (fabsf(baski) >= 0.25f) ? string(M("baskı ", "pressure ")) + Yuzde(baski, 1, true) : string();

        float kzTutar = 0.0f, kzOran = 0.0f;
        bool kzVar = PozisyonKarZarari(h, kzTutar, kzOran);
        Color kzRenk = (kzTutar >= 0.0f) ? RENK_YESIL : RENK_KIRMIZI;

        if (!kd.kompakt) {
            YaziCiz(h.kod, kx + 18, ky + 10, 20, WHITE);
            YaziCiz(h.tamAd.al(), kx + 18, ky + 32, 12, RENK_SOLUK_METIN);

            // Sag ust: PATRON rozeti ve bilanco geri sayimi
            int ustSag = sagX;
            if (h.sahibiyim) {
                const char* rozet = M("PATRONUSUN", "YOU'RE THE BOSS");
                int rw = YaziOlc(rozet, 11);
                Rectangle rr = {(float)(sagX - rw - 16), kart.y + 10, (float)rw + 16, 18};
                DrawRectangleRounded(rr, 0.5f, 6, RENK_ALTIN);
                YaziCiz(rozet, (int)rr.x + 8, (int)rr.y + 3, 11, BLACK);
                ustSag = (int)rr.x - 10;
            }
            YaziCizSaga(BilancoMetni(o, h, false), ustSag, ky + 13, 11, RENK_SOLUK_METIN);
            if (o.EmirlerAcik()) EmirEtiketleriCiz(h, sagX, ky + 30);

            YaziCiz(fiyatMetni, kx + 18, ky + 50, 22, okRenk);
            if (!baskiMetni.empty()) {
                int bx = kx + 18 + YaziOlc(fiyatMetni, 22) + 10;
                if (bx + YaziOlc(baskiMetni, 11) < kx + (int)kart.width - 136) // mini grafige girmesin
                    YaziCiz(baskiMetni, bx, ky + 58, 11, RENK_UYARI);
            }

            string sahiplik;
            if (h.adet >= 0)
                sahiplik = string(M("Elde: ", "Held: ")) + to_string(h.adet) + M(" adet   Değer: ", "   Value: ") +
                           Para(h.fiyat * (float)h.adet);
            else
                sahiplik = string(M("Açığa: ", "Short: ")) + to_string(-h.adet) + M(" adet   Borç: ", "   Owed: ") +
                           Para(h.fiyat * (float)(-h.adet));
            YaziCiz(sahiplik, kx + 18, ky + 74, 12, h.adet < 0 ? RENK_UYARI : RENK_SOLUK_METIN);

            // Pozisyon K/Z: uzun hali sigmiyorsa (buyuk rakamlar) sadece yuzde.
            if (kzVar) {
                string uzun = Yuzde(kzOran, 1, true) + "  (" + ParaIsaretli(kzTutar) + ")";
                string kisa = Yuzde(kzOran, 1, true);
                int solBitis = kx + 18 + YaziOlc(sahiplik, 12) + 12;
                string kzM = (sagX - YaziOlc(uzun, 12) >= solBitis) ? uzun : kisa;
                if (sagX - YaziOlc(kzM, 12) >= solBitis) YaziCizSaga(kzM, sagX, ky + 74, 12, kzRenk);
            }

            MiniSparklineCiz({kart.x + kart.width - 130, kart.y + 46, 116, 24}, h.gecmis, h.renk);
        } else {
            // KOMPAKT KART (4-6 hisse acikken): tam ad ve mini grafik gizlenir.
            YaziCiz(h.kod, kx + 18, ky + 3, 12, WHITE);
            int kodBitis = kx + 18 + YaziOlc(h.kod, 12) + 10;
            int ustSag = sagX;
            if (h.sahibiyim) {
                const char* rozet = M("PATRON", "BOSS");
                int rw = YaziOlc(rozet, 10);
                Rectangle rr = {(float)(sagX - rw - 10), kart.y + 3, (float)rw + 10, 13};
                DrawRectangleRounded(rr, 0.5f, 6, RENK_ALTIN);
                YaziCiz(rozet, (int)rr.x + 5, (int)rr.y + 2, 10, BLACK);
                ustSag = (int)rr.x - 8;
            }
            string bil = BilancoMetni(o, h, true);
            if (ustSag - YaziOlc(bil, 10) >= kodBitis) YaziCizSaga(bil, ustSag, ky + 4, 10, RENK_SOLUK_METIN);

            YaziCiz(fiyatMetni, kx + 18, ky + 16, 13, okRenk);
            int satirX = kx + 18 + YaziOlc(fiyatMetni, 13) + 10;
            if (!baskiMetni.empty()) {
                YaziCiz(baskiMetni, satirX, ky + 18, 10, RENK_UYARI);
                satirX += YaziOlc(baskiMetni, 10) + 10;
            }
            int emirSayisi = AktifEmirSayisi(h);
            if (emirSayisi > 0) {
                string em = to_string(emirSayisi) + M(" emir", emirSayisi > 1 ? " orders" : " order");
                if (sagX - YaziOlc(em, 10) >= satirX) YaziCizSaga(em, sagX, ky + 18, 10, RENK_MAVI_VURGU);
            }

            string sahiplik;
            if (h.adet >= 0)
                sahiplik = string(M("Elde: ", "Held: ")) + to_string(h.adet) + M("  Değer: ", "  Value: ") +
                           Para(h.fiyat * (float)h.adet);
            else
                sahiplik = string(M("Açığa: ", "Short: ")) + to_string(-h.adet) + M("  Borç: ", "  Owed: ") +
                           Para(h.fiyat * (float)(-h.adet));
            YaziCiz(sahiplik, kx + 18, ky + 30, 10, h.adet < 0 ? RENK_UYARI : RENK_SOLUK_METIN);
            if (kzVar) {
                string kzM = Yuzde(kzOran, 1, true);
                int solBitis = kx + 18 + YaziOlc(sahiplik, 10) + 8;
                if (sagX - YaziOlc(kzM, 10) >= solBitis) YaziCizSaga(kzM, sagX, ky + 30, 10, kzRenk);
            }
        }

        a.alBtn[i].Ciz(kd.kompakt ? 11 : 14);
        a.satBtn[i].Ciz(kd.kompakt ? 11 : 14);
        if (o.EmirlerAcik()) a.emirBtn[i].Ciz(kd.kompakt ? 11 : 13);
    }
}

// Kart alaninin altinda kalan bos yere: kilitli hisse ipucu + portfoy ozeti.
static void KartAltiCiz(Uygulama& u, const KartDuzeni& kd) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    float y = a.kartY0 + kd.sayi * (kd.kartYuk + a.kartArasi) + 4;

    if (kd.sayi < (int)o.hisseler.size()) {
        int kilitli = (int)o.hisseler.size() - kd.sayi;
        string ipucu = "+" + to_string(kilitli) + M(" hisse daha kilitli", kilitli > 1 ? " more stocks locked" : " more stock locked");
        if (o.rutbeIndex + 1 < (int)RUTBELER.size())
            ipucu += string(M(" - sıradaki '", " - the next one unlocks at '")) + RUTBELER[o.rutbeIndex + 1].ad.al() +
                     M("' rütbesinde açılır", "'");
        // Tek satira sigmayabiliyor (ozellikle uzun rutbe adlarinda); sarilir.
        if (y + 16 < a.adetPanelY) y += YaziCizSarili(ipucu, (int)a.solX + 4, (int)y, 12, (int)a.solW - 8, RENK_SOLUK_METIN, 2, 15) + 4;
    }

    float bosYuk = a.adetPanelY - y - 8.0f;
    if (bosYuk < 96.0f) return; // 3+ hisse acikken yer kalmaz, panel kendiliginden gizlenir
    Rectangle oz = {a.solX, y, a.solW, fminf(bosYuk, 116.0f)};
    PanelCiz(oz, 0.08f, RENK_PANEL, RENK_KENARLIK);
    YaziCiz(M("PORTFÖY ÖZETİ", "PORTFOLIO"), (int)oz.x + 14, (int)oz.y + 10, 14, RENK_SOLUK_METIN);
    string s1 = string(M("Pozisyonlar: ", "Positions: ")) + Para(o.PortfoyDegeri());
    string s2 = string(M("Nakit: ", "Cash: ")) + Para(o.bakiye);
    string s3 = string(M("Gerçekleşen K/Z: ", "Realized P/L: ")) + ParaIsaretli(o.gerceklesenKar);
    string s4 = string(M("İşlem: ", "Trades: ")) + to_string(o.islemSayisi);
    YaziCiz(s1, (int)oz.x + 14, (int)oz.y + 32, 13, RENK_METIN);
    YaziCiz(s2, (int)oz.x + 14, (int)oz.y + 52, 13, RENK_METIN);
    YaziCiz(s3, (int)oz.x + 14, (int)oz.y + 72, 13, o.gerceklesenKar >= 0 ? RENK_YESIL : RENK_KIRMIZI);
    int sagX = (int)(oz.x + oz.width) - 14;
    if (sagX - YaziOlc(s4, 13) > (int)oz.x + 14 + YaziOlc(s3, 13) + 10) YaziCizSaga(s4, sagX, (int)oz.y + 72, 13, RENK_SOLUK_METIN);
    else YaziCizSaga(s4, sagX, (int)oz.y + 52, 13, RENK_SOLUK_METIN);
}

// ---------------------------------------------------------------------------
// ISLEM ADEDI PANELI
// ---------------------------------------------------------------------------
static void AdetPaneliCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    Rectangle p = a.adetPanel;
    PanelCiz(p, 0.08f, RENK_PANEL, RENK_KENARLIK);
    const char* baslik = M("İşlem Adedi", "Trade Size");
    YaziCiz(baslik, (int)p.x + 14, (int)p.y + 10, 14, RENK_SOLUK_METIN);
    string secili = (o.secilenAdet > 0) ? string(M("Seçili: ", "Selected: ")) + to_string(o.secilenAdet)
                                        : string(M("Seçili: MAKS", "Selected: MAX"));
    int seciliX = (int)p.x + 14 + YaziOlc(baslik, 14) + 18;
    YaziCiz(secili, seciliX, (int)p.y + 10, 14, RENK_ALTIN);

    // Tahmini islem maliyeti: oyuncu TIKLAMADAN once ne odeyecegini gormeli;
    // yoksa maliyet gorunmez bir vergi gibi hissettirir.
    string maliyet = (o.secilenAdet > 0)
                         ? string(M("maliyet ~", "cost ~")) + Yuzde(NotrMaliyetOrani(o.secilenAdet) * 100.0f, 2)
                         : string(M("maliyet: adede göre", "cost: by size"));
    int sagX = (int)(p.x + p.width) - 14;
    if (sagX - YaziOlc(maliyet, 12) > seciliX + YaziOlc(secili, 14) + 10)
        YaziCizSaga(maliyet, sagX, (int)p.y + 12, 12, RENK_SOLUK_METIN);

    for (int i = 0; i < 8; i++) {
        Buton& b = a.adetBtn[i];
        Color eski = b.renk;
        if (a.adetDegerleri[i] == o.secilenAdet) b.renk = RENK_ALTIN;
        b.Ciz(14);
        b.renk = eski;
    }
    a.sifirlaBtn.Ciz(15);
}
