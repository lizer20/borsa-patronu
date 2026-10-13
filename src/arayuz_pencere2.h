// ============================================================================
// arayuz_pencere2.h - emir penceresi, kredi masasi, oyun sonu penceresi
// ============================================================================

// ---------------------------------------------------------------------------
// EMIR PENCERESI (stop / kar al / limit alim)
// ---------------------------------------------------------------------------
// Seviyeler GUNCEL fiyata gore hazir yuzdelerdir; pencere acikken simulasyon
// durdugu icin fiyat degismez. En yakin seviye bile fiyattan %3 uzakta oldugundan
// "duraklatip aninda islem yapma" yolu acilmaz.
static void EmirPenceresiGuncelle(Uygulama& u, float dt, Vector2 fare, bool tik) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (a.emirHisse < 0 || a.emirHisse >= (int)o.hisseler.size()) {
        a.emirHisse = -1;
        return;
    }
    const int idx = a.emirHisse;
    Hisse& h = o.hisseler[idx];
    const bool acik = h.adet < 0;

    for (int t = 0; t < EMIR_TUR_SAYISI; t++) {
        // Stop ve kar al bir pozisyonu korur; limit alim aciktayken verilemez.
        bool kullanilabilir = (t == EMIR_LIMIT_ALIM) ? (h.adet >= 0) : (h.adet != 0);
        for (int k = 0; k < 4; k++) {
            Buton& b = a.emirSeviyeBtn[t][k];
            b.aktif = kullanilabilir;
            if (b.Guncelle(dt, fare, tik)) {
                float oran = EmirSeviyeOrani(t, k, acik);
                EmirKoy(o, idx, t, h.fiyat * (1.0f + oran), (t == EMIR_LIMIT_ALIM) ? o.secilenAdet : 0);
                Cal(o.sesler.tikla);
            }
        }
        a.emirKaldirBtn[t].aktif = h.emirler[t].aktif;
        if (a.emirKaldirBtn[t].Guncelle(dt, fare, tik)) {
            EmirKaldir(o, idx, t);
            Cal(o.sesler.tikla);
        }
    }

    bool disariTik = tik && !CheckCollisionPointRec(fare, a.emirPanel);
    if (a.emirKapatBtn.Guncelle(dt, fare, tik) || disariTik) {
        a.emirHisse = -1;
        Cal(o.sesler.tikla);
    }
}

