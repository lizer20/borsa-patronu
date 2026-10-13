/*
 * ============================================================================
 *  BORSA PATRONU  -  Trading Tycoon
 * ============================================================================
 *  Onceki Python/Tkinter prototipinin, gercek bir "is yonetimi" oyunu gibi
 *  hissettirecek sekilde C++ + raylib ile bastan yazilmis surumu.
 *
 *  NEDEN C++ / raylib?
 *    Tkinter + matplotlib bir "form uygulamasi" gibi gorunuyordu: statik
 *    butonlar, donen bir grafik penceresi, animasyon yok, ses yok, ilerleme
 *    hissi yok. raylib ise dogrudan bir cizim tuvali + oyun donguleri sunuyor;
 *    bu sayede animasyonlu sayaclar, parcacik efektleri, "glow" cizgiler,
 *    kayan haber bandi, rutbe/hedef sistemi ve prosedurel ses efektleri gibi
 *    gercek bir oyunda beklediginiz her seyi elle kodlayabiliyoruz.
 *
 *  ONEMLI MIMARI NOT (eski "after() cokme" hatasi bu tasarimda YOK):
 *    Tkinter surumunde gecikmeli olaylar (piyasa tick, haber, flash efekti)
 *    Tk'nin after() zamanlayicisiyla, pencere disinda planlanan geri
 *    cagirmalarla yonetiliyordu; pencere kapaninca bu "hayalet" cagrilar
 *    coku yaratiyordu. Burada TEK BIR oyun dongusu var (while pencere acik),
 *    her seyGetFrameTime() ile gelen dt kadar ilerliyor (zamanlayicilar,
 *    gecikmeli haber etkileri, animasyonlar, parcaciklar...). Pencere
 *    kapaninca dongu zaten biter; ortada "arka planda calisan, sahipsiz
 *    kalmis" hicbir zamanlayici olamaz. Bu hata sinifi tasarim geregi
 *    imkansiz hale geldi.
 *
 *  OYUN OZELLIKLERI:
 *    - Rutbe sistemi (Acemi Tuccar -> Efsane Yatirimci) + rutbe ilerleme cubugu
 *    - Sirali hedefler (15K, 30K, 60K, 120K, 250K TL) + kutlama efekti
 *    - Animasyonlu sayaclar (net deger, fiyatlar yumusak gecisle degisir)
 *    - Parcacik efektleri (alim/satim/haber anlarinda)
 *    - "Toast" bildirimleri (rutbe atlama, hedef tamamlama, ilk islem...)
 *    - Kayan haber bandi (Bloomberg tarzi terminal gorunumu)
 *    - Her hissede mini "sparkline" grafik + ana buyuk canli grafik (glow)
 *    - Haber ANINDA duyurulur, etkisi 5 saniye sonra fiyata yansir
 *    - Prosedurel ses efektleri (harici ses dosyasi YOK, kod ile uretiliyor)
 *    - Duraklat / Sifirla / Ses ac-kapa butonlari, baslangic menusu
 *
 *  DERLEME:
 *    raylib kurulu olmasi gerekir (https://www.raylib.com/ - vcpkg, brew,
 *    apt "libraylib-dev" veya resmi kurulum betikleriyle kurabilirsiniz).
 *
 *    Linux/Mac ornegi:
 *        g++ -std=c++17 borsa_patronu.cpp -o borsa_patronu \
 *            -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
 *
 *    Windows (MinGW) ornegi:
 *        g++ -std=c++17 borsa_patronu.cpp -o borsa_patronu.exe ^
 *            -lraylib -lopengl32 -lgdi32 -lwinmm
 *
 *    (Tam kurulum talimatlari icin raylib'in resmi "wiki/Working on Windows"
 *     veya "wiki/Working on GNU Linux" sayfalarina bakabilirsiniz.)
 * ============================================================================
 */

#include "raylib.h"
#include "raymath.h"  // Clamp(), Lerp() icin (raylib ile birlikte gelen resmi yardimci baslik)

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// ============================================================================
// SABITLER VE RENK PALETI
// ============================================================================
static const int PENCERE_GENISLIK = 1280;
static const int PENCERE_YUKSEKLIK = 720; // 1280x720 = tam 16:9, 1920x1080/2560x1440 gibi yaygin ekranlarda pillarbox (sag/sol siyah bosluk) kalmaz

// Oyun surumu - ana menude kosede gosterilir. itch.io'da gelen geri
// bildirimleri ("su surumde su oluyor") eslestirebilmek icin ONEMLI: her
// yayin oncesi updates/vX.Y klasoru ile AYNI degere guncelle.
static const char* OYUN_SURUMU = "v1.1";

static const char* KAYIT_DOSYA_ADI = "borsa_kayit.sav";

// Kayit dosyasini .exe'nin YANINA yaziyoruz, "calisma dizinine" DEGIL.
// Eskiden duz "borsa_kayit.sav" (goreli yol) kullaniliyordu; bu, oyunu hep
// .exe'nin klasorunden calistirdigin surece sorunsuz - ama dagitilan bir
// surumde oyuncu bir KISAYOLDAN veya itch.io uygulamasindan baslattiginda
// calisma dizini bambaska bir klasor olabiliyor. O zaman kayit oraya duser,
// "Kayitli Oyunu Yukle" bos gorunur ve oyuncu kaydini kaybetmis sanir.
// GetApplicationDirectory() .exe'nin klasorunu (sonunda ayrac ile) verir;
// raylib'in ic statik tamponunu kullandigi icin hemen kopyaliyoruz.
static const string& KayitYolu() {
    static string yol;
    if (yol.empty()) {
        const char* dizin = GetApplicationDirectory();
        yol = (dizin != nullptr ? string(dizin) : string()) + KAYIT_DOSYA_ADI;
    }
    return yol;
}

static const float BASLANGIC_BAKIYE = 10000.0f;
static const int GECMIS_UZUNLUK = 90;     // ana grafikte tutulan veri noktasi
static const int SPARKLINE_UZUNLUK = 30;  // kart ici mini grafikte tutulan veri

static const float PIYASA_TICK_SN = 1.0f;        // fiyat guncelleme sikligi
static const float HABER_TICK_SN = 4.0f;         // haber kontrol sikligi
static const float HABER_OLASILIK = 0.22f;       // her kontrolde haber cikma ihtimali
static const float HABER_ETKI_GECIKME_SN = 5.0f; // haber -> fiyata yansima suresi
static const float GUN_SURESI_SN = 18.0f;        // bir "oyun gunu" kac saniye surer

// ---- PIYASA MODELI AYARLARI (gercekci + ongorulemez dengesi) ----
// Genel "piyasa modu" (mean-reverting AR(1) sureci): KALICILIK 1'e ne kadar
// yakinsa faktor o kadar yavas degisir (uzun suren yukselis/dusus "trend"
// hissi verir); her adimda rastgele bir SOK eklenir.
static const float MARKET_KALICILIK = 0.965f;
static const float MARKET_SOK_BUYUKLUK = 0.006f;

// Her hissenin "adil deger"ine dogru cekilme gucu (asiri yukselen/dusen bir
// hisse zamanla kendi temeline geri donme egiliminde olur - siniirsiz
// savrulmayi onler).
static const float REVERSION_GUCU = 0.05f;

// Hisseye ozgu (idiosinkratik) gurultu ve oynaklik kumelenmesi (GARCH benzeri):
// buyuk bir sok sonrasi oynaklik yukselir, sakin donemlerde tabana yavasca soner.
static const float BASE_VOLATILITE = 0.008f;
static const float VOL_TABAN = BASE_VOLATILITE;
static const float VOL_SONME_HIZI = 0.965f;
static const float VOL_SOK_ESIGI = 0.02f;
static const float VOL_SOK_CARPANI = 2.6f;
static const float VOL_TAVAN = 0.05f;

// ---- ISLEM MALIYETI VE PIYASA ETKISI (mikroyapi) ----
// v1.1'de BASTAN yazildi. Eski model iki yerde hataliydi:
//   (a) Etki DOGRUSAL'di (adet x katsayi) ve 172 adette tavana carpiyordu; 250 ile
//       1000 adet ayni maliyete geliyordu, yani buyukluk karari anlamsizlasiyordu.
//   (b) DAHA ONEMLISI: dolum ESKI fiyattan yapilip etki SONRA uygulaniyordu. Yani
//       oyuncu kendi ittigi fiyattan satabiliyordu -> al-sat cevirmek RISKSIZ ve
//       TEKRARLANABILIR %6 kar veriyordu (10.000 TL, 6 turda 14.175 TL). Amaclananin
//       TAM TERSI. Artik dolum fiyati etkiden SONRA hesaplanir (asagi bak).
//
// Yeni model gercek piyasa mikroyapisini taklit eder:
//   1. SPREAD: her isleme sabit bir alis/satis farki maliyeti (alirken ustune,
//      satarken altina yarisi kadar). Kucuk islemlerde baskin maliyet budur.
//   2. KAREKOK ETKI YASASI: gercek piyasalarda fiyat etkisi emir buyuklugunun
//      KAREKOKU ile artar (Almgren-Chriss / Bouchaud "square-root law"), dogrusal
//      degil. Sirketin TAMAMINI (1000 hisse) birden alsan etki TICARET_ETKI_OLCEK.
//   3. DOLUM PAYI: emir defterinde "yuruyorsun" - ortalama olarak etkinin YARISINI
//      dolum fiyati olarak odersin, ama piyasa fiyati TAM etki kadar hareket eder.
// Sonuc: al-sat dongusu her turda zarar ettirir (tur basi ~%0,3-0,5), buyuk
// pozisyon almak gercekten pahalidir, kucuk islem neredeyse bedavadir.
static const float ALIM_SATIM_SPREAD = 0.003f;   // %0,3 alis-satis farki
static const float TICARET_ETKI_OLCEK = 0.06f;   // 1000 hissenin tamami icin %6
static const float ETKI_DOLUM_PAYI = 0.5f;       // etkinin dolum fiyatina yansiyan kismi

// Sirketin "adil degeri" de -cok yavas- kendi kaderini yasar (uzun vadede
// isin gercekten iyi/kotu gitmesi gibi), boylece uzun oturumlarda da
// ongorulemezlik biter tukenmez.
static const float TEMEL_DEGER_GUNLUK_OYNAKLIK = 0.02f;

// ---- DEVRALMA (TAKEOVER) MEKANIGI ----
// Her sirketin toplam 1000 hissesi var; 501 (yani yarisindan fazlasi) elinde
// olunca o sirketi DEVRALMIS olursun ve onu "yonetmeye" baslarsin.
static const int TOPLAM_HISSE_SAYISI = 1000;
static const float YONETIM_OLAY_MIN_SN = 14.0f; // devraldigin sirkette bir sonraki
static const float YONETIM_OLAY_MAX_SN = 24.0f; // yonetim karari ne zaman gelir
static const float RISK_PRIMI_TAVANI = 0.035f;  // riskli kararlarin biriktirebilecegi azami "kalici" oynaklik
static const float RISK_PRIMI_TABANI = 0.0f;

// ---- KURESEL (MAKRO) OLAYLAR ----
// Tek bir sirketi degil TUM piyasayi etkileyen, seyrek ama buyuk etkili
// olaylar (FED faiz karari, jeopolitik krizler, merkez bankasi politikalari
// vb.). Amac: oyunu zamanla zorlastirmak - kotu bir kuresel olay TUM
// hisselerin oynaklik tabanini KALICI olarak yukseltir (bkz. kuresekRiskPrimi),
// tipki bir sirkette riskli yonetim karari verilmesi gibi ama TUM piyasaya
// birden uygulanir. Iyi bir kuresel olay bu primi biraz azaltabilir de.
static const float KURESEL_OLAY_TICK_SN = 10.0f;        // kuresel olay kontrol sikligi
static const float KURESEL_OLAY_OLASILIK = 0.20f;       // her kontrolde olay cikma ihtimali
static const float KURESEL_OLAY_ETKI_GECIKME_SN = 6.0f; // duyuru -> piyasaya yansima suresi
static const float KURESEL_RISK_PRIMI_TAVANI = 0.02f;   // kuresel primin biriktirebilecegi azami deger
static const float KURESEL_RISK_PRIMI_TABANI = 0.0f;

// "Yeni Oyun" butonunun iki asamali onayinda, ilk tiktan sonra ikinci tikin
// beklendigi sure (bkz. Oyun::sifirlaOnaySuresi). Sure dolarsa buton normal
// haline doner ve tek tikla sifirlama tekrar mumkun olmaz.
static const float SIFIRLA_ONAY_SN = 3.0f;

// ---- TEMETTU ----
// Hisse tutmanin tek getirisi fiyat artisiydi; oyun saf al-sat idi. Artik her
// TEMETTU_PERIYOT_GUN gunde bir, elindeki hisselerin GUNCEL degerinin
// TEMETTU_ORANI kadari nakit olarak odenir - "uzun vadeli yatirimci" oynanisi
// da gecerli bir strateji oluyor. (Bir gun GUN_SURESI_SN = 18 sn oldugu icin
// 5 gun ~ 90 saniye; %0,5 -> dakikada ~%0,33 gelir.)
static const int TEMETTU_PERIYOT_GUN = 5;
static const float TEMETTU_ORANI = 0.005f;

// ---- ICERIDEN BILGI (INSIDER) MEKTUBU ----
// UST SEVIYELERDE, NADIREN ust barda altin bir ZARF belirir. Tiklarsan sana bir
// hissenin YAKIN GELECEKTE ne kadar yukselecegini soyler ve o yukselis
// gerceklesmeden once pozisyon alma sansi verir.
//
// TASARIM NOTU: yukselis, mektup OLUSTUGU anda `bekleyenHaberler` kuyruguna
// planlanir - yani mektubu okusan da okumasan da GERCEKLESIR. Mektup olayi
// yaratmiyor, sadece onceden HABER VERIYOR. Okumazsan sadece firsati kacirmis
// olursun; bu, mekanigin "iceriden bilgi" hissini dogru veren tek kurulum.
// Ayrica geç oyunun sertligine (bkz. Acemi korumasi) karsi bir denge unsuru.
static const int INSIDER_MIN_RUTBE = 3;             // "Borsa Kaplani" ve ustu
static const float INSIDER_TICK_SN = 20.0f;         // kontrol sikligi
static const float INSIDER_OLASILIK = 0.10f;        // her kontrolde gelme ihtimali (~3,3 dk'da bir)
static const float INSIDER_GECIKME_MIN_SN = 12.0f;  // ipucu -> yukselisin gerceklesmesi
static const float INSIDER_GECIKME_MAX_SN = 20.0f;
static const float INSIDER_MEKTUP_OMUR_SN = 12.0f;  // tiklanmazsa zarf kaybolur (firsat kacar)
static const float INSIDER_ARTIS_MIN = 0.08f;       // %8
static const float INSIDER_ARTIS_MAX = 0.22f;       // %22

// ---- ACEMI KORUMASI (erken oyun zorluk dengesi) ----
// SORUN: kuresel olay havuzunda 10 olayin 6'si kotu VE kotu olaylar iyilerden
// daha sert vuruyor (ortalama -%7,2'ye karsi +%5,3). Bu, olay basina -%2,2'lik
// bir beklenen kayip demek; olaylar ortalama 50 saniyede bir geldigi icin yeni
// oyuncu daha ilk dakikalarda surekli dayak yiyor ve kuresel risk primi ~5
// dakikada TAVANA vuruyordu (yani oyun en oynak haline hemen geliyordu).
//
// COZUM: olayin iyi mi kotu mu olacagini RUTBEYE gore agirliklandiriyoruz.
// Rutbe 0'da (sadece ilk hisse acikken) iyi haber olasiligi yuksek, rutbe
// ilerledikce kademeli olarak eski (zor) dengeye iner. SON rutbede degerler
// eskisiyle BIREBIR AYNI - yani oyunun zor sonu korunuyor, sadece giris
// yumusatiliyor. Havuzlarin kendisine dokunulmadi; sadece SECIM agirligi degisti.
static const float ACEMI_HISSE_IYI_TABAN = 0.67f;   // son rutbe: havuzun dogal orani (%67)
static const float ACEMI_HISSE_IYI_BONUS = 0.18f;   // rutbe 0: %85
static const float ACEMI_KURESEL_IYI_TABAN = 0.40f; // son rutbe: havuzun dogal orani (%40)
static const float ACEMI_KURESEL_IYI_BONUS = 0.35f; // rutbe 0: %75

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

// Sirketin sektoru. Kuresel olaylar artik TUM hisseleri ayni yonde itmiyor;
// bazi olaylarin belirli bir sektore EKSTRA (veya ters yonlu) etkisi var -
// orn. petrol soku havayolunu fazladan vurur, savunmayi pek etkilemez.
enum class Sektor { YOK, SAVUNMA, HAVACILIK, BANKA, SANAYI, HOLDING, PERAKENDE };

struct HaberSablonu {
    string metin;
    float carpan;
};

struct SirketBilgi {
    string kod;
    string tamAd;
    Color renk;
    Sektor sektor;
    float baslangicFiyat;
    float beta; // Genel piyasa hareketine ne kadar duyarli (1.0 = piyasa ile ayni, >1 daha oynak)
    vector<HaberSablonu> haberler; // bu sirkete ozel olasi haberler (iyi/kotu)
};

// 6 hisse (RUTBELER ile ayni sayida: her rutbe atlayisinda bir yenisi acilir).
// Beta degerleri gercekci bir sezgiyle secildi: savunma sanayi (ASELS) genel
// piyasadan biraz bagimsiz hareket eder (dusuk beta), havayolu (THYAO) hem
// yakit/turizm dongulerine hem genel piyasaya karsi daha oynaktir (yuksek
// beta), banka (GARAN) faiz/makro haberlere karsi piyasaya yakin hareket eder,
// demir-celik (EREGL) emtia dongusune bagli oynak bir sanayi hissesi, perakende
// (BIMAS) savunmaci/dusuk-beta bir tuketim hissesi, holding (KCHOL) ise genel
// piyasaya yakin, cok kollu bir hisse.
//
// ACILMA SIRASI: oyuna ilk girildiginde SADECE en dusuk baslangic fiyatina
// sahip hisse acik/gorunur olur; her rutbe atlayisinda bir sonraki en ucuz
// hisse acilir (bkz. Oyun::Baslat() - acilmaSeviyesi alani baslangicFiyat'a
// gore siralanarak atanir, dizideki sira ONEMLI DEGIL).
static const vector<SirketBilgi> SIRKETLER = {
    {"ASELS", "Aselsan Savunma San.", {31, 119, 180, 255}, Sektor::SAVUNMA, 45.0f, 0.7f, {
        {"Ihracat rekoru kirildi!", 1.16f},
        {"Tedarik zinciri sorunlari yasaniyor.", 0.87f},
        {"Yeni nesil radar ihalesi kazanildi.", 1.10f},
    }},
    {"THYAO", "Turk Hava Yollari", {214, 39, 40, 255}, Sektor::HAVACILIK, 250.0f, 1.3f, {
        {"Turizm sezonu rekor kirdi.", 1.11f},
        {"Yakit maliyetleri arti yapti.", 0.90f},
        {"Yeni ucus hatlari acildi.", 1.07f},
    }},
    {"GARAN", "Garanti BBVA", {44, 160, 44, 255}, Sektor::BANKA, 60.0f, 1.05f, {
        {"Kar beklentisi asildi.", 1.09f},
        {"Kredi faizleri yukseldi.", 0.92f},
        {"Kredi notu yukseltildi.", 1.06f},
    }},
    {"EREGL", "Eregli Demir Celik", {255, 127, 14, 255}, Sektor::SANAYI, 38.0f, 1.2f, {
        {"Celik ihracati rekor kirdi!", 1.14f},
        {"Demir cevheri fiyatlari yukseldi, maliyetler arti.", 0.89f},
        {"Yeni yuksek firin yatirimi devreye girdi.", 1.09f},
    }},
    {"KCHOL", "Koc Holding", {23, 190, 207, 255}, Sektor::HOLDING, 145.0f, 1.0f, {
        {"Otomotiv ve enerji kollarinda guclu ceyrek sonuclari.", 1.11f},
        {"Kur riski holding bilancosunu baskiladi.", 0.91f},
        {"Yeni bir yurtdisi sirket alimi acikandi.", 1.08f},
    }},
    {"BIMAS", "BIM Magazalar", {148, 103, 189, 255}, Sektor::PERAKENDE, 430.0f, 0.55f, {
        {"Magaza sayisi hizla artiyor, ciro rekor kirdi.", 1.10f},
        {"Enflasyon tuketici harcamalarini baskiliyor.", 0.92f},
        {"Yeni 'BIM Online' girisimi ilgi gordu.", 1.07f},
    }},
};

// ============================================================================
// YONETIM KARARLARI (bir sirketi DEVRALDIKTAN sonra periyodik olarak gelir)
// ============================================================================
// Her kararin iki secenegi var: RISKLI (hizli kar ama kalici risk/oynaklik
// birakir) ve GUVENLI (kucuk/yok etkili ama sirketi saglikli tutar).
struct YonetimSecenek {
    string etiket;          // buton uzerinde gorunen kisa metin
    float temelDegerCarpani; // fiyata VE adil degere uygulanan aninda etki
    float riskPrimiDegisimi; // stogun KALICI oynaklik tabanina eklenen/cikan miktar
    string logMetni;         // haber bandina yazilacak sonuc mesaji
};

struct YonetimOlaySablonu {
    string baslik;
    string aciklama;
    YonetimSecenek riskli;
    YonetimSecenek guvenli;
};

