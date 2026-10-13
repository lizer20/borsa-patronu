// ============================================================================
// piyasa_sim.h - gun dongusu, temettu, yonetim kararlari, rehber ve
//                SimulasyonAdimi(): oyunun pencereden BAGIMSIZ kalp atisi
// ============================================================================
// SimulasyonAdimi() hicbir raylib pencere/cizim fonksiyonu cagirmaz; bu yuzden
// denge testi (test/denge_testi.cpp) pencere acmadan saniyede binlerce oyun gunu
// isletebilir. Arayuz sadece "su an calissin mi" kararini verir.

// Devraldigin sirkette acilan yonetim kararini uygular.
static void YonetimKarariUygula(Oyun& o, bool riskli) {
    if (!o.yonetimOlayiAcik) return;
    if (o.yonetimHisseIndex < 0 || o.yonetimHisseIndex >= (int)o.hisseler.size() ||
        o.yonetimSablonIndex < 0 || o.yonetimSablonIndex >= (int)YONETIM_OLAYLARI.size()) {
        o.yonetimOlayiAcik = false;
        return;
    }
    Hisse& hy = o.hisseler[o.yonetimHisseIndex];
    const YonetimOlaySablonu& sablon = YONETIM_OLAYLARI[o.yonetimSablonIndex];
    const YonetimSecenek& secim = riskli ? sablon.riskli : sablon.guvenli;

    hy.fiyat = fmaxf(0.5f, hy.fiyat * secim.temelDegerCarpani);
    hy.temelDeger = fmaxf(0.5f, hy.temelDeger * secim.temelDegerCarpani);
    hy.gosterilenFiyat.Ayarla(hy.fiyat);
    hy.flashSuresi = 0.6f;
    hy.flashIyi = (secim.temelDegerCarpani >= 1.0f);
    hy.riskPrimi = Clamp(hy.riskPrimi + secim.riskPrimiDegisimi, RISK_PRIMI_TABANI, RISK_PRIMI_TAVANI);
    HaberBandinaEkle(o, hy.kod + M(" YÖNETİM KARARI: şirket ", " MANAGEMENT DECISION: the company ") + secim.logMetni.al());

    o.yonetimOlayiAcik = false;
    o.yonetimHisseIndex = -1;
    o.yonetimSablonIndex = -1;
    hy.yonetimOlayZamanlayici = RastgeleAralik(YONETIM_OLAY_MIN_SN, YONETIM_OLAY_MAX_SN);
    RutbeVeHedefKontrolEt(o);
}

// Her TEMETTU_PERIYOT_GUN gunde bir: uzun pozisyonlar temettu ALIR, aciga
// satilan hisseler icin temettu ODENIR (gercek piyasadaki gibi).
static void TemettuOde(Oyun& o) {
    o.sonTemettuGunu = o.gun;
    float gelen = 0.0f, giden = 0.0f;
    for (auto& h : o.hisseler) {
        if (h.adet > 0) gelen += h.fiyat * (float)h.adet * TEMETTU_ORANI;
        else if (h.adet < 0) giden += h.fiyat * (float)(-h.adet) * TEMETTU_ORANI;
    }
    float net = gelen - giden;
    if (gelen < 0.01f && giden < 0.01f) return;

    o.bakiye += net;
    if (o.bakiye < 0.0f) {
        o.borc += -o.bakiye;
        o.bakiye = 0.0f;
    }
    o.toplamTemettu += gelen;
    if (net >= 0.0f) {
        o.toastlar.Ekle(M("TEMETTÜ ÖDENDİ", "DIVIDEND PAID"),
                        ParaIsaretli(net) + M(" portföyünden", " from your portfolio"), RENK_YESIL, 4.0f);
        HaberBandinaEkle(o, M("TEMETTÜ: Portföyünden ", "DIVIDEND: You received ") + Para(net) +
                                 M(" temettü geldi.", " from your portfolio."));
        Cal(o.sesler.satimKar);
    } else {
        o.toastlar.Ekle(M("TEMETTÜ YÜKÜ", "DIVIDEND OWED"),
                        M("Açığa sattığın hisseler için ", "You paid ") + Para(-net) +
                            M(" ödedin.", " on shares you are short."),
                        RENK_UYARI, 4.0f);
        HaberBandinaEkle(o, M("TEMETTÜ: Açığa sattığın hisselerin temettüsü için ", "DIVIDEND: You paid ") + Para(-net) +
                                 M(" ödedin.", " in dividends on your short positions."));
    }
    if (o.toplamTemettu >= 5000.0f) BasarimVer(o, B_TEMETTU);
}

