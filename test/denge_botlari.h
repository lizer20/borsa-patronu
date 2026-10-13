// ============================================================================
// denge_botlari.h - denge testinin botlari ve turnuvasi
// ============================================================================
// Botlar oyuncunun kullandigi AYNI fonksiyonlari cagirir (IslemYap, KrediAl...),
// yani olctukleri sey gercek oyun kurallaridir. Kendi kararlari icin AYRI bir
// rastgele sayi ureteci (g_botRng) kullanirlar; boylece ayni tohumda butun botlar
// AYNI piyasayi gorur ve karsilastirma adil olur.
//
//   Tutucu     : nakdi oldukca esit agirlikla hisse alir, HIC satmaz (al-tut).
//   Degerci    : al-tut, ama degerleme etiketini okur: PAHALI/BALON olani satar,
//                en ucuz MAKUL-alti hisseyi alir. (Dusuk tempolu, "okuyan" oyuncu.)
//   Haberci    : oyunun cekirdek dongusu - haberi "okur" (%85 dogru), iyi haberde
//                alir, etkiden 2 sn sonra satar; kotu haberde elindekini satar.
//   Haberci+K  : ayni, ama kredi, aciga satis ve guvenli liman da kullanir.
//   Rastgele   : 3 saniyede bir rastgele alir/satar (dikkatsiz oyuncu).
//   Cevirici   : her saniye MAKS al / MAKS sat (islem maliyetini olcer; batmali).
//   Pompaci    : v1.1'deki acigi dener - 5'erli hizli alim, sonra toplu satis.
//
// Devralma / yonetim kararlari turnuvada degil AYRI bir deneyde olculur
// (DevralmaDeneyi): botlar devralmaya yetecek nakdi nadiren biriktirdigi icin
// turnuva o mekanigi neredeyse hic calistirmaz.

enum BotTuru { BOT_TUTUCU, BOT_DEGERCI, BOT_HABERCI, BOT_HABERCI_KALDIRAC, BOT_RASTGELE, BOT_CEVIRICI, BOT_POMPACI, BOT_SAYISI };
static const char* BOT_ADLARI[BOT_SAYISI] = {"Tutucu", "Degerci", "Haberci", "Haberci+K", "Rastgele", "Cevirici", "Pompaci"};

static Rng g_botRng;

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
    int gorev = 0;
    float netGun[3] = {0, 0, 0};  // 60., 120., 240. gun sonunda net deger
    unsigned long long ortamOzeti = 0; // kuresel olaylarin ve piyasa modunun ozeti (bagimsizlik testi)
};

static const int NET_OLCUM_GUNLERI[3] = {60, 120, 240};

static void BotPozisyonKapat(Oyun& o, int i) {
    Hisse& h = o.hisseler[i];
    if (h.adet > 0) IslemYap(o, i, false, h.adet, IslemKaynagi::OYUNCU);
    else if (h.adet < 0) IslemYap(o, i, true, min(-h.adet, MaksAlinabilir(h, o.bakiye)), IslemKaynagi::OYUNCU);
}

// `oran` kadar nakitle (0..1) alir.
static void BotNakitleAl(Oyun& o, int i, float oran) {
    Hisse& h = o.hisseler[i];
    if (h.adet < 0) return;
    int n = min(MaksAlinabilir(h, o.bakiye * oran), h.azamiAdet - h.adet);
    if (n > 0) IslemYap(o, i, true, n, IslemKaynagi::OYUNCU);
}

