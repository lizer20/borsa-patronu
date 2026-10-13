// ============================================================================
// piyasa_islem.h - alim/satim, islem maliyeti (mikroyapi), emirler, kredi,
//                  aciga satis ve teminat cagrisi
// ============================================================================

static void RutbeVeHedefKontrolEt(Oyun& o); // piyasa_olay.h

static void HaberBandinaEkle(Oyun& o, const string& s) {
    o.haberBandi.push_back(s);
    if (o.haberBandi.size() > 12) o.haberBandi.pop_front();
    // Bant en yeni haberi HEMEN gostersin diye basa sarilir. ISTISNA: su an bir
    // "SON DAKIKA" haberi ekranda SABIT duruyorsa kaydirmayi bozmuyoruz, yoksa
    // araya giren bir alim/satim mesaji haberi ekrandan kaydirip goturur.
    if (o.haberSabitKalmaSuresi <= 0.0f) o.bantKaydirma = 0.0f;
}

static void ParlamaBaslat(Oyun& o, Color renk) {
    o.flashAlpha = 0.55f;
    o.flashRenk = renk;
}

// Rutbe ilerledikce 1.0'dan 0.0'a inen "acemi korumasi" katsayisi. Korumasiz
// senaryolarda (Kriz Yili) en bastan 0'dir.
static float AcemiKorumasi(const Oyun& o) {
    if (!o.Sen().acemiKorumasi) return 0.0f;
    int sonRutbe = (int)RUTBELER.size() - 1;
    if (sonRutbe <= 0) return 0.0f;
    return Clamp(1.0f - (float)o.rutbeIndex / (float)sonRutbe, 0.0f, 1.0f);
}

// ---------------------------------------------------------------------------
// MIKROYAPI: akis tabanli islem maliyeti (ayrintili aciklama temel.h icinde)
// ---------------------------------------------------------------------------
// f(Q): Q adetlik net akisin yarattigi anlik baski (isaretli oran).
static float EtkiEgrisi(float q) {
    float a = fabsf(q);
    if (a <= 0.0f) return 0.0f;
    float v = ETKI_OLCEK * sqrtf(a / (float)TOPLAM_HISSE_SAYISI);
    return (q >= 0.0f) ? v : -v;
}

// F(Q) = integral(0..Q) f = (2/3) * ETKI_OLCEK * |Q|^1.5 / sqrt(N). Cift fonksiyon.
static float EtkiIntegrali(float q) {
    float a = fabsf(q);
    return (2.0f / 3.0f) * ETKI_OLCEK * a * sqrtf(a / (float)TOPLAM_HISSE_SAYISI);
}

// Bu emrin GERCEKTEN gerceklesecegi birim fiyat: piyasa fiyati x (1 + emrin
// akisi Q0'dan Q1'e tasirken odedigi ortalama baski +/- spread'in yarisi).
static float DolumFiyati(const Hisse& h, int adet, bool alim) {
    if (adet <= 0) return h.fiyat;
    float d = alim ? (float)adet : -(float)adet;
    float ortBaski = (EtkiIntegrali(h.akis + d) - EtkiIntegrali(h.akis)) / d;
    float sapma = ortBaski + (alim ? 0.5f : -0.5f) * ALIM_SATIM_SPREAD;
    return fmaxf(0.01f, h.fiyat * (1.0f + sapma));
}

// Notr durumdaki (baski yokken) bir hissede `adet` buyuklugunde tek emrin
// maliyet orani. Adet panelindeki "maliyet ~%x" gostergesi bunu kullanir.
static float NotrMaliyetOrani(int adet) {
    if (adet <= 0) return 0.0f;
    return ALIM_SATIM_SPREAD * 0.5f + (2.0f / 3.0f) * ETKI_OLCEK * sqrtf((float)adet / (float)TOPLAM_HISSE_SAYISI);
}

// `nakit` ile bu hisseden en fazla kac adet alinabilir? Dolum fiyati adede bagli
// oldugu icin "nakit / fiyat" yanlis cevap verir; yaklasip sonra dogruluyoruz.
static int MaksAlinabilir(const Hisse& h, float nakit) {
    if (h.fiyat <= 0.01f || nakit <= 0.0f) return 0;
    int n = (int)(nakit / h.fiyat);
    for (int i = 0; i < 6 && n > 0; i++) n = (int)(nakit / DolumFiyati(h, n, true));
    while (n > 0 && DolumFiyati(h, n, true) * (float)n > nakit) n--;
    return max(0, n);
}

