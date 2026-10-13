// ============================================================================
// temel.h - sabitler, renk paleti ve kucuk matematik yardimcilari
// ============================================================================
// Bu dosyada OYUN DENGESINI belirleyen butun sayilar bir arada durur. Bir sayiyi
// degistirdikten sonra "python build.py test" ile denge testini calistir ve
// botlarin rutbelere kac gunde ulastigini onceki raporla karsilastir.

static const int PENCERE_GENISLIK = 1280;
static const int PENCERE_YUKSEKLIK = 720; // tam 16:9 - yaygin monitorlerde pillarbox kalmaz

// Ana menude kosede gosterilen surum (src/surum.h icinden gelir).
static const char* OYUN_SURUMU = SURUM_METIN;

static const char* KAYIT_DOSYA_ADI = "borsa_kayit.sav";
static const char* PROFIL_DOSYA_ADI = "borsa_profil.sav";

static const float BASLANGIC_BAKIYE = 10000.0f; // "Klasik" senaryonun baslangic sermayesi
static const int GECMIS_UZUNLUK = 90;     // ana grafikte tutulan veri noktasi
static const int SPARKLINE_UZUNLUK = 30;  // kart ici mini grafikte tutulan veri

static const float PIYASA_TICK_SN = 1.0f;        // fiyat guncelleme sikligi
static const float HABER_TICK_SN = 4.0f;         // haber kontrol sikligi
static const float HABER_OLASILIK = 0.22f;       // her kontrolde haber cikma ihtimali
static const float HABER_ETKI_GECIKME_SN = 5.0f; // haber -> fiyata yansima suresi
static const float GUN_SURESI_SN = 18.0f;        // bir "oyun gunu" kac saniye surer

// ---- PIYASA MODELI ----
// Her saniye, her hisse icin getiri:
//   r = beyazGurultu*volatilite + beta*PIYASA + OZEL - REVERSION*(fiyat/adilDeger - 1)
// PIYASA: tum hisselerin ortak faktoru (AR(1)). OZEL: hisseye ozgu, yine KALICI
// bir faktor (AR(1)) - v2.0 ile eklendi.
//
// NEDEN OZEL FAKTOR? v1.x surumlerinde hisseye ozgu tek sey beyaz gurultuydu.
// Olculen sonuc: iki hissenin 30 SANIYELIK getirileri arasindaki korelasyon 0,98
// idi (1 saniyelikte 0,72) - yani butun hisseler ayni cizgiyi ciziyordu ve
// cesitlendirme hicbir sey korumuyordu. Gercek borsalarda ayni endeksteki
// hisselerin korelasyonu ~0,3-0,5 arasidir. Ortak faktor kisildi, yerine her
// hissenin KENDI trendi geldi.
//
// OLCULEN YENI DEGERLER (test/denge_testi.cpp, bolum 2):
//   korelasyon: 1 sn ~0,40, 30 sn ~0,53      (eskiden 0,72 / 0,98)
//   tik basi std: BIMAS %0,35 ... THYAO %0,60 (eskisiyle AYNI tutuldu)
//   adil degerden tipik sapma: %6 ... %13     (eskisiyle ayni)
// Yani hisselerin tek tek ne kadar oynak oldugu DEGISMEDI; degisen sey artik
// birbirlerinden ayrisabilmeleri. "Hangisini alayim" sorusunun bir anlami var,
// ama piyasa modu hala hissediliyor.
static const float MARKET_KALICILIK = 0.965f;     // yari omur ~19,5 sn
static const float MARKET_SOK_BUYUKLUK = 0.0045f; // piyasa faktoru std ~%0,50
static const float OZEL_KALICILIK = 0.965f;
static const float OZEL_SOK_BUYUKLUK = 0.0040f;
// Ozel faktorun buyuklugu betayla birlikte artar (savunmaci BIMAS kendi basina
// da sakin, oynak THYAO kendi basina da oynak): olcek = TABAN + EGIM*beta.
static const float OZEL_OLCEK_TABAN = 0.25f;
static const float OZEL_OLCEK_EGIM = 0.75f;
// PIYASA MODU gostergesinin "risk-on / risk-off" esigi (piyasa faktorunun ~0,6 sigmasi).
static const float PIYASA_MODU_ESIGI = 0.0030f;