static void EmirPenceresiCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (a.emirHisse < 0 || a.emirHisse >= (int)o.hisseler.size()) return;
    const Hisse& h = o.hisseler[a.emirHisse];
    const bool acik = h.adet < 0;
    const bool dar = DarEkran();

    ModalKarart(0.6f);
    Rectangle p = a.emirPanel;
    PanelCiz(p, 0.04f, RENK_PANEL, RENK_ALTIN);
    const int px = (int)p.x + 24, py = (int)p.y;
    const int sagX = (int)(p.x + p.width) - 24;
    const int ic = (int)p.width - 48;

    YaziCiz(string(M("EMİR - ", "ORDERS - ")) + h.kod, px, py + 20, 22, RENK_ALTIN);
    YaziCizSaga(string(M("Fiyat: ", "Price: ")) + Para(h.fiyat), sagX, py + 25, 15, RENK_METIN);

    string poz;
    if (h.adet > 0)
        poz = string(M("Pozisyon: ", "Position: ")) + to_string(h.adet) + M(" adet, ortalama ", " shares, average ") +
              Para(h.maliyet / (float)h.adet);
    else if (h.adet < 0)
        poz = string(M("Açığa: ", "Short: ")) + to_string(-h.adet) + M(" adet, ortalama ", " shares, average ") +
              Para(h.maliyet / (float)(-h.adet));
    else
        poz = M("Pozisyonun yok - zarar kes ve kâr al için önce pozisyon aç.",
                "No position - open one first to use stop-loss and take profit.");
    YaziCizSarili(poz, px, py + 52, 13, ic, RENK_SOLUK_METIN, 2, 16);

    const Color renkler[EMIR_TUR_SAYISI] = {RENK_KIRMIZI, RENK_YESIL, RENK_MAVI_VURGU};
    for (int t = 0; t < EMIR_TUR_SAYISI; t++) {
        int ry = py + 90 + (int)(t * a.emirSatirYuk);
        const char* ad;
        string aciklama;
        if (t == EMIR_STOP) {
            ad = M("ZARAR KES (STOP)", "STOP-LOSS");
            aciklama = acik ? M("Fiyat bu seviyenin ÜSTÜNE çıkarsa tüm açığı kapatır.",
                                "Covers your whole short if the price rises ABOVE this level.")
                            : M("Fiyat bu seviyenin ALTINA inerse tüm pozisyonu satar.",
                                "Sells your whole position if the price falls BELOW this level.");
        } else if (t == EMIR_KAR_AL) {
            ad = M("KÂR AL (HEDEF)", "TAKE PROFIT");
            aciklama = acik ? M("Fiyat bu seviyenin ALTINA inerse tüm açığı kapatır.",
                                "Covers your whole short if the price falls BELOW this level.")
                            : M("Fiyat bu seviyenin ÜSTÜNE çıkarsa tüm pozisyonu satar.",
                                "Sells your whole position if the price rises ABOVE this level.");
        } else {
            ad = M("LİMİT ALIM", "LIMIT BUY");
            string miktar = (o.secilenAdet > 0) ? to_string(o.secilenAdet) + M(" adet", " shares")
                                                : string(M("alabildiğin kadar", "as many shares as you can afford"));
            aciklama = (g_dil == Dil::EN) ? ("Buys " + miktar + " if the price falls BELOW this level.")
                                          : ("Fiyat bu seviyenin ALTINA inerse " + miktar + " alır.");
        }
        YaziCiz(ad, px, ry, 15, renkler[t]);

        // Durum yazisi: genis pencerede en saga, dar pencerede "Kaldir" dugmesinin soluna.
        int durumSag = dar ? (int)a.emirKaldirBtn[t].rect.x - 10 : sagX;
        int adBitis = px + YaziOlc(ad, 15) + 12;
        if (h.emirler[t].aktif) {
            string uzun = string(M("Aktif: ", "Active: ")) + Para(h.emirler[t].fiyat);
            string kisa = Para(h.emirler[t].fiyat);
            string d = (durumSag - YaziOlc(uzun, 14) >= adBitis) ? uzun : kisa;
            if (durumSag - YaziOlc(d, 14) >= adBitis) YaziCizSaga(d, durumSag, ry + 1, 14, RENK_ALTIN);
        } else if (durumSag - YaziOlc(M("emir yok", "no order"), 13) >= adBitis) {
            YaziCizSaga(M("emir yok", "no order"), durumSag, ry + 1, 13, RENK_SOLUK_METIN);
        }

        YaziCizSarili(aciklama, px, ry + (dar ? 26 : 22), 12, ic, RENK_SOLUK_METIN, dar ? 2 : 1, 15);
        for (int k = 0; k < 4; k++) a.emirSeviyeBtn[t][k].Ciz(14);
        a.emirKaldirBtn[t].Ciz(13);
    }
    a.emirKapatBtn.Ciz(15);
}

// ---------------------------------------------------------------------------
// KREDI MASASI
// ---------------------------------------------------------------------------
static void KrediPenceresiGuncelle(Uygulama& u, float dt, Vector2 fare, bool tik) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    for (int k = 0; k < 4; k++) {
        if (a.krediAlBtn[k].Guncelle(dt, fare, tik))
            KrediAl(o, KREDI_TUTARLARI[k] > 0.0f ? KREDI_TUTARLARI[k] : o.KalanKaldirac());
        if (a.krediOdeBtn[k].Guncelle(dt, fare, tik))
            KrediOde(o, KREDI_TUTARLARI[k] > 0.0f ? KREDI_TUTARLARI[k] : 1e12f);
    }
    bool disariTik = tik && !CheckCollisionPointRec(fare, a.krediPanel);
    if (a.krediKapatBtn.Guncelle(dt, fare, tik) || disariTik) {
        a.krediAcik = false;
        Cal(o.sesler.tikla);
    }
}