// ---------------------------------------------------------------------------
// DEVRALMA
// ---------------------------------------------------------------------------
static void DevralmaKontrolEt(Oyun& o, int idx) {
    Hisse& h = o.hisseler[idx];
    bool coguldaMi = (h.adet * 2 > TOPLAM_HISSE_SAYISI);
    if (coguldaMi && !h.sahibiyim) {
        h.sahibiyim = true;
        h.yonetimOlayZamanlayici = RastgeleAralik(YONETIM_OLAY_MIN_SN, YONETIM_OLAY_MAX_SN);
        o.toastlar.Ekle(M("ŞİRKETİ DEVRALDIN!", "COMPANY TAKEN OVER!"),
                        h.kod + M(" artık senin kontrolünde.", " is now under your control."), RENK_ALTIN, 6.0f);
        HaberBandinaEkle(o, string(M("DEVİR ALINDI: ", "TAKEOVER: ")) + to_string(h.adet) + "/" +
                                 to_string(TOPLAM_HISSE_SAYISI) + " " + h.kod +
                                 M(" hissesiyle şirketin patronu oldun!", " shares - you are now the boss!"));
        o.parcaciklar.Patlat({(float)PENCERE_GENISLIK / 2.0f, 200.0f}, RENK_ALTIN, 40, 260.0f);
        ParlamaBaslat(o, RENK_ALTIN);
        Cal(o.sesler.rutbeAtlama);
        BasarimVer(o, B_PATRON);
        int n = o.SahipOlunanSirketSayisi();
        if (n >= 3) BasarimVer(o, B_HOLDING);
        if (n >= (int)o.hisseler.size()) BasarimVer(o, B_TEKEL);
    } else if (!coguldaMi && h.sahibiyim) {
        h.sahibiyim = false;
        o.toastlar.Ekle(M("KONTROLÜ KAYBETTİN", "CONTROL LOST"),
                        h.kod + M(" artık senin kontrolünde değil.", " is no longer under your control."),
                        RENK_KIRMIZI, 5.0f);
        HaberBandinaEkle(o, h.kod + M(": çoğunluk hissesini kaybettin, kontrol elden gitti.",
                                      ": you lost the majority stake and control of the company."));
    }
}

// ---------------------------------------------------------------------------
// ISLEM
// ---------------------------------------------------------------------------
enum class IslemKaynagi { OYUNCU, EMIR, TEMINAT };

struct IslemSonucu {
    bool basarili = false;
    int adet = 0;
    float dolum = 0.0f;
    float kar = 0.0f;
};

// Oyuncunun sectigi adede gore bu islemde kac adet el degistirecek?
// secilenAdet > 0 ise o; MAKS (-1) modunda duruma gore mumkun olan en fazlasi.
static int IslemAdedi(const Oyun& o, const Hisse& h, bool alim) {
    int sec = o.secilenAdet;
    if (alim) {
        if (h.adet < 0) { // acik kapatma
            int acik = -h.adet;
            if (sec > 0) return min(sec, acik);
            return min(acik, MaksAlinabilir(h, o.bakiye));
        }
        if (sec > 0) return sec;
        return max(0, min(MaksAlinabilir(h, o.bakiye), TOPLAM_HISSE_SAYISI - h.adet));
    }
    if (h.adet > 0) return (sec > 0) ? sec : h.adet; // uzun satis
    if (!o.AcigaSatisAcik()) return 0;
    if (sec > 0) return sec; // aciga satis (sinirlari IslemYap denetler)
    if (h.fiyat <= 0.01f) return 0;
    int kapasite = ACIGA_HISSE_SINIRI + h.adet; // h.adet <= 0
    return max(0, min(kapasite, (int)(o.KalanKaldirac() * 0.98f / h.fiyat)));
}

