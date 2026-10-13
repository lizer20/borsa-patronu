// ============================================================================
// piyasa_kaldirac.h - bekleyen emirler, kredi, teminat cagrisi ve iflas
// ============================================================================

// ---------------------------------------------------------------------------
// BEKLEYEN EMIRLER (stop / kar al / limit alim)
// ---------------------------------------------------------------------------
static void EmirKoy(Oyun& o, int idx, int tur, float fiyat, int adet) {
    if (idx < 0 || idx >= (int)o.hisseler.size() || tur < 0 || tur >= EMIR_TUR_SAYISI) return;
    Emir& e = o.hisseler[idx].emirler[tur];
    e.aktif = true;
    e.fiyat = fmaxf(0.5f, fiyat);
    e.adet = adet;
}

static void EmirKaldir(Oyun& o, int idx, int tur) {
    if (idx < 0 || idx >= (int)o.hisseler.size() || tur < 0 || tur >= EMIR_TUR_SAYISI) return;
    o.hisseler[idx].emirler[tur].aktif = false;
}

static int AktifEmirSayisi(const Hisse& h) {
    int n = 0;
    for (int t = 0; t < EMIR_TUR_SAYISI; t++)
        if (h.emirler[t].aktif) n++;
    return n;
}

static void EmirGerceklestiBildir(Oyun& o, const string& baslik, const Hisse& h, const IslemSonucu& s) {
    o.gerceklesenEmirSayisi++;
    o.toastlar.Ekle(baslik, h.kod + ": " + to_string(s.adet) + M(" adet @ ", " @ ") + Para(s.dolum), RENK_UYARI, 5.0f);
    Cal(o.sesler.emirGerceklesti);
    if (o.gerceklesenEmirSayisi >= 5) BasarimVer(o, B_EMIR);
    GorevOlay(o, G_EMIR);
}

// Her simulasyon adiminda cagrilir. Emirler PIYASA EMRI olarak gerceklesir:
// tetik fiyati garanti degildir (haberle bosluk yapan fiyatta dolum daha kotu olur).
static void EmirleriKontrolEt(Oyun& o) {
    if (!o.EmirlerAcik()) return;
    for (int i = 0; i < (int)o.hisseler.size(); i++) {
        if (!o.HisseAcikMi(i)) continue;
        Hisse& h = o.hisseler[i];

        if (h.adet != 0) {
            const bool uzun = h.adet > 0;
            Emir& st = h.emirler[EMIR_STOP];
            Emir& ka = h.emirler[EMIR_KAR_AL];
            bool stopTetik = st.aktif && (uzun ? h.fiyat <= st.fiyat : h.fiyat >= st.fiyat);
            bool karTetik = ka.aktif && (uzun ? h.fiyat >= ka.fiyat : h.fiyat <= ka.fiyat);
            if (stopTetik || karTetik) {
                st.aktif = false;
                ka.aktif = false;
                IslemSonucu s = IslemYap(o, i, !uzun, abs(h.adet), IslemKaynagi::EMIR);
                if (s.basarili)
                    EmirGerceklestiBildir(o, stopTetik ? M("ZARAR KES ÇALIŞTI", "STOP-LOSS FILLED")
                                                       : M("KÂR AL ÇALIŞTI", "TARGET FILLED"), h, s);
                continue;
            }
        }

        Emir& lm = h.emirler[EMIR_LIMIT_ALIM];
        if (lm.aktif && h.adet >= 0 && h.fiyat <= lm.fiyat) {
            lm.aktif = false;
            int maks = min(MaksAlinabilir(h, o.bakiye), h.azamiAdet - h.adet);
            int n = (lm.adet > 0) ? min(lm.adet, maks) : maks;
            if (n <= 0) {
                o.toastlar.Ekle(M("LİMİT EMRİ İPTAL", "LIMIT ORDER CANCELLED"),
                                h.kod + M(": bakiye yetersiz.", ": not enough cash."), RENK_UYARI, 4.0f);
                continue;
            }
            IslemSonucu s = IslemYap(o, i, true, n, IslemKaynagi::EMIR);
            if (s.basarili) EmirGerceklestiBildir(o, M("LİMİT ALIM ÇALIŞTI", "LIMIT BUY FILLED"), h, s);
        }
    }
}

