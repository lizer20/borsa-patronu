// ============================================================================
// oyun.h - oyun DURUMU (veri yapilari). Mantik piyasa_*.h, cizim arayuz_*.h icinde.
// ============================================================================
// Bu yapilarda arayuze ait hicbir sey (buton, pencere bayragi) yoktur; boylece
// simulasyon pencere acmadan da calistirilabilir (bkz. test/denge_testi.cpp).

// ---- Bekleyen emirler (v2.0) ----
// Her hissede her turden en fazla BIR emir durur.
//   STOP       : zarar kes. Uzun pozisyonda fiyat seviyenin ALTINA inince tumunu
//                satar; acik pozisyonda fiyat seviyenin USTUNE cikinca tumunu kapatir.
//   KAR_AL     : hedef. STOP'un tersi yonde tetiklenir.
//   LIMIT_ALIM : fiyat seviyenin altina inince `adet` kadar alir (-1 = MAKS).
// Emirler piyasa emri olarak gerceklesir: tetik fiyati GARANTI degildir. Haberle
// fiyat bosluk yaparsa dolum, gercek piyasadaki gibi seviyenin otesinde olur.
enum EmirTuru { EMIR_STOP = 0, EMIR_KAR_AL = 1, EMIR_LIMIT_ALIM = 2, EMIR_TUR_SAYISI = 3 };

struct Emir {
    bool aktif = false;
    float fiyat = 0.0f;
    int adet = 0; // sadece LIMIT_ALIM icin; -1 = alinabildigi kadar
};

struct Hisse {
    string kod;
    IkiDil tamAd = {"", ""};
    Color renk = WHITE;
    Sektor sektor = Sektor::YOK;
    float beta = 1.0f;
    float fiyat = 0.0f, oncekiFiyat = 0.0f;
    AnimFloat gosterilenFiyat;
    deque<float> gecmis;

    // POZISYON. adet > 0: uzun (elde hisse var). adet < 0: ACIGA SATIS.
    //   uzun  : maliyet = bu hisselere odenen toplam tutar
    //   aciga : maliyet = aciga satarken ALINAN toplam tutar (pozitif tutulur)
    // Her iki durumda ortalama fiyat = maliyet / |adet|.
    int adet = 0;
    float maliyet = 0.0f;

    float flashSuresi = 0.0f; // >0 ise kart yanip sonuyor
    bool flashIyi = true;

    // --- Piyasa modeli ---
    float temelDeger = 0.0f;  // fiyatin etrafinda dolandigi "adil deger"
    float volatilite = 0.01f; // o anki beyaz gurultu oynakligi (sok sonrasi yukselir, soner)
    float ozelFaktor = 0.0f;  // hisseye ozgu kalici trend faktoru (AR(1))
    float akis = 0.0f;        // oyuncunun yakin zamandaki net emir akisi (bkz. temel.h, mikroyapi)

    // --- Devralma (takeover) / yonetim ---
    bool sahibiyim = false;
    float riskPrimi = 0.0f; // riskli yonetim kararlarindan biriken KALICI oynaklik
    float yonetimOlayZamanlayici = 0.0f;

    // --- Kademeli acilma ---
    // Bu hisse hangi rutbe seviyesinde acilir? 0 = oyuna baslarken acik.
    int acilmaSeviyesi = 0;

    // --- Emirler ve bilanco takvimi ---
    Emir emirler[EMIR_TUR_SAYISI];
    int bilancoGunu = 0;     // siradaki bilanconun aciklanacagi oyun gunu
    int bilancoBeklenti = 0; // oyuncuya gosterilen ipucu: +1 olumlu, 0 belirsiz, -1 olumsuz
};

// Duyurulmus ama henuz fiyata yansimamis haber
struct BeklemedeHaber {
    int hisseIndex;
    float carpan;
    string metin;
    float kalanSure;
};

