// ============================================================================
// denge_botlari.h - denge testinin botlari ve turnuvasi
// ============================================================================
// Botlar oyuncunun kullandigi AYNI fonksiyonlari cagirir (IslemYap, KrediAl...),
// yani olctukleri sey gercek oyun kurallaridir.
//
//   Tutucu     : nakdi oldukca esit agirlikla hisse alir, hic satmaz (al-tut).
//   Haberci    : oyunun cekirdek dongusunu oynar - haberi "okur" (%85 dogru),
//                iyi haberde alir, etkiden 2 sn sonra satar; kotu haberde elindekini satar.
//   Haberci+K  : ayni, ama acildiginda kredi ve aciga satis da kullanir.
//   Rastgele   : 3 saniyede bir rastgele alir/satar (dikkatsiz oyuncu).
//   Cevirici   : her saniye MAKS al / MAKS sat (islem maliyetini olcer; batmali).
//   Pompaci    : v1.1'deki acigi dener - 5'erli hizli alim, sonra toplu satis.

enum BotTuru { BOT_TUTUCU, BOT_HABERCI, BOT_HABERCI_KALDIRAC, BOT_RASTGELE, BOT_CEVIRICI, BOT_POMPACI, BOT_SAYISI };
static const char* BOT_ADLARI[BOT_SAYISI] = {"Tutucu", "Haberci", "Haberci+K", "Rastgele", "Cevirici", "Pompaci"};

struct BotCikis {
    int hisse;
    float zaman; // oyunSuresi bu degeri gecince pozisyon kapatilir
};

struct BotDurum {
    int tur = BOT_TUTUCU;
    float sayac = 0.0f;
    int faz = 0;
    vector<BotCikis> cikislar;
    float okumaIsabeti = 0.85f;
};

struct BotSonuc {
    int rutbeGunu[8];
    bool iflas = false;
    int islem = 0;
    int teminat = 0;
    float netGun[3] = {0, 0, 0}; // 60., 120., 240. gun sonunda net deger
};

static const int NET_OLCUM_GUNLERI[3] = {60, 120, 240};

static void BotPozisyonKapat(Oyun& o, int i) {
    Hisse& h = o.hisseler[i];
    if (h.adet > 0) IslemYap(o, i, false, h.adet, IslemKaynagi::OYUNCU);
    else if (h.adet < 0) IslemYap(o, i, true, min(-h.adet, MaksAlinabilir(h, o.bakiye)), IslemKaynagi::OYUNCU);
}

// `oran` kadar nakitle (0..1) hisse alir.
static void BotNakitleAl(Oyun& o, int i, float oran) {
    Hisse& h = o.hisseler[i];
    if (h.adet < 0) return;
    int n = min(MaksAlinabilir(h, o.bakiye * oran), TOPLAM_HISSE_SAYISI - h.adet);
    if (n > 0) IslemYap(o, i, true, n, IslemKaynagi::OYUNCU);
}

