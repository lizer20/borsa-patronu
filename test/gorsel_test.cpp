// ============================================================================
// gorsel_test.cpp - ekran goruntusu ureten GORSEL test + YERLESIM DENETCISI
// ============================================================================
// Oyunu gizli bir pencerede acar, durumu elle kurup (sahte portfoy, rutbe,
// acik pencereler) her sahneyi IKI DILDE cizer ve:
//   1) test/cikti/ altina PNG olarak kaydeder (gozle bakmak icin),
//   2) cizilen her yazi/dugme/panelin kutusunu karsilastirip
//        - ust uste binen yazilari,
//        - bir dugmenin ustune binen yazilari,
//        - panelinden ya da ekrandan tasan yazilari,
//        - dugmesine sigmayan etiketleri
//      test/cikti/yerlesim_raporu.txt dosyasina yazar.
//
// Calistirma:  python build.py test
//
// Yeni bir ekran/pencere eklediginde buraya bir sahne ekle; cakismalar elle
// oynamadan, iki dilde birden yakalanir. (v1.0'da elle bulunan uc cakismanin
// ucunu de bu denetci kendiliginden yakalar.)

#define BORSA_TEST
#define BORSA_ANA_YOK
#include "../borsa_patronu.cpp"
#include <cstdarg>

static FILE* g_rapor = nullptr;
static int g_sorunSayisi = 0;
static int g_sahneSayisi = 0;
static const char* g_dilEki = "tr";

static bool g_sessiz = false; // denetcinin kendini sinamasi sirasinda cikti basma
static void Yaz(const char* bicim, ...) {
    if (g_sessiz) return;
    char tampon[2048];
    va_list a;
    va_start(a, bicim);
    vsnprintf(tampon, sizeof(tampon), bicim, a);
    va_end(a);
    fputs(tampon, stdout);
    if (g_rapor) fputs(tampon, g_rapor);
}

static bool Kesisiyor(Rectangle a, Rectangle b, float pay) {
    float x0 = fmaxf(a.x, b.x), x1 = fminf(a.x + a.width, b.x + b.width);
    float y0 = fmaxf(a.y, b.y), y1 = fminf(a.y + a.height, b.y + b.height);
    return (x1 - x0) > pay && (y1 - y0) > pay;
}
static bool Icinde(Rectangle ic, Rectangle dis, float pay) {
    return ic.x >= dis.x - pay && ic.y >= dis.y - pay && ic.x + ic.width <= dis.x + dis.width + pay &&
           ic.y + ic.height <= dis.y + dis.height + pay;
}

static void YerlesimDenetle(const char* sahne) {
    const vector<TestKutu>& k = g_testKutular;
    int once = g_sorunSayisi;
    auto sorun = [&](const char* tur, const TestKutu& a, const TestKutu* b) {
        g_sorunSayisi++;
        if (b) Yaz("  [%s/%s] %s: \"%s\" (%.0f,%.0f %.0fx%.0f)  <->  \"%s\" (%.0f,%.0f %.0fx%.0f)\n", g_dilEki, sahne, tur,
                   a.metin.c_str(), a.r.x, a.r.y, a.r.width, a.r.height, b->metin.c_str(), b->r.x, b->r.y, b->r.width, b->r.height);
        else Yaz("  [%s/%s] %s: \"%s\" (%.0f,%.0f %.0fx%.0f)\n", g_dilEki, sahne, tur, a.metin.c_str(), a.r.x, a.r.y, a.r.width, a.r.height);
    };
    const Rectangle ekran = {0, 0, (float)PENCERE_GENISLIK, (float)PENCERE_YUKSEKLIK};
    for (size_t i = 0; i < k.size(); i++) {
        const TestKutu& a = k[i];
        if (a.tur == 3) { sorun("ETIKET DUGMEYE SIGMIYOR", a, nullptr); continue; }
        if (a.tur == 2) continue;
        if (!Icinde(a.r, ekran, 1.0f)) sorun("EKRANDAN TASIYOR", a, nullptr);
        for (size_t j = 0; j < k.size(); j++) {
            const TestKutu& b = k[j];
            if (i == j || a.katman != b.katman || b.tur == 3) continue;
            if (b.tur == 2) { // panel: kesisiyorsa tamamen icinde olmali
                if (Kesisiyor(a.r, b.r, 2.0f) && !Icinde(a.r, b.r, 1.0f) && !Icinde(b.r, a.r, 1.0f)) sorun("PANELDEN TASIYOR", a, &b);
                continue;
            }
            if (j < i) continue; // her cifti bir kez
            if (!Kesisiyor(a.r, b.r, 1.5f)) continue;
            if (a.tur == 0 && b.tur == 0) sorun("YAZILAR UST USTE", a, &b);
            else if (a.tur != b.tur) sorun("YAZI DUGMENIN USTUNDE", a, &b);
            else sorun("DUGMELER UST USTE", a, &b);
        }
    }
    if (g_sorunSayisi == once) return;
}