static const vector<YonetimOlaySablonu> YONETIM_OLAYLARI = {
    {
        "PERSONEL KARARI",
        "Giderleri kismak icin bir karar vermen gerekiyor.",
        {"Yarisini Isten Cikar", 1.15f, 0.012f, "calisanlarin yarisini isten cikardi! Karlar arti ama moral cokmus durumda."},
        {"Kadroyu Koru", 1.02f, 0.0f, "kadroyu korudu; tasarruf sinirli ama ekip motive."},
    },
    {
        "YATIRIM FIRSATI",
        "Sirket icin yeni bir buyume firsati cikti.",
        {"Agresif Genisleme", 1.18f, 0.015f, "agresif bir genislemeye gitti! Buyume hizlandi ama borc riski arti."},
        {"Temkinli Buyume", 1.04f, 0.0f, "temkinli bir buyume secti; istikrarli ama yavas ilerliyor."},
    },
    {
        "DENETIM RAPORU",
        "Ic denetimde kucuk bir usulsuzluk riski tespit edildi.",
        {"Ortbas Et", 1.10f, 0.02f, "raporu ortbas etti. Kisa vadede rahatladi ama skandal riski tavan yapti!"},
        {"Seffaf Ol ve Duzelt", 0.98f, -0.004f, "seffafligi secti; kucuk bir maliyeti oldu ama guveni arti."},
    },
    {
        "UCRET GORUSMESI",
        "Calisanlar zam talep ediyor.",
        {"Talebi Reddet", 1.08f, 0.01f, "zam talebini reddetti. Tasarruf sagladi ama grev riski dogdu."},
        {"Zam Ver", 0.97f, -0.003f, "calisanlara zam verdi; maliyetli oldu ama moral yerinde."},
    },
};

struct Rutbe {
    string ad;
    float esikNetDeger;
};

static const vector<Rutbe> RUTBELER = {
    {"Acemi Tuccar", 0},
    {"Deneyimli Yatirimci", 15000},
    {"Uzman Trader", 30000},
    {"Borsa Kaplani", 60000},
    {"Borsa Krali", 120000},
    {"Efsane Yatirimci", 250000},
};

// ============================================================================
// KURESEL (MAKRO) OLAYLAR - tek bir sirketi degil TUM piyasayi etkiler
// ============================================================================
// temelCarpan: TUM hisselerin temel degerine/fiyatina uygulanan etki, ama HER
// hissede kendi BETA'siyla olceklenerek (yuksek beta'li hisseler kuresel
// haberlerden orantili olarak daha cok etkilenir - tipki piyasaFaktoru gibi).
// kuresekRiskPrimiDegisimi: TUM hisselerin oynaklik TABANINA kalici olarak
// eklenen/cikan miktar (bkz. Oyun::kuresekRiskPrimi) - oyunu zamanla
// zorlastiran kalici etki budur.
struct KureselOlaySablonu {
    string metin;
    float temelCarpan;
    float kuresekRiskPrimiDegisimi;
    // Bu olaydan EKSTRA etkilenen sektor ve carpani (1.0 = ekstra etki yok).
    // Genel etkinin USTUNE uygulanir; Sektor::YOK ise olay tum sektorlere esit.
    Sektor vurgulananSektor = Sektor::YOK;
    float sektorCarpani = 1.0f;
};

static const vector<KureselOlaySablonu> KURESEL_OLAYLARI = {
    {"FED faiz artirdi! Kuresel piyasalarda satis dalgasi basladi.", 0.93f, 0.006f, Sektor::BANKA, 0.95f},
    {"FED faiz indirdi, kuresel piyasalarda risk istahi arti.", 1.06f, -0.003f, Sektor::BANKA, 1.05f},
    {"Hurmuz Bogazi'nda gerginlik - petrol tasimaciligi durduruldu! Petrol sokunda panik.", 0.90f, 0.010f, Sektor::HAVACILIK, 0.93f},
    {"Akdeniz'de bir petrol tankeri kaza yapti, tedarik zinciri endiseleri arti.", 0.95f, 0.006f, Sektor::HAVACILIK, 0.96f},
    {"Merkez Bankasi surpriz bir kemer sikma politikasi acikladi, piyasalar temkinli.", 0.96f, 0.004f},
    {"Kuresel enflasyon verileri beklentilerin altinda geldi, piyasalar rahatladi.", 1.05f, -0.002f, Sektor::PERAKENDE, 1.04f},
    {"Jeopolitik gerginlik tirmandi, yatirimcilar guvenli limana kaciyor.", 0.92f, 0.008f, Sektor::SAVUNMA, 1.08f},
    {"Buyuk bir kuresel ticaret anlasmasi imzalandi, iyimserlik piyasaya yayildi.", 1.06f, -0.002f, Sektor::SANAYI, 1.05f},
    {"Kuresel resesyon endiseleri piyasalari sarsti.", 0.91f, 0.007f, Sektor::PERAKENDE, 1.04f},
    {"Merkez Bankasi faiz indirimiyle piyasaya likidite pompaladi.", 1.04f, -0.002f},
};

// ============================================================================
// YARDIMCI FONKSIYONLAR
// ============================================================================
static float Yumusat(float mevcut, float hedef, float dt, float hiz) {
    // Us-tabanli yumusatma: her karede farkin bir kismini kapatir (frame-rate bagimsiz)
    float t = 1.0f - expf(-hiz * dt);
    return mevcut + (hedef - mevcut) * t;
}

// Yaklasik "can egrisi" (normal) dagilimli rastgele sayi: 4 tekduze [-1,1]
// ornegin ortalamasi, merkezi limit teoremiyle couple orta deger civarina
// yigilan, ama yine de nadiren buyuk sapmalar verebilen bir dagilim uretir.
// Piyasa/haber soklarinin "cogu kucuk, bazen buyuk" hissini vermesi icin
// duz (uniform) rastgeleden daha gercekci.
static float RastgeleGauss() {
    float toplam = 0.0f;
    for (int i = 0; i < 4; i++) toplam += (float)GetRandomValue(-1000, 1000) / 1000.0f;
    return toplam / 4.0f;
}

static string SayiFormatla(double deger, int ondalik = 2) {
    // 12345.678 -> "12.345,68"  (Turkce binlik/ondalik ayraci, harici locale gerekmez)
    bool negatif = deger < 0;
    if (negatif) deger = -deger;
    char tampon[64];
    snprintf(tampon, sizeof(tampon), "%.*f", ondalik, deger);
    string s(tampon);
    size_t noktaKonum = s.find('.');
    string tamKisim = (noktaKonum == string::npos) ? s : s.substr(0, noktaKonum);
    string ondalikKisim = (noktaKonum == string::npos) ? "" : s.substr(noktaKonum + 1);

    string sonuc;
    int sayac = 0;
    for (int i = (int)tamKisim.size() - 1; i >= 0; i--) {
        sonuc.push_back(tamKisim[i]);
        sayac++;
        if (sayac % 3 == 0 && i != 0) sonuc.push_back('.');
    }
    reverse(sonuc.begin(), sonuc.end());
    if (!ondalikKisim.empty()) sonuc += "," + ondalikKisim;
    return (negatif ? "-" : "") + sonuc;
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

// ============================================================================
// PROSEDUREL SES URETIMI  (harici .wav/.mp3 dosyasi gerekmez)
// ============================================================================
static Sound TonUret(const vector<pair<float, float>>& notalar, float sesSeviyesi = 0.32f) {
    const int ornekHizi = 44100;
    int toplamOrnek = 0;
    for (auto& n : notalar) toplamOrnek += (int)(n.second * ornekHizi);
    if (toplamOrnek <= 0) toplamOrnek = 1;

    vector<short> veri(toplamOrnek, 0);
    int yazilan = 0;
    for (auto& nota : notalar) {
        float frekans = nota.first;
        int adet = (int)(nota.second * ornekHizi);
        int fadeOrnek = (int)(0.01f * ornekHizi); // 10ms fade in/out -> "tik" sesini onler
        for (int i = 0; i < adet && yazilan < toplamOrnek; i++, yazilan++) {
            float t = (float)i / ornekHizi;
            float genlik = sesSeviyesi;
            if (i < fadeOrnek) genlik *= (float)i / fadeOrnek;
            if (i > adet - fadeOrnek) genlik *= (float)(adet - i) / fadeOrnek;
            float ornek = sinf(2.0f * PI * frekans * t) * genlik;
            veri[yazilan] = (short)(ornek * 32000.0f);
        }
    }

    Wave dalga{};
    dalga.frameCount = toplamOrnek;
    dalga.sampleRate = ornekHizi;
    dalga.sampleSize = 16;
    dalga.channels = 1;
    dalga.data = veri.data();
    // LoadSoundFromWave verileri kendi ic tamponuna kopyalar; fonksiyon donunce
    // 'veri' vektorunun serbest birakilmasi guvenlidir.
    return LoadSoundFromWave(dalga);
}

struct SesEfektleri {
    Sound alim, satimKar, satimZarar, haberDuyuru, haberEtki, rutbeAtlama, hataUyari, tikla;
    bool yuklendi = false;

    void Yukle() {
        // Ses cihazi gercekten hazir degilse (hoparlor yok, surucu sorunu vb.)
        // hic ses uretmeyi denemiyoruz; boylece PlaySound/UnloadSound gecersiz
        // bir Sound uzerinde asla cagrilmaz ve olasi bir sürücü/ortam sorunu
        // oyunu cokertmez, sadece sessiz calisir.
        if (!IsAudioDeviceReady()) {
            yuklendi = false;
            return;
        }
        alim = TonUret({{740.0f, 0.05f}, {988.0f, 0.06f}});
        satimKar = TonUret({{523.0f, 0.05f}, {659.0f, 0.05f}, {880.0f, 0.09f}});
        satimZarar = TonUret({{392.0f, 0.05f}, {330.0f, 0.09f}});
        haberDuyuru = TonUret({{523.0f, 0.05f}, {523.0f, 0.05f}});
        haberEtki = TonUret({{440.0f, 0.07f}});
        rutbeAtlama = TonUret({{523.0f, 0.08f}, {659.0f, 0.08f}, {784.0f, 0.08f}, {1046.0f, 0.16f}});
        hataUyari = TonUret({{200.0f, 0.12f}});
        tikla = TonUret({{900.0f, 0.02f}});
        yuklendi = true;
    }
    void Bosalt() {
        if (!yuklendi) return;
        UnloadSound(alim);
        UnloadSound(satimKar);
        UnloadSound(satimZarar);
        UnloadSound(haberDuyuru);
        UnloadSound(haberEtki);
        UnloadSound(rutbeAtlama);
        UnloadSound(hataUyari);
        UnloadSound(tikla);
        yuklendi = false;
    }
};

static bool sesAcik = true;
static SesEfektleri* g_sesler = nullptr; // Cal() icinde yuklendi bayragini kontrol edebilmek icin
static void Cal(Sound& s) {
    if (sesAcik && g_sesler && g_sesler->yuklendi) PlaySound(s);
}

// ============================================================================
// PARCACIK SISTEMI (alim/satim/haber anlarinda "juicy" efekt)
// ============================================================================
struct Parcacik {
    Vector2 pos;
    Vector2 hiz;
    float omur, maxOmur;
    float boyut;
    Color renk;
};

struct ParcacikSistemi {
    vector<Parcacik> liste;

    void Patlat(Vector2 merkez, Color renk, int adet = 18, float hiz = 220.0f) {
        for (int i = 0; i < adet; i++) {
            float aci = (float)GetRandomValue(0, 360) * DEG2RAD;
            float s = hiz * (0.4f + (float)GetRandomValue(0, 100) / 100.0f);
            Parcacik p;
            p.pos = merkez;
            p.hiz = {cosf(aci) * s, sinf(aci) * s - 60.0f};
            p.maxOmur = 0.5f + (float)GetRandomValue(0, 60) / 100.0f;
            p.omur = p.maxOmur;
            p.boyut = 2.0f + (float)GetRandomValue(0, 30) / 10.0f;
            p.renk = renk;
            liste.push_back(p);
        }
    }

    void Guncelle(float dt) {
        for (auto& p : liste) {
            p.pos.x += p.hiz.x * dt;
            p.pos.y += p.hiz.y * dt;
            p.hiz.y += 260.0f * dt; // yercekimi
            p.omur -= dt;
        }
        liste.erase(remove_if(liste.begin(), liste.end(),
                              [](const Parcacik& p) { return p.omur <= 0.0f; }),
                    liste.end());
    }

    void Ciz() {
        for (auto& p : liste) {
            float oran = p.omur / p.maxOmur;
            Color renk = Fade(p.renk, oran);
            DrawCircleV(p.pos, p.boyut * oran, renk);
        }
    }
};

// ============================================================================
// TOAST (BILDIRIM) SISTEMI
// ============================================================================
struct Toast {
    string baslik;
    string govde;
    Color renk;
    float omur;
    float maxOmur;
    AnimFloat kaymaX; // ekrana sagdan kayarak girer
};

struct ToastSistemi {
    vector<Toast> liste;

    void Ekle(const string& baslik, const string& govde, Color renk, float sure = 4.0f) {
        Toast t;
        t.baslik = baslik;
        t.govde = govde;
        t.renk = renk;
        t.maxOmur = sure;
        t.omur = sure;
        t.kaymaX.deger = 340.0f; // baslangicta ekran disinda (sagda)
        t.kaymaX.hiz = 8.0f;
        t.kaymaX.Ayarla(0.0f);
        liste.push_back(t);
        if (liste.size() > 4) liste.erase(liste.begin());
    }

    void Guncelle(float dt) {
        for (auto& t : liste) {
            t.kaymaX.Guncelle(dt);
            t.omur -= dt;
            if (t.omur < 0.6f) t.kaymaX.Ayarla(340.0f); // cikista tekrar kaysin
        }
        liste.erase(remove_if(liste.begin(), liste.end(),
                              [](const Toast& t) { return t.omur <= 0.0f; }),
                    liste.end());
    }

    void Ciz(int ekranGenislik) {
        float y = 118.0f;
        for (auto& t : liste) {
            float alpha = Clamp(t.omur / 0.5f, 0.0f, 1.0f);
            float x = (float)ekranGenislik - 320.0f + t.kaymaX.deger;
            Rectangle kutu = {x, y, 300.0f, 66.0f};
            DrawRectangleRounded({kutu.x + 3, kutu.y + 4, kutu.width, kutu.height}, 0.18f, 8,
                                 Fade(BLACK, 0.35f * alpha));
            DrawRectangleRounded(kutu, 0.18f, 8, Fade(RENK_PANEL_ACIK, alpha));
            DrawRectangleRounded({kutu.x, kutu.y, 6.0f, kutu.height}, 0.9f, 4, Fade(t.renk, alpha));
            DrawText(t.baslik.c_str(), (int)kutu.x + 18, (int)kutu.y + 10, 16, Fade(t.renk, alpha));
            DrawText(t.govde.c_str(), (int)kutu.x + 18, (int)kutu.y + 33, 14, Fade(RENK_METIN, alpha));
            y += 74.0f;
        }
    }
};

// ============================================================================
// BUTON YARDIMCISI (hover/press animasyonlu, "juicy" his icin)
// ============================================================================
struct Buton {
    Rectangle rect;
    string etiket;
    Color renk;
    AnimFloat olcek;
    bool aktif = true;

    Buton() { olcek.deger = 1.0f; olcek.hedef = 1.0f; olcek.hiz = 14.0f; }

    bool Guncelle(float dt, Vector2 fare, bool tiklandi) {
        bool ustunde = aktif && CheckCollisionPointRec(fare, rect);
        olcek.Ayarla(ustunde ? 1.06f : 1.0f);
        olcek.Guncelle(dt);
        return aktif && ustunde && tiklandi;
    }

    void Ciz(int fontBoyu = 16) const {
        float cx = rect.x + rect.width / 2.0f;
        float cy = rect.y + rect.height / 2.0f;
        float w = rect.width * olcek.deger;
        float h = rect.height * olcek.deger;
        Rectangle r = {cx - w / 2.0f, cy - h / 2.0f, w, h};
        Color c = aktif ? renk : Fade(renk, 0.4f);
        DrawRectangleRounded({r.x + 2, r.y + 3, r.width, r.height}, 0.3f, 8, Fade(BLACK, 0.35f));
        DrawRectangleRounded(r, 0.3f, 8, c);
        int tw = MeasureText(etiket.c_str(), fontBoyu);
        DrawText(etiket.c_str(), (int)(cx - tw / 2.0f), (int)(cy - fontBoyu / 2.0f), fontBoyu, WHITE);
    }
};

// ============================================================================
// HISSE (SIRKET) MODELI
// ============================================================================
struct Hisse {
    string kod, tamAd;
    Color renk;
    Sektor sektor = Sektor::YOK;
    float beta = 1.0f;
    float fiyat, oncekiFiyat;
    AnimFloat gosterilenFiyat;
    deque<float> gecmis;
    int adet = 0;
    float maliyet = 0.0f;
    float flashSuresi = 0.0f; // >0 ise kart yanip sonuyor
    bool flashIyi = true;

    // --- Daha gercekci piyasa modeli icin ---
    float temelDeger = 0.0f;   // fiyatin etrafinda dolanip durdugu "adil deger"
    float volatilite = 0.01f; // o an ki oynaklik (haber/sok sonrasi yukselir, zamanla soner)

    // --- Devralma (takeover) / yonetim mekanigi ---
    bool sahibiyim = false;        // toplam hissenin yarisindan fazlasi elimde mi?
    float riskPrimi = 0.0f;        // riskli yonetim kararlarindan biriken KALICI oynaklik
    float yonetimOlayZamanlayici = 0.0f; // sahipken bir sonraki yonetim karari ne zaman gelir

    // --- Kademeli acilma (progression) ---
    // Bu hisse hangi rutbe seviyesinde acilir? 0 = oyuna baslarken acik (en
    // dusuk baslangic fiyatina sahip hisse), 1 = ilk rutbe atlayisinda acilir,
    // vs. Oyun::Baslat() icinde SIRKETLER'in baslangicFiyat'ina gore siralanip
    // atanir (bkz. Oyun::HisseAcikMi()).
    int acilmaSeviyesi = 0;
};

// Duyurulmus ama henuz fiyata yansimamis haber
struct BeklemedeHaber {
    int hisseIndex;
    float carpan;
    string metin;
    float kalanSure;
};

// Duyurulmus ama henuz piyasaya yansimamis KURESEL (tum piyasayi etkileyen) olay
struct BeklemedeKuresel {
    float carpan;
    float kuresekRiskPrimiDegisimi;
    string metin;
    float kalanSure;
    // Sektorel ek etki. KAYIT DOSYASINA YAZILMAZ (kayit formatinin ortasindaki
    // tekrarli bloga alan eklemek eski kayitlari bozardi); yuklenen bekleyen bir
    // olay sektorel vurgusunu kaybeder, genel etkisi aynen uygulanir. En fazla
    // 6 saniyelik bir pencerede olabilecek, zararsiz bir kayip.
    Sektor vurgulananSektor = Sektor::YOK;
    float sektorCarpani = 1.0f;
};

// ============================================================================
// OYUN DURUMU
// ============================================================================
enum class Ekran { MENU, OYUN };

struct Oyun {
    Ekran ekran = Ekran::MENU;
    bool duraklatildi = false;

    float bakiye = BASLANGIC_BAKIYE;
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

    // Genel "piyasa modu": yavas degisen, kalici (mean-reverting) ortak bir
    // faktor. Tum hisseler buna kendi beta'lariyla tepki verir, boylece
    // hisseler artik birbirinden tamamen bagimsiz degil, gercek piyasalardaki
    // gibi zaman zaman BIRLIKTE yukselip birlikte duser.
    float piyasaFaktoru = 0.0f;

    // Kuresel (makro) olaylarin biriktirdigi KALICI oynaklik primi - TUM
    // hisselerin oynaklik tabanina eklenir (bkz. PiyasaGuncelle). Kotu kuresel
    // olaylar bunu yukseltir, iyi olaylar biraz azaltir; oyunu zamanla
    // zorlastiran/gevseten kalici bir "dunya riski" gostergesi.
    float kureselRiskPrimi = 0.0f;

    int rutbeIndex = 0;
    int hedefIndex = 0; // RUTBELER[hedefIndex+1] bir sonraki hedef

    int secilenAdet = 1;

    deque<string> haberBandi; // kayan bant icin son haberler
    float bantKaydirma = 0.0f;
    float haberSabitKalmaSuresi = 0.0f; // >0 iken bant KAYMAZ (haberi okuyabilesin diye)

    ParcacikSistemi parcaciklar;
    ToastSistemi toastlar;
    SesEfektleri sesler;

    float flashAlpha = 0.0f; // rutbe/hedef kutlamasinda ekran parlamasi
    Color flashRenk = RENK_ALTIN;

    bool ilkIslemYapildi = false;

    // --- Istatistikler (duraklatma ekraninda gosterilir) ---
    int islemSayisi = 0;             // toplam alim + satim
    float gerceklesenKar = 0.0f;     // sadece SATISLARDAN realize edilen kar/zarar
    float enIyiIslem = 0.0f;         // tek bir satistaki en yuksek kar
    float enKotuIslem = 0.0f;        // tek bir satistaki en buyuk zarar

    // --- Temettu ---
    int sonTemettuGunu = 0;          // en son hangi oyun gununde odendi

    // --- Grafik gorunumu ---
    // false = TL ekseni (mutlak fiyat), true = yuzde ekseni (her hisse pencerenin
    // basina gore normalize). 6 hisse acildiginda BIMAS (430 TL) ile EREGL (38 TL)
    // ayni eksende ezildigi icin ucuz hisseler duz cizgiye donuyordu.
    bool normalizeGrafik = false;

    // "Yeni Oyun" (sifirlama) artik iki asamali onay bekler: ilk tiklamada
    // buton "Emin misin?" haline gelir ve bu sayac calisir; sayac bitmeden
    // ikinci kez tiklanirsa oyun gercekten sifirlanir. Tek yanlis tikla tum
    // ilerlemenin silinmesini onler.
    float sifirlaOnaySuresi = 0.0f;

    bool ayarlarAcik = false; // ayarlar penceresi acik mi?

    // Cikis onayi: oyuncu pencereyi X ile kapatirken / Cikis'a basarken
    // kaydedilmemis ilerlemesini SESSIZCE kaybetmesin diye araya giren onay
    // penceresi. Hem ana menude hem oyun icinde calisir (bkz. main()).
    bool cikisOnayiAcik = false;

    // --- Iceriden bilgi (insider) mektubu ---
    float insiderSayaci = 0.0f;       // kontrol zamanlayicisi
    bool insiderMektupVar = false;    // ust barda tiklanabilir zarf duruyor mu
    float insiderMektupOmur = 0.0f;   // zarf kaybolmadan once kalan sure
    bool insiderOkundu = false;       // zarf acildi mi (bilgi seridi gosteriliyor)
    int insiderHisseIndex = -1;
    float insiderArtisOrani = 0.0f;   // beklenen yukselis (0.14 = %14)
    float insiderKalanSure = 0.0f;    // yukselise kalan sure (sadece gosterge)

    // --- Devralma / yonetim karari penceresi ---
    bool yonetimOlayiAcik = false;
    int yonetimHisseIndex = -1;
    int yonetimSablonIndex = -1;

    // --- Butonlar ---
    // hisseler.size() kadar (su an 6), ama SADECE o an ACIK olan hisseler icin
    // her karede pozisyonlandirilip guncellenir (bkz. main() icindeki kart cizim
    // dongusu) - kilitli hisselerin butonlari o karede hic dokunulmadigi icin
    // eski/gecersiz konumda kalir, bu yuzden Guncelle() de onlar icin cagrilmaz.
    vector<Buton> alButonlari;
    vector<Buton> satButonlari;
    Buton adetButonlari[8]; // 1,5,10,25,50,100,250,MAKS(-1)
    Buton sifirlaButonu, duraklatButonu, ayarlarButonu, kaydetButonu;
    Buton devamButonu;   // duraklatma panelinin EN USTUNDEKI "Devam Et" butonu
    Buton grafikModBtn;  // grafik ekseni: TL <-> %
    Buton anaMenuButonu, cikisButonu; // oyun ekranindan: menuye don / oyundan cik

    // Ayarlar penceresi ici butonlari
    Buton sesButonu;                 // Ses Ac/Kapat (artik ayarlar icinde)
    Buton modPencereliBtn, modTamEkranBtn, modKenarsizBtn, kucultBtn, ayarlarKapatBtn;

    // Yonetim karari penceresi ici butonlari
    Buton yonetimRiskliBtn, yonetimGuvenliBtn;

    // Cikis onayi penceresi ici butonlari
    Buton cikisKaydetBtn, cikisKaydetmeBtn, cikisVazgecBtn;

    void Baslat() {
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
            h.volatilite = BASE_VOLATILITE;
            h.gosterilenFiyat.Zorla(s.baslangicFiyat);
            h.gosterilenFiyat.hiz = 6.0f;
            for (int i = 0; i < GECMIS_UZUNLUK; i++) h.gecmis.push_back(s.baslangicFiyat);
            hisseler.push_back(h);
        }

        // Acilma sirasini belirle: indeksleri baslangicFiyat'a gore artan
        // sirada dizip, o siradaki konumu acilmaSeviyesi olarak ata (0 = en
        // ucuz -> oyuna baslarken acik, 1 = ilk rutbe atlayisinda acilir, ...).
        {
            vector<int> siraliIndeksler(hisseler.size());
            for (size_t i = 0; i < hisseler.size(); i++) siraliIndeksler[i] = (int)i;
            sort(siraliIndeksler.begin(), siraliIndeksler.end(), [&](int a, int b) {
                return SIRKETLER[a].baslangicFiyat < SIRKETLER[b].baslangicFiyat;
            });
            for (size_t sira = 0; sira < siraliIndeksler.size(); sira++)
                hisseler[siraliIndeksler[sira]].acilmaSeviyesi = (int)sira;
        }

        alButonlari.assign(hisseler.size(), Buton());
        satButonlari.assign(hisseler.size(), Buton());
        for (auto& b : alButonlari) { b.renk = RENK_YESIL; b.etiket = "AL"; }
        for (auto& b : satButonlari) { b.renk = RENK_KIRMIZI; b.etiket = "SAT"; }

        piyasaFaktoru = 0.0f;
        kureselRiskPrimi = 0.0f;
        bakiye = BASLANGIC_BAKIYE;
        gosterilenNetDeger.Zorla(BASLANGIC_BAKIYE);
        gosterilenBakiye.Zorla(BASLANGIC_BAKIYE);
        bekleyenHaberler.clear();
        bekleyenKureselHaberler.clear();
        piyasaSayaci = 0.0f;
        haberSayaci = 0.0f;
        kureselHaberSayaci = 0.0f;
        gunSayaci = 0.0f;
        gun = 1;
        rutbeIndex = 0;
        hedefIndex = 0;
        secilenAdet = 1;
        haberBandi.clear();
        haberBandi.push_back("BORSA PATRONU'NA HOS GELDIN - Piyasalar acildi, bol kazanclar!");
        bantKaydirma = 0.0f;
        haberSabitKalmaSuresi = 0.0f;
        ilkIslemYapildi = false;
        islemSayisi = 0;
        gerceklesenKar = 0.0f;
        enIyiIslem = 0.0f;
        enKotuIslem = 0.0f;
        sonTemettuGunu = 0;
        sifirlaOnaySuresi = 0.0f;
        cikisOnayiAcik = false;
        duraklatildi = false;
        yonetimOlayiAcik = false;
        yonetimHisseIndex = -1;
        yonetimSablonIndex = -1;
        insiderSayaci = 0.0f;
        insiderMektupVar = false;
        insiderMektupOmur = 0.0f;
        insiderOkundu = false;
        insiderHisseIndex = -1;
        insiderArtisOrani = 0.0f;
        insiderKalanSure = 0.0f;
    }

    float PortfoyDegeri() const {
        float toplam = 0.0f;
        for (auto& h : hisseler) toplam += h.fiyat * h.adet;
        return toplam;
    }

    float NetDeger() const { return bakiye + PortfoyDegeri(); }

    const vector<HaberSablonu>& HaberHavuzu(int idx) const { return SIRKETLER[idx].haberler; }

    // Bu hisse su anki rutbe seviyesinde acik (gorunur/islem yapilabilir) mi?
    bool HisseAcikMi(int idx) const { return rutbeIndex >= hisseler[idx].acilmaSeviyesi; }
};