static void BotHamle(Oyun& o, BotDurum& b, float dt) {
    b.sayac += dt;
    vector<int> acik = AcikHisseler(o);
    if (acik.empty()) return;

    switch (b.tur) {
        case BOT_TUTUCU: {
            if (b.sayac < 2.0f) break;
            b.sayac = 0.0f;
            if (o.bakiye < 0.03f * o.NetDeger()) break;
            int hedef = acik[0];
            for (int i : acik)
                if (o.hisseler[i].fiyat * o.hisseler[i].adet < o.hisseler[hedef].fiyat * o.hisseler[hedef].adet) hedef = i;
            Hisse& h = o.hisseler[hedef];
            int n = min(25, min(MaksAlinabilir(h, o.bakiye), TOPLAM_HISSE_SAYISI - h.adet));
            if (n > 0) IslemYap(o, hedef, true, n, IslemKaynagi::OYUNCU);
            break;
        }
        case BOT_HABERCI:
        case BOT_HABERCI_KALDIRAC: {
            const bool kaldirac = (b.tur == BOT_HABERCI_KALDIRAC);
            // Zamani gelen cikislar
            for (auto it = b.cikislar.begin(); it != b.cikislar.end();) {
                if (o.oyunSuresi >= it->zaman) {
                    BotPozisyonKapat(o, it->hisse);
                    it = b.cikislar.erase(it);
                } else {
                    ++it;
                }
            }
            if (kaldirac && b.cikislar.empty() && o.borc > 0.0f) KrediOde(o, 1e9f);

            // Yeni sirket haberi / bilanco (bu adimda olusanlar)
            for (auto& bh : o.bekleyenHaberler) {
                if (bh.metin == "(insider)") continue;
                if (bh.kalanSure < HABER_ETKI_GECIKME_SN - dt * 1.5f) continue;
                bool gercekIyi = bh.carpan > 1.0f;
                bool okunan = (Rastgele01() < b.okumaIsabeti) ? gercekIyi : !gercekIyi;
                int i = bh.hisseIndex;
                if (!o.HisseAcikMi(i)) continue;
                if (okunan) {
                    if (o.hisseler[i].adet < 0) BotPozisyonKapat(o, i);
                    if (kaldirac && o.KrediAcik()) KrediAl(o, o.KalanKaldirac() * 0.6f);
                    BotNakitleAl(o, i, 0.7f);
                    b.cikislar.push_back({i, o.oyunSuresi + HABER_ETKI_GECIKME_SN + 2.0f});
                } else {
                    if (o.hisseler[i].adet > 0) BotPozisyonKapat(o, i);
                    if (kaldirac && o.AcigaSatisAcik() && o.hisseler[i].adet == 0) {
                        Hisse& h = o.hisseler[i];
                        int n = min(ACIGA_HISSE_SINIRI, (int)(o.KalanKaldirac() * 0.5f / h.fiyat));
                        if (n > 0) IslemYap(o, i, false, n, IslemKaynagi::OYUNCU);
                        b.cikislar.push_back({i, o.oyunSuresi + HABER_ETKI_GECIKME_SN + 2.0f});
                    }
                }
            }
            // Yeni kuresel haber
            for (auto& bk : o.bekleyenKureselHaberler) {
                if (bk.kalanSure < KURESEL_OLAY_ETKI_GECIKME_SN - dt * 1.5f) continue;
                bool gercekIyi = bk.carpan > 1.0f;
                bool okunan = (Rastgele01() < b.okumaIsabeti) ? gercekIyi : !gercekIyi;
                if (okunan) {
                    int hedef = acik[0]; // en yuksek betali acik hisse
                    for (int i : acik)
                        if (o.hisseler[i].beta > o.hisseler[hedef].beta) hedef = i;
                    BotNakitleAl(o, hedef, 0.7f);
                    b.cikislar.push_back({hedef, o.oyunSuresi + KURESEL_OLAY_ETKI_GECIKME_SN + 2.0f});
                } else {
                    for (int i : acik)
                        if (o.hisseler[i].adet > 0) BotPozisyonKapat(o, i);
                }
            }
            break;
        }
        case BOT_RASTGELE: {
            if (b.sayac < 3.0f) break;
            b.sayac = 0.0f;
            int i = acik[GetRandomValue(0, (int)acik.size() - 1)];
            int lotlar[4] = {5, 10, 25, 50};
            int n = lotlar[GetRandomValue(0, 3)];
            Hisse& h = o.hisseler[i];
            if (GetRandomValue(0, 1)) {
                n = min(n, min(MaksAlinabilir(h, o.bakiye), TOPLAM_HISSE_SAYISI - h.adet));
                if (n > 0) IslemYap(o, i, true, n, IslemKaynagi::OYUNCU);
            } else if (h.adet > 0) {
                IslemYap(o, i, false, min(n, h.adet), IslemKaynagi::OYUNCU);
            }
            break;
        }
        case BOT_CEVIRICI: {
            if (b.sayac < 1.0f) break;
            b.sayac = 0.0f;
            int i = acik[0];
            o.secilenAdet = -1;
            if (o.hisseler[i].adet > 0) HisseSat(o, i);
            else HisseAl(o, i);
            break;
        }
        case BOT_POMPACI: {
            int i = acik[0];
            Hisse& h = o.hisseler[i];
            if (b.faz == 0) { // 15 sn boyunca saniyede ~5 kez 5'er adet al
                int n = min(5, MaksAlinabilir(h, o.bakiye));
                if (n > 0 && h.adet + n <= TOPLAM_HISSE_SAYISI) IslemYap(o, i, true, n, IslemKaynagi::OYUNCU);
                if (n > 0 && GetRandomValue(0, 3) == 0) { // ortalama 5 tik/sn (adim 0,25 sn)
                    int m = min(5, MaksAlinabilir(h, o.bakiye));
                    if (m > 0 && h.adet + m <= TOPLAM_HISSE_SAYISI) IslemYap(o, i, true, m, IslemKaynagi::OYUNCU);
                }
                if (b.sayac >= 15.0f || n <= 0) { b.faz = 1; b.sayac = 0.0f; }
            } else if (b.faz == 1) { // toplu sat
                if (h.adet > 0) IslemYap(o, i, false, h.adet, IslemKaynagi::OYUNCU);
                b.faz = 2;
                b.sayac = 0.0f;
            } else if (b.sayac >= 40.0f) { // bekle, sonra bastan
                b.faz = 0;
                b.sayac = 0.0f;
            }
            break;
        }
    }
}