static void KrediPenceresiCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    ModalKarart(0.6f);
    Rectangle p = a.krediPanel;
    PanelCiz(p, 0.04f, RENK_PANEL, RENK_ALTIN);
    const int px = (int)p.x + 24, py = (int)p.y;
    const int sagX = (int)(p.x + p.width) - 24;

    YaziCiz(M("KREDİ MASASI", "CREDIT DESK"), px, py + 20, 22, RENK_ALTIN);

    float yukumluluk = o.Yukumluluk();
    float gunlukMasraf = o.borc * KREDI_GUNLUK_FAIZ + o.AcigaDegeri() * ACIGA_GUNLUK_UCRET;
    struct Satir { string etiket, deger; Color renk; };
    Color oranRenk = RENK_METIN;
    string oranMetni = "-";
    if (yukumluluk >= 1.0f) {
        float oran = o.TeminatOrani();
        oranRenk = (oran < TEMINAT_UYARI_ORANI) ? RENK_KIRMIZI : (oran < 1.0f ? RENK_UYARI : RENK_YESIL);
        oranMetni = Yuzde(fminf(oran, 9.99f) * 100.0f, 0);
    }
    Satir satirlar[6] = {
        {M("Net değer (özkaynak)", "Net worth (equity)"), Para(o.NetDeger()), RENK_METIN},
        {M("Borç", "Debt"), Para(o.borc), o.borc > 0.5f ? RENK_UYARI : RENK_METIN},
        {M("Açığa satış yükümlülüğü", "Short-sale liability"), Para(o.AcigaDegeri()), RENK_METIN},
        {M("Kullanılabilir limit", "Available limit"), Para(o.KalanKaldirac(), 0), RENK_YESIL},
        {string(M("Günlük faiz ", "Daily interest ")) + Yuzde(KREDI_GUNLUK_FAIZ * 100.0f, 2),
         string("~") + Para(gunlukMasraf) + M(" / gün", " / day"), RENK_METIN},
        {string(M("Teminat (zorla kapatma: ", "Margin (forced close: ")) + Yuzde(TEMINAT_CAGRI_ORANI * 100.0f, 0) +
             M(" altı)", " or lower)"),
         oranMetni, oranRenk},
    };
    for (int i = 0; i < 6; i++) {
        int y = py + 62 + i * 24;
        YaziCiz(satirlar[i].etiket, px, y, 14, RENK_SOLUK_METIN);
        YaziCizSaga(satirlar[i].deger, sagX, y, 14, satirlar[i].renk);
    }

    YaziCiz(M("BORÇ AL", "BORROW"), px, py + 214, 13, RENK_SOLUK_METIN);
    YaziCiz(M("BORÇ ÖDE", "REPAY"), px, py + 290, 13, RENK_SOLUK_METIN);
    for (int k = 0; k < 4; k++) {
        a.krediAlBtn[k].Ciz(14);
        a.krediOdeBtn[k].Ciz(14);
    }
    a.krediKapatBtn.Ciz(15);
}

// ---------------------------------------------------------------------------
// OYUN SONU (zirve / sure doldu / iflas)
// ---------------------------------------------------------------------------
// YYYYAAGG -> "GG.AA.YYYY"
static string TarihMetni(int tarih) {
    char tampon[16];
    snprintf(tampon, sizeof(tampon), "%02d.%02d.%04d", tarih % 100, (tarih / 100) % 100, tarih / 10000);
    return tampon;
}

// Gunun Piyasasi skorunu panoya kopyalar (paylasmak icin).
static void SkoruKopyala(Uygulama& u) {
    Oyun& o = u.oyun;
    string metin = string("Borsa Patronu - ") + M("Günün Piyasası ", "Daily Market ") + TarihMetni(o.gunlukTarih) + ": " +
                   Para(o.bitisNetDeger, 0) + "  https://emrebiltekin.me/borsa-patronu/";
    SetClipboardText(metin.c_str());
    u.ui.kopyaBildirimSuresi = 2.5f;
}

static bool SonPenceresindeKopyaVar(const Oyun& o) { return o.sonTuru == SON_SURE && o.Sen().gunluk; }

static void OyunSonuGuncelle(Uygulama& u, float dt, Vector2 fare, bool tik) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (a.kopyaBildirimSuresi > 0.0f) a.kopyaBildirimSuresi -= dt;
    if (SonPenceresindeKopyaVar(o) && a.sonKopyaBtn.Guncelle(dt, fare, tik)) {
        SkoruKopyala(u);
        Cal(o.sesler.tikla);
    }
    if (a.sonBirBtn.Guncelle(dt, fare, tik)) {
        Cal(o.sesler.tikla);
        if (o.sonTuru == SON_IFLAS) {
            o.Baslat(o.senaryo, false); // ayni senaryoyu bastan
        } else {
            o.sonPenceresiAcik = false; // kum havuzu olarak devam
            o.sonTuru = SON_YOK;
        }
    } else if (a.sonIkiBtn.Guncelle(dt, fare, tik)) {
        Cal(o.sesler.tikla);
        if (o.sonTuru == SON_IFLAS) {
            o.Baslat(o.senaryo, false);
            u.oyunVar = false; // menude "BASLA" gorunsun
        } else {
            o.sonPenceresiAcik = false;
            o.sonTuru = SON_YOK;
        }
        u.ekran = Ekran::MENU;
    }
}