// Her hissenin "adil deger"ine dogru cekilme gucu.
static const float REVERSION_GUCU = 0.05f;

// Beyaz gurultu ve oynaklik kumelenmesi (GARCH benzeri).
// DIKKAT: RastgeleGauss()'un standart sapmasi 1 DEGIL 0,289'dur; yani buradaki
// 0,008 gercekte tik basi ~%0,23 demektir.
static const float BASE_VOLATILITE = 0.008f;
static const float VOL_TABAN = BASE_VOLATILITE;
static const float VOL_SONME_HIZI = 0.965f;
static const float VOL_SOK_ESIGI = 0.02f;
static const float VOL_SOK_CARPANI = 2.6f;
static const float VOL_TAVAN = 0.05f;

// ---- ISLEM MALIYETI / MIKROYAPI (v2.0: AKIS TABANLI) ----
// Tarihce:
//   v1.0  dolum eski fiyattan, etki sonra -> al-sat cevirmek tur basi +%6 RISKSIZ kar.
//   v1.1  karekok etki + dolum fiyati -> tek seferde al-sat artik zarar. AMA etki
//         EMIR BASINA hesaplandigi icin karekokun icbukeyligi acik birakti: 100
//         tane 5'lik alim, fiyati 1 tane 500'lukten 10 KAT fazla itiyordu.
//         "Parca parca al, toplu sat" tur basi +%7 veriyordu (gercek kodla olculdu).
//   v2.0  etki artik emir basina degil, hissenin BIRIKIMLI NET EMIR AKISINA (akis)
//         bagli bir DURUM FONKSIYONU. Emri bolmek hicbir sey degistirmez.
//
// Model:
//   akis (Q)  : oyuncunun o hissedeki yakin zamandaki net alim(+)/satim(-) adedi.
//               Her saniye AKIS_SONME ile sifira dogru soner (emir defteri dolar).
//   f(Q)      = ETKI_OLCEK * isaret(Q) * sqrt(|Q| / TOPLAM_HISSE)   <- anlik "baski"
//   Q0'dan Q1'e giden bir emrin ortalama dolum sapmasi = (F(Q1)-F(Q0)) / (Q1-Q0),
//               F = f'nin integrali. Ustune spread'in yarisi eklenir.
//   Piyasa fiyati oyuncunun emriyle DEGISMEZ; baski sadece dolum fiyatini etkiler.
//
// Neden suistimal edilemez: odenen prim F(Q)'da "birikir". Kapali bir al-sat
// dongusunde (net adet = 0) toplam nakit akisi = -spread - (sonen baski) <= 0.
// Yani hangi sirayla, kac parcada islem yaparsan yap dongu ASLA kar ettirmez.
// Sabirli olmak (baskinin sonmesini beklemek) ucuzlatir, acele etmek pahalilastirir
// - gercek piyasadaki "emri zamana yay" mantigiyla ayni.
//
// AYAR: ALIM_SATIM_SPREAD cevirme maliyetini, ETKI_OLCEK buyuk/aceleci emrin
// maliyetini, AKIS_SONME baskinin ne kadar surdugunu belirler. Notr durumdan tek
// emrin maliyeti v1.1 ile AYNI tutuldu (1 adet %0,25 ... 1000 adet %3,15).
static const float ALIM_SATIM_SPREAD = 0.003f; // %0,3
static const float ETKI_OLCEK = 0.045f;        // 1000 adetlik akista anlik baski %4,5
static const float AKIS_SONME = 0.89f;         // saniye basi; yari omur ~6 sn

// Sirketin "adil degeri" de -cok yavas- kendi kaderini yasar.
static const float TEMEL_DEGER_GUNLUK_OYNAKLIK = 0.02f;