// Denetcinin kendisi calisiyor mu? Bilerek hatali kutular verip HEPSINI
// yakaladigini dogrular; yoksa "0 sorun" sonucu hicbir sey kanitlamaz.
static bool DenetciKendiniSinar() {
    int eskiSorun = g_sorunSayisi;
    g_testKutular.clear();
    auto ekle = [](Rectangle r, const char* m, int katman, int tur) { g_testKutular.push_back({r, m, katman, tur}); };
    ekle({100, 100, 200, 100}, "panel", 0, 2);
    ekle({110, 110, 80, 12}, "icerde", 0, 0);            // sorun yok
    ekle({250, 150, 90, 12}, "panelden tasan", 0, 0);    // 1: panelden tasiyor
    ekle({400, 400, 100, 12}, "yazi A", 0, 0);
    ekle({450, 404, 100, 12}, "yazi B", 0, 0);           // 2: yazilar ust uste
    ekle({600, 400, 80, 30}, "dugme", 0, 1);
    ekle({640, 410, 100, 12}, "dugmeye binen", 0, 0);    // 3: yazi dugmenin ustunde
    ekle({1250, 300, 80, 12}, "ekran disi", 0, 0);       // 4: ekrandan tasiyor
    ekle({700, 500, 60, 30}, "cok uzun etiket", 0, 3);   // 5: etiket sigmiyor
    ekle({400, 400, 100, 12}, "baska katman", 1, 0);     // sorun yok (farkli katman)
    g_sessiz = true;
    YerlesimDenetle("kendini-sinama");
    g_sessiz = false;
    int bulunan = g_sorunSayisi - eskiSorun;
    g_sorunSayisi = eskiSorun;
    g_testKutular.clear();
    return bulunan == 5;
}

// Sahneyi cizer, PNG kaydeder, yerlesimi denetler.
static void Kaydet(Uygulama& u, const char* ad) {
    g_testKayit = true;
    KareCiz(u);
    g_testKayit = false;
    Image img = LoadImageFromTexture(u.sahne.texture);
    ImageFlipVertical(&img);
    ExportImage(img, TextFormat("test/cikti/%s_%s.png", g_dilEki, ad));
    UnloadImage(img);
    YerlesimDenetle(ad);
    g_sahneSayisi++;
}

static void Kareler(Uygulama& u, int n, float dt = 1.0f / 60.0f) {
    for (int i = 0; i < n; i++) KareGuncelle(u, dt, Girdi());
}
static void Tikla(Uygulama& u, Rectangle r) {
    Girdi g;
    g.fare = {r.x + r.width / 2.0f, r.y + r.height / 2.0f};
    g.tiklandi = true;
    KareGuncelle(u, 1.0f / 60.0f, g);
    Kareler(u, 2);
}

// Durumu sifirlar: belirli rutbede, temiz bir oyun; bildirimler temizlenmis.
static void OyunKur(Uygulama& u, int senaryo, int rutbe, unsigned int tohum) {
    SetRandomSeed(tohum);
    u.ui = Arayuz();
    ArayuzKur(u.ui, (int)SIRKETLER.size());
    u.oyun.Baslat(senaryo, false);
    u.oyun.rutbeIndex = rutbe;
    u.oyun.hedefIndex = rutbe;
    u.oyunVar = true;
    u.ekran = Ekran::OYUN;
    if (rutbe > 0) u.oyun.bakiye = RUTBELER[rutbe].esikNetDeger * 1.05f;
    u.oyun.enDusukNetDeger = u.oyun.bakiye;
    Kareler(u, 60 * 40);                 // 40 saniye: grafik dolsun
    u.oyun.toastlar.liste.clear();
    u.oyun.yonetimOlayiAcik = false;
}