// Oyun gunu ilerledi.
static void YeniGun(Oyun& o) {
    o.gun++;
    // Sirketlerin adil degeri de -cok yavas- kendi yolunda ilerler.
    for (auto& h : o.hisseler) {
        float carpan = 1.0f + RastgeleGauss() * TEMEL_DEGER_GUNLUK_OYNAKLIK;
        h.temelDeger = fmaxf(0.5f, h.temelDeger * carpan);
    }
    GunlukFinansman(o);
    if (o.gun - o.sonTemettuGunu >= TEMETTU_PERIYOT_GUN) TemettuOde(o);
    BilancoKontrolEt(o);

    // Sureli senaryo: sure doldu -> skor = net deger. Sonrasinda oyuncu isterse
    // kum havuzu olarak devam eder (skor bir daha degismez).
    int sinir = o.Sen().gunSiniri;
    if (sinir > 0 && !o.sureDoldu && o.gun > sinir) {
        o.sureDoldu = true;
        o.sonTuru = SON_SURE;
        o.sonPenceresiAcik = true;
        o.bitisGunu = sinir;
        o.bitisNetDeger = o.NetDeger();
        o.yeniRekor = RekorNetGuncelle(o.senaryo, o.bitisNetDeger);
        HaberBandinaEkle(o, M("SÜRE DOLDU: Skorun ", "TIME IS UP: Your score is ") + Para(o.bitisNetDeger) + ".");
        Cal(o.sesler.rutbeAtlama);
    }
}

// ---------------------------------------------------------------------------
// REHBER
// ---------------------------------------------------------------------------
static void RehberiBitir(Oyun& o) {
    o.rehberAdim = -1;
    if (!g_profil.rehberGoruldu) {
        g_profil.rehberGoruldu = true;
        ProfilKaydet();
    }
}

static void RehberIlerle(Oyun& o) {
    if (o.rehberAdim < 0) return;
    o.rehberAdim++;
    o.rehberBekleme = 0.6f; // yeni kutu belirmeden once kisa bir nefes
    o.rehberSatisSayisi = 0;
    if (o.rehberAdim >= (int)REHBER_ADIMLARI.size()) RehberiBitir(o);
}

// Bazi adimlar oyuncunun ISTENEN SEYI YAPMASIYLA kendiliginden gecer; digerleri
// "Anladim" dugmesini bekler (arayuz RehberIlerle cagirir).
static void RehberGuncelle(Oyun& o, float dt) {
    if (o.rehberAdim < 0) return;
    if (o.rehberBekleme > 0.0f) o.rehberBekleme -= dt;
    RehberHedef hedef = REHBER_ADIMLARI[o.rehberAdim].hedef;
    if (hedef == RehberHedef::AL_BUTONU && o.islemSayisi >= 1) RehberIlerle(o);
    else if (hedef == RehberHedef::KART && o.rehberSatisSayisi >= 1) RehberIlerle(o);
}