// ============================================================================
// OYUN MANTIGI FONKSIYONLARI
// ============================================================================
static void HaberBandinaEkle(Oyun& o, const string& s) {
    o.haberBandi.push_back(s);
    if (o.haberBandi.size() > 12) o.haberBandi.pop_front();
    // Bant, en yeni haberi HEMEN goster diye basa sarilir; eskiden bant surekli
    // kayan tek bir uzun metin oldugu icin yeni bir haber en sona ekleniyordu
    // ve kullanici oraya gelene kadar (bazen hic gelmeden, cunku yeni haberler
    // araya girip metni surekli uzatiyordu) goremiyordu. Artik her yeni haberde
    // kaydirma sifirlanir ve metin en yeniden en eskiye dogru yazilir; boylece
    // en yeni haber ekrana HER ZAMAN saga girer girmez ilk gorulen sey olur.
    // ISTISNA: su an "SABIT KALMA" modundaysak (asagida SON DAKIKA haberi icin
    // ekrana tam oturtulup 5 saniye sabitlendi) bu kaydirmayi BOZMUYORUZ,
    // yoksa arada gelen bir alim/satim mesaji haberi ekrandan kaydirip goturur.
    if (o.haberSabitKalmaSuresi <= 0.0f) o.bantKaydirma = 0.0f;
}

static void ParlamaBaslat(Oyun& o, Color renk) {
    o.flashAlpha = 0.55f;
    o.flashRenk = renk;
}

static void RutbeVeHedefKontrolEt(Oyun& o) {
    float net = o.NetDeger();

    // Rutbe yukselisi
    int yeniRutbe = o.rutbeIndex;
    for (size_t i = 0; i < RUTBELER.size(); i++) {
        if (net >= RUTBELER[i].esikNetDeger) yeniRutbe = (int)i;
    }
    if (yeniRutbe > o.rutbeIndex) {
        int eskiRutbe = o.rutbeIndex;
        o.rutbeIndex = yeniRutbe;
        o.toastlar.Ekle("RUTBE ATLADIN!", RUTBELER[o.rutbeIndex].ad, RENK_ALTIN, 5.0f);
        HaberBandinaEkle(o, "TEBRIKLER: Artik bir '" + RUTBELER[o.rutbeIndex].ad + "' oldun!");
        ParlamaBaslat(o, RENK_ALTIN);
        Cal(o.sesler.rutbeAtlama);

        // ---- ZIRVE: son rutbeye ulasildi (oyunun "bitis" ani) ----
        // Oyun burada KAPANMAZ - oyuncu isterse imparatorlugunu buyutmeye
        // devam edebilir (kum havuzu). Ama bir "bitirdim" hissi olmadan
        // oyuncu zirveye ulastigini fark etmiyordu; artik ozel bir kutlama
        // ve haber bandi mesaji veriyoruz (bkz. ust bardaki "ZIRVE" etiketi).
        // Rutbe asla DUSMEDIGI icin bu blok bir oyunda en fazla bir kez calisir.
        if (o.rutbeIndex == (int)RUTBELER.size() - 1) {
            o.toastlar.Ekle("ZIRVEYE ULASTIN!", "Artik piyasanin patronu sensin.", RENK_ALTIN, 8.0f);
            HaberBandinaEkle(o, "BORSA TARIHINE GECTIN: " +
                                    SayiFormatla(RUTBELER.back().esikNetDeger, 0) +
                                    " TL net degerle 'Efsane Yatirimci' unvanini aldin! Oyunu bitirdin - "
                                    "dilersen imparatorlugunu buyutmeye devam et.");
            o.parcaciklar.Patlat({(float)PENCERE_GENISLIK / 2.0f, 260.0f}, RENK_ALTIN, 80, 320.0f);
        }

        // Bu rutbe siramasinda acilan yeni hisse(ler) var mi? (birden fazla
        // rutbe birden atlanirsa - orn. buyuk bir satis sonrasi - araliktaki
        // hepsi tek seferde acilir).
        for (auto& h : o.hisseler) {
            if (h.acilmaSeviyesi > eskiRutbe && h.acilmaSeviyesi <= yeniRutbe) {
                o.toastlar.Ekle("YENI HISSE ACILDI!", h.kod + " - " + h.tamAd, h.renk, 5.0f);
                HaberBandinaEkle(o, "YENI HISSE: " + h.kod + " (" + h.tamAd + ") artik islem gorebiliyor!");
            }
        }
    }

    // Hedef tamamlama (rutbe esikleriyle ayni listeyi hedef olarak kullaniyoruz,
    // bir sonrakini gostererek surekli bir "sirada ne var" hissi veriyor)
    if (o.hedefIndex + 1 < (int)RUTBELER.size() && net >= RUTBELER[o.hedefIndex + 1].esikNetDeger) {
        o.hedefIndex++;
        if (o.hedefIndex + 1 < (int)RUTBELER.size()) {
            o.toastlar.Ekle("HEDEF TAMAMLANDI!",
                            "Yeni hedef: " + SayiFormatla(RUTBELER[o.hedefIndex + 1].esikNetDeger, 0) + " TL",
                            RENK_YESIL, 5.0f);
        }
    }
}

// Alim/satim sonrasi cagrilir: sahiplik %50'yi gecti/altina dustu mu kontrol
// eder ve DEVRALMA / KONTROL KAYBI olaylarini tetikler.
static void DevralmaKontrolEt(Oyun& o, int idx) {
    Hisse& h = o.hisseler[idx];
    bool coguldaMi = (h.adet * 2 > TOPLAM_HISSE_SAYISI);
    if (coguldaMi && !h.sahibiyim) {
        h.sahibiyim = true;
        h.yonetimOlayZamanlayici = (float)GetRandomValue((int)(YONETIM_OLAY_MIN_SN * 10), (int)(YONETIM_OLAY_MAX_SN * 10)) / 10.0f;
        o.toastlar.Ekle("SIRKETI DEVRALDIN!", h.kod + " artik senin kontrolunde.", RENK_ALTIN, 6.0f);
        HaberBandinaEkle(o, "DEVIR ALINDI: " + to_string(h.adet) + "/" + to_string(TOPLAM_HISSE_SAYISI) +
                                 " hisseyle " + h.kod + "'nin patronu oldun!");
        o.parcaciklar.Patlat({(float)PENCERE_GENISLIK / 2.0f, 200.0f}, RENK_ALTIN, 40, 260.0f);
        ParlamaBaslat(o, RENK_ALTIN);
        Cal(o.sesler.rutbeAtlama);
    } else if (!coguldaMi && h.sahibiyim) {
        h.sahibiyim = false;
        o.toastlar.Ekle("KONTROLU KAYBETTIN", h.kod + " artik senin kontrolunde degil.", RENK_KIRMIZI, 5.0f);
        HaberBandinaEkle(o, h.kod + "'DA COGUNLUK HISSEYI KAYBETTIN, kontrol elden gitti.");
    }
}

// Emir buyuklugunun fiyata etkisi - KAREKOK yasasi (bkz. yukaridaki not).
// adet = TOPLAM_HISSE_SAYISI oldugunda tam TICARET_ETKI_OLCEK doner; adet zaten
// bu sayiyla sinirli oldugu icin ayrica bir tavana gerek yok.
static float TicaretEtkisi(int adet) {
    if (adet <= 0) return 0.0f;
    return TICARET_ETKI_OLCEK * sqrtf((float)adet / (float)TOPLAM_HISSE_SAYISI);
}

// Bu emrin GERCEKTEN gerceklesecegi birim fiyat. Alimda piyasa fiyatinin USTUNDE,
// satista ALTINDA - yani islem maliyetini oyuncu oder. Piyasa fiyatinin kendisi
// ayrica (ve tam etki kadar) HisseAl/HisseSat icinde kaydirilir.
static float DolumFiyati(float piyasaFiyati, int adet, bool alim) {
    float sapma = ALIM_SATIM_SPREAD * 0.5f + TicaretEtkisi(adet) * ETKI_DOLUM_PAYI;
    return fmaxf(0.01f, piyasaFiyati * (alim ? (1.0f + sapma) : (1.0f - sapma)));
}

// Bu islemde kac adet el degistirecek? secilenAdet > 0 ise dogrudan o; MAKS
// (-1) modunda alimda bakiyenin yettigi (ve sirkette kalan) kadar, satista
// elde bulunanin tamami.
static int IslemAdedi(const Oyun& o, const Hisse& h, bool alim) {
    if (o.secilenAdet > 0) return o.secilenAdet;
    if (!alim) return h.adet;
    if (h.fiyat <= 0.01f) return 0;
    // Dolum fiyati adede bagli oldugu icin "bakiye / fiyat" dogru cevabi vermez
    // (daha cok alirsan birim fiyat da yukselir). Birkac adimda asagidan yakinsiyoruz.
    int alinabilir = (int)(o.bakiye / h.fiyat);
    for (int i = 0; i < 4; i++) {
        float birim = DolumFiyati(h.fiyat, alinabilir, true);
        alinabilir = (birim > 0.01f) ? (int)(o.bakiye / birim) : 0;
    }
    return max(0, min(alinabilir, TOPLAM_HISSE_SAYISI - h.adet));
}

static void HisseAl(Oyun& o, int idx) {
    Hisse& h = o.hisseler[idx];
    int adet = IslemAdedi(o, h, true);
    if (adet <= 0) {
        o.toastlar.Ekle("Islem Yapilamadi", h.kod + " icin yeterli bakiye yok.", RENK_KIRMIZI, 3.0f);
        Cal(o.sesler.hataUyari);
        return;
    }
    if (h.adet + adet > TOPLAM_HISSE_SAYISI) {
        o.toastlar.Ekle("Hisse Kalmadi", h.kod + "'nin toplam " + to_string(TOPLAM_HISSE_SAYISI) +
                        " hissesi var, bu kadarini alamazsin.", RENK_KIRMIZI, 3.5f);
        Cal(o.sesler.hataUyari);
        return;
    }
    // Dolum fiyati = piyasa fiyati + spread/2 + etkinin yarisi (oyuncu oder).
    float etki = TicaretEtkisi(adet);
    float dolum = DolumFiyati(h.fiyat, adet, true);
    float toplam = dolum * adet;
    if (toplam > o.bakiye) {
        o.toastlar.Ekle("Yetersiz Bakiye", h.kod + " icin " + SayiFormatla(toplam) + " TL gerekiyor.",
                        RENK_KIRMIZI, 3.0f);
        Cal(o.sesler.hataUyari);
        return;
    }
    o.bakiye -= toplam;
    h.maliyet += toplam; // maliyet tabani islem maliyetini de icerir (gercekci)
    h.adet += adet;
    o.islemSayisi++;
    float maliyetYuzde = (dolum / h.fiyat - 1.0f) * 100.0f;
    HaberBandinaEkle(o, "ALIM: " + to_string(adet) + " adet " + h.kod + " @ " + SayiFormatla(dolum) +
                             " TL (+%" + SayiFormatla(maliyetYuzde, 2) + " islem maliyeti)");
    Cal(o.sesler.alim);

    // PIYASA ETKISI: kendi buyuk alimin bir sonraki fiyati hafifce yukari
    // iter (gercek piyasalarda buyuk emirler kendi aleyhine fiyat hareket
    // ettirir - "sonsuz bedava kar" icin surekli max adet basmayi anlamsizlastirir).
    // Piyasa fiyati TAM etki kadar yukselir (dolumda yarisini odedin).
    h.fiyat *= (1.0f + etki);
    h.gosterilenFiyat.Ayarla(h.fiyat);
    if (!o.ilkIslemYapildi) {
        o.ilkIslemYapildi = true;
        o.toastlar.Ekle("Ilk Islemin!", "Trader kariyerine hos geldin.", RENK_MAVI_VURGU, 4.0f);
    }
    DevralmaKontrolEt(o, idx);
    RutbeVeHedefKontrolEt(o);
}

static void HisseSat(Oyun& o, int idx) {
    Hisse& h = o.hisseler[idx];
    int adet = IslemAdedi(o, h, false);
    if (adet <= 0) {
        o.toastlar.Ekle("Yetersiz Hisse", "Elinde hic " + h.kod + " yok.", RENK_KIRMIZI, 3.0f);
        Cal(o.sesler.hataUyari);
        return;
    }
    if (h.adet < adet) {
        o.toastlar.Ekle("Yetersiz Hisse", "Elinde sadece " + to_string(h.adet) + " adet " + h.kod + " var.",
                        RENK_KIRMIZI, 3.0f);
        Cal(o.sesler.hataUyari);
        return;
    }
    float ortMaliyet = (h.adet > 0) ? (h.maliyet / h.adet) : 0.0f;
    // Satista da dolum piyasa fiyatinin ALTINDA gerceklesir.
    float etki = TicaretEtkisi(adet);
    float dolum = DolumFiyati(h.fiyat, adet, false);
    float toplam = dolum * adet;
    float kar = (dolum - ortMaliyet) * adet;
    h.maliyet -= ortMaliyet * adet;
    h.adet -= adet;
    o.bakiye += toplam;
    o.islemSayisi++;
    o.gerceklesenKar += kar;
    o.enIyiIslem = fmaxf(o.enIyiIslem, kar);
    o.enKotuIslem = fminf(o.enKotuIslem, kar);
    HaberBandinaEkle(o, "SATIS: " + to_string(adet) + " adet " + h.kod + " @ " + SayiFormatla(dolum) +
                             " TL (" + (kar >= 0 ? "+" : "") + SayiFormatla(kar) + " TL, -%" +
                             SayiFormatla((1.0f - dolum / h.fiyat) * 100.0f, 2) + " islem maliyeti)");
    Cal(kar >= 0 ? o.sesler.satimKar : o.sesler.satimZarar);

    // PIYASA ETKISI: buyuk bir satis da fiyati asagi bastirir (bkz. HisseAl).
    h.fiyat *= (1.0f - etki);
    h.fiyat = fmaxf(0.5f, h.fiyat);
    h.gosterilenFiyat.Ayarla(h.fiyat);
    if (!o.ilkIslemYapildi) {
        o.ilkIslemYapildi = true;
        o.toastlar.Ekle("Ilk Islemin!", "Trader kariyerine hos geldin.", RENK_MAVI_VURGU, 4.0f);
    }
    DevralmaKontrolEt(o, idx);
    RutbeVeHedefKontrolEt(o);
}

static void PiyasaGuncelle(Oyun& o) {
    // 1) GENEL PIYASA MODU: yavas degisen, kalici (mean-reverting) ortak bir
    // faktor. Bu, hisselerin birbirinden tamamen BAGIMSIZ olmasini onler;
    // gercek borsalarda oldugu gibi bazen hepsi birlikte yukselir/duser.
    o.piyasaFaktoru = o.piyasaFaktoru * MARKET_KALICILIK + RastgeleGauss() * MARKET_SOK_BUYUKLUK;

    for (auto& h : o.hisseler) {
        h.oncekiFiyat = h.fiyat;

        // 2) Kendine ozgu gurultu, o an ki oynakliga gore (sakin/firtinali donemler).
        float ideosinkratik = RastgeleGauss() * h.volatilite;

        // 3) Genel piyasa hareketinden kendi beta'sina gore pay alir.
        float piyasaEtkisi = o.piyasaFaktoru * h.beta;

        // 4) "Adil deger"ine dogru hafif bir cekim: fiyat cok sapmissa geri
        // cagirir, ama her turlu ongorulemezligi bitirmeyecek kadar yumusaktir.
        float sapmaOrani = (h.fiyat - h.temelDeger) / h.temelDeger;
        float cekimEtkisi = -sapmaOrani * REVERSION_GUCU;

        float toplamOran = ideosinkratik + piyasaEtkisi + cekimEtkisi;
        h.fiyat = fmaxf(0.5f, h.fiyat * (1.0f + toplamOran));
        h.gecmis.push_back(h.fiyat);
        if ((int)h.gecmis.size() > GECMIS_UZUNLUK) h.gecmis.pop_front();
        h.gosterilenFiyat.Ayarla(h.fiyat);

        // 5) Oynaklik kumelenmesi (basitlestirilmis GARCH): buyuk bir hareket
        // sonrasi oynaklik sicrar, sakin turlerde yavasca tabanina soner.
        if (fabsf(toplamOran) > VOL_SOK_ESIGI) {
            h.volatilite = fminf(VOL_TAVAN, h.volatilite * VOL_SOK_CARPANI);
        } else {
            // riskPrimi: riskli yonetim kararlarinin kalici bedeli (bu HISSEYE
            // ozel). kureselRiskPrimi: kuresel/makro olaylarin kalici bedeli
            // (TUM hisselere ortak) - ikisi de taban oynakligi yukseltir ve
            // asla eskisi kadar sakinlesmez. VOL_TAVAN ile sinirlandirilir ki
            // cok fazla birikince taban tavani asip mantiksiz olmasin.
            float taban = fminf(VOL_TABAN + h.riskPrimi + o.kureselRiskPrimi, VOL_TAVAN);
            h.volatilite = fmaxf(taban, h.volatilite * VOL_SONME_HIZI);
        }
    }
    RutbeVeHedefKontrolEt(o);
}

