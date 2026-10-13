// ============================================================================
// denge_testi.cpp - PENCERESIZ denge ve suistimal testi
// ============================================================================
// Oyunun gercek mantik kodunu (src/piyasa_*.h) pencere acmadan calistirir:
//   1) SUISTIMAL TESTLERI : al-sat donguleri gercekten zarar ettiriyor mu?
//   2) PIYASA ISTATISTIKLERI: oynaklik, ortak faktor payi, korelasyon.
//   3) BOT TURNUVASI      : farkli stratejiler rutbelere kac gunde ulasiyor?
//
// Calistirma:  python build.py test     (rapor: test/denge_raporu.txt)
//
// Bir denge sabitini (src/temel.h) degistirdiginde bunu calistir ve "rutbeye
// ulasma gunu" sutunlarini onceki raporla karsilastir. Suistimal testlerinden
// biri ARTIYA gecerse o degisiklik bir acik yaratmis demektir.

#define BORSA_BASIZ
#include "../borsa_patronu.cpp"
#include <cstdarg>

static FILE* g_rapor = nullptr;
static void Yaz(const char* bicim, ...) {
    char tampon[1024];
    va_list a;
    va_start(a, bicim);
    vsnprintf(tampon, sizeof(tampon), bicim, a);
    va_end(a);
    fputs(tampon, stdout);
    if (g_rapor) fputs(tampon, g_rapor);
}

static int IlkAcikHisse(const Oyun& o) {
    for (int i = 0; i < HisseSayisi(); i++)
        if (o.hisseler[i].acilmaSeviyesi == 0) return i;
    return 0;
}

#include "denge_botlari.h"

// ---------------------------------------------------------------------------
// 1) SUISTIMAL TESTLERI
// ---------------------------------------------------------------------------
// Fiyat gurultusu OLMADAN (piyasa tiki isletilmeden ya da sadece baski sonmesi
// isletilerek) saf al-sat dongulerinin sonucu. Hepsi <= 0 olmali.
static int g_suistimalHata = 0;

static void SuistimalSatiri(const char* ad, float bas, float son) {
    float yuzde = (son / bas - 1.0f) * 100.0f;
    bool sorun = son > bas + 0.01f;
    if (sorun) g_suistimalHata++;
    Yaz("  %-52s %10.2f -> %10.2f TL  (%+6.2f%%)  %s\n", ad, bas, son, yuzde, sorun ? "<<< ACIK!" : "ok");
}

// Sadece emir baskisinin sonmesini isletir (fiyat sabit kalir) - "beklersem ne olur".
static void BaskiSondur(Oyun& o, int saniye) {
    for (int s = 0; s < saniye; s++)
        for (auto& h : o.hisseler) {
            h.akis *= AKIS_SONME;
            if (fabsf(h.akis) < 0.5f) h.akis = 0.0f;
        }
}

