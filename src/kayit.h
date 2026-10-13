// ============================================================================
// kayit.h - oyun kaydi (SAVE / LOAD)
// ============================================================================
// Basit, insan tarafindan da okunabilir, satir-satir bir metin formati.
//
// SURUM GECMISI: V1 (3 hisse) -> V2 (6 hisse + kuresel olaylar) -> V3 (kuresel
// olay sayaci + ses) -> V4 (istatistikler, temettu, grafik ekseni) -> V5 (senaryo,
// borc, emirler, bilanco takvimi, rehber, yeni istatistikler).
//
// KURAL: her yeni surum alanlarini dosyanin SONUNA ekler; ortadaki hicbir sey
// degismez. Bu sayede V2/V3/V4 kayitlari hala YUKLENEBILIR: ortak kisim ayni
// sirayla okunur, eksik alanlar varsayilaninda kalir, dosya bir dahaki kayitta
// kendiliginden V5'e yukselir. (V1 reddedilir - onda ortak kisim bile uyusmuyor.)
// Yeni alan eklerken ayni yontemi kullan: SONA ekle, yeni etiket tanimla, eskileri
// kabul edilenler arasinda birak.

static const char* KAYIT_ETIKETI_V5 = "BORSAPATRONU_SAVE_V5";
static const char* KAYIT_ETIKETI_V4 = "BORSAPATRONU_SAVE_V4";
static const char* KAYIT_ETIKETI_V3 = "BORSAPATRONU_SAVE_V3";
static const char* KAYIT_ETIKETI_V2 = "BORSAPATRONU_SAVE_V2";

static bool KayitVarMi() { return DepoVarMi(KAYIT_DOSYA_ADI); }

static string OyunuMetneYaz(const Oyun& o) {
    ostringstream f;
    f.precision(9);
    f << KAYIT_ETIKETI_V5 << "\n";

    f << o.bakiye << "\n";
    f << o.gun << "\n";
    f << o.gunSayaci << "\n";
    f << o.piyasaSayaci << "\n";
    f << o.haberSayaci << "\n";
    f << o.piyasaFaktoru << "\n";
    f << o.kureselRiskPrimi << "\n";
    f << o.rutbeIndex << "\n";
    f << o.hedefIndex << "\n";
    f << o.secilenAdet << "\n";
    f << (o.ilkIslemYapildi ? 1 : 0) << "\n";
    f << (o.duraklatildi ? 1 : 0) << "\n";

    f << o.hisseler.size() << "\n";
    for (auto& h : o.hisseler) {
        f << h.fiyat << "\n";
        f << h.oncekiFiyat << "\n";
        f << h.adet << "\n";
        f << h.maliyet << "\n";
        f << h.temelDeger << "\n";
        f << h.volatilite << "\n";
        f << (h.sahibiyim ? 1 : 0) << "\n";
        f << h.riskPrimi << "\n";
        f << h.yonetimOlayZamanlayici << "\n";
        f << h.gecmis.size() << "\n";
        for (float v : h.gecmis) f << v << " ";
        f << "\n";
    }

    f << o.bekleyenHaberler.size() << "\n";
    for (auto& bh : o.bekleyenHaberler) {
        f << bh.hisseIndex << "\n";
        f << bh.carpan << "\n";
        f << bh.kalanSure << "\n";
        f << bh.metin << "\n";
    }

    f << o.haberBandi.size() << "\n";
    for (auto& s : o.haberBandi) f << s << "\n";

    f << o.bekleyenKureselHaberler.size() << "\n";
    for (auto& bk : o.bekleyenKureselHaberler) {
        f << bk.carpan << "\n";
        f << bk.riskPrimiDegisimi << "\n";
        f << bk.kalanSure << "\n";
        f << bk.metin << "\n";
    }

    // ---- V3 ----
    f << o.kureselHaberSayaci << "\n";
    f << (sesAcik ? 1 : 0) << "\n";
    f << (IsAudioDeviceReady() ? GetMasterVolume() : 1.0f) << "\n";

    // ---- V4 ----
    f << o.islemSayisi << "\n";
    f << o.gerceklesenKar << "\n";
    f << o.enIyiIslem << "\n";
    f << o.enKotuIslem << "\n";
    f << o.sonTemettuGunu << "\n";
    f << (o.normalizeGrafik ? 1 : 0) << "\n";

    // ---- V5 ----
    f << o.senaryo << "\n";
    f << o.borc << "\n";
    f << o.oyunSuresi << "\n";
    f << o.toplamTemettu << "\n";
    f << o.toplamFaiz << "\n";
    f << o.gerceklesenEmirSayisi << "\n";
    f << o.teminatCagrisiSayisi << "\n";
    f << o.enDusukNetDeger << "\n";
    f << o.enYuksekBorc << "\n";
    f << (o.zirveyeUlasildi ? 1 : 0) << "\n";
    f << (o.sureDoldu ? 1 : 0) << "\n";
    f << o.bitisGunu << "\n";
    f << o.bitisNetDeger << "\n";
    f << o.rehberAdim << "\n";
    for (auto& h : o.hisseler) {
        f << h.ozelFaktor << "\n";
        f << h.akis << "\n";
        f << h.bilancoGunu << "\n";
        f << h.bilancoBeklenti << "\n";
        for (int t = 0; t < EMIR_TUR_SAYISI; t++) {
            f << (h.emirler[t].aktif ? 1 : 0) << "\n";
            f << h.emirler[t].fiyat << "\n";
            f << h.emirler[t].adet << "\n";
        }
    }
    return f.str();
}