static int LimanIndeksi(const Oyun& o, const char* kod) {
    for (int i = HisseSayisi(); i < (int)o.hisseler.size(); i++)
        if (o.hisseler[i].kod == kod) return i;
    return -1;
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
            int n = min(25, min(MaksAlinabilir(h, o.bakiye), h.azamiAdet - h.adet));
            if (n > 0) IslemYap(o, hedef, true, n, IslemKaynagi::OYUNCU);
            break;
        }
        case BOT_DEGERCI: {
            if (b.sayac < 2.0f) break;
            b.sayac = 0.0f;
            // 1) Pahalilasani kademeli sat (baski birikmesin diye 50'ser).
            for (int i : acik) {
                Hisse& h = o.hisseler[i];
                if (h.adet > 0 && DegerlemeSeviyesi(h) >= 3) IslemYap(o, i, false, min(h.adet, 50), IslemKaynagi::OYUNCU);
            }
            // 2) Nakitle, pahali OLMAYAN hisselerin en ucuzunu al.
            if (o.bakiye < 0.03f * o.NetDeger()) break;
            int hedef = -1;
            for (int i : acik) {
                if (DegerlemeSeviyesi(o.hisseler[i]) >= 3) continue;
                if (hedef < 0 || Degerleme(o.hisseler[i]) < Degerleme(o.hisseler[hedef])) hedef = i;
            }
            if (hedef < 0) break; // her sey pahali: nakitte bekle
            Hisse& h = o.hisseler[hedef];
            int n = min(25, min(MaksAlinabilir(h, o.bakiye), h.azamiAdet - h.adet));
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

            // Yeni sirket haberi / bilanco / duzeltme (bu adimda olusanlar)
            for (size_t k = 0; k < o.bekleyenHaberler.size(); k++) {
                const BeklemedeHaber bh = o.bekleyenHaberler[k]; // kopya: islem sirasinda dizi degisebilir
                if (bh.tur == HABER_INSIDER) continue;
                if (bh.kalanSure < HABER_ETKI_GECIKME_SN - dt * 1.5f) continue;
                bool gercekIyi = bh.carpan > 1.0f;
                bool okunan = (g_botRng.Birim() < b.okumaIsabeti) ? gercekIyi : !gercekIyi;
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
            for (size_t k = 0; k < o.bekleyenKureselHaberler.size(); k++) {
                const BeklemedeKuresel bk = o.bekleyenKureselHaberler[k];
                if (bk.kalanSure < KURESEL_OLAY_ETKI_GECIKME_SN - dt * 1.5f) continue;
                bool gercekIyi = bk.carpan > 1.0f;
                bool okunan = (g_botRng.Birim() < b.okumaIsabeti) ? gercekIyi : !gercekIyi;
                if (okunan) {
                    int hedef = acik[0]; // en yuksek betali acik hisse
                    for (int i : acik)
                        if (o.hisseler[i].beta > o.hisseler[hedef].beta) hedef = i;
                    BotNakitleAl(o, hedef, 0.7f);
                    b.cikislar.push_back({hedef, o.oyunSuresi + KURESEL_OLAY_ETKI_GECIKME_SN + 2.0f});
                } else {
                    for (int i : acik)
                        if (o.hisseler[i].adet > 0) BotPozisyonKapat(o, i);
                    // Kotu kuresel haber: guvenli limana sigin (acildiysa).
                    int dolar = LimanIndeksi(o, "DOLAR");
                    if (kaldirac && dolar >= 0 && o.HisseAcikMi(dolar)) {
                        BotNakitleAl(o, dolar, 0.7f);
                        b.cikislar.push_back({dolar, o.oyunSuresi + KURESEL_OLAY_ETKI_GECIKME_SN + 2.0f});
                    }
                }
            }
            break;
        }
        case BOT_RASTGELE: {
            if (b.sayac < 3.0f) break;
            b.sayac = 0.0f;
            int i = acik[g_botRng.Aralik(0, (int)acik.size() - 1)];
            int lotlar[4] = {5, 10, 25, 50};
            int n = lotlar[g_botRng.Aralik(0, 3)];
            Hisse& h = o.hisseler[i];
            if (g_botRng.Aralik(0, 1)) {
                n = min(n, min(MaksAlinabilir(h, o.bakiye), h.azamiAdet - h.adet));
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
                if (n > 0 && h.adet + n <= h.azamiAdet) IslemYap(o, i, true, n, IslemKaynagi::OYUNCU);
                if (n > 0 && g_botRng.Aralik(0, 3) == 0) { // ortalama 5 tik/sn (adim 0,25 sn)
                    int m = min(5, MaksAlinabilir(h, o.bakiye));
                    if (m > 0 && h.adet + m <= h.azamiAdet) IslemYap(o, i, true, m, IslemKaynagi::OYUNCU);
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
    SimTohumla((uint64_t)seed);
    g_botRng.Tohumla((uint64_t)seed * 7919ULL + (uint64_t)botTuru);
    g_tarihZorla = 20260101 + (seed % 28); // Gunun Piyasasi icin sabit bir "bugun"
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
    int sonGun = 0;
    const float dt = 0.25f;

    while (o.gun <= maxGun) {
        SimulasyonAdimi(o, dt);
        if (o.yonetimOlayiAcik) YonetimKarariUygula(o, g_botRng.Aralik(0, 1) == 1);
        if (o.sonPenceresiAcik) {
            if (o.sonTuru == SON_IFLAS) { s.iflas = true; break; }
            o.sonPenceresiAcik = false; // zirve / sure: kum havuzu olarak devam
        }
        // Ortam ozeti: yeni kuresel olaylar + gun basi piyasa modu.
        for (auto& bk : o.bekleyenKureselHaberler)
            if (bk.kalanSure >= KURESEL_OLAY_ETKI_GECIKME_SN - dt * 1.5f)
                s.ortamOzeti = s.ortamOzeti * 1315423911ULL + (unsigned long long)(bk.carpan * 1000.0f) + (unsigned long long)o.gun * 31ULL;
        if (o.gun != sonGun) {
            sonGun = o.gun;
            s.ortamOzeti = s.ortamOzeti * 2654435761ULL + (unsigned long long)(long long)(o.piyasaFaktoru * 1e6f);
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
    s.gorev = o.tamamlananGorev;
    g_tarihZorla = 0;
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
    Yaz("  iflas  islem  gorev   net@60g  net@120g  net@240g  tem.cagrisi\n");

    for (int bot = 0; bot < BOT_SAYISI; bot++) {
        vector<vector<float>> gunler(R);
        vector<float> net[3];
        int iflas = 0;
        long islem = 0, teminat = 0, gorev = 0;
        for (int g = 0; g < OYUN; g++) {
            BotSonuc s = BotOyunu(bot, 1000 + g, MAX_GUN, S_KLASIK);
            for (int r = 1; r < R; r++)
                if (s.rutbeGunu[r] > 0) gunler[r].push_back((float)s.rutbeGunu[r]);
            for (int k = 0; k < 3; k++) net[k].push_back(s.netGun[k]);
            if (s.iflas) iflas++;
            islem += s.islem;
            teminat += s.teminat;
            gorev += s.gorev;
        }
        Yaz("  %-10s", BOT_ADLARI[bot]);
        for (int r = 1; r < R; r++) {
            if (gunler[r].empty()) Yaz("   %%  0 /  -  ");
            else Yaz("   %%%3d /%4.0f ", (int)(100 * gunler[r].size() / OYUN), Medyan(gunler[r]));
        }
        Yaz("  %%%3d  %5ld  %5.1f  %8.0f  %8.0f  %8.0f  %6.2f\n", 100 * iflas / OYUN, islem / OYUN, (double)gorev / OYUN,
            Medyan(net[0]), Medyan(net[1]), Medyan(net[2]), (double)teminat / OYUN);
    }

    // Senaryo karsilastirmasi: cekirdek donguyu oynayan bot (Haberci) her senaryoda.
    Yaz("\n  Senaryolar (Haberci botu, %d oyun):\n", OYUN / 2);
    for (int sen = 0; sen < SENARYO_SAYISI; sen++) {
        vector<float> zirve, net60, skor;
        int iflas = 0;
        int sinir = SENARYOLAR[sen].gunSiniri;
        for (int g = 0; g < OYUN / 2; g++) {
            BotSonuc s = BotOyunu(BOT_HABERCI, 5000 + g, sinir > 0 ? max(sinir + 1, 60) : MAX_GUN, sen);
            if (s.rutbeGunu[R - 1] > 0) zirve.push_back((float)s.rutbeGunu[R - 1]);
            net60.push_back(s.netGun[0]);
            if (s.iflas) iflas++;
        }
        if (sinir > 0)
            Yaz("    %-18s (sureli, %d gun)   net@60g medyan %8.0f TL   iflas %%%d\n", SENARYOLAR[sen].ad.en, sinir, Medyan(net60),
                100 * iflas / (OYUN / 2));
        else
            Yaz("    %-18s zirve: %%%3d / medyan %4.0f gun   net@60g medyan %8.0f TL   iflas %%%d\n", SENARYOLAR[sen].ad.en,
                (int)(100 * zirve.size() / (OYUN / 2)), Medyan(zirve), Medyan(net60), 100 * iflas / (OYUN / 2));
    }
}

// ---------------------------------------------------------------------------
// DEVRALMA DENEYI: yonetim kararlari dengeli mi?
// ---------------------------------------------------------------------------
// Oyuncu 2. rutbede (30.000 TL) en ucuz hisseden 501 adet alip sirketi devralir
// ve baska HICBIR sey yapmaz; tek fark yonetim kararlarindaki tutumudur.
// v2.0'da "hep riskli" bu deneyde oyunlarin %100'unde, medyan 29 gunde zirveye
// cikiyordu (riskin bedeli yoktu). Istenen siralama:
//     akilli  >  hep guvenli  >  devralmayan ~ rastgele  >  hep riskli
enum DevralmaTutumu { DT_HEP_RISKLI, DT_HEP_GUVENLI, DT_AKILLI, DT_RASTGELE, DT_DEVRALMAZ, DT_SAYISI };

static int DevralmaDeneyi() {
    static const char* AD[DT_SAYISI] = {"devral + hep riskli", "devral + hep guvenli", "devral + akilli (sans>=%55)", "devral + rastgele",
                                        "devralma (400 adet tut)"};
    const int OYUN = 100, MAX_GUN = 150;
    Yaz("\n=== 5) DEVRALMA DENEYI (30.000 TL ile en ucuz sirket; %d oyun, %d gun) ===\n", OYUN, MAX_GUN);
    Yaz("  %-30s  zirve   medyan gun   net@30g   net@60g   riskli karar   tutan\n", "tutum");
    float net60[DT_SAYISI] = {0};
    for (int tutum = 0; tutum < DT_SAYISI; tutum++) {
        vector<float> gunler, n30, n60;
        long karar = 0, riskliN = 0, tutanN = 0;
        for (int g = 0; g < OYUN; g++) {
            SimTohumla(9000 + (uint64_t)g);
            g_botRng.Tohumla(4000 + (uint64_t)g);
            Oyun o;
            o.Baslat(S_KLASIK, false);
            o.rehberAdim = -1;
            o.rutbeIndex = 2;
            o.hedefIndex = 2;
            o.bakiye = 30000.0f;
            int hedef = 0;
            for (int i = 0; i < HisseSayisi(); i++)
                if (o.hisseler[i].acilmaSeviyesi == 0) hedef = i;
            const int alinacak = (tutum == DT_DEVRALMAZ) ? 400 : 501;
            int alinan = 0, bitis = -1;
            const int gun0 = o.gun;
            float a30 = 0, a60 = 0;
            float sonAlim = -10.0f;
            while (o.gun - gun0 <= MAX_GUN) {
                SimulasyonAdimi(o, 0.25f);
                if (o.yonetimOlayiAcik) {
                    const Hisse& hy = o.hisseler[o.yonetimHisseIndex];
                    bool riskli = false;
                    if (tutum == DT_HEP_RISKLI) riskli = true;
                    else if (tutum == DT_AKILLI) riskli = YonetimBasariOlasiligi(hy) >= 0.55f;
                    else if (tutum == DT_RASTGELE) riskli = g_botRng.Aralik(0, 1) == 1;
                    float once = hy.fiyat;
                    YonetimKarariUygula(o, riskli);
                    karar++;
                    if (riskli) {
                        riskliN++;
                        if (o.hisseler[hedef].fiyat > once) tutanN++;
                    }
                }
                if (o.sonPenceresiAcik) {
                    if (o.sonTuru == SON_IFLAS) break;
                    o.sonPenceresiAcik = false;
                }
                if (alinan < alinacak && o.oyunSuresi - sonAlim >= 2.0f) { // 25'er adet, 2 sn arayla
                    sonAlim = o.oyunSuresi;
                    int n = min(25, alinacak - alinan);
                    if (IslemYap(o, hedef, true, n, IslemKaynagi::OYUNCU).basarili) alinan += n;
                }
                if (bitis < 0 && o.rutbeIndex >= (int)RUTBELER.size() - 1) bitis = o.gun - gun0;
                if (o.gun - gun0 == 30) a30 = o.NetDeger();
                if (o.gun - gun0 == 60) a60 = o.NetDeger();
            }
            if (bitis >= 0) gunler.push_back((float)bitis);
            n30.push_back(a30);
            n60.push_back(a60);
        }
        net60[tutum] = Medyan(n60);
        Yaz("  %-30s  %%%3d   ", AD[tutum], (int)(100 * gunler.size() / OYUN));
        if (gunler.empty()) Yaz("     -     ");
        else Yaz("   %4.0f    ", Medyan(gunler));
        Yaz(" %8.0f  %8.0f", Medyan(n30), net60[tutum]);
        if (karar > 0) Yaz("       %%%3.0f        %%%3.0f", 100.0 * riskliN / karar, riskliN ? 100.0 * tutanN / riskliN : 0.0);
        Yaz("\n");
    }
    int hata = 0;
    auto bekle = [&](bool kosul, const char* ad) {
        Yaz("  %-64s %s\n", ad, kosul ? "ok" : "HATA");
        if (!kosul) hata++;
    };
    bekle(net60[DT_HEP_RISKLI] < net60[DT_HEP_GUVENLI], "hep riskli, hep guvenliden KOTU (riskin bedeli var)");
    bekle(net60[DT_HEP_RISKLI] < net60[DT_DEVRALMAZ], "hep riskli, hic devralmamaktan da kotu");
    bekle(net60[DT_AKILLI] > net60[DT_HEP_GUVENLI], "akilli (sansa bakan) tutum en iyisi");
    bekle(net60[DT_HEP_GUVENLI] > net60[DT_DEVRALMAZ], "devralmak (guvenli yonetimle) devralmamaktan iyi");
    return hata;
}

// Ayni tohum ayni oyunu veriyor mu? Gunun Piyasasi'nda ortam oyuncudan bagimsiz mi?
static int BelirlenimTestleri() {
    Yaz("\n=== 4) BELIRLENIM (determinizm) ===\n");
    int hata = 0;
    BotSonuc a = BotOyunu(BOT_HABERCI, 4242, 80, S_KLASIK);
    BotSonuc b = BotOyunu(BOT_HABERCI, 4242, 80, S_KLASIK);
    bool ayni = (a.netGun[0] == b.netGun[0]) && (a.ortamOzeti == b.ortamOzeti) && (a.islem == b.islem);
    Yaz("  %-64s %s\n", "ayni tohum + ayni bot = birebir ayni oyun", ayni ? "ok" : "HATA");
    if (!ayni) hata++;

    BotSonuc g1 = BotOyunu(BOT_TUTUCU, 77, 31, S_GUNLUK);
    BotSonuc g2 = BotOyunu(BOT_HABERCI, 77, 31, S_GUNLUK);
    BotSonuc g3 = BotOyunu(BOT_RASTGELE, 77, 31, S_GUNLUK);
    bool bagimsiz = (g1.ortamOzeti == g2.ortamOzeti) && (g2.ortamOzeti == g3.ortamOzeti) && g1.ortamOzeti != 0;
    Yaz("  %-64s %s\n", "Gunun Piyasasi: 3 farkli bot AYNI kuresel olaylari/piyasa modunu gorur", bagimsiz ? "ok" : "HATA");
    if (!bagimsiz) hata++;
    BotSonuc g4 = BotOyunu(BOT_TUTUCU, 78, 31, S_GUNLUK);
    bool farkli = g4.ortamOzeti != g1.ortamOzeti;
    Yaz("  %-64s %s\n", "Gunun Piyasasi: farkli gun = farkli piyasa", farkli ? "ok" : "HATA");
    if (!farkli) hata++;
    return hata;
}