// ---------------------------------------------------------------------------
// SIMULASYON ADIMI
// ---------------------------------------------------------------------------
// Bir yonetim karari ya da oyun sonu penceresi beklerken zaman ilerlemez
// (oyuncu/bot once karar vermeli).
static void SimulasyonAdimi(Oyun& o, float dt) {
    if (o.yonetimOlayiAcik || o.sonPenceresiAcik) return;

    o.oyunSuresi += dt;
    o.piyasaSayaci += dt;
    o.haberSayaci += dt;
    o.kureselHaberSayaci += dt;
    o.gunSayaci += dt;

    if (o.gunSayaci >= GUN_SURESI_SN) {
        o.gunSayaci -= GUN_SURESI_SN;
        YeniGun(o);
        if (o.sonPenceresiAcik) return;
    }
    if (o.piyasaSayaci >= PIYASA_TICK_SN) {
        o.piyasaSayaci -= PIYASA_TICK_SN;
        PiyasaGuncelle(o);
    }
    if (o.haberSayaci >= HABER_TICK_SN) {
        o.haberSayaci -= HABER_TICK_SN;
        HaberKontrolEt(o);
    }
    BekleyenHaberleriGuncelle(o, dt);
    if (o.kureselHaberSayaci >= KURESEL_OLAY_TICK_SN) {
        o.kureselHaberSayaci -= KURESEL_OLAY_TICK_SN;
        KureselHaberKontrolEt(o);
    }
    BekleyenKureselHaberleriGuncelle(o, dt);

    // ---- Iceriden bilgi mektubu ----
    // Sadece ust rutbelerde uretilir; bir kez ortaya ciktiysa sayaclari isler.
    if (o.rutbeIndex >= INSIDER_MIN_RUTBE && !o.insiderMektupVar && !o.insiderOkundu) {
        o.insiderSayaci += dt;
        if (o.insiderSayaci >= INSIDER_TICK_SN) {
            o.insiderSayaci -= INSIDER_TICK_SN;
            InsiderMektupKontrolEt(o);
        }
    }
    if (o.insiderMektupVar) {
        o.insiderMektupOmur -= dt;
        o.insiderKalanSure -= dt;
        if (o.insiderMektupOmur <= 0.0f) {
            // Zarf okunmadan kayboldu - yukselis yine de gerceklesecek.
            o.insiderMektupVar = false;
            o.insiderHisseIndex = -1;
        }
    }
    if (o.insiderOkundu) {
        o.insiderKalanSure -= dt;
        if (o.insiderKalanSure <= 0.0f) {
            o.insiderOkundu = false;
            o.insiderHisseIndex = -1;
        }
    }

    // ---- Bekleyen emirler, teminat ----
    EmirleriKontrolEt(o);
    TeminatKontrolEt(o);
    RutbeVeHedefKontrolEt(o);
    if (o.sonPenceresiAcik) return;

    // ---- Yonetim kararlari: cogunluk sahibi oldugun sirketlerde periyodik ----
    for (int i = 0; i < (int)o.hisseler.size(); i++) {
        Hisse& h = o.hisseler[i];
        if (!h.sahibiyim) continue;
        h.yonetimOlayZamanlayici -= dt;
        if (h.yonetimOlayZamanlayici <= 0.0f) {
            o.yonetimOlayiAcik = true;
            o.yonetimHisseIndex = i;
            o.yonetimSablonIndex = GetRandomValue(0, (int)YONETIM_OLAYLARI.size() - 1);
            break;
        }
    }

    // ---- Haber bandi kaymasi ----
    if (o.haberSabitKalmaSuresi > 0.0f) o.haberSabitKalmaSuresi -= dt; // SON DAKIKA haberi sabit, okunsun
    else o.bantKaydirma += dt * 70.0f;

    RehberGuncelle(o, dt);
}

// Sadece GORSEL durum (animasyonlu sayaclar, parcaciklar, toastlar). Simulasyon
// duraklatilmis olsa da her karede calisir; boylece duraklatma menusunden gelen
// "Oyun Kaydedildi" gibi bildirimler de ekrana kayarak girer.
static void GorselGuncelle(Oyun& o, float dt) {
    for (auto& h : o.hisseler) {
        h.gosterilenFiyat.Guncelle(dt);
        if (h.flashSuresi > 0.0f) h.flashSuresi -= dt;
    }
    o.gosterilenNetDeger.Ayarla(o.NetDeger());
    o.gosterilenNetDeger.Guncelle(dt);
    o.gosterilenBakiye.Ayarla(o.bakiye);
    o.gosterilenBakiye.Guncelle(dt);
    o.parcaciklar.Guncelle(dt);
    o.toastlar.Guncelle(dt);
    if (o.flashAlpha > 0.0f) o.flashAlpha = fmaxf(0.0f, o.flashAlpha - dt * 0.6f);
}