static bool OyunuKaydet(const Oyun& o) { return DepoYaz(KAYIT_DOSYA_ADI, OyunuMetneYaz(o)); }

// Yukleme basarisiz olursa Oyun TAMAMEN degismeden kalir (bozuk/eski bir dosya
// oyunu yari bir durumda birakmasin diye once bir kopyaya okunur).
static bool OyunuMetindenYukle(Oyun& o, const string& icerik) {
    SatirOkuyucu r(icerik);
    string etiket = r.Str();
    int surum = 0;
    if (etiket == KAYIT_ETIKETI_V5) surum = 5;
    else if (etiket == KAYIT_ETIKETI_V4) surum = 4;
    else if (etiket == KAYIT_ETIKETI_V3) surum = 3;
    else if (etiket == KAYIT_ETIKETI_V2) surum = 2;
    else return false; // taninmayan / cok eski (V1)

    Oyun yeni = o; // ses efektleri gibi oturuma ait seyler korunur
    yeni.Baslat(S_KLASIK, false);
    const int sonRutbe = (int)RUTBELER.size() - 1;

    yeni.bakiye = r.Float();
    yeni.gun = max(1, r.Int());
    yeni.gunSayaci = r.Float();
    yeni.piyasaSayaci = r.Float();
    yeni.haberSayaci = r.Float();
    yeni.piyasaFaktoru = r.Float();
    yeni.kureselRiskPrimi = Clamp(r.Float(), KURESEL_RISK_PRIMI_TABANI, KURESEL_RISK_PRIMI_TAVANI);
    yeni.rutbeIndex = max(0, min(sonRutbe, r.Int()));
    yeni.hedefIndex = max(0, min(sonRutbe, r.Int()));
    yeni.secilenAdet = r.Int();
    yeni.ilkIslemYapildi = r.Bool();
    yeni.duraklatildi = r.Bool();
    if (yeni.secilenAdet != -1 && (yeni.secilenAdet < 1 || yeni.secilenAdet > TOPLAM_HISSE_SAYISI)) yeni.secilenAdet = 1;

    int hisseSayisi = r.Int();
    if (r.hata || hisseSayisi != (int)yeni.hisseler.size()) return false;
    for (auto& h : yeni.hisseler) {
        h.fiyat = fmaxf(0.5f, r.Float());
        h.oncekiFiyat = fmaxf(0.5f, r.Float());
        h.adet = max(-ACIGA_HISSE_SINIRI, min(TOPLAM_HISSE_SAYISI, r.Int()));
        h.maliyet = fmaxf(0.0f, r.Float());
        h.temelDeger = fmaxf(0.5f, r.Float());
        h.volatilite = Clamp(r.Float(), 0.0f, VOL_TAVAN);
        h.sahibiyim = r.Bool();
        h.riskPrimi = Clamp(r.Float(), RISK_PRIMI_TABANI, RISK_PRIMI_TAVANI);
        h.yonetimOlayZamanlayici = r.Float();
        r.Int(); // gecmis sayisi (satirin kendisinden okunuyor)
        string gecmisSatir = r.Str();
        h.gecmis.clear();
        istringstream iss(gecmisSatir);
        float v;
        while (iss >> v && (int)h.gecmis.size() < GECMIS_UZUNLUK) h.gecmis.push_back(fmaxf(0.5f, v));
        if (h.gecmis.empty()) h.gecmis.push_back(h.fiyat); // grafik hicbir zaman bos kalmasin
        h.gosterilenFiyat.Zorla(h.fiyat);
        h.flashSuresi = 0.0f;
        h.sahibiyim = (h.adet * 2 > TOPLAM_HISSE_SAYISI); // tutarlilik: sahiplik adetten turetilir
    }
    if (r.hata) return false;

    int bekleyenSayisi = r.Int();
    if (r.hata || bekleyenSayisi < 0 || bekleyenSayisi > 200) return false;
    yeni.bekleyenHaberler.clear();
    for (int i = 0; i < bekleyenSayisi; i++) {
        BeklemedeHaber bh;
        bh.hisseIndex = r.Int();
        bh.carpan = r.Float();
        bh.kalanSure = r.Float();
        bh.metin = r.Str();
        if (bh.hisseIndex >= 0 && bh.hisseIndex < hisseSayisi && bh.carpan > 0.1f && bh.carpan < 10.0f)
            yeni.bekleyenHaberler.push_back(bh);
    }

    int haberSayisi = r.Int();
    if (r.hata || haberSayisi < 0 || haberSayisi > 200) return false;
    yeni.haberBandi.clear();
    for (int i = 0; i < haberSayisi; i++) yeni.haberBandi.push_back(r.Str());

    int kureselSayisi = r.Int();
    if (r.hata || kureselSayisi < 0 || kureselSayisi > 200) return false;
    yeni.bekleyenKureselHaberler.clear();
    for (int i = 0; i < kureselSayisi; i++) {
        BeklemedeKuresel bk;
        bk.carpan = r.Float();
        bk.riskPrimiDegisimi = r.Float();
        bk.kalanSure = r.Float();
        bk.metin = r.Str();
        if (bk.carpan > 0.1f && bk.carpan < 10.0f) yeni.bekleyenKureselHaberler.push_back(bk);
    }
    if (r.hata) return false;

    // Ses ayarlari once yerel degiskenlere okunur; ancak yukleme TAMAMEN
    // basarili olunca uygulanir.
    bool yeniSesAcik = sesAcik;
    float yeniSesSeviyesi = -1.0f;
    if (surum >= 3) {
        yeni.kureselHaberSayaci = r.Float();
        yeniSesAcik = r.Bool();
        yeniSesSeviyesi = Clamp(r.Float(), 0.0f, 1.0f);
    }
    if (surum >= 4) {
        yeni.islemSayisi = max(0, r.Int());
        yeni.gerceklesenKar = r.Float();
        yeni.enIyiIslem = r.Float();
        yeni.enKotuIslem = r.Float();
        yeni.sonTemettuGunu = r.Int();
        yeni.normalizeGrafik = r.Bool();
    }
    if (surum >= 5) {
        int sen = r.Int();
        yeni.senaryo = (sen >= 0 && sen < SENARYO_SAYISI) ? sen : S_KLASIK;
        yeni.borc = fmaxf(0.0f, r.Float());
        yeni.oyunSuresi = fmaxf(0.0f, r.Float());
        yeni.toplamTemettu = fmaxf(0.0f, r.Float());
        yeni.toplamFaiz = fmaxf(0.0f, r.Float());
        yeni.gerceklesenEmirSayisi = max(0, r.Int());
        yeni.teminatCagrisiSayisi = max(0, r.Int());
        yeni.enDusukNetDeger = r.Float();
        yeni.enYuksekBorc = fmaxf(0.0f, r.Float());
        yeni.zirveyeUlasildi = r.Bool();
        yeni.sureDoldu = r.Bool();
        yeni.bitisGunu = max(0, r.Int());
        yeni.bitisNetDeger = r.Float();
        yeni.rehberAdim = r.Int();
        if (yeni.rehberAdim < -1 || yeni.rehberAdim >= (int)REHBER_ADIMLARI.size()) yeni.rehberAdim = -1;
        for (auto& h : yeni.hisseler) {
            h.ozelFaktor = Clamp(r.Float(), -0.05f, 0.05f);
            h.akis = Clamp(r.Float(), -2000.0f, 2000.0f);
            h.bilancoGunu = r.Int();
            h.bilancoBeklenti = max(-1, min(1, r.Int()));
            for (int t = 0; t < EMIR_TUR_SAYISI; t++) {
                h.emirler[t].aktif = r.Bool();
                h.emirler[t].fiyat = fmaxf(0.5f, r.Float());
                h.emirler[t].adet = r.Int();
            }
        }
    } else {
        // V2-V4 kayitlari: yeni alanlar icin makul varsayilanlar.
        yeni.senaryo = S_KLASIK;
        for (auto& h : yeni.hisseler) {
            if (h.adet < 0) { h.adet = 0; h.maliyet = 0.0f; } // eski surumde aciga satis yoktu
            h.bilancoGunu = yeni.gun + 3 + h.acilmaSeviyesi * 2;
        }
        yeni.zirveyeUlasildi = (yeni.rutbeIndex >= sonRutbe); // zirve penceresi bir daha acilmasin
        yeni.enDusukNetDeger = fminf(yeni.NetDeger(), BASLANGIC_BAKIYE);
        yeni.rehberAdim = -1;
    }
    if (r.hata) return false;

    // Gecici/gorsel durumlar yuklenmez, temiz baslar. Acik bir yonetim karari
    // penceresi kaydedilmez: zamanlayici zaten <=0 oldugu icin bir sonraki
    // simulasyon adiminda kendiliginden yeniden acilir.
    yeni.bantKaydirma = 0.0f;
    yeni.haberSabitKalmaSuresi = 0.0f;
    yeni.yonetimOlayiAcik = false;
    yeni.yonetimHisseIndex = -1;
    yeni.yonetimSablonIndex = -1;
    yeni.sonTuru = SON_YOK;
    yeni.sonPenceresiAcik = false;
    yeni.gosterilenNetDeger.Zorla(yeni.NetDeger());
    yeni.gosterilenBakiye.Zorla(yeni.bakiye);

    o = yeni;
    sesAcik = yeniSesAcik;
    if (yeniSesSeviyesi >= 0.0f && IsAudioDeviceReady()) SetMasterVolume(yeniSesSeviyesi);
    return true;
}

static bool OyunuYukle(Oyun& o) {
    string icerik;
    if (!DepoOku(KAYIT_DOSYA_ADI, icerik)) return false;
    return OyunuMetindenYukle(o, icerik);
}