static int SiradakiHisse(const Oyun& o, int sira) { // acilma sirasina gore
    for (int i = 0; i < (int)o.hisseler.size(); i++)
        if (o.hisseler[i].acilmaSeviyesi == sira) return i;
    return 0;
}

static void PozisyonVer(Oyun& o, int idx, int adet, float ortOran) {
    Hisse& h = o.hisseler[idx];
    h.adet = adet;
    h.maliyet = fabsf((float)adet) * h.fiyat * ortOran;
    h.sahibiyim = (adet * 2 > TOPLAM_HISSE_SAYISI);
    h.yonetimOlayZamanlayici = 9999.0f;
}

static void Sahneler(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;

    // ---- menuler ----
    u.ui = Arayuz();
    ArayuzKur(u.ui, (int)SIRKETLER.size());
    u.ekran = Ekran::MENU;
    u.oyunVar = false;
    Kareler(u, 3);
    Kaydet(u, "01_menu");
    u.ui.menuMesaj = M("Kayıt okunamadı (bozuk ya da çok eski olabilir).", "Could not read the save (it may be corrupt or too old).");
    u.ui.menuMesajSure = 99.0f;
    u.oyunVar = true;
    Kaydet(u, "02_menu_devam");
    u.ui.menuMesaj.clear();
    u.ui.cikisOnayiAcik = true;
    u.oyun.ilkIslemYapildi = true;
    Kaydet(u, "03_cikis_onayi");
    u.ui.cikisOnayiAcik = false;

    u.ekran = Ekran::SENARYO;
    g_profil.rekorGun[S_KLASIK] = 87;
    g_profil.rekorNet[S_ZAMAN] = 182340.0f;
    u.ui.senaryoOnay = 1;
    u.ui.senaryoOnaySuresi = 99.0f;
    Kaydet(u, "04_senaryo");
    u.ekran = Ekran::BASARIM;
    for (int i = 0; i < BASARIM_SAYISI; i += 2) g_profil.basarim[i] = true;
    Kaydet(u, "05_basarim");

    // ---- rehber: 6 adim ----
    OyunKur(u, S_KLASIK, 0, 11);
    for (int adim = 0; adim < (int)REHBER_ADIMLARI.size(); adim++) {
        o.rehberAdim = adim;
        o.rehberBekleme = 0.0f;
        Kaydet(u, TextFormat("06_rehber_%d", adim + 1));
    }
    o.rehberAdim = -1;

    // ---- 1 hisse, pozisyon + baski + bildirimler ----
    OyunKur(u, S_KLASIK, 0, 12);
    {
        int i0 = SiradakiHisse(o, 0);
        PozisyonVer(o, i0, 180, 0.93f);
        o.hisseler[i0].akis = 160.0f;
        o.toastlar.Ekle(M("KÜRESEL GELİŞME!", "GLOBAL NEWS!"), KURESEL_OLAYLARI[2].metin.al(), RENK_UYARI, 99.0f);
        o.toastlar.Ekle(M("BAŞARIM AÇILDI!", "ACHIEVEMENT UNLOCKED!"), BASARIMLAR[B_KAR_1K].ad.al(), RENK_ALTIN, 99.0f);
        Kareler(u, 30);
        Kaydet(u, "07_oyun_1hisse");
        o.toastlar.liste.clear();
    }

    // ---- 3 hisse: emirler, aciga satis, sureli senaryo ust bari ----
    OyunKur(u, S_ZAMAN, 2, 13);
    {
        int i0 = SiradakiHisse(o, 0), i1 = SiradakiHisse(o, 1), i2 = SiradakiHisse(o, 2);
        PozisyonVer(o, i0, 320, 1.08f);
        PozisyonVer(o, i1, -150, 1.04f);
        PozisyonVer(o, i2, 40, 0.90f);
        EmirKoy(o, i0, EMIR_STOP, o.hisseler[i0].fiyat * 0.95f, 0);
        EmirKoy(o, i0, EMIR_KAR_AL, o.hisseler[i0].fiyat * 1.10f, 0);
        EmirKoy(o, i0, EMIR_LIMIT_ALIM, o.hisseler[i0].fiyat * 0.97f, 25);
        EmirKoy(o, i1, EMIR_STOP, o.hisseler[i1].fiyat * 1.05f, 0);
        o.hisseler[i1].akis = -120.0f;
        o.secilenAdet = 25;
        Kareler(u, 2);
        Kaydet(u, "08_oyun_3hisse");
        a.emirHisse = i0;
        Kaydet(u, "09_emir_uzun");
        a.emirHisse = i1;
        Kaydet(u, "10_emir_aciga");
        o.secilenAdet = -1;
        o.hisseler[i2].adet = 0;
        a.emirHisse = i2;
        Kaydet(u, "11_emir_pozisyonsuz");
        a.emirHisse = -1;
        o.duraklatildi = true;
        Kaydet(u, "12_duraklatma");
        a.ayarlarAcik = true;
        Kaydet(u, "13_ayarlar");
        a.ayarlarAcik = false;
        o.duraklatildi = false;
    }

    // ---- 6 hisse: kompakt kartlar, buyuk rakamlar, kredi, zarf, yuzde ekseni ----
    OyunKur(u, S_KLASIK, 5, 14);
    {
        for (int s = 0; s < 6; s++) PozisyonVer(o, SiradakiHisse(o, s), (s % 2 == 0) ? 1000 : 640, 0.71f + 0.07f * s);
        PozisyonVer(o, SiradakiHisse(o, 3), -500, 1.12f);
        int i5 = SiradakiHisse(o, 5);
        EmirKoy(o, i5, EMIR_STOP, o.hisseler[i5].fiyat * 0.9f, 0);
        EmirKoy(o, i5, EMIR_KAR_AL, o.hisseler[i5].fiyat * 1.2f, 0);
        o.hisseler[SiradakiHisse(o, 1)].akis = 900.0f;
        o.bakiye = 1234567.89f;
        o.borc = 480000.0f;
        o.gerceklesenKar = -1234567.0f;
        o.islemSayisi = 12345;
        o.enIyiIslem = 987654.0f;
        o.enKotuIslem = -456789.0f;
        o.insiderMektupVar = true;
        o.insiderMektupOmur = 8.0f;
        o.insiderHisseIndex = i5;
        o.insiderArtisOrani = 0.187f;
        o.insiderKalanSure = 14.3f;
        o.piyasaFaktoru = 0.01f;
        Kareler(u, 2);
        Kaydet(u, "14_oyun_6hisse");
        o.insiderMektupVar = false;
        o.insiderOkundu = true;
        o.normalizeGrafik = true;
        o.piyasaFaktoru = -0.01f;
        Kaydet(u, "15_oyun_6hisse_yuzde");
        a.krediAcik = true;
        Kaydet(u, "16_kredi");
        a.krediAcik = false;
        o.duraklatildi = true;
        Kaydet(u, "17_duraklatma_buyuk_rakam");
        o.duraklatildi = false;

        for (int s = 0; s < (int)YONETIM_OLAYLARI.size(); s++) {
            o.yonetimOlayiAcik = true;
            o.yonetimHisseIndex = i5;
            o.yonetimSablonIndex = s;
            Kaydet(u, TextFormat("18_yonetim_%d", s + 1));
        }
        o.yonetimOlayiAcik = false;

        o.sonPenceresiAcik = true;
        o.sonTuru = SON_ZIRVE; o.bitisGunu = 143; o.yeniRekor = true; o.toplamTemettu = 123456.0f; o.toplamFaiz = 65432.0f;
        Kaydet(u, "19_son_zirve");
        o.yeniRekor = false;
        Kaydet(u, "20_son_zirve_rekorsuz");
        o.senaryo = S_ZAMAN; o.sonTuru = SON_SURE; o.bitisGunu = 45; o.bitisNetDeger = 1234567.0f;
        Kaydet(u, "21_son_sure");
        o.sonTuru = SON_IFLAS; o.bitisGunu = 212;
        Kaydet(u, "22_son_iflas");
        o.sonPenceresiAcik = false;
        o.sonTuru = SON_YOK;
        o.senaryo = S_KLASIK;
    }

    // ---- 4 ve 5 hisse (ara kompakt boylar) ----
    OyunKur(u, S_KRIZ, 3, 15);
    for (int s = 0; s < 4; s++) PozisyonVer(o, SiradakiHisse(o, s), 501 + s * 100, 1.0f + 0.05f * s);
    Kareler(u, 2);
    Kaydet(u, "23_oyun_4hisse");
    OyunKur(u, S_SIFIR, 4, 16);
    for (int s = 0; s < 5; s++) PozisyonVer(o, SiradakiHisse(o, s), 77 + s * 13, 0.97f);
    Kareler(u, 2);
    Kaydet(u, "24_oyun_5hisse");
}