// ---- DEVRALMA (TAKEOVER) ----
static const int TOPLAM_HISSE_SAYISI = 1000;
static const float YONETIM_OLAY_MIN_SN = 14.0f;
static const float YONETIM_OLAY_MAX_SN = 24.0f;
static const float RISK_PRIMI_TAVANI = 0.035f;
static const float RISK_PRIMI_TABANI = 0.0f;

// ---- KURESEL (MAKRO) OLAYLAR ----
static const float KURESEL_OLAY_TICK_SN = 10.0f;
static const float KURESEL_OLAY_OLASILIK = 0.20f;
static const float KURESEL_OLAY_ETKI_GECIKME_SN = 6.0f;
static const float KURESEL_RISK_PRIMI_TAVANI = 0.02f;
static const float KURESEL_RISK_PRIMI_TABANI = 0.0f;

static const float SIFIRLA_ONAY_SN = 3.0f; // "Yeni Oyun" iki asamali onay suresi

// ---- TEMETTU ----
static const int TEMETTU_PERIYOT_GUN = 5;
static const float TEMETTU_ORANI = 0.005f;

// ---- ICERIDEN BILGI (INSIDER) MEKTUBU ----
// Yukselis, mektup OLUSTUGU anda planlanir; okusan da okumasan da gerceklesir.
static const int INSIDER_MIN_RUTBE = 3;
static const float INSIDER_TICK_SN = 20.0f;
static const float INSIDER_OLASILIK = 0.10f;
static const float INSIDER_GECIKME_MIN_SN = 12.0f;
static const float INSIDER_GECIKME_MAX_SN = 20.0f;
static const float INSIDER_MEKTUP_OMUR_SN = 12.0f;
static const float INSIDER_ARTIS_MIN = 0.08f;
static const float INSIDER_ARTIS_MAX = 0.22f;

// ---- ACEMI KORUMASI (erken oyun zorluk dengesi) ----
// Olayin iyi mi kotu mu olacagi RUTBEYE gore agirliklandirilir. TABAN = son
// rutbedeki iyi olasiligi (oyunun zor sonu), BONUS = rutbe 0'da eklenen.
static const float ACEMI_HISSE_IYI_TABAN = 0.67f;
static const float ACEMI_HISSE_IYI_BONUS = 0.18f;
static const float ACEMI_KURESEL_IYI_TABAN = 0.40f;
static const float ACEMI_KURESEL_IYI_BONUS = 0.35f;

// ---- BILANCO TAKVIMI (v2.0) ----
// Her sirket BILANCO_PERIYOT_GUN gunde bir bilanco aciklar. TARIH onceden bellidir
// (kartta geri sayim), SONUC belli degildir; ama kartta bir "beklenti" ipucu durur.
// Beklenti BILANCO_TUTMA_OLASILIGI ihtimalle dogru cikar. Gercek piyasadaki gibi
// beklenti FIYATLANMISTIR: beklenen sonuc kucuk, SURPRIZ buyuk hareket ettirir.
//   beklenti tuttu    -> BILANCO_UYUMLU_MIN..MAX   (beklenti yonunde)
//   beklenti tutmadi  -> BILANCO_SURPRIZ_MIN..MAX  (ters yonde, daha sert)
//   beklenti belirsiz -> BILANCO_BELIRSIZ_MIN..MAX (yazi-tura)
// Beklenen deger beklenti yonunde hafif arti (~%1,6) ama ters kuyrugu kalin.
static const int BILANCO_PERIYOT_GUN = 12;
static const float BILANCO_TUTMA_OLASILIGI = 0.70f;
static const float BILANCO_UYUMLU_MIN = 0.05f, BILANCO_UYUMLU_MAX = 0.09f;
static const float BILANCO_SURPRIZ_MIN = 0.08f, BILANCO_SURPRIZ_MAX = 0.14f;
static const float BILANCO_BELIRSIZ_MIN = 0.06f, BILANCO_BELIRSIZ_MAX = 0.12f;

// ---- GELISMIS ARACLAR: rutbeyle kademeli acilir (v2.0) ----
static const int EMIR_MIN_RUTBE = 1;   // stop / kar al / limit emirleri
static const int ACIGA_MIN_RUTBE = 2;  // aciga satis
static const int KREDI_MIN_RUTBE = 3;  // kredi (kaldirac)

