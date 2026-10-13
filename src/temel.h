// ============================================================================
// temel.h - sabitler, renk paleti ve kucuk matematik yardimcilari
// ============================================================================
// Bu dosyada OYUN DENGESINI belirleyen butun sayilar bir arada durur. Bir sayiyi
// degistirdikten sonra "python build.py test" ile denge testini calistir ve
// botlarin rutbelere kac gunde ulastigini onceki raporla karsilastir.

// ---- SANAL SAHNE BOYUTU (v2.1: degisken) ----
// Oyun "sanal" bir sahneye cizilir, sonra gercek pencereye olceklenir. v2.0'a
// kadar sahne sabit 1280x720 idi; 16:9 olmayan her pencerede (ornegin bir
// tarayici sekmesi ~2:1'dir) sagda/solda siyah bosluk kaliyordu. Artik sahne
// pencerenin en-boy oranina UYAR:
//   YATAY (oran >= DIKEY_ESIK_ORAN):
//          16:9'dan GENIS pencere -> yukseklik 720, genislik 1280..1920
//                                    (fazla alan grafige gider);
//          16:9'dan KARE pencere  -> genislik 1280, yukseklik 720..960
//                                    (fazla alan kartlara ve grafige gider).
//          Yani 4:3 ile 8:3 arasindaki her pencere BOSLUKSUZ dolar.
//   DIKEY (telefon, oran < esik): genislik 480..720, yukseklik 900..1180;
//          tamamen ayri, dokunmaya uygun bir yerlesim kullanilir (arayuz_dikey.h).
// Bu araliklarin disinda kalan (cok uzun / cok basik) pencerelerde sahne
// ortalanir ve kalan yer bos kalir.
// PENCERE_GENISLIK / PENCERE_YUKSEKLIK bu yuzden SABIT DEGIL, o anki sahne
// boyutudur; yerlesim kodu her zaman bunlara gore hesap yapar.
static int PENCERE_GENISLIK = 1280;
static int PENCERE_YUKSEKLIK = 720;
[[maybe_unused]] static bool g_dikey = false; // dikey (telefon) yerlesimi etkin mi

static const int YATAY_YUKSEKLIK = 720;
static const int YATAY_MAX_YUKSEKLIK = 960;
static const int YATAY_MIN_GENISLIK = 1280;
static const int YATAY_MAX_GENISLIK = 1920;
static const float DIKEY_ESIK_ORAN = 0.80f; // en/boy bunun altindaysa dikey yerlesim
static const int DIKEY_MIN_GENISLIK = 480;
static const int DIKEY_MIN_YUKSEKLIK = 900;
static const int DIKEY_MAX_YUKSEKLIK = 1180;

// Ana menude kosede gosterilen surum (src/surum.h icinden gelir).
static const char* OYUN_SURUMU = SURUM_METIN;

static const char* KAYIT_DOSYA_ADI = "borsa_kayit.sav";
static const char* PROFIL_DOSYA_ADI = "borsa_profil.sav";

static const float BASLANGIC_BAKIYE = 10000.0f; // "Klasik" senaryonun baslangic sermayesi
static const int GECMIS_UZUNLUK = 90;     // ana grafikte tutulan veri noktasi
static const int SPARKLINE_UZUNLUK = 30;  // kart ici mini grafikte tutulan veri