// Tiklama akisi: sahte fare ile gercek dugmelere basip sonucu dogrular.
static void EtkilesimTesti(Uygulama& u) {
    Yaz("\n--- etkilesim testi (sahte tiklamalar) ---\n");
    int hata = 0;
    auto bekle = [&](bool kosul, const char* ad) {
        Yaz("  %-58s %s\n", ad, kosul ? "ok" : "HATA");
        if (!kosul) { hata++; g_sorunSayisi++; }
    };
    g_dil = Dil::TR;
    g_profil = Profil();
    u.ui = Arayuz();
    ArayuzKur(u.ui, (int)SIRKETLER.size());
    u.ekran = Ekran::MENU;
    u.oyunVar = false;
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;

    Kareler(u, 2);
    Tikla(u, a.basBtn.rect);
    bekle(u.ekran == Ekran::OYUN && u.oyunVar, "menu: BASLA oyunu acar");
    bekle(o.rehberAdim == 0, "ilk oyunda rehber kendiliginden baslar");
    o.rehberBekleme = 0.0f;
    KartDuzeni kd = KartDuzeniHesapla(o, a);
    int i0 = kd.acik[0];
    float bakiyeOnce = o.bakiye;
    Tikla(u, a.alBtn[i0].rect);
    bekle(o.hisseler[i0].adet == 1 && o.bakiye < bakiyeOnce, "AL dugmesi 1 adet alir");
    bekle(o.rehberAdim == 1, "rehber: alimdan sonra 2. adima gecer");
    o.rehberBekleme = 0.0f;
    Kareler(u, 2);
    RehberKutusuHesapla(a, o);
    Tikla(u, a.rehberTamamBtn.rect);
    bekle(o.rehberAdim == 2, "rehber: 'Anladim' sonraki adima gecirir");
    Tikla(u, a.satBtn[i0].rect);
    bekle(o.hisseler[i0].adet == 0, "SAT dugmesi satar");
    bekle(o.rehberAdim == 3, "rehber: satistan sonra 4. adima gecer");
    o.rehberBekleme = 0.0f;
    RehberKutusuHesapla(a, o);
    Tikla(u, a.rehberKapatBtn.rect);
    bekle(o.rehberAdim == -1 && g_profil.rehberGoruldu, "rehber: 'Rehberi Kapat' kapatir ve profilde isaretler");

    Tikla(u, a.adetBtn[7].rect);
    bekle(o.secilenAdet == -1, "adet paneli: MAKS secilir");
    Tikla(u, a.alBtn[i0].rect);
    bekle(o.hisseler[i0].adet > 100 && o.bakiye < o.hisseler[i0].fiyat * 2.0f, "MAKS alim nakdin tamamini kullanir");
    int adetOnce = o.hisseler[i0].adet;
    Tikla(u, a.satBtn[i0].rect);
    bekle(o.hisseler[i0].adet == 0 && adetOnce > 0, "MAKS satis tum pozisyonu kapatir");
    bekle(o.NetDeger() < BASLANGIC_BAKIYE, "al-sat dongusu net degeri DUSURUR");

    Tikla(u, a.duraklatBtn.rect);
    bekle(o.duraklatildi, "Duraklat dugmesi duraklatir");
    float sureOnce = o.oyunSuresi;
    Kareler(u, 30);
    bekle(o.oyunSuresi == sureOnce, "duraklatmada simulasyon durur");
    int adetD = o.hisseler[i0].adet;
    Tikla(u, a.alBtn[i0].rect);
    bekle(o.hisseler[i0].adet == adetD, "duraklatmada AL tiklanamaz");
    Tikla(u, a.sifirlaBtn.rect);
    bekle(a.sifirlaOnaySuresi == 0.0f, "duraklatmada Yeni Oyun tiklanamaz");
    Tikla(u, a.ayarlarBtn.rect);
    bekle(a.ayarlarAcik, "duraklatma menusu: Ayarlar acilir");
    Tikla(u, a.dilEnBtn.rect);
    bekle(g_dil == Dil::EN, "ayarlar: English secilir");
    Tikla(u, a.dilTrBtn.rect);
    bekle(g_dil == Dil::TR, "ayarlar: Turkce secilir");
    Tikla(u, a.ayarlarKapatBtn.rect);
    bekle(!a.ayarlarAcik && o.duraklatildi, "ayarlar kapanir, duraklatma surer");
    Tikla(u, a.devamBtn.rect);
    bekle(!o.duraklatildi, "Devam Et oyunu surdurur");

    // emirler
    o.rutbeIndex = 3;
    o.bakiye = 200000.0f;
    Kareler(u, 2);
    kd = KartDuzeniHesapla(o, a);
    o.secilenAdet = 100;
    Tikla(u, a.alBtn[i0].rect);
    Tikla(u, a.emirBtn[i0].rect);
    bekle(a.emirHisse == i0, "EMIR dugmesi emir penceresini acar");
    sureOnce = o.oyunSuresi;
    Kareler(u, 20);
    bekle(o.oyunSuresi == sureOnce, "emir penceresi acikken simulasyon durur");
    Tikla(u, a.emirSeviyeBtn[EMIR_STOP][1].rect);
    bekle(o.hisseler[i0].emirler[EMIR_STOP].aktif && o.hisseler[i0].emirler[EMIR_STOP].fiyat < o.hisseler[i0].fiyat,
          "emir: zarar kes seviyesi fiyatin altina konur");
    Tikla(u, a.emirKaldirBtn[EMIR_STOP].rect);
    bekle(!o.hisseler[i0].emirler[EMIR_STOP].aktif, "emir: Kaldir emri siler");
    Tikla(u, a.emirSeviyeBtn[EMIR_STOP][0].rect);
    Tikla(u, a.emirKapatBtn.rect);
    bekle(a.emirHisse == -1, "emir penceresi kapanir");
    o.hisseler[i0].fiyat *= 0.90f; // fiyat stopun altina duser
    Kareler(u, 3);
    bekle(o.hisseler[i0].adet == 0 && o.gerceklesenEmirSayisi == 1, "zarar kes emri fiyat dusunce pozisyonu satar");

    // aciga satis
    o.secilenAdet = 50;
    Tikla(u, a.satBtn[i0].rect);
    bekle(o.hisseler[i0].adet == -50, "elde hisse yokken SAT aciga satar (rutbe 2+)");
    Tikla(u, a.alBtn[i0].rect);
    bekle(o.hisseler[i0].adet == 0, "AL acigi kapatir");

    // kredi
    Tikla(u, a.krediBtn.rect);
    bekle(a.krediAcik, "Kredi dugmesi kredi masasini acar");
    Tikla(u, a.krediAlBtn[1].rect);
    bekle(fabsf(o.borc - 5000.0f) < 1.0f, "kredi: +5.000 borc alir");
    Tikla(u, a.krediOdeBtn[3].rect);
    bekle(o.borc == 0.0f, "kredi: TUMU borcu kapatir");
    Tikla(u, a.krediKapatBtn.rect);
    bekle(!a.krediAcik, "kredi masasi kapanir");

    // teminat cagrisi
    o.secilenAdet = -1;
    o.bakiye = 10000.0f; // pozisyon, borca gore anlamli buyuklukte olsun
    KrediAl(o, 1e9f);
    Tikla(u, a.alBtn[i0].rect);
    bekle(o.borc > 9000.0f && o.hisseler[i0].adet > 300, "kaldiracli alim yapilir");
    o.hisseler[i0].fiyat *= 0.55f;
    o.hisseler[i0].temelDeger *= 0.55f;
    Kareler(u, 3);
    bekle(o.teminatCagrisiSayisi == 1 && o.borc == 0.0f, "fiyat cokunce teminat cagrisi borcu zorla kapatir");

    // kayit gidis-donus (bellekte; gercek dosyaya dokunmaz)
    o.toastlar.liste.clear();
    EmirKoy(o, i0, EMIR_LIMIT_ALIM, 12.5f, 40);
    o.borc = 321.0f;
    string metin = OyunuMetneYaz(o);
    Oyun kopya = o;
    kopya.Baslat(S_KLASIK, false);
    bool yuklendi = OyunuMetindenYukle(kopya, metin);
    bekle(yuklendi && fabsf(kopya.bakiye - o.bakiye) < 0.01f && kopya.rutbeIndex == o.rutbeIndex &&
              kopya.hisseler[i0].emirler[EMIR_LIMIT_ALIM].aktif && kopya.hisseler[i0].emirler[EMIR_LIMIT_ALIM].adet == 40 &&
              fabsf(kopya.borc - 321.0f) < 0.01f && kopya.senaryo == o.senaryo,
          "kayit: yaz -> oku ayni durumu verir (V5)");
    {
        // Eski surum uyumlulugu: V5 metninden SONDAKI V5 blogunu atip etiketi
        // degistirerek gercek bir V4 dosyasi uretiyoruz (V5 sadece sona alan ekler).
        vector<string> satirlar;
        istringstream ss(metin);
        string satir;
        while (std::getline(ss, satir)) satirlar.push_back(satir);
        size_t v5Satir = 14 + o.hisseler.size() * (4 + EMIR_TUR_SAYISI * 3);
        string v4 = string(KAYIT_ETIKETI_V4) + "\n";
        for (size_t i = 1; i + v5Satir < satirlar.size(); i++) v4 += satirlar[i] + "\n";
        Oyun eski = o;
        eski.Baslat(S_ZAMAN, true);
        bool v4Yuklendi = OyunuMetindenYukle(eski, v4);
        bekle(v4Yuklendi && eski.senaryo == S_KLASIK && eski.rehberAdim == -1 && eski.borc == 0.0f &&
                  fabsf(eski.bakiye - o.bakiye) < 0.01f && eski.islemSayisi == o.islemSayisi,
              "kayit: eski V4 dosyasi yuklenir, yeni alanlar varsayilanda kalir");
    }
    bekle(!OyunuMetindenYukle(kopya, "BORSAPATRONU_SAVE_V1\n1\n2\n"), "kayit: V1 dosyasi reddedilir");
    bekle(!OyunuMetindenYukle(kopya, metin.substr(0, metin.size() / 2)), "kayit: yarim (bozuk) dosya reddedilir, cokme yok");
    bekle(!OyunuMetindenYukle(kopya, "BORSAPATRONU_SAVE_V5\nabc\n"), "kayit: sayi yerine metin iceren dosya reddedilir");
    o.borc = 0.0f;

    Yaz("  etkilesim testi: %s\n", hata == 0 ? "TEMIZ" : "BASARISIZ");
}