// Rutbe ilerledikce 1.0'dan 0.0'a inen "acemi korumasi" katsayisi.
// Rutbe 0 -> 1.0 (tam koruma), son rutbe -> 0.0 (koruma yok, eski denge).
static float AcemiKorumasi(const Oyun& o) {
    int sonRutbe = (int)RUTBELER.size() - 1;
    if (sonRutbe <= 0) return 0.0f;
    return Clamp(1.0f - (float)o.rutbeIndex / (float)sonRutbe, 0.0f, 1.0f);
}

// Bir olay havuzundan, IYI (carpan > 1) olaylara 'iyiSans' olasilikla agirlik
// vererek bir eleman secer ve indeksini doner. Istenen turden olay havuzda yoksa
// digerine duser - yani havuz tek tarafliysa fonksiyon yine de calisir.
// carpanAl: sablondan "fiyat carpani" alanini okuyan lambda (sirket haberinde
// .carpan, kuresel olayda .temelCarpan oldugu icin sablon parametresi).
template <typename T, typename CarpanAl>
static int AgirlikliOlaySec(const vector<T>& havuz, float iyiSans, CarpanAl carpanAl) {
    if (havuz.empty()) return 0;
    vector<int> iyiler, kotular;
    for (int i = 0; i < (int)havuz.size(); i++) {
        if (carpanAl(havuz[i]) > 1.0f) iyiler.push_back(i);
        else kotular.push_back(i);
    }
    bool iyiIstiyoruz = ((float)GetRandomValue(0, 1000) / 1000.0f) < iyiSans;
    const vector<int>* secilenGrup = nullptr;
    if (iyiIstiyoruz && !iyiler.empty()) secilenGrup = &iyiler;
    else if (!iyiIstiyoruz && !kotular.empty()) secilenGrup = &kotular;
    else secilenGrup = iyiler.empty() ? &kotular : &iyiler;
    if (secilenGrup->empty()) return GetRandomValue(0, (int)havuz.size() - 1);
    return (*secilenGrup)[GetRandomValue(0, (int)secilenGrup->size() - 1)];
}

static void HaberKontrolEt(Oyun& o) {
    if ((float)GetRandomValue(0, 1000) / 1000.0f >= HABER_OLASILIK) return;

    // Haberler sadece SU AN ACIK olan hisseler icin cikar - henuz kilitli bir
    // hisse hakkinda haber gorup oyuncuyu spoiler'lamayalim.
    vector<int> acikIndeksler;
    for (int i = 0; i < (int)o.hisseler.size(); i++)
        if (o.HisseAcikMi(i)) acikIndeksler.push_back(i);
    if (acikIndeksler.empty()) return;
    int idx = acikIndeksler[GetRandomValue(0, (int)acikIndeksler.size() - 1)];
    const auto& havuz = o.HaberHavuzu(idx);
    float iyiSans = ACEMI_HISSE_IYI_TABAN + ACEMI_HISSE_IYI_BONUS * AcemiKorumasi(o);
    const auto& sablon = havuz[AgirlikliOlaySec(havuz, iyiSans,
                                                [](const HaberSablonu& h) { return h.carpan; })];

    BeklemedeHaber bh;
    bh.hisseIndex = idx;
    bh.carpan = sablon.carpan;
    bh.metin = sablon.metin;
    bh.kalanSure = HABER_ETKI_GECIKME_SN;
    o.bekleyenHaberler.push_back(bh);

    HaberBandinaEkle(o, "SON DAKIKA [" + o.hisseler[idx].kod + "]: " + sablon.metin + "  (" +
                             to_string((int)HABER_ETKI_GECIKME_SN) + " sn icinde fiyata yansiyacak)");
    Cal(o.sesler.haberDuyuru);

    // Haberi okuyabilmen icin: bandi, bu haber ekranin SOL kenarindan tam
    // gorunecek sekilde konumlandirip HABER_ETKI_GECIKME_SN saniye boyunca
    // SABITLIYORUZ (kaydirmiyoruz). Sure dolup fiyata etki edince (bkz.
    // BekleyenHaberleriGuncelle) bant tekrar normal kaymaya baslar.
    o.bantKaydirma = (float)PENCERE_GENISLIK;
    o.haberSabitKalmaSuresi = HABER_ETKI_GECIKME_SN;
}

static void BekleyenHaberleriGuncelle(Oyun& o, float dt) {
    for (auto it = o.bekleyenHaberler.begin(); it != o.bekleyenHaberler.end();) {
        it->kalanSure -= dt;
        if (it->kalanSure <= 0.0f) {
            Hisse& h = o.hisseler[it->hisseIndex];
            h.fiyat = fmaxf(0.5f, h.fiyat * it->carpan);
            // Haber sadece anlik fiyati degil, hissenin "adil degerini" de
            // kaydirir - yoksa fiyat birkac saniye icinde reversion cekimiyle
            // sanki haber hic olmamis gibi eski yerine geri surunur.
            h.temelDeger = fmaxf(0.5f, h.temelDeger * it->carpan);
            // Haber sonrasi belirsizlik artar: oynaklik gecici olarak yukselir.
            h.volatilite = fminf(VOL_TAVAN, h.volatilite * VOL_SOK_CARPANI);
            h.gosterilenFiyat.Ayarla(h.fiyat);
            h.flashSuresi = 0.6f;
            h.flashIyi = it->carpan > 1.0f;
            Cal(o.sesler.haberEtki);
            it = o.bekleyenHaberler.erase(it);
        } else {
            ++it;
        }
    }
}

// Iceriden bilgi mektubu: ust seviyelerde, nadiren tetiklenir. Yukselisi
// SIMDIDEN planlar (bkz. yukaridaki tasarim notu) ve ust barda tiklanabilir bir
// zarf birakir.
static void InsiderMektupKontrolEt(Oyun& o) {
    if ((float)GetRandomValue(0, 1000) / 1000.0f >= INSIDER_OLASILIK) return;

    vector<int> acikIndeksler;
    for (int i = 0; i < (int)o.hisseler.size(); i++)
        if (o.HisseAcikMi(i)) acikIndeksler.push_back(i);
    if (acikIndeksler.empty()) return;

    int idx = acikIndeksler[GetRandomValue(0, (int)acikIndeksler.size() - 1)];
    float t1 = (float)GetRandomValue(0, 1000) / 1000.0f;
    float t2 = (float)GetRandomValue(0, 1000) / 1000.0f;
    float artis = INSIDER_ARTIS_MIN + (INSIDER_ARTIS_MAX - INSIDER_ARTIS_MIN) * t1;
    float gecikme = INSIDER_GECIKME_MIN_SN + (INSIDER_GECIKME_MAX_SN - INSIDER_GECIKME_MIN_SN) * t2;

    // Yukselisi simdiden kuyruga koy: mektup okunmasa da gerceklesecek.
    BeklemedeHaber bh;
    bh.hisseIndex = idx;
    bh.carpan = 1.0f + artis;
    bh.metin = "(iceriden bilgi)";
    bh.kalanSure = gecikme;
    o.bekleyenHaberler.push_back(bh);

    o.insiderMektupVar = true;
    o.insiderMektupOmur = INSIDER_MEKTUP_OMUR_SN;
    o.insiderOkundu = false;
    o.insiderHisseIndex = idx;
    o.insiderArtisOrani = artis;
    o.insiderKalanSure = gecikme;

    // Haber bandina BILEREK yazilmaz - bu bilgi sadece SANA geliyor.
    o.toastlar.Ekle("GIZLI ZARF", "Ust barda seni bekleyen bir mektup var.", RENK_ALTIN, 5.0f);
    Cal(o.sesler.haberDuyuru);
}

// Kuresel/makro olay kontrolu: HABER_TICK_SN'den daha seyrek ve TUM piyasayi
// birden etkiler (bkz. KURESEL_OLAYLARI). Tek bir hisse degil, TUM hisseler
// kendi beta'lariyla orantili tepki verir.
static void KureselHaberKontrolEt(Oyun& o) {
    if ((float)GetRandomValue(0, 1000) / 1000.0f >= KURESEL_OLAY_OLASILIK) return;

    float iyiSans = ACEMI_KURESEL_IYI_TABAN + ACEMI_KURESEL_IYI_BONUS * AcemiKorumasi(o);
    const auto& sablon = KURESEL_OLAYLARI[AgirlikliOlaySec(
        KURESEL_OLAYLARI, iyiSans, [](const KureselOlaySablonu& k) { return k.temelCarpan; })];

    BeklemedeKuresel bk;
    bk.carpan = sablon.temelCarpan;
    bk.kuresekRiskPrimiDegisimi = sablon.kuresekRiskPrimiDegisimi;
    bk.metin = sablon.metin;
    bk.kalanSure = KURESEL_OLAY_ETKI_GECIKME_SN;
    bk.vurgulananSektor = sablon.vurgulananSektor;
    bk.sektorCarpani = sablon.sektorCarpani;
    o.bekleyenKureselHaberler.push_back(bk);

    // BILEREK renk (yesil/kirmizi) KULLANMIYORUZ: kuresel haberin iyi mi kotu
    // mu oldugunu onceden ele vermek oyunu kolaylastirirdi - oyuncu haberi
    // OKUYUP kendi karar vermeli, sonucu ancak KURESEL_OLAY_ETKI_GECIKME_SN
    // saniye sonra fiyatlara yansiyinca ogrenecek.
    HaberBandinaEkle(o, "KURESEL GELISME: " + sablon.metin + "  (" +
                             to_string((int)KURESEL_OLAY_ETKI_GECIKME_SN) + " sn icinde piyasaya yansiyacak)");
    o.toastlar.Ekle("KURESEL GELISME!", sablon.metin, RENK_UYARI, 6.0f);
    Cal(o.sesler.haberDuyuru);
    ParlamaBaslat(o, RENK_UYARI);

    // Ayni sekilde haberi okuyabilmen icin bandi SABITLE (bkz. HaberKontrolEt).
    o.bantKaydirma = (float)PENCERE_GENISLIK;
    o.haberSabitKalmaSuresi = KURESEL_OLAY_ETKI_GECIKME_SN;
}

static void BekleyenKureselHaberleriGuncelle(Oyun& o, float dt) {
    for (auto it = o.bekleyenKureselHaberler.begin(); it != o.bekleyenKureselHaberler.end();) {
        it->kalanSure -= dt;
        if (it->kalanSure <= 0.0f) {
            for (auto& h : o.hisseler) {
                // Kuresel etki HER hissede kendi BETA'siyla olceklenir - tipki
                // piyasaFaktoru gibi (yuksek beta'li hisseler orantili daha
                // cok etkilenir, dusuk beta'lilar daha az sarsilir).
                float etki = (it->carpan - 1.0f) * h.beta;
                // SEKTOREL EK ETKI: olay belirli bir sektoru vurguluyorsa o
                // sektordeki hisse ekstra (bazen TERS yonde) etkilenir - orn.
                // jeopolitik gerginlik piyasayi dusururken savunmayi yukseltir.
                if (it->vurgulananSektor != Sektor::YOK && h.sektor == it->vurgulananSektor)
                    etki += (it->sektorCarpani - 1.0f);
                h.fiyat = fmaxf(0.5f, h.fiyat * (1.0f + etki));
                h.temelDeger = fmaxf(0.5f, h.temelDeger * (1.0f + etki));
                h.volatilite = fminf(VOL_TAVAN, h.volatilite * VOL_SOK_CARPANI);
                h.gosterilenFiyat.Ayarla(h.fiyat);
                h.flashSuresi = 0.6f;
                h.flashIyi = it->carpan >= 1.0f;
            }
            // Kalici kuresel risk primi guncellenir (oyunu zamanla zorlastiran/
            // gevseten kalici etki - bkz. PiyasaGuncelle'deki oynaklik tabani).
            o.kureselRiskPrimi = Clamp(o.kureselRiskPrimi + it->kuresekRiskPrimiDegisimi,
                                       KURESEL_RISK_PRIMI_TABANI, KURESEL_RISK_PRIMI_TAVANI);
            Cal(o.sesler.haberEtki);
            it = o.bekleyenKureselHaberler.erase(it);
        } else {
            ++it;
        }
    }
}

// ============================================================================
// KAYIT / YUKLEME (SAVE / LOAD) SISTEMI
// ============================================================================
// Basit, insan tarafindan da okunabilir, satir-satir bir metin formati.
// Her deger kendi satirinda; bu sayede >> ile getline() karisip senkron
// kaymasi gibi klasik bir hataya dusmeden hepsini tek tip (getline + parse)
// okuyabiliyoruz. Ilk satir bir surum etiketi: ileride format degisirse
// eski kayitlari tanimak/reddetmek icin kullanilir.
static const char* KAYIT_SURUM_ETIKETI = "BORSAPATRONU_SAVE_V4";
static const char* KAYIT_SURUM_ETIKETI_V3 = "BORSAPATRONU_SAVE_V3";
// V3, V2'ye gore SADECE dosyanin SONUNA alan ekledi (kuresel olay sayaci +
// ses ayarlari). Bu yuzden V2 kayitlari HALA YUKLENEBILIR: ortak kisim ayni
// sirayla okunur, eksik alanlar varsayilaninda birakilir ve oyun bir dahaki
// kaydedisinde dosya kendiliginden V3'e yukselir. (V1 hala reddedilir - o
// formatta 3 hisse vardi, ortak kisim bile uyusmuyor.)
static const char* KAYIT_SURUM_ETIKETI_V2 = "BORSAPATRONU_SAVE_V2";

static bool KayitDosyasiVarMi() {
    ifstream f(KayitYolu());
    return f.good();
}

static bool OyunuKaydet(const Oyun& o, const string& dosyaAdi) {
    ofstream f(dosyaAdi, ios::trunc);
    if (!f.is_open()) return false;
    f << KAYIT_SURUM_ETIKETI << "\n";
    f.precision(9);

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
        f << bk.kuresekRiskPrimiDegisimi << "\n";
        f << bk.kalanSure << "\n";
        f << bk.metin << "\n";
    }

    // ---- V3 ile eklenen alanlar ----
    // Dosyanin EN SONUNA eklendiler ki onlerindeki her sey V2 ile birebir ayni
    // sirada kalsin ve eski V2 kayitlari yuklenmeye devam edebilsin.
    // kureselHaberSayaci eskiden hic kaydedilmiyordu (piyasaSayaci/haberSayaci
    // kaydedilirken atlanmisti), bu yuzden yuklemeden sonra kuresel olay
    // zamanlamasi hep sifirdan basliyordu.
    f << o.kureselHaberSayaci << "\n";
    f << (sesAcik ? 1 : 0) << "\n";
    f << (IsAudioDeviceReady() ? GetMasterVolume() : 1.0f) << "\n";

    // ---- V4 ile eklenen alanlar (yine dosyanin SONUNA) ----
    // Istatistikler, temettu takvimi ve grafik ekseni tercihi.
    f << o.islemSayisi << "\n";
    f << o.gerceklesenKar << "\n";
    f << o.enIyiIslem << "\n";
    f << o.enKotuIslem << "\n";
    f << o.sonTemettuGunu << "\n";
    f << (o.normalizeGrafik ? 1 : 0) << "\n";

    return f.good();
}

// Yukleme basarisiz olursa Oyun tamamen degismeden birakilir (bozuk/eski
// bir dosya oyunu yari bir durumda birakip cokertmesin diye).
static bool OyunuYukle(Oyun& o, const string& dosyaAdi) {
    ifstream f(dosyaAdi);
    if (!f.is_open()) return false;

    auto satirOku = [&](string& hedef) -> bool { return (bool)std::getline(f, hedef); };

    string satir;
    if (!satirOku(satir)) return false;
    // Uc surum de kabul edilir: V4 (guncel), V3 ve V2. Her yeni surum sadece
    // dosyanin SONUNA alan ekledigi icin ortak kisim ayni sirada okunur; eksik
    // alanlar varsayilaninda kalir ve dosya bir dahaki kaydediste V4'e yukselir.
    const bool surumV4 = (satir == KAYIT_SURUM_ETIKETI);
    const bool surumV3 = (satir == KAYIT_SURUM_ETIKETI_V3);
    const bool surumV2 = (satir == KAYIT_SURUM_ETIKETI_V2);
    if (!surumV4 && !surumV3 && !surumV2) return false; // taninmayan / cok eski (V1)

    try {
        Oyun yeni = o; // sadece butonlar/statik alanlari korumak icin baslangic kopyasi

        auto okuStr = [&]() -> string {
            string s;
            if (!satirOku(s)) throw runtime_error("kayit dosyasi eksik/bozuk");
            return s;
        };
        auto okuFloat = [&]() -> float { return stof(okuStr()); };
        auto okuInt = [&]() -> int { return stoi(okuStr()); };

        yeni.bakiye = okuFloat();
        yeni.gun = okuInt();
        yeni.gunSayaci = okuFloat();
        yeni.piyasaSayaci = okuFloat();
        yeni.haberSayaci = okuFloat();
        yeni.piyasaFaktoru = okuFloat();
        yeni.kureselRiskPrimi = okuFloat();
        yeni.rutbeIndex = okuInt();
        yeni.hedefIndex = okuInt();
        yeni.secilenAdet = okuInt();
        yeni.ilkIslemYapildi = (okuInt() != 0);
        yeni.duraklatildi = (okuInt() != 0);

        int hisseSayisi = okuInt();
        if (hisseSayisi != (int)yeni.hisseler.size()) throw runtime_error("hisse sayisi uyusmuyor");
        for (auto& h : yeni.hisseler) {
            h.fiyat = okuFloat();
            h.oncekiFiyat = okuFloat();
            h.adet = okuInt();
            h.maliyet = okuFloat();
            h.temelDeger = okuFloat();
            h.volatilite = okuFloat();
            h.sahibiyim = (okuInt() != 0);
            h.riskPrimi = okuFloat();
            h.yonetimOlayZamanlayici = okuFloat();
            int gecmisSayisi = okuInt();
            string gecmisSatir = okuStr();
            h.gecmis.clear();
            istringstream iss(gecmisSatir);
            float v;
            while (iss >> v) h.gecmis.push_back(v);
            if (h.gecmis.empty()) h.gecmis.push_back(h.fiyat); // guvenlik: grafik hicbir zaman bos kalmasin
            (void)gecmisSayisi;
            h.gosterilenFiyat.Zorla(h.fiyat);
            h.flashSuresi = 0.0f;
        }

        int bekleyenSayisi = okuInt();
        yeni.bekleyenHaberler.clear();
        for (int i = 0; i < bekleyenSayisi; i++) {
            BeklemedeHaber bh;
            bh.hisseIndex = okuInt();
            bh.carpan = okuFloat();
            bh.kalanSure = okuFloat();
            bh.metin = okuStr();
            yeni.bekleyenHaberler.push_back(bh);
        }

        int haberSayisi = okuInt();
        yeni.haberBandi.clear();
        for (int i = 0; i < haberSayisi; i++) yeni.haberBandi.push_back(okuStr());

        int bekleyenKureselSayisi = okuInt();
        yeni.bekleyenKureselHaberler.clear();
        for (int i = 0; i < bekleyenKureselSayisi; i++) {
            BeklemedeKuresel bk;
            bk.carpan = okuFloat();
            bk.kuresekRiskPrimiDegisimi = okuFloat();
            bk.kalanSure = okuFloat();
            bk.metin = okuStr();
            yeni.bekleyenKureselHaberler.push_back(bk);
        }

        // ---- V3 ile eklenen alanlar ----
        // V2 kayitlarda bunlar YOKTUR; o zaman degerler oldugu gibi birakilir
        // (kureselHaberSayaci 0'dan sayar, ses ayarlari mevcut haliyle kalir).
        // Once yerel degiskenlere okuyup ancak yukleme TAMAMEN basarili olunca
        // uyguluyoruz - yarim kalan bir okuma ses ayarini bozmasin.
        bool yeniSesAcik = sesAcik;
        float yeniSesSeviyesi = -1.0f;
        if (surumV4 || surumV3) {
            yeni.kureselHaberSayaci = okuFloat();
            yeniSesAcik = (okuInt() != 0);
            yeniSesSeviyesi = Clamp(okuFloat(), 0.0f, 1.0f);
        }
        if (surumV4) {
            yeni.islemSayisi = okuInt();
            yeni.gerceklesenKar = okuFloat();
            yeni.enIyiIslem = okuFloat();
            yeni.enKotuIslem = okuFloat();
            yeni.sonTemettuGunu = okuInt();
            yeni.normalizeGrafik = (okuInt() != 0);
        }
        yeni.grafikModBtn.etiket = yeni.normalizeGrafik ? "Eksen: %" : "Eksen: TL";
        yeni.sesButonu.etiket = yeniSesAcik ? "Ses: Acik" : "Ses: Kapali";

        // Gecici/gorsel durumlar yuklenmez, temiz baslar: bant kaydirmasi,
        // "sabit kalma" sureci ve acik yonetim karari penceresi (o sirada
        // zamanlayici zaten <=0 oldugundan, karar penceresi normal akisla
        // bir sonraki karede kendiliginden tekrar acilir - veri kaybi olmaz).
        yeni.bantKaydirma = 0.0f;
        yeni.haberSabitKalmaSuresi = 0.0f;
        yeni.yonetimOlayiAcik = false;
        yeni.yonetimHisseIndex = -1;
        yeni.yonetimSablonIndex = -1;

        yeni.gosterilenNetDeger.Zorla(yeni.NetDeger());
        yeni.gosterilenBakiye.Zorla(yeni.bakiye);
        yeni.sifirlaOnaySuresi = 0.0f; // bekleyen bir "Emin misin?" onayi devrolmasin

        o = yeni;
        sesAcik = yeniSesAcik;
        if (yeniSesSeviyesi >= 0.0f && IsAudioDeviceReady()) SetMasterVolume(yeniSesSeviyesi);
        return true;
    } catch (...) {
        return false;
    }
}