// Simulasyon SABIT adimlarla ilerler (kare hizindan bagimsiz). Ayni tohum + ayni
// adim sayisi = ayni piyasa; "Gunun Piyasasi" senaryosu ve tekrarlanabilir
// hata raporlari buna dayanir.
static const float SIM_ADIM_SN = 0.05f;

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
// YONETIM KARARLARI (v2.1): riskli secenek bir KUMARDIR.
// v2.0'a kadar riskli secenek her seferinde +%8..18 veriyor, tek bedeli biraz
// oynaklikti; olculdu: sirketi devralip HEP riskliyi secen oyuncu 30.000 TL'den
// zirveye medyan 29 gunde, oyunlarin %100'unde cikiyordu (haberleri okuyan bot
// ayni yolu ~80 gunde aliyor). Yani "risk" yoktu ve baska her sey anlamsizlasiyordu.
// Artik: riskli karar TUTARSA sablondaki kazanc, TUTMAZSA ona yakin bir kayip.
// Tutma olasiligi sirketin birikmis risk primine baglidir - ust uste risk alan
// sirket kirilganlasir; guvenli kararlar risk primini dusurup sirketi toparlar.
// Dogru oyun: sirket saglamken risk al, kirilganken toparla.
static const float YONETIM_BASARI_TABAN = 0.70f;       // temiz sirkette (risk primi 0) tutma olasiligi
static const float YONETIM_BASARI_RISK_EGIMI = 0.40f;  // risk primi tavandayken bu kadar duser (0,70 -> 0,30)
static const float YONETIM_BASARISIZLIK_ORANI = 0.75f; // tutmazsa kayip = sablondaki kazancin bu kati
static const float YONETIM_GUVENLI_TOPARLANMA = 0.006f; // her guvenli karar risk primini bu kadar dusurur
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

// ---- DEGERLEME (v2.1) ----
// SORUN (denge testiyle olculdu): hic satmayan, hicbir beceri gostermeyen bot
// oyunlarin %81'inde zirveye cikiyordu. Iyi haber kotuden sik geldigi icin her
// hisse SONSUZA DEK yukseliyordu; oyunun sonu beceri degil sabir olcuyordu.
//
// COZUM: her hissenin gizli bir "esas degeri" var. Sirket haberlerinin sadece
// bir kismi esasa yansir, kalani HEYECANDIR. Degerleme = adilDeger / esasDeger.
// Hisse pahalilastikca (degerleme > 1):
//   - o hisseye iyi haber gelme olasiligi DUSER (beklenti yukseldi, hayal
//     kirikligi kolaylasti),
//   - esigi asarsa her gun bir DUZELTME (sert dusus) riski dogar.
// Ucuzlayan hissede bunlarin tersi olur (TOPARLANMA). Kartta KELEPIR / UCUZ /
// MAKUL / PAHALI / BALON etiketi durur: oyuncunun okuyup kullanabilecegi, gercek
// piyasadaki "pahali hisse risklidir" sezgisinin oyundaki karsiligi.
//
// OYUNUN BASIYLA UYUM: kullanici oyunun basinin yumusak olmasini ozellikle istedi.
// Bu yuzden haberlerin HEYECAN payi rutbeyle buyur (rutbe 0'da 0, son rutbede
// DEGERLEME_HEYECAN_PAYI): ilk rutbelerde degerleme hic oynamaz, etiket hep
// MAKUL kalir. AMA etiketin ANLAMI her rutbede aynidir - PAHALI yazan hisse her
// zaman ayni cezayi tasir. (Ilk tasarimda ceza da rutbeyle olcekleniyordu; o
// zaman "PAHALI gorunce sat" kurali orta rutbelerde zarar ettiriyordu, cunku
// etiket yaziyordu ama etkisi yoktu. Denge testindeki Degerci botu bunu gosterdi.)
//
// SAYILARIN ANLAMI (son rutbede): iyi haber olasiligi 0,67 - EGIM*ln(degerleme).
// Haber getirisinin beklentisi degerleme ~1,10'da SIFIRA iner; bu, hisselerin
// etrafinda dolandigi dengedir. Etiket sinirlari buna gore secildi:
//   MAKUL  (<= 1,12): beklenen getiri arti ya da sifir
//   PAHALI (<= 1,27): beklenen getiri EKSI
//   BALON  (>  1,27): belirgin eksi + duzeltme riski (esik 1,30)
//   UCUZ / KELEPIR  : belirgin arti (+ toparlanma olasiligi)
static const float DEGERLEME_HEYECAN_PAYI = 0.5f;   // son rutbede haber etkisinin heyecan olan kismi
static const float DEGERLEME_HABER_EGIMI = 1.5f;    // iyi haber olasiligi, ln(degerleme) basina bu kadar duser
// Sirket zamanla degerlemesinin "icini doldurur": esas deger her gun, adil
// degerle arasindaki farkin bu kadarini kapatir. Bu olmasaydi denge noktasinda
// haber getirisi tam SIFIR olurdu (hisseler hic buyumezdi, haberleri oynayan
// oyuncu da belirgin yavaslardi). Bununla al-tut getirisi sifirlanmaz, yavaslar.
static const float DEGERLEME_YAKINSAMA_GUNLUK = 0.03f;
static const float DUZELTME_ESIGI = 1.30f;          // degerleme bunu asarsa duzeltme riski
static const float TOPARLANMA_ESIGI = 0.80f;        // bunun altindaysa toparlanma olasiligi
static const float DUZELTME_OLASILIK_EGIMI = 1.5f;  // gunluk olasilik = egim * (esigi asan miktar), en fazla 0,5
// Etiket sinirlari (degerleme orani)
static const float DEGERLEME_KELEPIR = 0.85f;
static const float DEGERLEME_UCUZ = 0.97f;
static const float DEGERLEME_PAHALI = 1.12f;
static const float DEGERLEME_BALON = 1.27f;