static void SuistimalTestleri() {
    Yaz("\n=== 1) SUISTIMAL TESTLERI (fiyat sabit; hepsi <= 0 olmali) ===\n");
    g_profilKalici = false;

    auto yeniOyun = [](float bakiye) {
        Oyun o;
        o.Baslat(S_KLASIK, false);
        o.bakiye = bakiye;
        o.rutbeIndex = (int)RUTBELER.size() - 1; // tum araclar acik
        return o;
    };

    {   // tek seferde MAKS al - MAKS sat, 8 tur
        Oyun o = yeniOyun(10000.0f);
        int idx = IlkAcikHisse(o);
        float bas = o.NetDeger();
        for (int t = 0; t < 8; t++) { o.secilenAdet = -1; HisseAl(o, idx); HisseSat(o, idx); }
        SuistimalSatiri("MAKS al / MAKS sat x8 (aninda)", bas, o.NetDeger());
    }
    for (int parca : {1, 5, 25}) {   // v1.1'deki acik: parca parca al, toplu sat
        Oyun o = yeniOyun(10000.0f);
        int idx = IlkAcikHisse(o);
        float bas = o.NetDeger();
        for (int t = 0; t < 5; t++) {
            o.secilenAdet = parca;
            for (int k = 0; k < 2000; k++) { float once = o.bakiye; HisseAl(o, idx); if (o.bakiye >= once) break; }
            o.secilenAdet = -1;
            HisseSat(o, idx);
        }
        char ad[96];
        snprintf(ad, sizeof(ad), "%d'erli al / toplu sat x5 (aninda)", parca);
        SuistimalSatiri(ad, bas, o.NetDeger());
    }
    {   // toplu al, parca parca sat
        Oyun o = yeniOyun(10000.0f);
        int idx = IlkAcikHisse(o);
        float bas = o.NetDeger();
        for (int t = 0; t < 5; t++) {
            o.secilenAdet = -1; HisseAl(o, idx);
            o.secilenAdet = 5;
            while (o.hisseler[idx].adet >= 5) HisseSat(o, idx);
            o.secilenAdet = -1; if (o.hisseler[idx].adet > 0) HisseSat(o, idx);
        }
        SuistimalSatiri("toplu al / 5'erli sat x5 (aninda)", bas, o.NetDeger());
    }
    {   // parca parca al, bekle (baski sonsun), toplu sat
        Oyun o = yeniOyun(10000.0f);
        int idx = IlkAcikHisse(o);
        float bas = o.NetDeger();
        for (int t = 0; t < 5; t++) {
            o.secilenAdet = 5;
            for (int k = 0; k < 2000; k++) { float once = o.bakiye; HisseAl(o, idx); if (o.bakiye >= once) break; if (k % 5 == 4) BaskiSondur(o, 1); }
            BaskiSondur(o, 30);
            o.secilenAdet = -1; HisseSat(o, idx);
            BaskiSondur(o, 30);
        }
        SuistimalSatiri("5'erli al (5 tik/sn) / 30 sn bekle / toplu sat x5", bas, o.NetDeger());
    }
    {   // aciga sat - kapat dongusu
        Oyun o = yeniOyun(50000.0f);
        int idx = IlkAcikHisse(o);
        float bas = o.NetDeger();
        for (int t = 0; t < 8; t++) { o.secilenAdet = -1; HisseSat(o, idx); o.secilenAdet = -1; HisseAl(o, idx); }
        SuistimalSatiri("MAKS aciga sat / kapat x8 (aninda)", bas, o.NetDeger());
    }
    {   // parca parca aciga sat, toplu kapat
        Oyun o = yeniOyun(50000.0f);
        int idx = IlkAcikHisse(o);
        float bas = o.NetDeger();
        for (int t = 0; t < 5; t++) {
            o.secilenAdet = 5;
            for (int k = 0; k < 100; k++) HisseSat(o, idx);
            o.secilenAdet = -1; HisseAl(o, idx);
        }
        SuistimalSatiri("5'erli aciga sat x100 / toplu kapat x5 (aninda)", bas, o.NetDeger());
    }
    {   // once aciga sat (baskiyi eksiye cek), sonra ucuza al, sonra acigi kapat?
        Oyun o = yeniOyun(50000.0f);
        float bas = o.NetDeger();
        // iki ayri hisse gerekmez; ayni hissede uzun+aciga ayni anda tutulamaz.
        // Bunun yerine: aciga sat -> kapat ve uzuna gec -> sat (baski isaret degistirir).
        int idx = IlkAcikHisse(o);
        for (int t = 0; t < 5; t++) {
            o.secilenAdet = 200; HisseSat(o, idx);      // 200 aciga
            o.secilenAdet = 200; HisseAl(o, idx);       // kapat
            o.secilenAdet = 200; HisseAl(o, idx);       // 200 uzun
            o.secilenAdet = 200; HisseSat(o, idx);      // sat
        }
        SuistimalSatiri("aciga sat / kapat / al / sat (200'er) x5", bas, o.NetDeger());
    }
    {   // kredi al - ode (faizsiz gun icinde) nakit uretiyor mu?
        Oyun o = yeniOyun(50000.0f);
        float bas = o.NetDeger();
        for (int t = 0; t < 20; t++) { KrediAl(o, 1e9f); KrediOde(o, 1e9f); }
        SuistimalSatiri("kredi al / ode x20", bas, o.NetDeger());
    }
    for (const char* kod : {"ALTIN", "DOLAR"}) {   // guvenli limanlarda al-sat dongusu
        Oyun o = yeniOyun(300000.0f);
        int idx = LimanIndeksi(o, kod);
        float bas = o.NetDeger();
        for (int t = 0; t < 5; t++) {
            o.secilenAdet = 5;
            for (int k = 0; k < 60; k++) HisseAl(o, idx);
            o.secilenAdet = -1;
            HisseSat(o, idx);
            HisseAl(o, idx);
            HisseSat(o, idx);
        }
        char ad[96];
        snprintf(ad, sizeof(ad), "%s: 5'erli al / toplu sat / MAKS al-sat x5", kod);
        SuistimalSatiri(ad, bas, o.NetDeger());
        // aciga satilamamali
        o.secilenAdet = 10;
        HisseSat(o, idx);
        if (o.hisseler[idx].adet < 0) { Yaz("  %s aciga satilabildi <<< HATA\n", kod); g_suistimalHata++; }
    }
    {   // rastgele islem dizileri: 300 farkli seed, fiyat sabit
        int artida = 0;
        float enIyi = -1e9f;
        for (int seed = 1; seed <= 300; seed++) {
            SetRandomSeed(seed);
            Oyun o = yeniOyun(30000.0f);
            int idx = IlkAcikHisse(o);
            float bas = o.NetDeger();
            int lotlar[6] = {1, 5, 10, 25, 50, 100};
            for (int k = 0; k < 400; k++) {
                o.secilenAdet = (GetRandomValue(0, 9) == 0) ? -1 : lotlar[GetRandomValue(0, 5)];
                if (GetRandomValue(0, 1)) HisseAl(o, idx); else HisseSat(o, idx);
                if (GetRandomValue(0, 3) == 0) BaskiSondur(o, GetRandomValue(1, 8));
            }
            // pozisyonu kapat
            o.secilenAdet = -1;
            if (o.hisseler[idx].adet > 0) HisseSat(o, idx); else if (o.hisseler[idx].adet < 0) HisseAl(o, idx);
            float fark = o.NetDeger() - bas;
            if (fark > 0.01f) artida++;
            enIyi = fmaxf(enIyi, fark);
        }
        Yaz("  %-52s artida biten: %d / 300, en iyi sonuc: %+.2f TL  %s\n", "rastgele 400 islemlik diziler (fiyat sabit)",
            artida, enIyi, artida > 0 ? "<<< ACIK!" : "ok");
        if (artida > 0) g_suistimalHata++;
    }

    Yaz("  Notr durumda tek emir maliyeti: ");
    for (int n : {1, 25, 100, 250, 1000}) Yaz("%d adet %%%.2f   ", n, NotrMaliyetOrani(n) * 100.0f);
    Yaz("\n");
}