// ---- KREDI / ACIGA SATIS / TEMINAT (v2.0) ----
// Yukumluluk = borc + aciga satilan hisselerin guncel degeri.
// Teminat orani = net deger / yukumluluk.
//   Yeni borc/aciga satis: sonrasinda yukumluluk <= KALDIRAC_LIMITI * net deger olmali.
//   Oran TEMINAT_UYARI_ORANI altina inerse uyari, TEMINAT_CAGRI_ORANI altina inerse
//   TEMINAT CAGRISI: tum aciklar kapatilir, borc kapanana kadar hisse satilir.
static const float KALDIRAC_LIMITI = 1.0f;
static const float TEMINAT_UYARI_ORANI = 0.60f;
static const float TEMINAT_CAGRI_ORANI = 0.40f;
static const float KREDI_GUNLUK_FAIZ = 0.003f;    // gunde %0,3 (bir gun = 18 sn)
static const float ACIGA_GUNLUK_UCRET = 0.0015f;  // odunc alinan hissenin gunluk ucreti
static const int ACIGA_HISSE_SINIRI = 500;        // bir hissede en fazla acik
static const float IFLAS_ORANI = 0.02f;           // net deger baslangicin %2'sinin altina inerse iflas

// Renk paleti (koyu, neon vurgulu "trading floor" temasi)
static const Color RENK_ARKAPLAN_UST = {14, 17, 23, 255};
static const Color RENK_ARKAPLAN_ALT = {8, 10, 14, 255};
static const Color RENK_PANEL = {22, 26, 34, 255};
static const Color RENK_PANEL_ACIK = {30, 35, 45, 255};
static const Color RENK_KENARLIK = {45, 52, 64, 255};
static const Color RENK_ALTIN = {255, 203, 0, 255};
static const Color RENK_YESIL = {6, 214, 160, 255};
static const Color RENK_KIRMIZI = {239, 71, 111, 255};
static const Color RENK_MAVI_VURGU = {17, 138, 178, 255};
static const Color RENK_METIN = {230, 233, 238, 255};
static const Color RENK_SOLUK_METIN = {138, 146, 163, 255};
static const Color RENK_UYARI = {255, 159, 28, 255}; // notr "dikkat" tonu - iyi/kotu ima etmez

// ============================================================================
// YARDIMCI FONKSIYONLAR
// ============================================================================
static float Yumusat(float mevcut, float hedef, float dt, float hiz) {
    // Us-tabanli yumusatma: her karede farkin bir kismini kapatir (frame-rate bagimsiz)
    float t = 1.0f - expf(-hiz * dt);
    return mevcut + (hedef - mevcut) * t;
}

// Yaklasik "can egrisi" dagilimli rastgele sayi: 4 tekduze [-1,1] orneginin
// ortalamasi. Standart sapmasi 1/sqrt(12) = 0,289; |x| asla 1'i gecemez
// (kuyruklar kesili - tek tikte fiyat ucamaz, bilincli tasarim).
static float RastgeleGauss() {
    float toplam = 0.0f;
    for (int i = 0; i < 4; i++) toplam += (float)GetRandomValue(-1000, 1000) / 1000.0f;
    return toplam / 4.0f;
}

// [0,1) araliginda tekduze rastgele sayi.
static float Rastgele01() { return (float)GetRandomValue(0, 9999) / 10000.0f; }
static float RastgeleAralik(float a, float b) { return a + (b - a) * Rastgele01(); }

// Yumusak deger degisimi icin kucuk yardimci (fiyat, net deger gibi seyler
// aniden degil, goz onunde akarak degisir)
struct AnimFloat {
    float deger = 0.0f;
    float hedef = 0.0f;
    float hiz = 5.0f;
    void Ayarla(float h) { hedef = h; }
    void Zorla(float h) { hedef = h; deger = h; }
    void Guncelle(float dt) { deger = Yumusat(deger, hedef, dt, hiz); }
};