// ---- GOREVLER (v2.1) ----
// Rutbeler arasi bosluk oyun ilerledikce uzuyor; arayi kisa hedefler doldurur.
// Her an TEK bir gorev vardir; tamamlaninca net degerin GOREV_ODUL_ORANI kadari
// nakit odul gelir, kisa bir aradan sonra yenisi verilir. Suresi dolan gorev
// cezasiz degisir.
static const float GOREV_ODUL_ORANI = 0.02f;
static const float GOREV_ODUL_TABAN = 150.0f;
static const int GOREV_SURE_GUN = 8;
static const float GOREV_ARA_SN = 6.0f;
static const float GOREV_ILK_BEKLEME_SN = 25.0f; // oyunun basinda ilk gorev gelmeden once

// ---- GELISMIS ARACLAR: rutbeyle kademeli acilir (v2.0) ----
static const int EMIR_MIN_RUTBE = 1;   // stop / kar al / limit emirleri
static const int ACIGA_MIN_RUTBE = 2;  // aciga satis
static const int KREDI_MIN_RUTBE = 3;  // kredi (kaldirac)
static const int LIMAN_MIN_RUTBE = 2;  // guvenli liman varliklari (altin, dolar)

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
// YESIL / KIRMIZI / UYARI sabit DEGIL: renk koru modunda degisirler (bkz. PaletiAyarla).
static Color RENK_YESIL = {6, 214, 160, 255};
static Color RENK_KIRMIZI = {239, 71, 111, 255};
static const Color RENK_MAVI_VURGU = {17, 138, 178, 255};
static const Color RENK_METIN = {230, 233, 238, 255};
static const Color RENK_SOLUK_METIN = {138, 146, 163, 255};
static Color RENK_UYARI = {255, 159, 28, 255}; // notr "dikkat" tonu - iyi/kotu ima etmez

// Renk koru modu: oyun kazanc/kaybi yesil/kirmiziyla anlatiyor, bu ikili en yaygin
// renk korlugu turlerinde ayirt edilemez. Mod acikken yerlerine MAVI / TURUNCU
// gelir (renk korlugune dayanikli standart ikili); "notr dikkat" rengi de
// turuncuyla karismasin diye mora doner. (Fiyat oklari ve +/- isaretleri zaten
// renkten bagimsiz ikinci bir ipucu veriyor.)
static bool g_renkKoru = false;
static void PaletiAyarla(bool renkKoru) {
    g_renkKoru = renkKoru;
    if (renkKoru) {
        RENK_YESIL = {64, 170, 255, 255};
        RENK_KIRMIZI = {255, 140, 0, 255};
        RENK_UYARI = {200, 160, 255, 255};
    } else {
        RENK_YESIL = {6, 214, 160, 255};
        RENK_KIRMIZI = {239, 71, 111, 255};
        RENK_UYARI = {255, 159, 28, 255};
    }
}