static void OyunSonuCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    ModalKarart(0.72f);
    Rectangle p = a.sonPanel;
    const bool iflas = (o.sonTuru == SON_IFLAS);
    PanelCiz(p, 0.04f, RENK_PANEL, iflas ? RENK_KIRMIZI : RENK_ALTIN);
    const int ortaX = (int)(p.x + p.width / 2.0f);
    const int py = (int)p.y;
    const int ic = (int)p.width - 40;

    string baslik, sonuc, rekor;
    Color rekorRenk = RENK_SOLUK_METIN;
    if (o.sonTuru == SON_ZIRVE) {
        baslik = M("ZİRVEYE ULAŞTIN!", "YOU REACHED THE TOP!");
        sonuc = (g_dil == Dil::EN)
                    ? "You became a " + string(RUTBELER.back().ad.en) + " in " + to_string(o.bitisGunu) + " days."
                    : string(RUTBELER.back().ad.tr) + " rütbesine " + to_string(o.bitisGunu) + " günde ulaştın.";
        int r = g_profil.rekorGun[o.senaryo];
        if (o.yeniRekor) { rekor = M("YENİ REKOR!", "NEW RECORD!"); rekorRenk = RENK_ALTIN; }
        else if (r > 0) rekor = string(M("Rekorun: ", "Your record: ")) + to_string(r) + M(" gün", " days");
    } else if (o.sonTuru == SON_SURE) {
        baslik = M("SÜRE DOLDU!", "TIME IS UP!");
        sonuc = to_string(o.bitisGunu) + M(" gün sonunda net değerin: ", " days later your net worth is: ") + Para(o.bitisNetDeger);
        float r = o.Sen().gunluk ? BugunkuGunlukSkor() : g_profil.rekorNet[o.senaryo];
        if (o.yeniRekor) { rekor = o.Sen().gunluk ? M("BUGÜNÜN EN İYİ SKORUN!", "YOUR BEST SCORE TODAY!") : M("YENİ REKOR!", "NEW RECORD!"); rekorRenk = RENK_ALTIN; }
        else if (r > 0.0f) rekor = string(o.Sen().gunluk ? M("Bugünkü en iyin: ", "Your best today: ") : M("Rekorun: ", "Your record: ")) + Para(r);
    } else {
        baslik = M("İFLAS ETTİN", "YOU WENT BANKRUPT");
        sonuc = (g_dil == Dil::EN) ? "Your capital ran out on day " + to_string(o.bitisGunu) + "."
                                   : to_string(o.bitisGunu) + ". günde sermayen tükendi.";
    }
    YaziCizOrtali(baslik, ortaX, py + 24, DarEkran() ? 24 : 28, iflas ? RENK_KIRMIZI : RENK_ALTIN);
    string senaryoMetni = string(M("Senaryo: ", "Scenario: ")) + o.Sen().ad.al();
    if (o.Sen().gunluk && o.gunlukTarih > 0) senaryoMetni += "  -  " + TarihMetni(o.gunlukTarih);
    YaziCizOrtali(senaryoMetni, ortaX, py + 62, 14, RENK_SOLUK_METIN);
    int y = py + 88;
    y += YaziCizSariliOrtali(sonuc, ortaX, y, 16, ic, RENK_METIN, 20) + 6;
    if (!rekor.empty()) YaziCizOrtali(rekor, ortaX, y, 14, rekorRenk);

    struct Satir { string etiket, deger; };
    Satir satirlar[7] = {
        {M("İşlem sayısı", "Trades"), to_string(o.islemSayisi)},
        {M("Gerçekleşen kâr/zarar", "Realized profit/loss"), ParaIsaretli(o.gerceklesenKar)},
        {M("En iyi satış", "Best sale"), ParaIsaretli(o.enIyiIslem)},
        {M("Toplanan temettü", "Dividends collected"), Para(o.toplamTemettu)},
        {M("Ödenen faiz ve ücret", "Interest and fees paid"), Para(o.toplamFaiz)},
        {M("Tamamlanan görev", "Missions completed"), to_string(o.tamamlananGorev)},
        {M("Başarımlar", "Achievements"), to_string(AcilanBasarimSayisi()) + " / " + to_string((int)BASARIM_SAYISI)},
    };
    const int kenar = DarEkran() ? 24 : 70;
    for (int i = 0; i < 7; i++) {
        int sy = py + 176 + i * 24;
        YaziCiz(satirlar[i].etiket, (int)p.x + kenar, sy, 14, RENK_SOLUK_METIN);
        YaziCizSaga(satirlar[i].deger, (int)(p.x + p.width) - kenar, sy, 14, RENK_METIN);
    }
    if (SonPenceresindeKopyaVar(o)) a.sonKopyaBtn.Ciz(14);
    a.sonBirBtn.Ciz(16);
    a.sonIkiBtn.Ciz(16);
}