static BotSonuc BotOyunu(int botTuru, int seed, int maxGun, int senaryo) {
    SetRandomSeed((unsigned int)seed);
    for (int i = 0; i < BASARIM_SAYISI; i++) g_profil.basarim[i] = false;
    Oyun o;
    o.Baslat(senaryo, false);
    BotDurum b;
    b.tur = botTuru;
    BotSonuc s;
    for (int i = 0; i < 8; i++) s.rutbeGunu[i] = -1;
    s.rutbeGunu[0] = 1;
    int sonRutbe = 0;
    int olcum = 0;
    const float dt = 0.25f;

    while (o.gun <= maxGun) {
        SimulasyonAdimi(o, dt);
        if (o.yonetimOlayiAcik) YonetimKarariUygula(o, GetRandomValue(0, 1) == 1);
        if (o.sonPenceresiAcik) {
            if (o.sonTuru == SON_IFLAS) { s.iflas = true; break; }
            o.sonPenceresiAcik = false; // zirve / sure: kum havuzu olarak devam
        }
        while (sonRutbe < o.rutbeIndex) s.rutbeGunu[++sonRutbe] = o.gun;
        while (olcum < 3 && o.gun >= NET_OLCUM_GUNLERI[olcum]) s.netGun[olcum++] = o.NetDeger();
        BotHamle(o, b, dt);
        o.toastlar.liste.clear();
        o.parcaciklar.liste.clear();
    }
    while (olcum < 3) s.netGun[olcum++] = s.iflas ? 0.0f : o.NetDeger();
    s.islem = o.islemSayisi;
    s.teminat = o.teminatCagrisiSayisi;
    return s;
}

static float Medyan(vector<float> v) {
    if (v.empty()) return 0.0f;
    sort(v.begin(), v.end());
    return v[v.size() / 2];
}

static void BotTurnuvasi() {
    const int OYUN = 120;
    const int MAX_GUN = 300;
    const int R = (int)RUTBELER.size();
    Yaz("\n=== 3) BOT TURNUVASI (Klasik senaryo, bot basina %d oyun, en fazla %d gun = %.0f dk) ===\n", OYUN, MAX_GUN,
        MAX_GUN * GUN_SURESI_SN / 60.0f);
    Yaz("  Her rutbe icin: ulasanlarin orani / ulasanlarin MEDYAN gunu.\n");
    Yaz("  %-10s", "bot");
    for (int r = 1; r < R; r++) Yaz("   rutbe %d   ", r);
    Yaz("  iflas  islem   net@60g  net@120g  net@240g  tem.cagrisi\n");

    for (int bot = 0; bot < BOT_SAYISI; bot++) {
        vector<vector<float>> gunler(R);
        vector<float> net[3];
        int iflas = 0;
        long islem = 0;
        long teminat = 0;
        for (int g = 0; g < OYUN; g++) {
            BotSonuc s = BotOyunu(bot, 1000 + g, MAX_GUN, S_KLASIK);
            for (int r = 1; r < R; r++)
                if (s.rutbeGunu[r] > 0) gunler[r].push_back((float)s.rutbeGunu[r]);
            for (int k = 0; k < 3; k++) net[k].push_back(s.netGun[k]);
            if (s.iflas) iflas++;
            islem += s.islem;
            teminat += s.teminat;
        }
        Yaz("  %-10s", BOT_ADLARI[bot]);
        for (int r = 1; r < R; r++) {
            if (gunler[r].empty()) Yaz("   %%  0 /  -  ");
            else Yaz("   %%%3d /%4.0f ", (int)(100 * gunler[r].size() / OYUN), Medyan(gunler[r]));
        }
        Yaz("  %%%3d  %5ld  %8.0f  %8.0f  %8.0f  %6.2f\n", 100 * iflas / OYUN, islem / OYUN, Medyan(net[0]), Medyan(net[1]),
            Medyan(net[2]), (double)teminat / OYUN);
    }

    // Senaryo karsilastirmasi: cekirdek donguyu oynayan bot (Haberci) her senaryoda.
    Yaz("\n  Senaryolar (Haberci botu, %d oyun):\n", OYUN / 2);
    for (int sen = 0; sen < SENARYO_SAYISI; sen++) {
        vector<float> zirve, net60;
        int iflas = 0;
        for (int g = 0; g < OYUN / 2; g++) {
            BotSonuc s = BotOyunu(BOT_HABERCI, 5000 + g, MAX_GUN, sen);
            if (s.rutbeGunu[R - 1] > 0) zirve.push_back((float)s.rutbeGunu[R - 1]);
            net60.push_back(s.netGun[0]);
            if (s.iflas) iflas++;
        }
        Yaz("    %-18s zirve: %%%3d / medyan %4.0f gun   net@60g medyan %8.0f TL   iflas %%%d\n", SENARYOLAR[sen].ad.tr,
            (int)(100 * zirve.size() / (OYUN / 2)), Medyan(zirve), Medyan(net60), 100 * iflas / (OYUN / 2));
    }
}