// ============================================================================
// CIZIM YARDIMCILARI
// ============================================================================
static void GlowCizgiCiz(const vector<Vector2>& noktalar, Color renk, float kalinlik = 2.5f) {
    // Basit "glow": altta soluk-kalin, ustte parlak-ince cizgi
    for (size_t i = 0; i + 1 < noktalar.size(); i++) {
        DrawLineEx(noktalar[i], noktalar[i + 1], kalinlik * 3.0f, Fade(renk, 0.12f));
    }
    for (size_t i = 0; i + 1 < noktalar.size(); i++) {
        DrawLineEx(noktalar[i], noktalar[i + 1], kalinlik, renk);
    }
}

// Kenarlikli, gradyanli panel (rounded-rect outline API'ye guvenmeden,
// versiyonlar arasi uyumlu, iki katmanli teknikle)
static void PanelCiz(Rectangle r, float yuvarlaklik, Color icRenk, Color kenarRenk) {
    DrawRectangleRounded({r.x - 2, r.y - 2, r.width + 4, r.height + 4}, yuvarlaklik, 10, kenarRenk);
    DrawRectangleRounded(r, yuvarlaklik, 10, icRenk);
}

static void MiniSparklineCiz(Rectangle alan, const deque<float>& gecmis, Color renk) {
    if (gecmis.size() < 2) return;
    int n = (int)gecmis.size();
    int baslangic = max(0, n - SPARKLINE_UZUNLUK);
    float minV = 1e9f, maxV = -1e9f;
    for (int i = baslangic; i < n; i++) {
        minV = fminf(minV, gecmis[i]);
        maxV = fmaxf(maxV, gecmis[i]);
    }
    if (maxV - minV < 0.01f) { minV -= 1.0f; maxV += 1.0f; }

    vector<Vector2> noktalar;
    int adet = n - baslangic;
    for (int i = 0; i < adet; i++) {
        float t = (adet <= 1) ? 0.0f : (float)i / (float)(adet - 1);
        float x = alan.x + t * alan.width;
        float degerOran = (gecmis[baslangic + i] - minV) / (maxV - minV);
        float y = alan.y + alan.height - degerOran * alan.height;
        noktalar.push_back({x, y});
    }
    GlowCizgiCiz(noktalar, renk, 1.6f);
}

// ============================================================================
// ANA CIZIM: MENU EKRANI
// ============================================================================
static void MenuCiz(float zaman, Buton& basBtn, Buton& yukleBtn, Buton& cikisBtn, const string& durumMesaji, Color durumRenk) {
    int w = PENCERE_GENISLIK, h = PENCERE_YUKSEKLIK;
    DrawRectangleGradientV(0, 0, w, h, RENK_ARKAPLAN_UST, RENK_ARKAPLAN_ALT);

    // Arkaplanda yavasca yukselen soluk "hisse cizgileri" atmosferi
    for (int i = 0; i < (int)SIRKETLER.size(); i++) {
        Color c = SIRKETLER[i].renk;
        vector<Vector2> pts;
        for (int x = 0; x <= 20; x++) {
            float fx = (float)x / 20.0f * w;
            float dalga = sinf(zaman * 0.6f + x * 0.4f + i * 2.0f) * 40.0f;
            float fy = h * 0.62f + i * 40.0f + dalga;
            pts.push_back({fx, fy});
        }
        for (size_t i2 = 0; i2 + 1 < pts.size(); i2++) DrawLineEx(pts[i2], pts[i2 + 1], 2.0f, Fade(c, 0.18f));
    }

    float pulse = 1.0f + sinf(zaman * 2.0f) * 0.03f;
    const char* baslik = "BORSA PATRONU";
    int boyut = (int)(64 * pulse);
    int tw = MeasureText(baslik, boyut);
    DrawText(baslik, w / 2 - tw / 2 + 3, (int)(h * 0.28f) + 3, boyut, Fade(BLACK, 0.5f));
    DrawText(baslik, w / 2 - tw / 2, (int)(h * 0.28f), boyut, RENK_ALTIN);

    const char* altyazi = "Kucuk bir sermayeyle borsa imparatorlugu kur";
    int tw2 = MeasureText(altyazi, 20);
    DrawText(altyazi, w / 2 - tw2 / 2, (int)(h * 0.28f) + boyut + 14, 20, RENK_SOLUK_METIN);

    basBtn.Ciz(22);
    yukleBtn.Ciz(16);
    // NOT: bu ipucu ile asagidaki durumMesaji AYNI satira (butonun 6px altina)
    // yaziliyor - ust uste binmemeleri icin ipucu sadece durum mesaji yokken
    // gosterilir (durum mesaji her zaman daha oncelikli/gecici bir bilgi).
    if (!yukleBtn.aktif && durumMesaji.empty()) {
        const char* yokMetni = "(Kayitli oyun bulunamadi)";
        int yokW = MeasureText(yokMetni, 12);
        DrawText(yokMetni, (int)(yukleBtn.rect.x + yukleBtn.rect.width / 2.0f - yokW / 2.0f),
                 (int)(yukleBtn.rect.y + yukleBtn.rect.height + 6), 12, RENK_SOLUK_METIN);
    }
    if (!durumMesaji.empty()) {
        int dtw = MeasureText(durumMesaji.c_str(), 14);
        DrawText(durumMesaji.c_str(), w / 2 - dtw / 2, (int)(yukleBtn.rect.y + yukleBtn.rect.height + 6), 14, durumRenk);
    }

    cikisBtn.Ciz(16);

    const char* ipucu = "Fare ile alim/satim yap, haberleri takip et, rutbe atla!";
    int tw3 = MeasureText(ipucu, 14);
    DrawText(ipucu, w / 2 - tw3 / 2, h - 50, 14, RENK_SOLUK_METIN);

    // Surum numarasi (sag alt kose) - itch.io'da gelen geri bildirimleri dogru
    // surumle eslestirebilmek icin.
    DrawText(OYUN_SURUMU, w - MeasureText(OYUN_SURUMU, 14) - 16, h - 26, 14, RENK_KENARLIK);
}