// ============================================================================
// YARDIMCI FONKSIYONLAR
// ============================================================================
static float Yumusat(float mevcut, float hedef, float dt, float hiz) {
    // Us-tabanli yumusatma: her karede farkin bir kismini kapatir (frame-rate bagimsiz)
    float t = 1.0f - expf(-hiz * dt);
    return mevcut + (hedef - mevcut) * t;
}

// ---------------------------------------------------------------------------
// SIMULASYONUN KENDI RASTGELE SAYI URETECI
// ---------------------------------------------------------------------------
// Simulasyon raylib'in GetRandomValue()'sunu KULLANMAZ: onu parcacik efektleri de
// kullaniyor, yani oyuncunun her tiklamasi piyasanin gelecegini degistirirdi.
// Iki ayri akis var:
//   g_rngPiyasa : ortam (fiyatlar, haberler, kuresel olaylar, bilancolar).
//                 Oyuncunun ne yaptigindan BAGIMSIZ sayida cekilir.
//   g_rngOyuncu : oyuncunun durumuna bagli olaylar (yonetim kararlari, iceriden
//                 bilgi mektubu). Bunlar ortam akisini KAYDIRMAZ.
// Ikisinin durumu da kayit dosyasina yazilir: kaydi yeniden yuklemek gelecegi
// yeniden "zar atmaz" (kotu haberden once kaydedip yukleyerek kacilamaz).
struct Rng {
    uint64_t durum = 0x9E3779B97F4A7C15ULL;

    void Tohumla(uint64_t tohum) {
        durum = tohum * 0x9E3779B97F4A7C15ULL + 0xD1B54A32D192ED03ULL;
        if (durum == 0) durum = 0x9E3779B97F4A7C15ULL;
        for (int i = 0; i < 8; i++) Sonraki();
    }
    uint32_t Sonraki() { // xorshift64*
        durum ^= durum >> 12;
        durum ^= durum << 25;
        durum ^= durum >> 27;
        return (uint32_t)((durum * 0x2545F4914F6CDD1DULL) >> 32);
    }
    float Birim() { return (float)(Sonraki() >> 8) * (1.0f / 16777216.0f); } // [0,1)
    int Aralik(int a, int b) { // [a,b] dahil
        if (b <= a) return a;
        return a + (int)(Sonraki() % (uint32_t)(b - a + 1));
    }
};

static Rng g_rngPiyasa;
static Rng g_rngOyuncu;

static void SimTohumla(uint64_t tohum) {
    g_rngPiyasa.Tohumla(tohum);
    g_rngOyuncu.Tohumla(tohum ^ 0x5DEECE66D1234567ULL);
}

// [0,1) araliginda tekduze rastgele sayi (ortam akisi).
static float Rastgele01() { return g_rngPiyasa.Birim(); }
static int RastgeleInt(int a, int b) { return g_rngPiyasa.Aralik(a, b); }

// Yaklasik "can egrisi" dagilimli rastgele sayi: 4 tekduze [-1,1] orneginin
// ortalamasi. Standart sapmasi 1/sqrt(12) = 0,289; |x| asla 1'i gecemez
// (kuyruklar kesili - tek tikte fiyat ucamaz, bilincli tasarim).
static float RastgeleGauss() {
    float toplam = 0.0f;
    for (int i = 0; i < 4; i++) toplam += Rastgele01() * 2.0f - 1.0f;
    return toplam / 4.0f;
}

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