// ---------------------------------------------------------------------------
// KREDI
// ---------------------------------------------------------------------------
// Istenen tutar limitten buyukse limite kirpilir. Gercekten alinan tutari doner.
static float KrediAl(Oyun& o, float tutar) {
    if (!o.KrediAcik()) return 0.0f;
    tutar = fminf(tutar, o.KalanKaldirac());
    if (tutar < 1.0f) {
        o.toastlar.Ekle(M("Kredi Limiti Dolu", "Credit Limit Reached"),
                        M("Daha fazla borç için net değerin yetmiyor.", "Your net worth cannot back more debt."),
                        RENK_KIRMIZI, 3.2f);
        Cal(o.sesler.hataUyari);
        return 0.0f;
    }
    o.borc += tutar;
    o.bakiye += tutar;
    o.enYuksekBorc = fmaxf(o.enYuksekBorc, o.borc);
    HaberBandinaEkle(o, M("KREDİ: ", "CREDIT: ") + Para(tutar, 0) + M(" borç aldın. Günlük faiz ", " borrowed. Daily interest ") +
                             Yuzde(KREDI_GUNLUK_FAIZ * 100.0f, 2) + ".");
    Cal(o.sesler.alim);
    return tutar;
}

// Istenen tutar nakitten/borctan buyukse kirpilir. Gercekten odenen tutari doner.
static float KrediOde(Oyun& o, float tutar) {
    tutar = fminf(tutar, fminf(o.bakiye, o.borc));
    if (tutar < 0.01f) {
        o.toastlar.Ekle(M("Ödeme Yapılamadı", "Cannot Repay"),
                        o.borc < 0.01f ? M("Borcun yok.", "You have no debt.")
                                       : M("Ödeyecek nakdin yok.", "You have no cash to repay with."),
                        RENK_KIRMIZI, 3.0f);
        Cal(o.sesler.hataUyari);
        return 0.0f;
    }
    o.borc -= tutar;
    o.bakiye -= tutar;
    if (o.borc < 0.01f) {
        o.borc = 0.0f;
        if (o.enYuksekBorc >= 10000.0f) BasarimVer(o, B_KALDIRAC);
        o.enYuksekBorc = 0.0f;
        HaberBandinaEkle(o, M("KREDİ: Borcunun tamamını kapattın.", "CREDIT: You have repaid your debt in full."));
    }
    Cal(o.sesler.satimKar);
    return tutar;
}

// Her yeni gunde: kredi faizi + aciga satilan hisselerin odunc ucreti.
// Nakit yetmezse kalan kisim borca eklenir (anapara buyur).
static void GunlukFinansman(Oyun& o) {
    float masraf = o.borc * KREDI_GUNLUK_FAIZ + o.AcigaDegeri() * ACIGA_GUNLUK_UCRET;
    if (masraf <= 0.0f) return;
    o.toplamFaiz += masraf;
    if (o.bakiye >= masraf) {
        o.bakiye -= masraf;
    } else {
        o.borc += masraf - o.bakiye;
        o.bakiye = 0.0f;
    }
}

// ---------------------------------------------------------------------------
// TEMINAT CAGRISI ve IFLAS
// ---------------------------------------------------------------------------
static void IflasEt(Oyun& o) {
    if (o.sonTuru == SON_IFLAS) return;
    o.sonTuru = SON_IFLAS;
    o.sonPenceresiAcik = true;
    o.bitisGunu = o.gun;
    o.bitisNetDeger = o.NetDeger();
    o.yeniRekor = false;
    HaberBandinaEkle(o, M("İFLAS: Sermayen tükendi.", "BANKRUPT: Your capital is gone."));
    Cal(o.sesler.alarm);
}