// Duyurulmus ama henuz piyasaya yansimamis KURESEL olay
struct BeklemedeKuresel {
    float carpan;
    float riskPrimiDegisimi;
    string metin;
    float kalanSure;
    // Sektorel ek etki KAYIT DOSYASINA YAZILMAZ (kayit formatinin ortasindaki
    // tekrarli bloga alan eklemek eski kayitlari bozardi); yuklenen bekleyen bir
    // olay sektorel vurgusunu kaybeder, genel etkisi aynen uygulanir.
    Sektor vurgulananSektor = Sektor::YOK;
    float sektorCarpani = 1.0f;
};

enum SonTuru { SON_YOK = 0, SON_ZIRVE = 1, SON_SURE = 2, SON_IFLAS = 3 };

// Bilanco beklentisi uretir. acemiKoruma (0..1) yuksekse olumlu beklenti daha olasi.
//   acemi 1.0 -> olumlu %50, belirsiz %30, olumsuz %20
//   acemi 0.0 -> olumlu %30, belirsiz %30, olumsuz %40
static int BilancoBeklentiUret(float acemiKoruma) {
    float r = Rastgele01();
    float olumlu = 0.30f + 0.20f * acemiKoruma;
    if (r < olumlu) return 1;
    if (r < olumlu + 0.30f) return 0;
    return -1;
}

struct Oyun {
    bool duraklatildi = false;
    int senaryo = S_KLASIK;

    float bakiye = BASLANGIC_BAKIYE;
    float borc = 0.0f; // kredi borcu
    AnimFloat gosterilenNetDeger;
    AnimFloat gosterilenBakiye;

    vector<Hisse> hisseler;
    vector<BeklemedeHaber> bekleyenHaberler;
    vector<BeklemedeKuresel> bekleyenKureselHaberler;

    float piyasaSayaci = 0.0f;
    float haberSayaci = 0.0f;
    float kureselHaberSayaci = 0.0f;
    float gunSayaci = 0.0f;
    int gun = 1;
    float oyunSuresi = 0.0f; // simulasyonun calistigi toplam sure (sn)

    float piyasaFaktoru = 0.0f;    // tum hisselerin ortak faktoru ("piyasa modu")
    float kureselRiskPrimi = 0.0f; // kuresel olaylarin biriktirdigi KALICI oynaklik

    int rutbeIndex = 0;
    int hedefIndex = 0;
    int secilenAdet = 1; // -1 = MAKS

    deque<string> haberBandi;
    float bantKaydirma = 0.0f;
    float haberSabitKalmaSuresi = 0.0f; // >0 iken bant KAYMAZ (haber okunsun diye)

    ParcacikSistemi parcaciklar;
    ToastSistemi toastlar;
    SesEfektleri sesler;

    float flashAlpha = 0.0f;
    Color flashRenk = RENK_ALTIN;

    bool ilkIslemYapildi = false;

    // --- Istatistikler ---
    int islemSayisi = 0;
    float gerceklesenKar = 0.0f;
    float enIyiIslem = 0.0f;
    float enKotuIslem = 0.0f;
    float toplamTemettu = 0.0f;
    float toplamFaiz = 0.0f;
    int gerceklesenEmirSayisi = 0;
    int teminatCagrisiSayisi = 0;
    float enDusukNetDeger = BASLANGIC_BAKIYE; // "Kullerinden Dogan" basarimi icin
    float enYuksekBorc = 0.0f;                // "Kaldirac Ustasi" basarimi icin (borc kapaninca sifirlanir)

    int sonTemettuGunu = 0;
    bool normalizeGrafik = false; // grafik ekseni: false = TL, true = %

    // --- Iceriden bilgi (insider) mektubu ---
    float insiderSayaci = 0.0f;
    bool insiderMektupVar = false;
    float insiderMektupOmur = 0.0f;
    bool insiderOkundu = false;
    int insiderHisseIndex = -1;
    float insiderArtisOrani = 0.0f;
    float insiderKalanSure = 0.0f;