// ---------------------------------------------------------------------------
// 2) PIYASA ISTATISTIKLERI
// ---------------------------------------------------------------------------
static void PiyasaIstatistikleri() {
    Yaz("\n=== 2) PIYASA ISTATISTIKLERI (haber/olay yok, 200.000 tik) ===\n");
    SimTohumla(12345);
    Oyun o;
    o.Baslat(S_KLASIK, false);
    const int N = 200000;
    const int PENCERE = 30; // "30 saniyelik getiri" icin
    const int H = HisseSayisi(); // hisseler; guvenli limanlar asagida ayrica olculur
    const int L = (int)o.hisseler.size() - H;
    vector<double> lR(L, 0), lR2(L, 0), lRE(L, 0);
    double eR = 0, eR2 = 0; // "endeks": hisse getirilerinin ortalamasi
    vector<double> topR(H, 0), topR2(H, 0), topSapma2(H, 0), topW(H, 0), topW2(H, 0), birikim(H, 0);
    vector<vector<double>> topRR(H, vector<double>(H, 0)), topWW(H, vector<double>(H, 0));
    double topM = 0, topM2 = 0;
    int pencereSayisi = 0;
    vector<double> r(H);
    for (int t = 0; t < N; t++) {
        vector<float> once(H);
        for (int i = 0; i < H; i++) once[i] = o.hisseler[i].fiyat;
        vector<float> lOnce(L);
        for (int k = 0; k < L; k++) lOnce[k] = o.hisseler[H + k].fiyat;
        PiyasaGuncelle(o);
        double m = o.piyasaFaktoru;
        topM += m; topM2 += m * m;
        for (int i = 0; i < H; i++) {
            r[i] = o.hisseler[i].fiyat / once[i] - 1.0;
            topR[i] += r[i]; topR2[i] += r[i] * r[i];
            birikim[i] += r[i];
            double sap = o.hisseler[i].fiyat / o.hisseler[i].temelDeger - 1.0;
            topSapma2[i] += sap * sap;
        }
        for (int i = 0; i < H; i++)
            for (int j = 0; j < H; j++) topRR[i][j] += r[i] * r[j];
        double endeks = 0;
        for (int i = 0; i < H; i++) endeks += r[i] / H;
        eR += endeks; eR2 += endeks * endeks;
        for (int k = 0; k < L; k++) {
            double lr = o.hisseler[H + k].fiyat / lOnce[k] - 1.0;
            lR[k] += lr; lR2[k] += lr * lr; lRE[k] += lr * endeks;
        }
        if ((t + 1) % PENCERE == 0) {
            for (int i = 0; i < H; i++) { topW[i] += birikim[i]; topW2[i] += birikim[i] * birikim[i]; }
            for (int i = 0; i < H; i++)
                for (int j = 0; j < H; j++) topWW[i][j] += birikim[i] * birikim[j];
            for (int i = 0; i < H; i++) birikim[i] = 0;
            pencereSayisi++;
        }
    }
    double varM = topM2 / N - (topM / N) * (topM / N);
    Yaz("  Piyasa faktoru std: %%%.3f\n", sqrt(varM) * 100.0);
    Yaz("  %-6s %5s %10s %20s\n", "hisse", "beta", "tik std", "adil deger sapmasi");
    vector<double> sd(H), sdW(H);
    for (int i = 0; i < H; i++) {
        double ort = topR[i] / N;
        sd[i] = sqrt(topR2[i] / N - ort * ort);
        double ortW = topW[i] / pencereSayisi;
        sdW[i] = sqrt(topW2[i] / pencereSayisi - ortW * ortW);
        Yaz("  %-6s %5.2f %9.3f%% %19.1f%%\n", o.hisseler[i].kod.c_str(), o.hisseler[i].beta, sd[i] * 100.0,
            sqrt(topSapma2[i] / N) * 100.0);
    }
    double topKor = 0, topKorW = 0, minKor = 9, maxKor = -9;
    int ciftSayisi = 0;
    for (int i = 0; i < H; i++)
        for (int j = i + 1; j < H; j++) {
            double kor = (topRR[i][j] / N - (topR[i] / N) * (topR[j] / N)) / (sd[i] * sd[j]);
            double korW = (topWW[i][j] / pencereSayisi - (topW[i] / pencereSayisi) * (topW[j] / pencereSayisi)) / (sdW[i] * sdW[j]);
            topKor += kor; topKorW += korW; ciftSayisi++;
            minKor = fmin(minKor, kor); maxKor = fmax(maxKor, kor);
        }
    Yaz("  Hisseler arasi korelasyon (1 sn getiri) : ortalama %.2f (en dusuk %.2f, en yuksek %.2f)\n", topKor / ciftSayisi, minKor, maxKor);
    Yaz("  Hisseler arasi korelasyon (%d sn getiri): ortalama %.2f   (gercek borsalarda ~0,3-0,5)\n", PENCERE, topKorW / ciftSayisi);
    double eSd = sqrt(eR2 / N - (eR / N) * (eR / N));
    for (int k = 0; k < L; k++) {
        double lSd = sqrt(lR2[k] / N - (lR[k] / N) * (lR[k] / N));
        double kor = (lRE[k] / N - (lR[k] / N) * (eR / N)) / (lSd * eSd);
        Yaz("  Guvenli liman %-6s tik std %.3f%%, hisse endeksiyle korelasyon %+.2f (EKSI olmali)\n",
            o.hisseler[H + k].kod.c_str(), lSd * 100.0, kor);
    }
}

int main() {
    g_rapor = fopen("test/denge_raporu.txt", "w");
    g_profilKalici = false;
    Yaz("BORSA PATRONU %s - DENGE TESTI RAPORU\n", OYUN_SURUMU);
    SuistimalTestleri();
    PiyasaIstatistikleri();
    BotTurnuvasi();
    int belirlenimHata = BelirlenimTestleri();
    int devralmaHata = DevralmaDeneyi();
    Yaz("\nSONUC: suistimal testi %s, belirlenim testi %s, devralma dengesi %s\n",
        g_suistimalHata == 0 ? "TEMIZ" : "BASARISIZ - yukaridaki 'ACIK' satirlarina bak",
        belirlenimHata == 0 ? "TEMIZ" : "BASARISIZ", devralmaHata == 0 ? "TEMIZ" : "BASARISIZ");
    if (g_rapor) fclose(g_rapor);
    return (g_suistimalHata == 0 && belirlenimHata == 0 && devralmaHata == 0) ? 0 : 1;
}