// ============================================================================
// PROGRAM BASLANGICI
// ============================================================================
int main(void) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(PENCERE_GENISLIK, PENCERE_YUKSEKLIK, "Borsa Patronu - Trading Tycoon");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    // Rastgele sayi ureticisini gercek zamanla tohumla: boylece her oturum
    // (fiyat hareketleri, haberler, piyasa modu) gercekten farkli olur.
    SetRandomSeed((unsigned int)time(NULL));

    // ---- SABIT COZUNURLUKTE CIZIM DOKUSU ----
    // Oyunu her zaman PENCERE_GENISLIK x PENCERE_YUKSEKLIK (1280x720, tam
    // 16:9) sanal cozunurlugune ciziyoruz, sonra bu dokuyu ekranin GERCEK
    // boyutuna (pencereli/tam ekran/kenarsiz farketmeksizin) en-boy oranini
    // koruyarak olcekleyip ortaliyoruz. 16:9 secilmesinin nedeni: 1920x1080,
    // 2560x1440, 3840x2160 gibi en yaygin monitor cozunurlukleri de 16:9
    // oldugu icin bu monitorlerde tam ekran/kenarsiz modda SIFIR pillarbox
    // (sag/sol siyah bosluk) kaliyor - olcek tam sayi (1920/1280=1.5,
    // 1080/720=1.5) cikiyor. Eski 16:10 (1280x800) tasarimda 1920x1080'de
    // sag/solda 96'sar px siyah bosluk kaliyordu.
    RenderTexture2D sahneDokusu = LoadRenderTexture(PENCERE_GENISLIK, PENCERE_YUKSEKLIK);
    SetTextureFilter(sahneDokusu.texture, TEXTURE_FILTER_BILINEAR);

    InitAudioDevice();

    Oyun oyun;
    oyun.sesler.Yukle();
    g_sesler = &oyun.sesler;
    oyun.Baslat();

    // --- Sabit buton konumlarini bir kere hesapla ---
    float solPanelX = 20, solPanelGenislik = 380;
    // kartY0/kartYukseklikTabani/kartArasi: hisse karti alaninin ANA (3 kartlik)
    // tasarimi. Su an ACIK olan hisse sayisi 3'u gecmedigi surece kartlar tam
    // olarak bu boyutta cizilir (oyunun ilk saatleri gibi); 4-6 hisse acildikca
    // ayni rezerve alana (asagidaki KART_ALANI_YUKSEKLIK) sigmasi icin kartlar
    // kucultulur - bkz. "SOL PANEL: HISSE KARTLARI" cizim blogu.
    // (130: PENCERE_YUKSEKLIK 800'den 720'ye (16:9) dusurulunce sol paneldeki
    // dikey alanin 80px azalmasini karsilamak icin 150'den 130'a indirildi -
    // asagidaki adet-paneli/Yeni-Oyun butonu gap'leri de ayni nedenle kucultuldu.)
    float kartY0 = 154, kartYukseklikTabani = 130, kartArasi = 12;
    const float KART_ALANI_YUKSEKLIK = 3 * (kartYukseklikTabani + kartArasi) - kartArasi; // = 414

    // NOT: alButonlari/satButonlari artik burada ONCEDEN konumlandirilmiyor -
    // her karede, o an ACIK olan hisselerin gercek (kucultulmus olabilecek)
    // kart boyutuna gore yeniden hesaplaniyor (bkz. asagidaki GUNCELLEME ve
    // CIZIM bolumlerindeki ortak kart-duzeni hesaplamasi).

    float adetPanelY = kartY0 + KART_ALANI_YUKSEKLIK + 4;
    // -1 = MAKS: alimda bakiyenin yettigi kadar, satista elindekinin TAMAMI.
    // 501 hisselik devralma esigine ulasmak eskiden onlarca tik gerektiriyordu;
    // 250 ve MAKS bu suruklenmeyi kaldiriyor. 3 sutun yerine 4 sutuna gecildi
    // ki panel yuksekligi (dolayisiyla altindaki duzen) hic degismesin.
    int adetDegerleri[8] = {1, 5, 10, 25, 50, 100, 250, -1};
    for (int i = 0; i < 8; i++) {
        oyun.adetButonlari[i].rect = {solPanelX + 14 + (i % 4) * 86.0f, adetPanelY + 30 + (i / 4) * 34.0f, 80, 32};
        oyun.adetButonlari[i].renk = RENK_MAVI_VURGU;
        oyun.adetButonlari[i].etiket = (adetDegerleri[i] > 0) ? to_string(adetDegerleri[i]) : string("MAKS");
    }

    float altButonY = adetPanelY + 30 + 2 * 34 + 8;
    oyun.sifirlaButonu.rect = {solPanelX + 14, altButonY, solPanelGenislik - 28, 30};
    oyun.sifirlaButonu.renk = RENK_KENARLIK;
    oyun.sifirlaButonu.etiket = "Yeni Oyun";

    // Ust barda artik sadece TEK buton var: Duraklat. Kaydet / Ayarlar /
    // Ana Menu / Cikis, sadelik icin oyunu duraklattiktan sonra acilan
    // "Duraklatma Menusu" panelinin icine tasindi (asagida DURAKLATMA_MENU_*).
    oyun.duraklatButonu.rect = {(float)PENCERE_GENISLIK - 100, 16, 80, 30};
    oyun.duraklatButonu.renk = RENK_KENARLIK;
    oyun.duraklatButonu.etiket = "Duraklat";

    // Grafik ekseni degistirme butonu.
    // DIKKAT: panelin SAG ustune konulamaz - orada sag hizali "PIYASA MODU: ..."
    // yazisi var (en uzun hali ~330px) ve buton tam uzerine biniyordu. Bu yuzden
    // buton, panel basliginin ("CANLI PIYASA TAKIBI", ~200px) hemen SAGINA,
    // solda kalan bos alana alindi.
    oyun.grafikModBtn.rect = {solPanelX + solPanelGenislik + 20 + 250, kartY0 + 12, 120, 26};
    oyun.grafikModBtn.renk = RENK_KENARLIK;
    oyun.grafikModBtn.etiket = "Eksen: TL";

    // Iceriden bilgi zarfi: ust barda, Duraklat butonunun hemen solunda.
    // (Duraklat 1180..1260 arasinda oldugu icin cakisma yok.)
    const Rectangle insiderRect = {(float)PENCERE_GENISLIK - 148, 13, 38, 34};

    // ---- DURAKLATMA MENUSU ICI DUZENI ----
    // Panel yuksekligi 400 -> 460: en uste "Devam Et" butonu eklendi (5 buton).
    // Ust bardaki Duraklat butonu, duraklatma KARARTMA KATMANININ ALTINDA kaldigi
    // icin (o katmandan once ciziliyor) neredeyse gorunmez oluyordu; oyuncunun
    // oyuna donmek icin bakacagi yer artik panelin kendisi.
    const float dmPanelW = 340, dmPanelH = 460;
    const float dmPanelX = (PENCERE_GENISLIK - dmPanelW) / 2.0f;
    const float dmPanelY = (PENCERE_YUKSEKLIK - dmPanelH) / 2.0f;
    const float dmBtnW = dmPanelW - 60;
    const float dmBtnH = 52;
    const float dmBtnGap = 16;
    float dmBtnY = dmPanelY + 80;

    // "DURAKLATILDI" basliginin hemen altinda, birincil eylem olarak yesil.
    oyun.devamButonu.rect = {dmPanelX + 30, dmBtnY, dmBtnW, dmBtnH};
    oyun.devamButonu.renk = RENK_YESIL;
    oyun.devamButonu.etiket = "Devam Et";
    dmBtnY += dmBtnH + dmBtnGap;

    oyun.kaydetButonu.rect = {dmPanelX + 30, dmBtnY, dmBtnW, dmBtnH};
    oyun.kaydetButonu.renk = RENK_MAVI_VURGU;
    oyun.kaydetButonu.etiket = "Kaydet";
    dmBtnY += dmBtnH + dmBtnGap;

    oyun.ayarlarButonu.rect = {dmPanelX + 30, dmBtnY, dmBtnW, dmBtnH};
    oyun.ayarlarButonu.renk = RENK_KENARLIK;
    oyun.ayarlarButonu.etiket = "Ayarlar";
    dmBtnY += dmBtnH + dmBtnGap;

    oyun.anaMenuButonu.rect = {dmPanelX + 30, dmBtnY, dmBtnW, dmBtnH};
    oyun.anaMenuButonu.renk = RENK_KENARLIK;
    oyun.anaMenuButonu.etiket = "Ana Menu";
    dmBtnY += dmBtnH + dmBtnGap;

    oyun.cikisButonu.rect = {dmPanelX + 30, dmBtnY, dmBtnW, dmBtnH};
    oyun.cikisButonu.renk = RENK_KIRMIZI;
    oyun.cikisButonu.etiket = "Cikis";

    // ---- AYARLAR PENCERESI ICI DUZENI ----
    const float panelW = 520, panelH = 440;
    const float panelX = (PENCERE_GENISLIK - panelW) / 2.0f;
    const float panelY = (PENCERE_YUKSEKLIK - panelH) / 2.0f;

    oyun.sesButonu.rect = {panelX + 30, panelY + 90, 200, 38};
    oyun.sesButonu.renk = RENK_KENARLIK;
    oyun.sesButonu.etiket = "Ses: Acik";

    float modBtnW = (panelW - 60 - 20) / 3.0f;
    oyun.modPencereliBtn.rect = {panelX + 30, panelY + 230, modBtnW, 38};
    oyun.modPencereliBtn.renk = RENK_MAVI_VURGU;
    oyun.modPencereliBtn.etiket = "Pencereli";

    oyun.modTamEkranBtn.rect = {panelX + 30 + modBtnW + 10, panelY + 230, modBtnW, 38};
    oyun.modTamEkranBtn.renk = RENK_MAVI_VURGU;
    oyun.modTamEkranBtn.etiket = "Tam Ekran";

    oyun.modKenarsizBtn.rect = {panelX + 30 + 2 * (modBtnW + 10), panelY + 230, modBtnW, 38};
    oyun.modKenarsizBtn.renk = RENK_MAVI_VURGU;
    oyun.modKenarsizBtn.etiket = "Kenarsiz";

    oyun.kucultBtn.rect = {panelX + 30, panelY + 280, panelW - 60, 38};
    oyun.kucultBtn.renk = RENK_KENARLIK;
    oyun.kucultBtn.etiket = "Simge Durumuna Kucult";

    oyun.ayarlarKapatBtn.rect = {panelX + 30, panelY + 380, panelW - 60, 40};
    oyun.ayarlarKapatBtn.renk = RENK_YESIL;
    oyun.ayarlarKapatBtn.etiket = "Kapat";

    // ---- YONETIM KARARI PENCERESI ICI DUZENI ----
    // Cogunluk hissesine sahip oldugun bir sirkette periyodik olarak acilan
    // "karar ver" penceresi: riskli (kisa vadede kazanc, kalici risk artisi)
    // veya guvenli (kucuk/hic getiri yok, risk artisi yok) secim.
    const float yonetimPanelW = 560, yonetimPanelH = 300;
    const float yonetimPanelX = (PENCERE_GENISLIK - yonetimPanelW) / 2.0f;
    const float yonetimPanelY = (PENCERE_YUKSEKLIK - yonetimPanelH) / 2.0f;
    float yonetimBtnW = (yonetimPanelW - 60 - 20) / 2.0f;
    oyun.yonetimRiskliBtn.rect = {yonetimPanelX + 30, yonetimPanelY + 210, yonetimBtnW, 56};
    oyun.yonetimRiskliBtn.renk = RENK_KIRMIZI;
    oyun.yonetimGuvenliBtn.rect = {yonetimPanelX + 30 + yonetimBtnW + 20, yonetimPanelY + 210, yonetimBtnW, 56};
    oyun.yonetimGuvenliBtn.renk = RENK_YESIL;

    // ---- CIKIS ONAYI PENCERESI ICI DUZENI ----
    const float coPanelW = 580, coPanelH = 240;
    const float coPanelX = (PENCERE_GENISLIK - coPanelW) / 2.0f;
    const float coPanelY = (PENCERE_YUKSEKLIK - coPanelH) / 2.0f;
    const float coBtnW = (coPanelW - 60 - 40) / 3.0f;
    const float coBtnY = coPanelY + 150;
    oyun.cikisKaydetBtn.rect = {coPanelX + 30, coBtnY, coBtnW, 54};
    oyun.cikisKaydetBtn.renk = RENK_YESIL;
    oyun.cikisKaydetBtn.etiket = "Kaydet ve Cik";
    oyun.cikisKaydetmeBtn.rect = {coPanelX + 30 + coBtnW + 20, coBtnY, coBtnW, 54};
    oyun.cikisKaydetmeBtn.renk = RENK_KIRMIZI;
    oyun.cikisKaydetmeBtn.etiket = "Kaydetmeden Cik";
    oyun.cikisVazgecBtn.rect = {coPanelX + 30 + 2 * (coBtnW + 20), coBtnY, coBtnW, 54};
    oyun.cikisVazgecBtn.renk = RENK_KENARLIK;
    oyun.cikisVazgecBtn.etiket = "Vazgec";

    Buton basBtn;
    basBtn.rect = {PENCERE_GENISLIK / 2.0f - 100, PENCERE_YUKSEKLIK * 0.56f, 200, 56};
    basBtn.renk = RENK_YESIL;
    basBtn.etiket = "BASLA";

    Buton yukleBtn;
    yukleBtn.rect = {PENCERE_GENISLIK / 2.0f - 110, basBtn.rect.y + basBtn.rect.height + 22, 220, 46};
    yukleBtn.renk = RENK_MAVI_VURGU;
    yukleBtn.etiket = "Kayitli Oyunu Yukle";
    yukleBtn.aktif = KayitDosyasiVarMi();

    Buton cikisMenuBtn;
    cikisMenuBtn.rect = {PENCERE_GENISLIK / 2.0f - 110, yukleBtn.rect.y + yukleBtn.rect.height + 30, 220, 46};
    cikisMenuBtn.renk = RENK_KIRMIZI;
    cikisMenuBtn.etiket = "CIKIS";

    string menuDurumMesaji;
    float menuDurumSuresi = 0.0f;
    Color menuDurumRenk = RENK_KIRMIZI;

    float zamanSayaci = 0.0f;
    bool cikisIstendi = false;

    // NOT: dongu kosulunda artik WindowShouldClose() YOK - pencere kapatma
    // istegini (X butonu / Alt+F4) asagida ELLE yakaliyoruz ki kaydedilmemis
    // ilerlemesi olan oyuncuya once onay penceresi gosterebilelim. raylib'in
    // WindowShouldClose()'u her cagrildiginda GLFW bayragini sifirlar, yani
    // istegi "yutup" oyunu acik tutmak guvenlidir.
    while (!cikisIstendi) {
        float dt = GetFrameTime();
        zamanSayaci += dt;
        bool pencereKapatmaIstegi = WindowShouldClose();

        // Gercek ekran / pencere boyutu (tam ekranda monitor cozunurlugu,
        // pencerelide PENCERE_GENISLIK x PENCERE_YUKSEKLIK olabilir) ile
        // sabit sanal cozunurlugumuz (1280x720) arasindaki olcek + bosluk.
        float ekranW = (float)GetScreenWidth();
        float ekranH = (float)GetScreenHeight();
        float olcek = fminf(ekranW / PENCERE_GENISLIK, ekranH / PENCERE_YUKSEKLIK);
        float sahneGenislik = PENCERE_GENISLIK * olcek;
        float sahneYukseklik = PENCERE_YUKSEKLIK * olcek;
        float sahneOfsetX = (ekranW - sahneGenislik) / 2.0f;
        float sahneOfsetY = (ekranH - sahneYukseklik) / 2.0f;

        // Fare konumunu gercek ekrandan sanal (1280x720) koordinat sistemine
        // ceviriyoruz; boylece butonlarin hit-test'i tam ekranda/kenarsizda
        // da dogru calisiyor.
        Vector2 gercekFare = GetMousePosition();
        Vector2 fare = {
            (gercekFare.x - sahneOfsetX) / olcek,
            (gercekFare.y - sahneOfsetY) / olcek,
        };
        bool tiklandi = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        // ---- CIKIS ONAYI ----
        // Kaydedilmemis bir ilerleme varsa (= en az bir islem yapilmissa) cikis
        // dogrudan degil, onay penceresinden gecer. Hic islem yapilmamissa
        // kaybedilecek bir sey yok, aninda cikilir.
        if (pencereKapatmaIstegi) {
            if (oyun.ilkIslemYapildi && !oyun.cikisOnayiAcik) oyun.cikisOnayiAcik = true;
            else if (!oyun.ilkIslemYapildi) cikisIstendi = true;
        }
        if (oyun.cikisOnayiAcik) {
            if (oyun.cikisKaydetBtn.Guncelle(dt, fare, tiklandi)) {
                if (OyunuKaydet(oyun, KayitYolu())) {
                    cikisIstendi = true;
                } else {
                    oyun.cikisOnayiAcik = false;
                    oyun.toastlar.Ekle("Kayit Basarisiz", "Dosyaya yazilamadi, cikis iptal edildi.",
                                       RENK_KIRMIZI, 4.0f);
                    Cal(oyun.sesler.hataUyari);
                }
            }
            if (oyun.cikisKaydetmeBtn.Guncelle(dt, fare, tiklandi)) cikisIstendi = true;
            if (oyun.cikisVazgecBtn.Guncelle(dt, fare, tiklandi)) {
                oyun.cikisOnayiAcik = false;
                Cal(oyun.sesler.tikla);
            }
            // Onay penceresi acikken tik ALTTAKI arayuze gecmesin (yanlislikla
            // hisse alinmasin, oyun sifirlanmasin...).
            tiklandi = false;
        }

        // ---- HISSE KARTI DUZENI (hem GUNCELLEME hem CIZIM bunu kullanir) ----
        // Sadece ACIK (kilidi acilmis) hisseler gorunur/tiklanabilir; acilma
        // sirasina gore (en ucuzdan en pahaliya) ustten alta dizilirler.
        // Rezerve edilen dikey alan (KART_ALANI_YUKSEKLIK) sabit oldugu icin,
        // 3'ten fazla hisse acildikca kart yuksekligi kucularak sigdirilir -
        // ilk 3 hisse acikken (oyunun buyuk kismi) boyut degismez (130px).
        vector<int> acikHisseIndeksleri;
        for (int i = 0; i < (int)oyun.hisseler.size(); i++)
            if (oyun.HisseAcikMi(i)) acikHisseIndeksleri.push_back(i);
        sort(acikHisseIndeksleri.begin(), acikHisseIndeksleri.end(), [&](int a, int b) {
            return oyun.hisseler[a].acilmaSeviyesi < oyun.hisseler[b].acilmaSeviyesi;
        });
        int acikSayisi = (int)acikHisseIndeksleri.size();
        float dinamikKartYukseklik = kartYukseklikTabani;
        if (acikSayisi > 0) {
            dinamikKartYukseklik = fminf(kartYukseklikTabani,
                                         (KART_ALANI_YUKSEKLIK - (acikSayisi - 1) * kartArasi) / (float)acikSayisi);
        }
        bool kompaktKart = dinamikKartYukseklik < 120.0f;

        // ---------------- GUNCELLEME ----------------
        if (oyun.ekran == Ekran::MENU) {
            if (menuDurumSuresi > 0.0f) menuDurumSuresi -= dt;
            else menuDurumMesaji.clear();

            if (basBtn.Guncelle(dt, fare, tiklandi)) {
                Cal(oyun.sesler.tikla);
                oyun.ekran = Ekran::OYUN;
            }
            if (yukleBtn.Guncelle(dt, fare, tiklandi)) {
                if (OyunuYukle(oyun, KayitYolu())) {
                    Cal(oyun.sesler.tikla);
                    oyun.ekran = Ekran::OYUN;
                    oyun.toastlar.Ekle("Oyun Yuklendi", "Kaydedilmis ilerlemen geri geldi.", RENK_YESIL, 4.0f);
                } else {
                    Cal(oyun.sesler.hataUyari);
                    menuDurumMesaji = "Kayit dosyasi okunamadi (bozuk olabilir).";
                    menuDurumRenk = RENK_KIRMIZI;
                    menuDurumSuresi = 3.5f;
                }
            }
            yukleBtn.aktif = KayitDosyasiVarMi();
            if (cikisMenuBtn.Guncelle(dt, fare, tiklandi)) {
                // Ana menuye "Ana Menu" ile donulmus, kaydedilmemis bir oyun
                // duruyor olabilir - o yuzden burada da onaydan geciyoruz.
                if (oyun.ilkIslemYapildi) oyun.cikisOnayiAcik = true;
                else cikisIstendi = true;
            }
        } else {
            // Ust barda tek daimi buton: Duraklat.
            if (oyun.duraklatButonu.Guncelle(dt, fare, tiklandi)) {
                oyun.duraklatildi = !oyun.duraklatildi;
                oyun.duraklatButonu.etiket = oyun.duraklatildi ? "Devam Et" : "Duraklat";
                Cal(oyun.sesler.tikla);
            }

            // ---- DURAKLATMA MENUSU: Kaydet / Ayarlar / Ana Menu / Cikis ----
            // Sadelik icin bu 4 aksiyon artik ust barda degil, sadece oyun
            // DURAKLATILMISKEN acilan bu menude. Zorunlu bir yonetim karari
            // acikken menu gizlenir (karardan kacilamaz).
            bool duraklatMenusuAcik = oyun.duraklatildi && !oyun.yonetimOlayiAcik;
            if (duraklatMenusuAcik) {
                if (oyun.devamButonu.Guncelle(dt, fare, tiklandi)) {
                    oyun.duraklatildi = false;
                    oyun.duraklatButonu.etiket = "Duraklat"; // ust bardaki buton da senkron kalsin
                    Cal(oyun.sesler.tikla);
                }
                if (oyun.kaydetButonu.Guncelle(dt, fare, tiklandi)) {
                    if (OyunuKaydet(oyun, KayitYolu())) {
                        oyun.toastlar.Ekle("Oyun Kaydedildi", "Ilerlemen basariyla kaydedildi.", RENK_YESIL, 3.0f);
                        Cal(oyun.sesler.tikla);
                    } else {
                        oyun.toastlar.Ekle("Kayit Basarisiz", "Dosyaya yazilamadi.", RENK_KIRMIZI, 3.5f);
                        Cal(oyun.sesler.hataUyari);
                    }
                }
                if (oyun.ayarlarButonu.Guncelle(dt, fare, tiklandi)) {
                    oyun.ayarlarAcik = !oyun.ayarlarAcik;
                    Cal(oyun.sesler.tikla);
                }
                if (oyun.anaMenuButonu.Guncelle(dt, fare, tiklandi)) {
                    oyun.ekran = Ekran::MENU;
                    Cal(oyun.sesler.tikla);
                }
                if (oyun.cikisButonu.Guncelle(dt, fare, tiklandi)) {
                    if (oyun.ilkIslemYapildi) oyun.cikisOnayiAcik = true;
                    else cikisIstendi = true;
                }
            }

            if (oyun.ayarlarAcik) {
                // Panelin disina tiklamak da paneli kapatsin (butona TAM
                // BASILDIGI o karede kapanmasin diye ustteki toggle'dan sonra
                // kontrol ediyoruz).
                Rectangle panelRectGuncel = {panelX, panelY, panelW, panelH};
                if (tiklandi && !CheckCollisionPointRec(fare, panelRectGuncel) &&
                    !CheckCollisionPointRec(fare, oyun.ayarlarButonu.rect)) {
                    oyun.ayarlarAcik = false;
                }

                if (oyun.sesButonu.Guncelle(dt, fare, tiklandi)) {
                    sesAcik = !sesAcik;
                    oyun.sesButonu.etiket = sesAcik ? "Ses: Acik" : "Ses: Kapali";
                    Cal(oyun.sesler.tikla);
                }
                if (oyun.modPencereliBtn.Guncelle(dt, fare, tiklandi)) {
                    if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) ToggleBorderlessWindowed();
                    if (IsWindowFullscreen()) ToggleFullscreen();
                    Cal(oyun.sesler.tikla);
                }
                if (oyun.modTamEkranBtn.Guncelle(dt, fare, tiklandi)) {
                    if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) ToggleBorderlessWindowed();
                    if (!IsWindowFullscreen()) ToggleFullscreen();
                    Cal(oyun.sesler.tikla);
                }
                if (oyun.modKenarsizBtn.Guncelle(dt, fare, tiklandi)) {
                    if (IsWindowFullscreen()) ToggleFullscreen();
                    if (!IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) ToggleBorderlessWindowed();
                    Cal(oyun.sesler.tikla);
                }
                if (oyun.kucultBtn.Guncelle(dt, fare, tiklandi)) {
                    Cal(oyun.sesler.tikla);
                    MinimizeWindow();
                }
                if (oyun.ayarlarKapatBtn.Guncelle(dt, fare, tiklandi)) {
                    oyun.ayarlarAcik = false;
                    Cal(oyun.sesler.tikla);
                }

                // Ses seviyesi kaydiricisi (basit "tut ve surukle" mantigi)
                Rectangle sliderTrack = {panelX + 30, panelY + 150, panelW - 60, 14};
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
                    CheckCollisionPointRec(fare, {sliderTrack.x - 8, sliderTrack.y - 10, sliderTrack.width + 16, sliderTrack.height + 20})) {
                    float oran = Clamp((fare.x - sliderTrack.x) / sliderTrack.width, 0.0f, 1.0f);
                    SetMasterVolume(oran);
                }
            }

            // ---- YONETIM KARARI PENCERESI: buton tiklamalari ----
            // Oyun duraklatilmis olsa bile (ayni ayarlar penceresi gibi) bu
            // pencere her zaman etkilesime acik - oyuncu karari erteleyemez.
            if (oyun.yonetimOlayiAcik && oyun.yonetimHisseIndex >= 0 && oyun.yonetimSablonIndex >= 0) {
                Hisse& hy = oyun.hisseler[oyun.yonetimHisseIndex];
                const YonetimOlaySablonu& sablon = YONETIM_OLAYLARI[oyun.yonetimSablonIndex];
                auto yonetimSeciminiUygula = [&](const YonetimSecenek& secim) {
                    hy.fiyat *= secim.temelDegerCarpani;
                    hy.temelDeger = fmaxf(0.5f, hy.temelDeger * secim.temelDegerCarpani);
                    hy.gosterilenFiyat.Ayarla(hy.fiyat);
                    hy.flashSuresi = 0.6f;
                    hy.flashIyi = (secim.temelDegerCarpani >= 1.0f);
                    hy.riskPrimi = Clamp(hy.riskPrimi + secim.riskPrimiDegisimi, RISK_PRIMI_TABANI, RISK_PRIMI_TAVANI);
                    HaberBandinaEkle(oyun, hy.kod + " YONETIM KARARI: " + secim.logMetni);
                    oyun.yonetimOlayiAcik = false;
                    oyun.yonetimHisseIndex = -1;
                    oyun.yonetimSablonIndex = -1;
                    // Bir sonraki karar penceresi icin zamanlayiciyi yeniden kur.
                    hy.yonetimOlayZamanlayici = (float)GetRandomValue((int)(YONETIM_OLAY_MIN_SN * 10),
                                                                       (int)(YONETIM_OLAY_MAX_SN * 10)) / 10.0f;
                };
                if (oyun.yonetimRiskliBtn.Guncelle(dt, fare, tiklandi)) {
                    yonetimSeciminiUygula(sablon.riskli);
                    Cal(oyun.sesler.tikla);
                }
                if (oyun.yonetimGuvenliBtn.Guncelle(dt, fare, tiklandi)) {
                    yonetimSeciminiUygula(sablon.guvenli);
                    Cal(oyun.sesler.tikla);
                }
            }

            // "Yeni Oyun": DURAKLATILMISKEN de tiklanabilir olmasi bir hataydi -
            // duraklatma menusu paneli (ortada) bu butonun uzerine denk gelmedigi
            // icin, karartma katmaninin altindaki butona basmak oyunu sifirliyordu.
            // Artik duraklatmada da (ayarlar/yonetim penceresi gibi) devre disi.
            // Ayrica iki asamali onay: ilk tik butonu "Emin misin?" yapar, onay
            // suresi (SIFIRLA_ONAY_SN) dolmadan gelen ikinci tik sifirlar.
            // Grafik ekseni: duraklatmada da degistirilebilir (salt gorunum).
            if (oyun.grafikModBtn.Guncelle(dt, fare, tiklandi && !oyun.ayarlarAcik &&
                                                         !oyun.yonetimOlayiAcik && !oyun.cikisOnayiAcik)) {
                oyun.normalizeGrafik = !oyun.normalizeGrafik;
                oyun.grafikModBtn.etiket = oyun.normalizeGrafik ? "Eksen: %" : "Eksen: TL";
                Cal(oyun.sesler.tikla);
            }

            if (oyun.sifirlaOnaySuresi > 0.0f)
                oyun.sifirlaOnaySuresi = fmaxf(0.0f, oyun.sifirlaOnaySuresi - dt);
            if (oyun.sifirlaButonu.Guncelle(dt, fare,
                                            tiklandi && !oyun.duraklatildi && !oyun.ayarlarAcik &&
                                                !oyun.yonetimOlayiAcik)) {
                if (oyun.sifirlaOnaySuresi > 0.0f) {
                    oyun.Baslat(); // Baslat() sifirlaOnaySuresi'ni de 0'a ceker
                    Cal(oyun.sesler.tikla);
                } else {
                    oyun.sifirlaOnaySuresi = SIFIRLA_ONAY_SN;
                    Cal(oyun.sesler.hataUyari);
                }
            }

            // ---- KLAVYE KISAYOLLARI ----
            // Oyun tamamen fare ile oynaniyordu; en sik yapilan iki islem
            // (duraklatma ve islem adedi degistirme) artik klavyeden de yapilabilir.
            // Modal bir pencere acikken devre disi - oradan kacilamasin.
            if (!oyun.ayarlarAcik && !oyun.yonetimOlayiAcik && !oyun.cikisOnayiAcik) {
                if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_P)) {
                    oyun.duraklatildi = !oyun.duraklatildi;
                    oyun.duraklatButonu.etiket = oyun.duraklatildi ? "Devam Et" : "Duraklat";
                    Cal(oyun.sesler.tikla);
                }
                if (!oyun.duraklatildi) {
                    int tuslar[8] = {KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR,
                                     KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT};
                    for (int i = 0; i < 8; i++) {
                        if (IsKeyPressed(tuslar[i])) {
                            oyun.secilenAdet = adetDegerleri[i];
                            Cal(oyun.sesler.tikla);
                        }
                    }
                }
                if (IsKeyPressed(KEY_M)) {
                    sesAcik = !sesAcik;
                    oyun.sesButonu.etiket = sesAcik ? "Ses: Acik" : "Ses: Kapali";
                }
            }

            if (!oyun.duraklatildi && !oyun.ayarlarAcik && !oyun.yonetimOlayiAcik &&
                !oyun.cikisOnayiAcik) {
                // Zamanlayicilar (Tkinter after() yerine: tek dongude dt ile ilerleme)
                oyun.piyasaSayaci += dt;
                oyun.haberSayaci += dt;
                oyun.kureselHaberSayaci += dt;
                oyun.gunSayaci += dt;
                if (oyun.gunSayaci >= GUN_SURESI_SN) {
                    oyun.gunSayaci -= GUN_SURESI_SN;
                    oyun.gun++;
                    // Her yeni gunde sirketlerin "adil degeri" de -cok yavas-
                    // kendi yolunda ilerler (isin uzun vadede iyiye/kotuye
                    // gitmesi gibi). Kisa vadeli fiyat hareketinden tamamen
                    // ayri bir surec oldugu icin uzun oturumlarda da oyunu
                    // "cozulebilir" hale getirmez.
                    for (auto& h : oyun.hisseler) {
                        float carpan = 1.0f + RastgeleGauss() * TEMEL_DEGER_GUNLUK_OYNAKLIK;
                        h.temelDeger = fmaxf(0.5f, h.temelDeger * carpan);
                    }

                    // ---- TEMETTU ODEMESI ----
                    if (oyun.gun - oyun.sonTemettuGunu >= TEMETTU_PERIYOT_GUN) {
                        oyun.sonTemettuGunu = oyun.gun;
                        float toplamTemettu = 0.0f;
                        for (auto& h : oyun.hisseler)
                            if (h.adet > 0) toplamTemettu += h.fiyat * h.adet * TEMETTU_ORANI;
                        if (toplamTemettu > 0.01f) {
                            oyun.bakiye += toplamTemettu;
                            oyun.toastlar.Ekle("TEMETTU ODENDI",
                                               "+" + SayiFormatla(toplamTemettu) + " TL portfoyunden",
                                               RENK_YESIL, 4.0f);
                            HaberBandinaEkle(oyun, "TEMETTU: Portfoyunden " +
                                                       SayiFormatla(toplamTemettu) + " TL temettu geldi.");
                            Cal(oyun.sesler.satimKar);
                        }
                    }
                }
                if (oyun.piyasaSayaci >= PIYASA_TICK_SN) {
                    oyun.piyasaSayaci -= PIYASA_TICK_SN;
                    PiyasaGuncelle(oyun);
                }
                if (oyun.haberSayaci >= HABER_TICK_SN) {
                    oyun.haberSayaci -= HABER_TICK_SN;
                    HaberKontrolEt(oyun);
                }
                BekleyenHaberleriGuncelle(oyun, dt);
                if (oyun.kureselHaberSayaci >= KURESEL_OLAY_TICK_SN) {
                    oyun.kureselHaberSayaci -= KURESEL_OLAY_TICK_SN;
                    KureselHaberKontrolEt(oyun);
                }
                BekleyenKureselHaberleriGuncelle(oyun, dt);

                // ---- ICERIDEN BILGI (INSIDER) MEKTUBU ----
                // Sadece ust rutbelerde uretilir; ama bir kez ortaya ciktiysa
                // rutbe ne olursa olsun sayaclari islemeye devam eder.
                if (oyun.rutbeIndex >= INSIDER_MIN_RUTBE && !oyun.insiderMektupVar && !oyun.insiderOkundu) {
                    oyun.insiderSayaci += dt;
                    if (oyun.insiderSayaci >= INSIDER_TICK_SN) {
                        oyun.insiderSayaci -= INSIDER_TICK_SN;
                        InsiderMektupKontrolEt(oyun);
                    }
                }
                if (oyun.insiderMektupVar) {
                    oyun.insiderMektupOmur -= dt;
                    oyun.insiderKalanSure -= dt;
                    if (oyun.insiderMektupOmur <= 0.0f) {
                        // Zarf okunmadan kayboldu - yukselis yine de gerceklesecek,
                        // oyuncu sadece firsati kacirdi.
                        oyun.insiderMektupVar = false;
                        oyun.insiderHisseIndex = -1;
                    }
                }
                if (oyun.insiderOkundu) {
                    oyun.insiderKalanSure -= dt;
                    if (oyun.insiderKalanSure <= 0.0f) {
                        oyun.insiderOkundu = false;
                        oyun.insiderHisseIndex = -1;
                    }
                }
                // Zarfa tiklama: bilgiyi acar (yukselisi DEGISTIRMEZ).
                if (oyun.insiderMektupVar && tiklandi &&
                    CheckCollisionPointRec(fare, insiderRect) && oyun.insiderHisseIndex >= 0) {
                    oyun.insiderMektupVar = false;
                    oyun.insiderOkundu = true;
                    Cal(oyun.sesler.rutbeAtlama);
                    oyun.parcaciklar.Patlat({insiderRect.x + insiderRect.width / 2.0f,
                                             insiderRect.y + insiderRect.height / 2.0f},
                                            RENK_ALTIN, 24, 200.0f);
                    oyun.toastlar.Ekle("ICERIDEN BILGI",
                                       oyun.hisseler[oyun.insiderHisseIndex].kod + " +%" +
                                           SayiFormatla(oyun.insiderArtisOrani * 100.0f, 1) + " (~" +
                                           to_string((int)(oyun.insiderKalanSure + 0.5f)) + " sn)",
                                       RENK_ALTIN, 7.0f);
                }

                // ---- YONETIM KARARLARI: cogunluk sahibi oldugun sirketlerde
                // periyodik olarak "karar ver" penceresi tetiklenir. ----
                for (int i = 0; i < (int)oyun.hisseler.size(); i++) {
                    Hisse& h = oyun.hisseler[i];
                    if (!h.sahibiyim) continue;
                    h.yonetimOlayZamanlayici -= dt;
                    if (h.yonetimOlayZamanlayici <= 0.0f) {
                        oyun.yonetimOlayiAcik = true;
                        oyun.yonetimHisseIndex = i;
                        oyun.yonetimSablonIndex = GetRandomValue(0, (int)YONETIM_OLAYLARI.size() - 1);
                        break;
                    }
                }

                for (auto& h : oyun.hisseler) {
                    h.gosterilenFiyat.Guncelle(dt);
                    if (h.flashSuresi > 0.0f) h.flashSuresi -= dt;
                }

                oyun.gosterilenNetDeger.Ayarla(oyun.NetDeger());
                oyun.gosterilenNetDeger.Guncelle(dt);
                oyun.gosterilenBakiye.Ayarla(oyun.bakiye);
                oyun.gosterilenBakiye.Guncelle(dt);

                oyun.parcaciklar.Guncelle(dt);
                oyun.toastlar.Guncelle(dt);
                if (oyun.haberSabitKalmaSuresi > 0.0f) {
                    // Bir "SON DAKIKA" haberi ekranda sabit duruyor, okunsun diye
                    // henuz kaydirmiyoruz.
                    oyun.haberSabitKalmaSuresi -= dt;
                } else {
                    oyun.bantKaydirma += dt * 70.0f;
                }
                if (oyun.flashAlpha > 0.0f) oyun.flashAlpha = fmaxf(0.0f, oyun.flashAlpha - dt * 0.6f);

                // Al/Sat butonlari (sadece ACIK hisseler icin; dinamik kart duzenine gore konumlanir)
                for (int slot = 0; slot < acikSayisi; slot++) {
                    int i = acikHisseIndeksleri[slot];
                    float kartY = kartY0 + slot * (dinamikKartYukseklik + kartArasi);
                    // Kompakt modda buton yuksekligi/konumu gercek kart
                    // yuksekligine (h) gore dinamik: h kucukse (n=6, ~59px)
                    // butonlar da kuculur ama en az 15px kalir (tiklanabilir
                    // kalsin diye); h buyukse (n=4, ~95px) 24px'e kadar buyur.
                    float btnH = kompaktKart ? fminf(24.0f, fmaxf(15.0f, dinamikKartYukseklik * 0.22f)) : 30.0f;
                    float btnY = kartY + (kompaktKart ? (dinamikKartYukseklik - btnH - 2.0f) : (dinamikKartYukseklik - 40.0f));
                    oyun.alButonlari[i].rect = {solPanelX + 14, btnY, 160, btnH};
                    oyun.satButonlari[i].rect = {solPanelX + 14 + 166, btnY, 160, btnH};

                    if (oyun.alButonlari[i].Guncelle(dt, fare, tiklandi)) {
                        HisseAl(oyun, i);
                        Rectangle r = oyun.alButonlari[i].rect;
                        oyun.parcaciklar.Patlat({r.x + r.width / 2, r.y}, RENK_YESIL, 16, 180.0f);
                    }
                    if (oyun.satButonlari[i].Guncelle(dt, fare, tiklandi)) {
                        HisseSat(oyun, i);
                        Rectangle r = oyun.satButonlari[i].rect;
                        oyun.parcaciklar.Patlat({r.x + r.width / 2, r.y}, RENK_ALTIN, 16, 180.0f);
                    }
                }
                // Adet secim butonlari
                for (int i = 0; i < 8; i++) {
                    if (oyun.adetButonlari[i].Guncelle(dt, fare, tiklandi)) {
                        oyun.secilenAdet = adetDegerleri[i];
                        Cal(oyun.sesler.tikla);
                    }
                }
                // Yeni fiyata gore hisse kart flaslarini tetikleyen efekt
                // (haber etkisi uygulandiginda BekleyenHaberleriGuncelle icinde flashSuresi ayarlaniyor)
            } else {
                // Duraklatildiginda da buton animasyonlari akici kalsin
                for (int slot = 0; slot < acikSayisi; slot++) {
                    int i = acikHisseIndeksleri[slot];
                    float kartY = kartY0 + slot * (dinamikKartYukseklik + kartArasi);
                    // Kompakt modda buton yuksekligi/konumu gercek kart
                    // yuksekligine (h) gore dinamik: h kucukse (n=6, ~59px)
                    // butonlar da kuculur ama en az 15px kalir (tiklanabilir
                    // kalsin diye); h buyukse (n=4, ~95px) 24px'e kadar buyur.
                    float btnH = kompaktKart ? fminf(24.0f, fmaxf(15.0f, dinamikKartYukseklik * 0.22f)) : 30.0f;
                    float btnY = kartY + (kompaktKart ? (dinamikKartYukseklik - btnH - 2.0f) : (dinamikKartYukseklik - 40.0f));
                    oyun.alButonlari[i].rect = {solPanelX + 14, btnY, 160, btnH};
                    oyun.satButonlari[i].rect = {solPanelX + 14 + 166, btnY, 160, btnH};
                    oyun.alButonlari[i].Guncelle(dt, fare, false);
                    oyun.satButonlari[i].Guncelle(dt, fare, false);
                }
                for (int i = 0; i < 8; i++) oyun.adetButonlari[i].Guncelle(dt, fare, false);
            }
        }

        // ---------------- CIZIM ----------------
        // Her seyi once sabit 1280x720 sanal sahneye ciziyoruz (asagida bu
        // dokuyu gercek ekrana olcekli basacagiz).
        BeginTextureMode(sahneDokusu);
        if (oyun.ekran == Ekran::MENU) {
            MenuCiz(zamanSayaci, basBtn, yukleBtn, cikisMenuBtn, menuDurumMesaji, menuDurumRenk);
        } else {
            int w = PENCERE_GENISLIK;
            DrawRectangleGradientV(0, 0, w, PENCERE_YUKSEKLIK, RENK_ARKAPLAN_UST, RENK_ARKAPLAN_ALT);

            // ---- HEADER ----
            DrawRectangleGradientV(0, 0, w, 100, RENK_PANEL, RENK_ARKAPLAN_UST);
            DrawRectangle(0, 98, w, 2, RENK_KENARLIK);

            DrawText("BORSA PATRONU", 20, 12, 24, RENK_ALTIN);
            string rutbeMetni = RUTBELER[oyun.rutbeIndex].ad;
            DrawRectangleRounded({20, 42, (float)MeasureText(rutbeMetni.c_str(), 14) + 24.0f, 22}, 0.5f, 6,
                                 Fade(RENK_MAVI_VURGU, 0.9f));
            DrawText(rutbeMetni.c_str(), 32, 47, 14, WHITE);

            // Rutbe ilerleme cubugu
            {
                float esikMevcut = RUTBELER[oyun.rutbeIndex].esikNetDeger;
                float esikSonraki = (oyun.rutbeIndex + 1 < (int)RUTBELER.size())
                                         ? RUTBELER[oyun.rutbeIndex + 1].esikNetDeger
                                         : esikMevcut + 1.0f;
                float oran = Clamp((oyun.NetDeger() - esikMevcut) / (esikSonraki - esikMevcut), 0.0f, 1.0f);
                Rectangle barArka = {20, 72, 220, 8};
                DrawRectangleRounded(barArka, 0.5f, 6, RENK_KENARLIK);
                DrawRectangleRounded({barArka.x, barArka.y, barArka.width * oran, barArka.height}, 0.5f, 6, RENK_ALTIN);
                // Son rutbedeysek ilerleme cubugunun anlami kalmiyor (hedef yok);
                // bunun yerine zirvede oldugunu belli eden bir etiket koyuyoruz.
                if (oyun.rutbeIndex + 1 >= (int)RUTBELER.size()) {
                    DrawText("ZIRVE", (int)(barArka.x + barArka.width + 10), (int)barArka.y - 3, 12, RENK_ALTIN);
                } else {
                    // Cubuk tek basina "ne kadar kaldi" sorusunu cevaplamiyordu;
                    // hedef rakami yaninda yaziyor.
                    string hedefMetni = SayiFormatla(oyun.NetDeger(), 0) + " / " +
                                        SayiFormatla(esikSonraki, 0) + " TL";
                    DrawText(hedefMetni.c_str(), (int)(barArka.x + barArka.width + 10),
                             (int)barArka.y - 3, 12, RENK_SOLUK_METIN);
                }
            }

            // Net deger / bakiye / kar-zarar (sag ust blok, ortalanmis)
            float netDeger = oyun.gosterilenNetDeger.deger;
            float karZarar = oyun.NetDeger() - BASLANGIC_BAKIYE;
            Color kzRenk = karZarar >= 0 ? RENK_YESIL : RENK_KIRMIZI;
            string netMetin = "Net Deger: " + SayiFormatla(netDeger) + " TL";
            string bakiyeMetni = "Nakit: " + SayiFormatla(oyun.gosterilenBakiye.deger) + " TL";
            string kzMetni = (karZarar >= 0 ? "+" : "") + SayiFormatla(karZarar) + " TL   (" +
                             (karZarar >= 0 ? "+" : "") + SayiFormatla((karZarar / BASLANGIC_BAKIYE) * 100.0f, 1) + "%)";
            int netW = MeasureText(netMetin.c_str(), 20);
            DrawText(netMetin.c_str(), w / 2 - netW / 2, 14, 20, RENK_METIN);
            int bakW = MeasureText(bakiyeMetni.c_str(), 13);
            DrawText(bakiyeMetni.c_str(), w / 2 - bakW / 2, 40, 13, RENK_SOLUK_METIN);
            int kzW = MeasureText(kzMetni.c_str(), 15);
            DrawText(kzMetni.c_str(), w / 2 - kzW / 2, 60, 15, kzRenk);

            string gunMetni = "Gun " + to_string(oyun.gun);
            int gunW = MeasureText(gunMetni.c_str(), 14);
            DrawText(gunMetni.c_str(), w / 2 - gunW / 2, 80, 14, RENK_SOLUK_METIN);

            oyun.duraklatButonu.Ciz(13);

            // ---- ICERIDEN BILGI ZARFI ----
            // Raylib'in varsayilan fontunda zarf glyph'i YOK (ASCII 224 karakter),
            // bu yuzden ikon ilkel sekillerle ciziliyor: govde + "V" seklinde kapak.
            if (oyun.insiderMektupVar) {
                float nabiz = 1.0f + sinf(zamanSayaci * 5.0f) * 0.09f;
                float zw = insiderRect.width * nabiz, zh = insiderRect.height * nabiz;
                Rectangle zr = {insiderRect.x + insiderRect.width / 2.0f - zw / 2.0f,
                                insiderRect.y + insiderRect.height / 2.0f - zh / 2.0f, zw, zh};
                DrawRectangleRounded({zr.x - 5, zr.y - 5, zr.width + 10, zr.height + 10}, 0.3f, 6,
                                     Fade(RENK_ALTIN, 0.22f));
                DrawRectangleRounded(zr, 0.18f, 6, RENK_ALTIN);
                DrawLineEx({zr.x + 3, zr.y + 4}, {zr.x + zr.width / 2.0f, zr.y + zr.height * 0.60f}, 2.5f,
                           RENK_ARKAPLAN_UST);
                DrawLineEx({zr.x + zr.width - 3, zr.y + 4}, {zr.x + zr.width / 2.0f, zr.y + zr.height * 0.60f},
                           2.5f, RENK_ARKAPLAN_UST);
                // Kalan sureyi gosteren ince cubuk (zarf ne kadar daha durur)
                float oran = Clamp(oyun.insiderMektupOmur / INSIDER_MEKTUP_OMUR_SN, 0.0f, 1.0f);
                DrawRectangle((int)zr.x, (int)(zr.y + zr.height + 3), (int)(zr.width * oran), 3, RENK_ALTIN);
            }

            // ---- KAYAN HABER BANDI ----
            Rectangle bant = {0, 100, (float)w, 38};
            DrawRectangle((int)bant.x, (int)bant.y, (int)bant.width, (int)bant.height, {6, 8, 10, 255});
            DrawRectangle(0, 138, w, 1, RENK_KENARLIK);
            {
                string tumHaberler = "EN YENI ->   ";
                // En yeniden en eskiye dogru yaziyoruz (deque'in sonu en yeni),
                // boylece kaydirma sifirlandiginda ekrana giren ilk sey en
                // taze haber oluyor.
                for (auto it = oyun.haberBandi.rbegin(); it != oyun.haberBandi.rend(); ++it)
                    tumHaberler += *it + "   |   ";
                int metinGenislik = MeasureText(tumHaberler.c_str(), 15);
                if (metinGenislik < 1) metinGenislik = 1;
                float x = (float)w - fmodf(oyun.bantKaydirma, (float)(metinGenislik + w));
                DrawText(tumHaberler.c_str(), (int)x, 111, 15, RENK_YESIL);
                DrawText(tumHaberler.c_str(), (int)(x + metinGenislik + 40), 111, 15, RENK_YESIL);
            }

            // ---- SOL PANEL: HISSE KARTLARI ----
            // Sadece ACIK hisseler cizilir (kilitli olanlar tamamen gizli -
            // "ilk basta sadece 1 hisse gorunsun" tasarimi). 3'ten fazla
            // hisse acikken kartlar KOMPAKT moda gecer (kucuk font, tamAd/
            // sparkline gizli) ki hepsi ayni rezerve alana sigsin.
            for (int slot = 0; slot < acikSayisi; slot++) {
                int i = acikHisseIndeksleri[slot];
                Hisse& h = oyun.hisseler[i];
                float kartY = kartY0 + slot * (dinamikKartYukseklik + kartArasi);
                Rectangle kart = {solPanelX, kartY, solPanelGenislik, dinamikKartYukseklik};

                Color icRenk = RENK_PANEL;
                if (h.flashSuresi > 0.0f) {
                    float oran = Clamp(h.flashSuresi / 0.6f, 0.0f, 1.0f);
                    Color hedefFlash = h.flashIyi ? RENK_YESIL : RENK_KIRMIZI;
                    icRenk.r = (unsigned char)Lerp((float)RENK_PANEL.r, (float)hedefFlash.r, oran * 0.5f);
                    icRenk.g = (unsigned char)Lerp((float)RENK_PANEL.g, (float)hedefFlash.g, oran * 0.5f);
                    icRenk.b = (unsigned char)Lerp((float)RENK_PANEL.b, (float)hedefFlash.b, oran * 0.5f);
                }
                PanelCiz(kart, 0.12f, icRenk, RENK_KENARLIK);
                DrawRectangleRounded({kart.x, kart.y, 8, kart.height}, 0.0f, 4, h.renk);

                bool yukseldi = h.fiyat >= h.oncekiFiyat;
                const char* ok = yukseldi ? "^" : "v";
                Color okRenk = yukseldi ? RENK_YESIL : RENK_KIRMIZI;
                string fiyatMetni = SayiFormatla(h.gosterilenFiyat.deger) + " TL " + ok;

                if (!kompaktKart) {
                    // 130px kart yuksekligine gore ayarlandi (eskiden 150px'ti;
                    // PENCERE_YUKSEKLIK 800->720 olunca sol panelde 80px daha
                    // az yer var - bkz. kartYukseklikTabani yorumu yukarida).
                    DrawText(h.kod.c_str(), (int)kart.x + 18, (int)kart.y + 10, 20, WHITE);
                    DrawText(h.tamAd.c_str(), (int)kart.x + 18, (int)kart.y + 32, 12, RENK_SOLUK_METIN);

                    if (h.sahibiyim) {
                        const char* rozet = "PATRONUSUN";
                        int rozetW = MeasureText(rozet, 11);
                        Rectangle rozetRect = {kart.x + kart.width - rozetW - 28, kart.y + 10, (float)rozetW + 16, 18};
                        DrawRectangleRounded(rozetRect, 0.5f, 6, RENK_ALTIN);
                        DrawText(rozet, (int)rozetRect.x + 8, (int)rozetRect.y + 3, 11, BLACK);
                    }

                    DrawText(fiyatMetni.c_str(), (int)kart.x + 18, (int)kart.y + 50, 22, okRenk);

                    string sahiplikMetni = "Elde: " + to_string(h.adet) + " adet   Deger: " +
                                           SayiFormatla(h.fiyat * h.adet) + " TL";
                    DrawText(sahiplikMetni.c_str(), (int)kart.x + 18, (int)kart.y + 74, 12, RENK_SOLUK_METIN);

                    // POZISYON KAR/ZARARI: kart "kac adet, ne kadar deger" diyordu
                    // ama o pozisyonda kazancta mi zararda mi oldugunu SOYLEMIYORDU.
                    // Ortalama maliyet zaten h.maliyet/h.adet olarak duruyordu.
                    if (h.adet > 0) {
                        float ortMaliyet = h.maliyet / h.adet;
                        if (ortMaliyet > 0.01f) {
                            float kzTutar = (h.fiyat - ortMaliyet) * h.adet;
                            float kzOran = (h.fiyat - ortMaliyet) / ortMaliyet * 100.0f;
                            Color kzR = (kzTutar >= 0.0f) ? RENK_YESIL : RENK_KIRMIZI;
                            string isaret = (kzTutar >= 0 ? "+" : "");
                            // Uzun hali "+13,6%  (+1.440,00 TL)". Adet/deger buyudukce
                            // (orn. 1000 adet BIMAS) soldaki sahiplik yazisiyla UST USTE
                            // BINIYOR; sigmiyorsa sadece yuzdeye dusuyoruz. Olcerek karar
                            // veriyoruz ki hicbir sayi buyuklugunde tasma olmasin.
                            string kzUzun = isaret + SayiFormatla(kzOran, 1) + "%  (" + isaret +
                                            SayiFormatla(kzTutar) + " TL)";
                            string kzKisa = isaret + SayiFormatla(kzOran, 1) + "%";
                            int solBitis = (int)kart.x + 18 + MeasureText(sahiplikMetni.c_str(), 12) + 12;
                            string kzM = kzUzun;
                            int kzW = MeasureText(kzM.c_str(), 12);
                            if ((int)(kart.x + kart.width - 14 - kzW) < solBitis) {
                                kzM = kzKisa;
                                kzW = MeasureText(kzM.c_str(), 12);
                            }
                            DrawText(kzM.c_str(), (int)(kart.x + kart.width - 14 - kzW),
                                     (int)kart.y + 74, 12, kzR);
                        }
                    }

                    Rectangle sparkAlan = {kart.x + kart.width - 130, kart.y + 44, 116, 36};
                    MiniSparklineCiz(sparkAlan, h.gecmis, h.renk);
                } else {
                    // Kompakt kart: sadece kod, fiyat, sahiplik ve rozet - tamAd
                    // ve mini grafik yer kazanmak icin gizlenir (ana grafikte
                    // fiyat gecmisi zaten gorunuyor). Ust icerik (kod/rozet/
                    // fiyat/sahiplik) EN KUCUK olasi kart yuksekligine (4-6
                    // hisse acikken ~59-95px) gore sabit/guvenli offsetlerle
                    // konumlanir; AL/SAT butonlari ise gercek yukseklige (h)
                    // gore dinamik olarak alta hizalanir, boylece kart daha
                    // buyukse (n=4) bosluk altta, kucukse (n=6) hep sigar.
                    DrawText(h.kod.c_str(), (int)kart.x + 18, (int)kart.y + 3, 12, WHITE);

                    if (h.sahibiyim) {
                        const char* rozet = "PATRON";
                        int rozetW = MeasureText(rozet, 8);
                        Rectangle rozetRect = {kart.x + kart.width - rozetW - 20, kart.y + 3, (float)rozetW + 10, 13};
                        DrawRectangleRounded(rozetRect, 0.5f, 6, RENK_ALTIN);
                        DrawText(rozet, (int)rozetRect.x + 5, (int)rozetRect.y + 2, 8, BLACK);
                    }

                    DrawText(fiyatMetni.c_str(), (int)kart.x + 18, (int)kart.y + 16, 13, okRenk);

                    string sahiplikMetni = "Elde: " + to_string(h.adet) + "  Deger: " +
                                           SayiFormatla(h.fiyat * h.adet) + " TL";
                    DrawText(sahiplikMetni.c_str(), (int)kart.x + 18, (int)kart.y + 30, 9, RENK_SOLUK_METIN);
                    if (h.adet > 0) {
                        float ortMaliyet = h.maliyet / h.adet;
                        if (ortMaliyet > 0.01f) {
                            float kzOran = (h.fiyat - ortMaliyet) / ortMaliyet * 100.0f;
                            Color kzR = (kzOran >= 0.0f) ? RENK_YESIL : RENK_KIRMIZI;
                            string kzM = (kzOran >= 0 ? "+" : "") + SayiFormatla(kzOran, 1) + "%";
                            int kzW = MeasureText(kzM.c_str(), 10);
                            // Kompakt kartta sahiplik yazisi 9px; yine de cakisma olmasin.
                            int solBitisK = (int)kart.x + 18 + MeasureText(sahiplikMetni.c_str(), 9) + 8;
                            if ((int)(kart.x + kart.width - 14 - kzW) >= solBitisK)
                                DrawText(kzM.c_str(), (int)(kart.x + kart.width - 14 - kzW),
                                         (int)kart.y + 29, 10, kzR);
                        }
                    }
                }

                oyun.alButonlari[i].Ciz(kompaktKart ? 11 : 14);
                oyun.satButonlari[i].Ciz(kompaktKart ? 11 : 14);
            }

            // Henuz acilmamis hisse varsa, kart alaninin bos kalan kismina
            // kucuk bir ipucu yazisi birak (hangi rutbede acilacagini soyler).
            if (acikSayisi < (int)oyun.hisseler.size()) {
                int kilitliSayisi = (int)oyun.hisseler.size() - acikSayisi;
                float ipucuY = kartY0 + acikSayisi * (dinamikKartYukseklik + kartArasi) + 6;
                string ipucuMetni = "+" + to_string(kilitliSayisi) + " hisse daha kilitli";
                if (oyun.rutbeIndex + 1 < (int)RUTBELER.size()) {
                    ipucuMetni += " - sonraki '" + RUTBELER[oyun.rutbeIndex + 1].ad + "' seviyesinde acilir";
                }
                DrawText(ipucuMetni.c_str(), (int)solPanelX + 4, (int)ipucuY, 12, RENK_SOLUK_METIN);
            }

            // ---- PORTFOY OZETI (kart alaninda kalan bos yer) ----
            // Kart alani her zaman 3 kartlik yer ayirir; 1-2 hisse acikken altta
            // 100-250px'lik bos bir bolge kaliyordu. Orayi, oyuncunun surekli
            // merak ettigi ozet bilgiyle dolduruyoruz. 3+ hisse acilinca yer
            // kalmadigi icin panel kendiliginden gizlenir.
            {
                float ipucuPay = (acikSayisi < (int)oyun.hisseler.size()) ? 26.0f : 6.0f;
                float bosBas = kartY0 + acikSayisi * (dinamikKartYukseklik + kartArasi) + ipucuPay;
                float bosYuk = adetPanelY - bosBas - 8.0f;
                if (bosYuk >= 92.0f) {
                    Rectangle oz = {solPanelX, bosBas, solPanelGenislik, fminf(bosYuk, 116.0f)};
                    PanelCiz(oz, 0.08f, RENK_PANEL, RENK_KENARLIK);
                    DrawText("PORTFOY OZETI", (int)oz.x + 14, (int)oz.y + 10, 14, RENK_SOLUK_METIN);

                    float pozDeger = oyun.PortfoyDegeri();
                    string satir1 = "Pozisyonlar: " + SayiFormatla(pozDeger) + " TL";
                    string satir2 = "Nakit: " + SayiFormatla(oyun.bakiye) + " TL";
                    string satir3 = string("Gerceklesen K/Z: ") +
                                    (oyun.gerceklesenKar >= 0 ? "+" : "") +
                                    SayiFormatla(oyun.gerceklesenKar) + " TL";
                    string satir4 = "Islem sayisi: " + to_string(oyun.islemSayisi);
                    DrawText(satir1.c_str(), (int)oz.x + 14, (int)oz.y + 32, 13, RENK_METIN);
                    DrawText(satir2.c_str(), (int)oz.x + 14, (int)oz.y + 52, 13, RENK_METIN);
                    DrawText(satir3.c_str(), (int)oz.x + 14, (int)oz.y + 72, 13,
                             oyun.gerceklesenKar >= 0 ? RENK_YESIL : RENK_KIRMIZI);
                    int s4w = MeasureText(satir4.c_str(), 13);
                    DrawText(satir4.c_str(), (int)(oz.x + oz.width - 14 - s4w), (int)oz.y + 72, 13,
                             RENK_SOLUK_METIN);
                }
            }

            // ---- ADET SECIM PANELI ----
            Rectangle adetPanel = {solPanelX, adetPanelY, solPanelGenislik, 100};
            PanelCiz(adetPanel, 0.08f, RENK_PANEL, RENK_KENARLIK);
            DrawText("Islem Adedi", (int)adetPanel.x + 14, (int)adetPanel.y + 10, 14, RENK_SOLUK_METIN);
            // Parantezli aciklama panelin sag kenarindan 58px tasiyordu; kisaltildi.
            string seciliMetin = (oyun.secilenAdet > 0)
                                    ? ("Secili: " + to_string(oyun.secilenAdet) + " adet")
                                    : string("Secili: MAKS");
            DrawText(seciliMetin.c_str(), (int)adetPanel.x + 150, (int)adetPanel.y + 10, 14, RENK_ALTIN);

            // ---- TAHMINI ISLEM MALIYETI ----
            // Oyuncu TIKLAMADAN ONCE ne odeyecegini gormeli; yoksa karekok etki
            // yasasi gorunmez bir vergi gibi hissettirir. MAKS'ta adet hisseye
            // gore degistigi icin sabit bir yuzde yazilamaz.
            {
                string maliyetMetni;
                if (oyun.secilenAdet > 0) {
                    float yuzde = (ALIM_SATIM_SPREAD * 0.5f +
                                   TicaretEtkisi(oyun.secilenAdet) * ETKI_DOLUM_PAYI) * 100.0f;
                    maliyetMetni = "maliyet ~%" + SayiFormatla(yuzde, 2);
                } else {
                    maliyetMetni = "maliyet: adede gore";
                }
                int mw = MeasureText(maliyetMetni.c_str(), 12);
                int mx = (int)(adetPanel.x + adetPanel.width - 14 - mw);
                // Soldaki "Secili: ..." yazisiyla cakismasin diye olcup kontrol ediyoruz.
                if (mx > (int)adetPanel.x + 150 + MeasureText(seciliMetin.c_str(), 14) + 10)
                    DrawText(maliyetMetni.c_str(), mx, (int)adetPanel.y + 12, 12, RENK_SOLUK_METIN);
            }
            for (int i = 0; i < 8; i++) {
                Buton& b = oyun.adetButonlari[i];
                bool secili = (adetDegerleri[i] == oyun.secilenAdet);
                Color eski = b.renk;
                if (secili) b.renk = RENK_ALTIN;
                b.Ciz(14);
                b.renk = eski;
            }

            {
                // Onay bekleniyorsa buton kirmizi "Emin misin?" haline gelir
                // (adet butonlarindaki gecici renk degistirme yontemiyle ayni).
                Buton& sb = oyun.sifirlaButonu;
                string eskiEtiket = sb.etiket;
                Color eskiSbRenk = sb.renk;
                if (oyun.sifirlaOnaySuresi > 0.0f) {
                    sb.etiket = "EMIN MISIN? Tekrar tikla";
                    sb.renk = RENK_KIRMIZI;
                }
                sb.Ciz(15);
                sb.etiket = eskiEtiket;
                sb.renk = eskiSbRenk;
            }

            // ---- SAG PANEL: ANA GRAFIK ----
            Rectangle grafikPanel = {solPanelX + solPanelGenislik + 20, kartY0, (float)w - (solPanelX + solPanelGenislik + 20) - 20,
                                     (float)PENCERE_YUKSEKLIK - kartY0 - 20};
            PanelCiz(grafikPanel, 0.03f, RENK_PANEL, RENK_KENARLIK);
            DrawText("CANLI PIYASA TAKIBI", (int)grafikPanel.x + 20, (int)grafikPanel.y + 16, 18, RENK_METIN);
            oyun.grafikModBtn.Ciz(12);
            string sureMetni = "(Son " + to_string(GECMIS_UZUNLUK) + " saniye)";
            DrawText(sureMetni.c_str(), (int)grafikPanel.x + 20, (int)grafikPanel.y + 40, 12, RENK_SOLUK_METIN);

            // "Piyasa Modu" gostergesi: genel piyasa faktorunun isaretine gore
            // hisselerin su an cogunlukla birlikte hangi yone egilimli oldugunu
            // gosterir (yeni korelasyon/momentum modelini gorunur kilar).
            {
                const char* modMetni;
                Color modRenk;
                if (oyun.piyasaFaktoru > 0.004f) {
                    modMetni = "PIYASA MODU: RISK-ON (Yukselis Egilimi)";
                    modRenk = RENK_YESIL;
                } else if (oyun.piyasaFaktoru < -0.004f) {
                    modMetni = "PIYASA MODU: RISK-OFF (Dusus Egilimi)";
                    modRenk = RENK_KIRMIZI;
                } else {
                    modMetni = "PIYASA MODU: NOTR";
                    modRenk = RENK_SOLUK_METIN;
                }
                int modW = MeasureText(modMetni, 13);
                DrawText(modMetni, (int)(grafikPanel.x + grafikPanel.width - 20 - modW),
                         (int)grafikPanel.y + 20, 13, modRenk);
            }

            // ---- ICERIDEN BILGI: OKUNDUKTAN SONRAKI GERI SAYIM SERIDI ----
            // Toast 7 saniyede kayboluyor ama yukselis 20 saniyeye kadar
            // gecikebiliyor; oyuncu "hangi hisseydi, ne kadar kaldi" diye
            // sormasin diye bilgi olay gerceklesene kadar burada duruyor.
            if (oyun.insiderOkundu && oyun.insiderHisseIndex >= 0) {
                Hisse& hi = oyun.hisseler[oyun.insiderHisseIndex];
                string insMetin = "ICERIDEN BILGI:  " + hi.kod + "   +%" +
                                  SayiFormatla(oyun.insiderArtisOrani * 100.0f, 1) + "   " +
                                  SayiFormatla(fmaxf(0.0f, oyun.insiderKalanSure), 1) + " sn";
                int insW = MeasureText(insMetin.c_str(), 13);
                Rectangle insKutu = {grafikPanel.x + grafikPanel.width - 20 - (float)insW - 20,
                                     grafikPanel.y + 38, (float)insW + 20, 22};
                DrawRectangleRounded(insKutu, 0.45f, 6, Fade(RENK_ALTIN, 0.16f));
                DrawRectangleRounded({insKutu.x, insKutu.y, 4, insKutu.height}, 0.9f, 4, RENK_ALTIN);
                DrawText(insMetin.c_str(), (int)insKutu.x + 12, (int)insKutu.y + 5, 13, RENK_ALTIN);
            }

            Rectangle cizimAlani = {grafikPanel.x + 60, grafikPanel.y + 70, grafikPanel.width - 100,
                                    grafikPanel.height - 140};

            // Min/maks (SADECE ACIK hisseler ortak eksen - kilitli hisselerin
            // fiyati grafige/lejanta hic yansimaz, "spoiler" olmasin diye)
            //
            // NORMALIZE MODDA her hisse, kendi gecmis penceresinin ILK degerine
            // gore YUZDE degisim olarak cizilir. Boylece 430 TL'lik BIMAS ile
            // 38 TL'lik EREGL ayni olcekte karsilastirilabilir hale gelir;
            // mutlak eksende ucuz hisseler duz cizgiye doner.
            auto grafikDegeri = [&](const Hisse& hh, float v) -> float {
                if (!oyun.normalizeGrafik) return v;
                float taban = hh.gecmis.empty() ? v : hh.gecmis.front();
                if (taban < 0.01f) return 0.0f;
                return (v / taban - 1.0f) * 100.0f;
            };
            float minV = 1e9f, maxV = -1e9f;
            for (int i : acikHisseIndeksleri) {
                const Hisse& hh = oyun.hisseler[i];
                for (float v : hh.gecmis) {
                    float d = grafikDegeri(hh, v);
                    minV = fminf(minV, d);
                    maxV = fmaxf(maxV, d);
                }
            }
            float esikFark = oyun.normalizeGrafik ? 0.5f : 1.0f;
            if (maxV - minV < esikFark) { minV -= esikFark * 5.0f; maxV += esikFark * 5.0f; }

            // Izgara
            for (int g = 0; g <= 4; g++) {
                float y = cizimAlani.y + cizimAlani.height * g / 4.0f;
                DrawLine((int)cizimAlani.x, (int)y, (int)(cizimAlani.x + cizimAlani.width), (int)y,
                        Fade(RENK_KENARLIK, 0.6f));
                float deger = maxV - (maxV - minV) * g / 4.0f;
                string eksenMetni = oyun.normalizeGrafik
                                        ? ((deger >= 0 ? "+" : "") + SayiFormatla(deger, 1) + "%")
                                        : SayiFormatla(deger, 0);
                DrawText(eksenMetni.c_str(), (int)cizimAlani.x - 55, (int)y - 6, 11, RENK_SOLUK_METIN);
            }

            for (int i : acikHisseIndeksleri) {
                Hisse& h = oyun.hisseler[i];
                vector<Vector2> noktalar;
                int n = (int)h.gecmis.size();
                for (int k = 0; k < n; k++) {
                    float t = (n <= 1) ? 0.0f : (float)k / (float)(n - 1);
                    float x = cizimAlani.x + t * cizimAlani.width;
                    float oran = (grafikDegeri(h, h.gecmis[k]) - minV) / (maxV - minV);
                    float y = cizimAlani.y + cizimAlani.height - oran * cizimAlani.height;
                    noktalar.push_back({x, y});
                }
                GlowCizgiCiz(noktalar, h.renk, 2.2f);
            }

            // Lejant (SADECE ACIK hisseler)
            float lejantX = grafikPanel.x + 20;
            float lejantY = grafikPanel.y + grafikPanel.height - 40;
            for (int i : acikHisseIndeksleri) {
                Hisse& h = oyun.hisseler[i];
                DrawRectangleRounded({lejantX, lejantY, 14, 14}, 0.4f, 4, h.renk);
                DrawText(h.kod.c_str(), (int)lejantX + 20, (int)lejantY - 1, 14, RENK_METIN);
                lejantX += 20 + (float)MeasureText(h.kod.c_str(), 14) + 24;
            }

            // ---- PARCACIKLAR VE TOASTLAR ----
            oyun.parcaciklar.Ciz();
            oyun.toastlar.Ciz(w);

            // ---- KUTLAMA PARLAMASI ----
            if (oyun.flashAlpha > 0.0f) {
                DrawRectangle(0, 0, w, PENCERE_YUKSEKLIK, Fade(oyun.flashRenk, oyun.flashAlpha * 0.25f));
            }

            // ---- DURAKLATMA KATMANI / MENUSU ----
            // Sadelik icin Kaydet/Ayarlar/Ana Menu/Cikis artik ust barda degil,
            // sadece burada: oyun duraklatilinca acilan bu menude.
            if (oyun.duraklatildi) {
                DrawRectangle(0, 0, w, PENCERE_YUKSEKLIK, Fade(BLACK, 0.55f));

                if (!oyun.yonetimOlayiAcik) {
                    Rectangle dmPanelRect = {dmPanelX, dmPanelY, dmPanelW, dmPanelH};
                    PanelCiz(dmPanelRect, 0.06f, RENK_PANEL, RENK_ALTIN);

                    const char* dmBaslik = "DURAKLATILDI";
                    int dmTw = MeasureText(dmBaslik, 30);
                    DrawText(dmBaslik, (int)(dmPanelX + dmPanelW / 2.0f - dmTw / 2.0f), (int)dmPanelY + 26, 30,
                             RENK_ALTIN);

                    // ---- ISTATISTIKLER ----
                    // Ayri bir ekran acmak yerine buraya konuldu: herkes bir
                    // noktada duraklatiyor, yani kesin gorulen bir yer.
                    {
                        string ist1 = "Islem: " + to_string(oyun.islemSayisi) +
                                      "   Gerceklesen K/Z: " + (oyun.gerceklesenKar >= 0 ? "+" : "") +
                                      SayiFormatla(oyun.gerceklesenKar) + " TL";
                        string ist2 = "En iyi satis: +" + SayiFormatla(oyun.enIyiIslem) +
                                      " TL    En kotu: " + SayiFormatla(oyun.enKotuIslem) + " TL";
                        int i1 = MeasureText(ist1.c_str(), 14);
                        int i2 = MeasureText(ist2.c_str(), 13);
                        DrawText(ist1.c_str(), w / 2 - i1 / 2, (int)(dmPanelY + dmPanelH + 18), 14,
                                 oyun.gerceklesenKar >= 0 ? RENK_YESIL : RENK_KIRMIZI);
                        DrawText(ist2.c_str(), w / 2 - i2 / 2, (int)(dmPanelY + dmPanelH + 40), 13,
                                 RENK_SOLUK_METIN);
                    }

                    oyun.devamButonu.Ciz(16);
                    oyun.kaydetButonu.Ciz(16);
                    oyun.ayarlarButonu.Ciz(16);
                    oyun.anaMenuButonu.Ciz(16);
                    oyun.cikisButonu.Ciz(16);
                }
            }

            // ---- AYARLAR PENCERESI ----
            if (oyun.ayarlarAcik) {
                DrawRectangle(0, 0, w, PENCERE_YUKSEKLIK, Fade(BLACK, 0.6f));
                Rectangle panelRectCiz = {panelX, panelY, panelW, panelH};
                PanelCiz(panelRectCiz, 0.04f, RENK_PANEL, RENK_KENARLIK);

                DrawText("AYARLAR", (int)panelX + 30, (int)panelY + 22, 24, RENK_ALTIN);

                DrawText("SES", (int)panelX + 30, (int)panelY + 68, 14, RENK_SOLUK_METIN);
                oyun.sesButonu.Ciz(14);

                float seviye = GetMasterVolume();
                Rectangle sliderTrack = {panelX + 30, panelY + 150, panelW - 60, 14};
                DrawRectangleRounded(sliderTrack, 0.5f, 8, RENK_KENARLIK);
                Rectangle sliderDolu = {sliderTrack.x, sliderTrack.y, sliderTrack.width * seviye, sliderTrack.height};
                DrawRectangleRounded(sliderDolu, 0.5f, 8, sesAcik ? RENK_YESIL : RENK_KENARLIK);
                DrawCircleV({sliderTrack.x + sliderTrack.width * seviye, sliderTrack.y + sliderTrack.height / 2.0f},
                            11.0f, RENK_ALTIN);
                string seviyeMetni = "Ses Seviyesi: " + to_string((int)(seviye * 100.0f)) + "%";
                DrawText(seviyeMetni.c_str(), (int)panelX + 30, (int)panelY + 128, 14, RENK_METIN);

                DrawText("PENCERE MODU", (int)panelX + 30, (int)panelY + 200, 14, RENK_SOLUK_METIN);

                bool tamEkranAktif = IsWindowFullscreen();
                bool kenarsizAktif = IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
                bool pencereliAktif = !tamEkranAktif && !kenarsizAktif;

                Color eskiRenk;
                eskiRenk = oyun.modPencereliBtn.renk;
                if (pencereliAktif) oyun.modPencereliBtn.renk = RENK_ALTIN;
                oyun.modPencereliBtn.Ciz(13);
                oyun.modPencereliBtn.renk = eskiRenk;

                eskiRenk = oyun.modTamEkranBtn.renk;
                if (tamEkranAktif) oyun.modTamEkranBtn.renk = RENK_ALTIN;
                oyun.modTamEkranBtn.Ciz(13);
                oyun.modTamEkranBtn.renk = eskiRenk;

                eskiRenk = oyun.modKenarsizBtn.renk;
                if (kenarsizAktif) oyun.modKenarsizBtn.renk = RENK_ALTIN;
                oyun.modKenarsizBtn.Ciz(13);
                oyun.modKenarsizBtn.renk = eskiRenk;

                oyun.kucultBtn.Ciz(14);
                oyun.ayarlarKapatBtn.Ciz(15);
            }

            // ---- YONETIM KARARI PENCERESI ----
            if (oyun.yonetimOlayiAcik && oyun.yonetimHisseIndex >= 0 && oyun.yonetimSablonIndex >= 0) {
                Hisse& hy = oyun.hisseler[oyun.yonetimHisseIndex];
                const YonetimOlaySablonu& sablon = YONETIM_OLAYLARI[oyun.yonetimSablonIndex];

                DrawRectangle(0, 0, w, PENCERE_YUKSEKLIK, Fade(BLACK, 0.65f));
                Rectangle yPanelRect = {yonetimPanelX, yonetimPanelY, yonetimPanelW, yonetimPanelH};
                PanelCiz(yPanelRect, 0.05f, RENK_PANEL, RENK_ALTIN);

                string baslikMetni = hy.kod + " - " + sablon.baslik;
                DrawText(baslikMetni.c_str(), (int)yonetimPanelX + 30, (int)yonetimPanelY + 22, 22, RENK_ALTIN);

                DrawText(sablon.aciklama.c_str(), (int)yonetimPanelX + 30, (int)yonetimPanelY + 60, 15, RENK_METIN);
                DrawText("Cogunluk hissesine sahip oldugun sirketin patronu olarak karar sana ait.",
                         (int)yonetimPanelX + 30, (int)yonetimPanelY + 84, 13, RENK_SOLUK_METIN);

                oyun.yonetimRiskliBtn.Ciz(15);
                oyun.yonetimGuvenliBtn.Ciz(15);

                DrawText(sablon.riskli.etiket.c_str(), (int)oyun.yonetimRiskliBtn.rect.x + 14,
                         (int)oyun.yonetimRiskliBtn.rect.y + 8, 15, WHITE);
                string riskliAlt = "Ani kazanc, kalici risk artisi";
                DrawText(riskliAlt.c_str(), (int)oyun.yonetimRiskliBtn.rect.x + 14,
                         (int)oyun.yonetimRiskliBtn.rect.y + 30, 11, Fade(WHITE, 0.85f));

                DrawText(sablon.guvenli.etiket.c_str(), (int)oyun.yonetimGuvenliBtn.rect.x + 14,
                         (int)oyun.yonetimGuvenliBtn.rect.y + 8, 15, WHITE);
                string guvenliAlt = "Az/hic kazanc, risk artisi yok";
                DrawText(guvenliAlt.c_str(), (int)oyun.yonetimGuvenliBtn.rect.x + 14,
                         (int)oyun.yonetimGuvenliBtn.rect.y + 30, 11, Fade(WHITE, 0.85f));
            }
        }

        // ---- CIKIS ONAYI PENCERESI ----
        // Ekran dalindan SONRA ciziliyor ki hem ana menude hem oyun icinde,
        // her seyin EN USTUNDE gorunsun.
        if (oyun.cikisOnayiAcik) {
            DrawRectangle(0, 0, PENCERE_GENISLIK, PENCERE_YUKSEKLIK, Fade(BLACK, 0.7f));
            Rectangle coPanelRect = {coPanelX, coPanelY, coPanelW, coPanelH};
            PanelCiz(coPanelRect, 0.05f, RENK_PANEL, RENK_ALTIN);

            const char* coBaslik = "CIKMAK ISTEDIGINE EMIN MISIN?";
            int coTw = MeasureText(coBaslik, 24);
            DrawText(coBaslik, (int)(coPanelX + coPanelW / 2.0f - coTw / 2.0f), (int)coPanelY + 30, 24, RENK_ALTIN);

            const char* coAlt = "Son kayittan sonraki ilerlemen kaybolacak.";
            int coTw2 = MeasureText(coAlt, 15);
            DrawText(coAlt, (int)(coPanelX + coPanelW / 2.0f - coTw2 / 2.0f), (int)coPanelY + 72, 15, RENK_METIN);

            string coDurum = "Net Deger: " + SayiFormatla(oyun.NetDeger()) + " TL   |   Gun " + to_string(oyun.gun);
            int coTw3 = MeasureText(coDurum.c_str(), 13);
            DrawText(coDurum.c_str(), (int)(coPanelX + coPanelW / 2.0f - coTw3 / 2.0f), (int)coPanelY + 100, 13,
                     RENK_SOLUK_METIN);

            oyun.cikisKaydetBtn.Ciz(14);
            oyun.cikisKaydetmeBtn.Ciz(14);
            oyun.cikisVazgecBtn.Ciz(14);
        }
        EndTextureMode();

        // Sanal sahneyi gercek ekrana, en-boy oranini bozmadan olcekleyip
        // ortalayarak bas. Ekran sanal sahneden buyukse (tam ekran vb.)
        // kalan kenarlar siyah dolgu olur; kucukse kucultulerek sigdirilir.
        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle kaynak = {0, 0, (float)sahneDokusu.texture.width, -(float)sahneDokusu.texture.height};
        Rectangle hedef = {sahneOfsetX, sahneOfsetY, sahneGenislik, sahneYukseklik};
        DrawTexturePro(sahneDokusu.texture, kaynak, hedef, {0, 0}, 0.0f, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(sahneDokusu);
    oyun.sesler.Bosalt();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