// Tum alim/satim buradan gecer. alim=true: uzun al VEYA acigi kapat.
// alim=false: uzun sat VEYA aciga sat. Tek islemde pozisyonun ISARETI degismez
// (uzundan dogrudan aciga gecilmez) - yanlislikla aciga dusmeyi onler.
static IslemSonucu IslemYap(Oyun& o, int idx, bool alim, int adet, IslemKaynagi kaynak) {
    IslemSonucu sonuc;
    Hisse& h = o.hisseler[idx];
    const bool oyuncu = (kaynak == IslemKaynagi::OYUNCU);
    auto hata = [&](const string& baslik, const string& govde) -> IslemSonucu {
        if (oyuncu) {
            o.toastlar.Ekle(baslik, govde, RENK_KIRMIZI, 3.2f);
            Cal(o.sesler.hataUyari);
        }
        return sonuc;
    };
    if (adet <= 0) return sonuc;

    string onEk;
    if (kaynak == IslemKaynagi::EMIR) onEk = M("EMİR - ", "ORDER - ");
    else if (kaynak == IslemKaynagi::TEMINAT) onEk = M("TEMİNAT - ", "MARGIN - ");
    float dolum = 0.0f, kar = 0.0f;

    if (alim && h.adet >= 0) {
        // ---------------- UZUN ALIM ----------------
        if (h.adet + adet > TOPLAM_HISSE_SAYISI)
            return hata(M("Hisse Kalmadı", "No Shares Left"),
                        h.kod + M(": şirketin toplam ", ": the company has only ") + to_string(TOPLAM_HISSE_SAYISI) +
                            M(" hissesi var.", " shares."));
        dolum = DolumFiyati(h, adet, true);
        float toplam = dolum * (float)adet;
        if (toplam > o.bakiye + 0.005f)
            return hata(M("Yetersiz Bakiye", "Not Enough Cash"),
                        h.kod + M(" için ", " needs ") + Para(toplam) + M(" gerekiyor.", "."));
        o.bakiye -= toplam;
        h.maliyet += toplam; // maliyet tabani islem maliyetini de icerir
        h.adet += adet;
        float maliyetYuzde = (dolum / h.fiyat - 1.0f) * 100.0f;
        HaberBandinaEkle(o, onEk + M("ALIM: ", "BUY: ") + to_string(adet) + M(" adet ", " ") + h.kod + " @ " +
                                 Para(dolum) + " (" + M("maliyet ", "cost ") + Yuzde(maliyetYuzde, 2) + ")");
        if (oyuncu) Cal(o.sesler.alim);
    } else if (alim) {
        // ---------------- ACIK KAPATMA ----------------
        int acik = -h.adet;
        if (adet > acik) adet = acik;
        dolum = DolumFiyati(h, adet, true);
        float toplam = dolum * (float)adet;
        if (oyuncu && toplam > o.bakiye + 0.005f)
            return hata(M("Yetersiz Bakiye", "Not Enough Cash"),
                        M("Açığı kapatmak için ", "Covering needs ") + Para(toplam) + M(" gerekiyor.", "."));
        float ortSatis = h.maliyet / (float)acik;
        kar = (ortSatis - dolum) * (float)adet;
        h.maliyet -= ortSatis * (float)adet;
        h.adet += adet;
        if (h.adet == 0) h.maliyet = 0.0f;
        o.bakiye -= toplam;
        // Emir ya da teminat cagrisiyla kapatilan acikta nakit yetmezse fark borca yazilir.
        if (o.bakiye < 0.0f) {
            o.borc += -o.bakiye;
            o.bakiye = 0.0f;
        }
        o.gerceklesenKar += kar;
        o.enIyiIslem = fmaxf(o.enIyiIslem, kar);
        o.enKotuIslem = fminf(o.enKotuIslem, kar);
        HaberBandinaEkle(o, onEk + M("AÇIK KAPATMA: ", "COVER: ") + to_string(adet) + M(" adet ", " ") + h.kod +
                                 " @ " + Para(dolum) + " (" + ParaIsaretli(kar) + ")");
        if (oyuncu) Cal(kar >= 0 ? o.sesler.satimKar : o.sesler.satimZarar);
        if (kar >= 1000.0f) BasarimVer(o, B_AYI);
    } else if (h.adet > 0) {
        // ---------------- UZUN SATIS ----------------
        if (adet > h.adet) {
            if (oyuncu)
                return hata(M("Yetersiz Hisse", "Not Enough Shares"),
                            M("Elinde sadece ", "You only hold ") + to_string(h.adet) + M(" adet ", " ") + h.kod +
                                M(" var.", "."));
            adet = h.adet;
        }
        float ortMaliyet = h.maliyet / (float)h.adet;
        dolum = DolumFiyati(h, adet, false);
        kar = (dolum - ortMaliyet) * (float)adet;
        h.maliyet -= ortMaliyet * (float)adet;
        h.adet -= adet;
        if (h.adet == 0) h.maliyet = 0.0f;
        o.bakiye += dolum * (float)adet;
        o.gerceklesenKar += kar;
        o.enIyiIslem = fmaxf(o.enIyiIslem, kar);
        o.enKotuIslem = fminf(o.enKotuIslem, kar);
        o.rehberSatisSayisi++;
        HaberBandinaEkle(o, onEk + M("SATIŞ: ", "SELL: ") + to_string(adet) + M(" adet ", " ") + h.kod + " @ " +
                                 Para(dolum) + " (" + ParaIsaretli(kar) + ")");
        if (oyuncu) Cal(kar >= 0 ? o.sesler.satimKar : o.sesler.satimZarar);
        if (kar >= 1000.0f) BasarimVer(o, B_KAR_1K);
        if (kar >= 25000.0f) BasarimVer(o, B_KAR_25K);
    } else {
        // ---------------- ACIGA SATIS ----------------
        if (!o.AcigaSatisAcik())
            return hata(M("Yetersiz Hisse", "No Shares"),
                        M("Elinde hiç ", "You do not hold any ") + h.kod + M(" yok.", "."));
        if (-h.adet + adet > ACIGA_HISSE_SINIRI)
            return hata(M("Açığa Satış Sınırı", "Short Limit"),
                        M("Bir hissede en fazla ", "You can short at most ") + to_string(ACIGA_HISSE_SINIRI) +
                            M(" adet açığa satılabilir.", " shares of one stock."));
        if (h.fiyat * (float)adet > o.KalanKaldirac() + 0.5f)
            return hata(M("Teminat Yetersiz", "Not Enough Margin"),
                        M("Açığa satış kapasiten: ", "Your short capacity: ") + Para(o.KalanKaldirac(), 0));
        dolum = DolumFiyati(h, adet, false);
        float gelir = dolum * (float)adet;
        o.bakiye += gelir;
        h.maliyet += gelir;
        h.adet -= adet;
        HaberBandinaEkle(o, onEk + M("AÇIĞA SATIŞ: ", "SHORT: ") + to_string(adet) + M(" adet ", " ") + h.kod +
                                 " @ " + Para(dolum));
        if (oyuncu) Cal(o.sesler.alim);
    }

    h.akis += alim ? (float)adet : -(float)adet;
    o.islemSayisi++;
    sonuc.basarili = true;
    sonuc.adet = adet;
    sonuc.dolum = dolum;
    sonuc.kar = kar;

    // Pozisyon kapandiysa onu koruyan emirlerin anlami kalmaz; aciktayken de
    // limit ALIM emri duramaz (once acik kapanmali).
    if (h.adet == 0) {
        h.emirler[EMIR_STOP].aktif = false;
        h.emirler[EMIR_KAR_AL].aktif = false;
    }
    if (h.adet < 0) h.emirler[EMIR_LIMIT_ALIM].aktif = false;

    if (!o.ilkIslemYapildi) {
        o.ilkIslemYapildi = true;
        o.toastlar.Ekle(M("İlk İşlemin!", "Your First Trade!"),
                        M("Trader kariyerine hoş geldin.", "Welcome to your trading career."), RENK_MAVI_VURGU, 4.0f);
        BasarimVer(o, B_ILK_ISLEM);
    }
    DevralmaKontrolEt(o, idx);
    RutbeVeHedefKontrolEt(o);
    return sonuc;
}