    // --- Yonetim karari (devraldigin sirkette) ---
    bool yonetimOlayiAcik = false;
    int yonetimHisseIndex = -1;
    int yonetimSablonIndex = -1;

    // --- Teminat / oyun sonu ---
    bool teminatUyarisiVerildi = false;
    int sonTuru = SON_YOK;          // oyun sonu penceresinin turu
    bool sonPenceresiAcik = false;  // pencere acikken simulasyon durur
    bool zirveyeUlasildi = false;   // son rutbeye bir kez ulasildi mi
    bool sureDoldu = false;         // sureli senaryoda sure bitti mi (sonrasi kum havuzu)
    int bitisGunu = 0;              // zirveye / sure sonuna hangi gun ulasildi
    float bitisNetDeger = 0.0f;
    bool yeniRekor = false;

    // --- Rehber (ilk oyun) ---
    int rehberAdim = -1;        // -1 = kapali
    float rehberBekleme = 0.0f; // adim degisince kutunun belirmesinden onceki kisa bekleme
    int rehberSatisSayisi = 0;  // rehberin "SAT'a bas" adimi icin

    const Senaryo& Sen() const { return SENARYOLAR[senaryo]; }
    float BaslangicSermayesi() const { return Sen().baslangicBakiye; }

    void Baslat(int senaryoId = S_KLASIK, bool rehberAcik = false) {
        if (senaryoId < 0 || senaryoId >= SENARYO_SAYISI) senaryoId = S_KLASIK;
        senaryo = senaryoId;
        const Senaryo& sen = Sen();

        hisseler.clear();
        for (auto& s : SIRKETLER) {
            Hisse h;
            h.kod = s.kod;
            h.tamAd = s.tamAd;
            h.renk = s.renk;
            h.sektor = s.sektor;
            h.beta = s.beta;
            h.fiyat = s.baslangicFiyat;
            h.oncekiFiyat = s.baslangicFiyat;
            h.temelDeger = s.baslangicFiyat;
            h.volatilite = fminf(VOL_TAVAN, BASE_VOLATILITE + sen.baslangicRiskPrimi);
            h.gosterilenFiyat.Zorla(s.baslangicFiyat);
            h.gosterilenFiyat.hiz = 6.0f;
            for (int i = 0; i < GECMIS_UZUNLUK; i++) h.gecmis.push_back(s.baslangicFiyat);
            hisseler.push_back(h);
        }

        // Acilma sirasi: baslangic fiyatina gore artan (0 = en ucuz, oyuna baslarken acik).
        {
            vector<int> sirali(hisseler.size());
            for (size_t i = 0; i < hisseler.size(); i++) sirali[i] = (int)i;
            sort(sirali.begin(), sirali.end(), [&](int a, int b) {
                return SIRKETLER[a].baslangicFiyat < SIRKETLER[b].baslangicFiyat;
            });
            for (size_t sira = 0; sira < sirali.size(); sira++)
                hisseler[sirali[sira]].acilmaSeviyesi = (int)sira;
        }

        // Ilk bilanco tarihleri: hisseler ust uste binmesin diye kaydirilmis.
        for (auto& h : hisseler) {
            h.bilancoGunu = 5 + h.acilmaSeviyesi * 2 + GetRandomValue(0, 3);
            h.bilancoBeklenti = BilancoBeklentiUret(sen.acemiKorumasi ? 1.0f : 0.0f);
        }

        duraklatildi = false;
        bakiye = sen.baslangicBakiye;
        borc = 0.0f;
        gosterilenNetDeger.Zorla(bakiye);
        gosterilenBakiye.Zorla(bakiye);
        bekleyenHaberler.clear();
        bekleyenKureselHaberler.clear();
        piyasaSayaci = haberSayaci = kureselHaberSayaci = gunSayaci = 0.0f;
        gun = 1;
        oyunSuresi = 0.0f;
        piyasaFaktoru = sen.baslangicPiyasaFaktoru;
        kureselRiskPrimi = sen.baslangicRiskPrimi;
        rutbeIndex = 0;
        hedefIndex = 0;
        secilenAdet = 1;
        haberBandi.clear();
        haberBandi.push_back(M("BORSA PATRONU'NA HOŞ GELDİN - Piyasalar açıldı, bol kazançlar!",
                               "WELCOME TO BORSA PATRONU - Markets are open, good luck!"));
        bantKaydirma = 0.0f;
        haberSabitKalmaSuresi = 0.0f;
        parcaciklar.liste.clear();
        toastlar.liste.clear();
        flashAlpha = 0.0f;
        ilkIslemYapildi = false;
        islemSayisi = 0;
        gerceklesenKar = enIyiIslem = enKotuIslem = 0.0f;
        toplamTemettu = toplamFaiz = 0.0f;
        gerceklesenEmirSayisi = 0;
        teminatCagrisiSayisi = 0;
        enDusukNetDeger = bakiye;
        enYuksekBorc = 0.0f;
        sonTemettuGunu = 0;
        insiderSayaci = 0.0f;
        insiderMektupVar = false;
        insiderMektupOmur = 0.0f;
        insiderOkundu = false;
        insiderHisseIndex = -1;
        insiderArtisOrani = 0.0f;
        insiderKalanSure = 0.0f;
        yonetimOlayiAcik = false;
        yonetimHisseIndex = -1;
        yonetimSablonIndex = -1;
        teminatUyarisiVerildi = false;
        sonTuru = SON_YOK;
        sonPenceresiAcik = false;
        zirveyeUlasildi = false;
        sureDoldu = false;
        bitisGunu = 0;
        bitisNetDeger = 0.0f;
        yeniRekor = false;
        rehberAdim = rehberAcik ? 0 : -1;
        rehberBekleme = 1.2f;
        rehberSatisSayisi = 0;
    }