// Teminat orani esigin altina indi: araci kurum pozisyonlari ZORLA kapatir.
//   1) butun aciga satislar geri alinir,
//   2) nakitle borc odenir,
//   3) borc bitene kadar en buyuk pozisyondan baslayarak hisse satilir.
static void TeminatCagrisi(Oyun& o) {
    o.teminatCagrisiSayisi++;
    o.teminatUyarisiVerildi = false;

    for (int i = 0; i < (int)o.hisseler.size(); i++)
        if (o.hisseler[i].adet < 0) IslemYap(o, i, true, -o.hisseler[i].adet, IslemKaynagi::TEMINAT);

    auto borcOde = [&]() {
        float t = fminf(o.bakiye, o.borc);
        if (t > 0.0f) {
            o.bakiye -= t;
            o.borc -= t;
        }
        if (o.borc < 0.01f) o.borc = 0.0f;
    };
    borcOde();

    for (int guvenlik = 0; guvenlik < 64 && o.borc > 0.01f; guvenlik++) {
        int enBuyuk = -1;
        float enDeger = 0.0f;
        for (int i = 0; i < (int)o.hisseler.size(); i++) {
            const Hisse& h = o.hisseler[i];
            if (h.adet > 0 && h.fiyat * (float)h.adet > enDeger) {
                enDeger = h.fiyat * (float)h.adet;
                enBuyuk = i;
            }
        }
        if (enBuyuk < 0) break; // satacak bir sey kalmadi
        Hisse& h = o.hisseler[enBuyuk];
        int n = (int)ceilf(o.borc / (h.fiyat * 0.93f)); // dolum piyasanin altinda kalir; payli hesap
        n = max(1, min(n, h.adet));
        IslemYap(o, enBuyuk, false, n, IslemKaynagi::TEMINAT);
        borcOde();
    }

    // Zorla kapanan borc "Kaldirac Ustasi" basarimini saymaz.
    o.enYuksekBorc = 0.0f;

    o.toastlar.Ekle(M("TEMİNAT ÇAĞRISI!", "MARGIN CALL!"),
                    M("Teminatın yetmedi: pozisyonların zorla kapatıldı.",
                      "Your margin ran out: positions were force-closed."),
                    RENK_KIRMIZI, 8.0f);
    HaberBandinaEkle(o, M("TEMİNAT ÇAĞRISI: Aracı kurum açıklarını kapattı ve borcunu tahsil etmek için hisselerini sattı.",
                          "MARGIN CALL: Your broker closed your shorts and sold shares to collect your debt."));
    ParlamaBaslat(o, RENK_KIRMIZI);
    Cal(o.sesler.alarm);

    if (o.borc > 0.01f) {
        // Her sey satildi ama borc bitmedi: net deger eksi. Kalan borc silinir, oyun biter.
        o.borc = 0.0f;
        o.bakiye = 0.0f;
        IflasEt(o);
    }
}

// Her simulasyon adiminda cagrilir.
static void TeminatKontrolEt(Oyun& o) {
    if (o.sonTuru == SON_IFLAS) return;
    float yukumluluk = o.Yukumluluk();
    if (yukumluluk >= 1.0f) {
        float oran = o.NetDeger() / yukumluluk;
        if (oran < TEMINAT_CAGRI_ORANI) {
            TeminatCagrisi(o);
        } else if (oran < TEMINAT_UYARI_ORANI) {
            if (!o.teminatUyarisiVerildi) {
                o.teminatUyarisiVerildi = true;
                o.toastlar.Ekle(M("TEMİNAT UYARISI", "MARGIN WARNING"),
                                M("Teminat oranın ", "Your margin ratio is ") + Yuzde(oran * 100.0f, 0) + ". " +
                                    Yuzde(TEMINAT_CAGRI_ORANI * 100.0f, 0) +
                                    M(" altında pozisyonların zorla kapatılır.", " or lower forces your positions closed."),
                                RENK_UYARI, 7.0f);
                Cal(o.sesler.hataUyari);
            }
        } else if (oran > TEMINAT_UYARI_ORANI + 0.10f) {
            o.teminatUyarisiVerildi = false;
        }
    } else {
        o.teminatUyarisiVerildi = false;
    }

    // Kaldiracsiz da olsa sermaye tukenmisse oyun biter (rutbe hic dusmedigi icin
    // aksi halde oyuncu hicbir sey alamadan sonsuza dek takili kalirdi).
    if (o.sonTuru != SON_IFLAS && o.NetDeger() < IFLAS_ORANI * o.BaslangicSermayesi()) {
        if (o.Yukumluluk() >= 1.0f) TeminatCagrisi(o);
        if (o.sonTuru != SON_IFLAS && o.NetDeger() < IFLAS_ORANI * o.BaslangicSermayesi()) IflasEt(o);
    }
}