// Oyuncunun AL dugmesi.
static bool HisseAl(Oyun& o, int idx) {
    Hisse& h = o.hisseler[idx];
    int adet = IslemAdedi(o, h, true);
    if (adet <= 0) {
        o.toastlar.Ekle(M("İşlem Yapılamadı", "Trade Failed"),
                        h.kod + M(" için yeterli bakiye yok.", ": not enough cash."), RENK_KIRMIZI, 3.0f);
        Cal(o.sesler.hataUyari);
        return false;
    }
    return IslemYap(o, idx, true, adet, IslemKaynagi::OYUNCU).basarili;
}

// Oyuncunun SAT dugmesi (elde hisse yoksa ve aciga satis acildiysa aciga satar).
static bool HisseSat(Oyun& o, int idx) {
    Hisse& h = o.hisseler[idx];
    int adet = IslemAdedi(o, h, false);
    if (adet <= 0) {
        if (h.adet <= 0 && o.AcigaSatisAcik())
            o.toastlar.Ekle(M("Teminat Yetersiz", "Not Enough Margin"),
                            M("Açığa satış için kapasiten kalmadı.", "No capacity left for short selling."),
                            RENK_KIRMIZI, 3.0f);
        else
            o.toastlar.Ekle(M("Yetersiz Hisse", "No Shares"),
                            M("Elinde hiç ", "You do not hold any ") + h.kod + M(" yok.", "."), RENK_KIRMIZI, 3.0f);
        Cal(o.sesler.hataUyari);
        return false;
    }
    return IslemYap(o, idx, false, adet, IslemKaynagi::OYUNCU).basarili;
}