    // Pozisyonlarin piyasa degeri. Aciga satislar NEGATIF katki yapar (borctur).
    float PortfoyDegeri() const {
        float toplam = 0.0f;
        for (auto& h : hisseler) toplam += h.fiyat * h.adet;
        return toplam;
    }

    // Sadece uzun pozisyonlarin degeri (portfoy ozeti icin).
    float UzunDeger() const {
        float toplam = 0.0f;
        for (auto& h : hisseler)
            if (h.adet > 0) toplam += h.fiyat * h.adet;
        return toplam;
    }

    // Aciga satilan hisselerin guncel degeri (pozitif; geri alma yukumlulugu).
    float AcigaDegeri() const {
        float toplam = 0.0f;
        for (auto& h : hisseler)
            if (h.adet < 0) toplam += h.fiyat * (float)(-h.adet);
        return toplam;
    }

    float NetDeger() const { return bakiye + PortfoyDegeri() - borc; }

    // Kaldiracli yukumluluk: borc + aciga satis degeri.
    float Yukumluluk() const { return borc + AcigaDegeri(); }

    // Teminat orani = net deger / yukumluluk. Yukumluluk yoksa cok buyuk bir sayi doner.
    float TeminatOrani() const {
        float y = Yukumluluk();
        return (y < 1.0f) ? 999.0f : NetDeger() / y;
    }

    // Yeni borc / aciga satis icin kalan kapasite (TL).
    float KalanKaldirac() const { return fmaxf(0.0f, NetDeger() * KALDIRAC_LIMITI - Yukumluluk()); }

    bool HisseAcikMi(int idx) const { return rutbeIndex >= hisseler[idx].acilmaSeviyesi; }
    bool EmirlerAcik() const { return rutbeIndex >= EMIR_MIN_RUTBE; }
    bool AcigaSatisAcik() const { return rutbeIndex >= ACIGA_MIN_RUTBE; }
    bool KrediAcik() const { return rutbeIndex >= KREDI_MIN_RUTBE; }

    int SahipOlunanSirketSayisi() const {
        int n = 0;
        for (auto& h : hisseler)
            if (h.sahibiyim) n++;
        return n;
    }
};