int main() {
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    g_profilKalici = false; // gercek profil dosyasina ASLA yazma
    Uygulama u;
    UygulamaBaslat(u);
    sesAcik = false;
    g_profil = Profil();
    g_rapor = fopen("test/cikti/yerlesim_raporu.txt", "w");
    Yaz("BORSA PATRONU %s - GORSEL TEST / YERLESIM RAPORU\n", OYUN_SURUMU);
    Yaz("font: %s\n\n", g_fontHazir ? "Turkce glifli atlas hazir" : "UYARI: varsayilan font (Turkce glif YOK)");
    if (!g_fontHazir) g_sorunSayisi++;
    bool denetciSaglam = DenetciKendiniSinar();
    Yaz("denetci kendini sinadi: %s\n\n", denetciSaglam ? "5 kasitli hatanin 5'ini yakaladi" : "BASARISIZ - denetci guvenilmez!");
    if (!denetciSaglam) g_sorunSayisi++;

    for (int d = 0; d < 2; d++) {
        g_dil = (d == 0) ? Dil::TR : Dil::EN;
        g_dilEki = (d == 0) ? "tr" : "en";
        g_profil = Profil();
        Sahneler(u);
    }
    int yerlesimSorunu = g_sorunSayisi;
    Yaz("\n%d sahne cizildi (iki dilde). Yerlesim sorunu: %d\n", g_sahneSayisi, yerlesimSorunu);

    EtkilesimTesti(u);

    Yaz("\nSONUC: %s\n", g_sorunSayisi == 0 ? "TEMIZ" : "SORUN VAR - yukaridaki satirlara bak");
    if (g_rapor) fclose(g_rapor);
    UygulamaKapat(u);
    return g_sorunSayisi == 0 ? 0 : 1;
}
